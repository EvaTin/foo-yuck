#include "stdafx.h"

namespace cinfo
{
	class my_mainmenu_commands : public mainmenu_commands
	{
	public:
		GUID get_command(size_t index) override
		{
			if (index == 0) return guid_mainmenu_scan;
			else if (index == 1) return guid_mainmenu_clear;
			else uBugCheck();
		}

		GUID get_parent() override
		{
			return guid_mainmenu_group;
		}

		bool get_description(size_t index, pfc::string_base& out) override
		{
			get_name(index, out);
			return true;
		}

		bool get_display(size_t index, pfc::string_base& out, size_t& flags) override
		{
			if (!library_manager::get()->is_library_enabled()) flags = flag_disabled;
			get_name(index, out);
			return true;
		}

		size_t get_command_count() override
		{
			return 2;
		}

		void execute(size_t index, service_ptr_t<service_base> callback) override
		{
			if (!library_manager::get()->is_library_enabled())
			{
				popup_message::g_show("You must configure your Media Library first.", component_name);
				return;
			}

			if (index == 0)
			{
				metadb_handle_list items;
				library_manager::get()->get_all_items(items);

				auto flags = threaded_process::flag_show_progress | threaded_process::flag_show_delayed | threaded_process::flag_show_item | threaded_process::flag_show_abort;
				auto cb = fb2k::service_new<cover_info>(items);
				threaded_process::get()->run_modeless(cb, flags, core_api::get_main_window(), "Scanning for covers...");
			}
			else if (index == 1)
			{
				pfc::list_t<metadb_index_hash> hashes;
				theAPI()->get_all_hashes(guid_metadb_index, hashes);
				for (size_t i = 0; i < hashes.get_count(); ++i)
				{
					set(hashes[i], fields());
				}
				refresh(hashes);
			}
		}

		void get_name(size_t index, pfc::string_base& out) override
		{
			if (index == 0) out = "Scan for Cover Info";
			else if (index == 1) out = "Clear all Cover Info";
		}
	};

	static mainmenu_group_popup_factory g_my_mainmenu_group(guid_mainmenu_group, mainmenu_groups::library, mainmenu_commands::sort_priority_base, component_name);
	FB2K_SERVICE_FACTORY(my_mainmenu_commands);
}
