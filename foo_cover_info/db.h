#pragma once
#include <set>

namespace cinfo
{
	struct fields
	{
		size_t front_cover_width = 0;
		size_t front_cover_height = 0;
		size_t front_cover_bytes = 0;
		uint64_t last_modified = 0;
	};

	using hash_set = std::set<metadb_index_hash>;

	bool hashHandle(const metadb_handle_ptr& handle, metadb_index_hash& hash);
	fields get(metadb_index_hash hash);
	metadb_index_manager::ptr theAPI();
	void refresh(const pfc::list_t<metadb_index_hash>& hashes);
	void set(metadb_index_hash hash, fields f);
}
