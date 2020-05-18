#include "stdafx.h"
#include "foo_run_main.h"

namespace
{
	DECLARE_COMPONENT_VERSION(component_name, component_version, component_info);
	VALIDATE_COMPONENT_FILENAME(component_dll_name);

	class my_commandline_handler : public commandline_handler
	{
	public:
		result on_token(const char* token) override
		{
			pfc::string s(token);

			if (s.startsWith("/run_main:"))
			{
				pfc::string8_fast command = s.subString(10).get_ptr();
				if (!execute_mainmenu_command(command))
					FB2K_console_formatter() << component_name << ": Command not found: " << command;

				return RESULT_PROCESSED;
			}

			return RESULT_NOT_OURS;
		}

	private:
		bool execute_mainmenu_command(pfc::stringp command)
		{
			// Ensure commands on the Edit menu are enabled
			ui_edit_context_manager::get()->set_context_active_playlist();

			auto hash = [](const GUID& g)
			{
				return hasher_md5::get()->process_single_string(pfc::print_guid(g).get_ptr()).xorHalve();
			};

			std::unordered_map<uint64_t, mainmenu_group::ptr> group_guid_map;

			for (auto e = service_enum_t<mainmenu_group>(); !e.finished(); ++e)
			{
				auto ptr = *e;
				group_guid_map.emplace(hash(ptr->get_guid()), ptr);
			}

			for (auto e = service_enum_t<mainmenu_commands>(); !e.finished(); ++e)
			{
				auto ptr = *e;
				for (size_t i = 0; i < ptr->get_command_count(); ++i)
				{
					pfc::string8_fast path;

					GUID parent = ptr->get_parent();
					while (parent != pfc::guid_null)
					{
						mainmenu_group::ptr group_ptr = group_guid_map.at(hash(parent));
						mainmenu_group_popup::ptr group_popup_ptr;

						if (group_ptr->cast(group_popup_ptr))
						{
							pfc::string8_fast str;
							group_popup_ptr->get_display_string(str);
							if (str.get_length())
							{
								str.add_char('/');
								str.add_string(path);
								path = str;
							}
						}
						parent = group_ptr->get_parent();
					}

					mainmenu_commands_v2::ptr v2_ptr;
					if (ptr->cast(v2_ptr) && v2_ptr->is_command_dynamic(i))
					{
						mainmenu_node::ptr node = v2_ptr->dynamic_instantiate(i);
						if (execute_mainmenu_command_recur(node, command, path))
						{
							return true;
						}
					}
					else
					{
						pfc::string8_fast name;
						ptr->get_name(i, name);
						path.add_string(name);
						if (_stricmp(command, path) == 0)
						{
							ptr->execute(i, nullptr);
							return true;
						}
					}
				}
			}
			return false;
		}

		bool execute_mainmenu_command_recur(mainmenu_node::ptr node, pfc::stringp command, pfc::stringp p)
		{
			pfc::string8_fast path, text;
			size_t flags;
			node->get_display(text, flags);
			path << p << text;

			switch (node->get_type())
			{
			case mainmenu_node::type_group:
				if (text.get_length()) path.add_char('/');
				for (size_t i = 0; i < node->get_children_count(); ++i)
				{
					mainmenu_node::ptr child = node->get_child(i);
					if (execute_mainmenu_command_recur(child, command, path))
					{
						return true;
					}
				}
				break;
			case mainmenu_node::type_command:
				if (_stricmp(command, path) == 0)
				{
					node->execute(nullptr);
					return true;
				}
				break;
			}
			return false;
		}
	};

	FB2K_SERVICE_FACTORY(my_commandline_handler)
};
