#include "stdafx.h"

namespace cinfo
{
	class my_contextmenu_item_simple : public contextmenu_item_simple
	{
	public:
		my_contextmenu_item_simple() {}

		GUID get_item_guid(uint32_t index) override
		{
			if (index == 0) return guid_context_scan;
			else if (index == 1) return guid_context_clear;
			else uBugCheck();
		}

		GUID get_parent() override
		{
			return guid_context_group;
		}

		bool context_get_display(uint32_t index, metadb_handle_list_cref handles, pfc::string_base& out, uint32_t& displayflags, const GUID& caller) override
		{
			get_item_name(index, out);
			return true;
		}

		bool get_item_description(uint32_t index, pfc::string_base& out) override
		{
			get_item_name(index, out);
			return true;
		}

		size_t get_num_items() override
		{
			return 2;
		}

		void context_command(size_t index, metadb_handle_list_cref handles, const GUID& caller) override
		{
			if (index == 0)
			{
				auto flags = threaded_process::flag_show_progress | threaded_process::flag_show_delayed | threaded_process::flag_show_item | threaded_process::flag_show_abort;
				auto cb = fb2k::service_new<cover_info>(handles);
				threaded_process::get()->run_modeless(cb, flags, core_api::get_main_window(), "Scanning for covers...");
			}
			else if (index == 1)
			{
				reset(handles);
			}
		}

		void get_item_name(size_t index, pfc::string_base& out) override
		{
			if (index == 0) out = "Scan for Cover Info";
			else if (index == 1) out = "Clear all Cover Info";
		}
	};

	static contextmenu_group_popup_factory g_my_context_group(guid_context_group, contextmenu_groups::root, component_name, 0);
	FB2K_SERVICE_FACTORY(my_contextmenu_item_simple);
}
