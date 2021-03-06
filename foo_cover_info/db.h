#pragma once

namespace cinfo
{
	struct fields
	{
		size_t front_cover_width = 0;
		size_t front_cover_height = 0;
		size_t front_cover_bytes = 0;
		uint64_t last_modified = 0;
		pfc::string8_fast front_cover_format;
	};

	using hash_list = pfc::list_t<metadb_index_hash>;
	using hash_set = std::set<metadb_index_hash>;

	bool hashHandle(const metadb_handle_ptr& handle, metadb_index_hash& hash);
	fields get(metadb_index_hash hash);
	metadb_index_manager::ptr theAPI();
	void get_hashes(metadb_handle_list_cref handles, hash_set& hashes);
	void refresh(const hash_list& hashes);
	void reset(metadb_handle_list_cref handles);
	void set(metadb_index_hash hash, fields f);
}
