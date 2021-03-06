#pragma once

namespace cinfo
{
	static constexpr const char* component_name = "Cover Info";
	static constexpr const char* component_dll_name = "foo_cover_info.dll";
	static constexpr const char* component_version = "0.0.5";
	static constexpr const char* component_info = "Copyright (C) 2020 marc2003\n\nBuild: " __TIME__ ", " __DATE__;

	static constexpr GUID guid_metadb_index = { 0x95ea6d78, 0x81, 0x4080, { 0xa5, 0x99, 0x64, 0xa1, 0x48, 0xba, 0xbc, 0xe5 } };

	static constexpr GUID guid_mainmenu_group = { 0xd7ef1380, 0xc88c, 0x4f80, { 0xb1, 0x89, 0x5d, 0x10, 0xac, 0x6f, 0x71, 0x8b } };
	static constexpr GUID guid_mainmenu_scan = { 0xfd204631, 0xf364, 0x4cfb, { 0x80, 0x6c, 0xcb, 0x6a, 0x68, 0xf6, 0xc3, 0x14 } };
	static constexpr GUID guid_mainmenu_clear = { 0x90162796, 0xe35e, 0x497a, { 0x8d, 0x6d, 0x5d, 0xe2, 0x78, 0x45, 0xf5, 0xee } };

	static constexpr GUID guid_context_group = { 0x4f2ff04a, 0xc78d, 0x45d0, { 0x9d, 0x41, 0xa4, 0x75, 0x3f, 0xd1, 0xa0, 0x6a } };
	static constexpr GUID guid_context_scan = { 0x1fac520f, 0x63e7, 0x46e4, { 0xb8, 0x22, 0x50, 0x7c, 0x92, 0x7b, 0x53, 0x37 } };
	static constexpr GUID guid_context_clear = { 0x3069bf41, 0xec99, 0x406b, { 0x95, 0x81, 0xae, 0x2b, 0xb3, 0x25, 0x65, 0x3c } };

	static constexpr std::array<const char*, 2> command_names =
	{
		"Scan for Cover Info",
		"Clear Cover Info"
	};

	class cover_info : public threaded_process_callback
	{
	public:
		cover_info(metadb_handle_list_cref handles);

		void run(threaded_process_status& status, abort_callback& abort) override;

	private:
		metadb_handle_list m_handles;
	};
}
