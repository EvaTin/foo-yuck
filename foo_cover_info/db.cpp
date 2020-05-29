#include "stdafx.h"

namespace cinfo
{
	static constexpr std::array<const char*, 3> field_names =
	{
		"front_cover_width",
		"front_cover_height",
		"front_cover_size",
	};

	metadb_index_manager::ptr g_cachedAPI;

	class my_metadb_index_client : public metadb_index_client
	{
	public:
		metadb_index_hash transform(const file_info& info, const playable_location& location) override
		{
			const char* str = file_path_display(location.get_path());
			return hasher_md5::get()->process_single_string(str).xorHalve();
		}
	};
	static auto g_client = new service_impl_single_t<my_metadb_index_client>;

	class my_init_stage_callback : public init_stage_callback
	{
	public:
		void on_init_stage(size_t stage) override
		{
			if (stage == init_stages::before_config_read)
			{
				auto api = metadb_index_manager::get();
				g_cachedAPI = api;
				try
				{
					api->add(g_client, guid_metadb_index, system_time_periods::week * 4);
				}
				catch (const std::exception& e)
				{
					api->remove(guid_metadb_index);
					FB2K_console_formatter() << component_name << " stats: Critical initialisation failure: " << e;
					return;
				}
				api->dispatch_global_refresh();
			}
		}
	};

	class my_initquit : public initquit
	{
	public:
		void on_quit() override
		{
			g_cachedAPI.release();
		}
	};

	class my_metadb_display_field_provider : public metadb_display_field_provider
	{
	public:
		bool process_field(size_t index, metadb_handle* handle, titleformat_text_out* out) override
		{
			metadb_index_hash hash;
			if (!hashHandle(handle, hash)) return false;

			const fields tmp = get(hash);

			switch (index)
			{
			case 0:
				if (tmp.front_cover_width == 0) return false;
				out->write_int(titleformat_inputtypes::meta, tmp.front_cover_width);
				return true;
			case 1:
				if (tmp.front_cover_height == 0) return false;
				out->write_int(titleformat_inputtypes::meta, tmp.front_cover_height);
				return true;
			case 2:
				if (tmp.front_cover_bytes == 0) return false;
				out->write(titleformat_inputtypes::meta, pfc::format_file_size_short(tmp.front_cover_bytes));
				return true;
			}
			return false;
		}

		size_t get_field_count() override
		{
			return field_names.size();
		}

		void get_field_name(size_t index, pfc::string_base& out) override
		{
			out.set_string(field_names[index]);
		}
	};

	FB2K_SERVICE_FACTORY(my_init_stage_callback);
	FB2K_SERVICE_FACTORY(my_initquit);
	FB2K_SERVICE_FACTORY(my_metadb_display_field_provider);

	bool hashHandle(const metadb_handle_ptr& handle, metadb_index_hash& hash)
	{
		return g_client->hashHandle(handle, hash);
	}

	fields get(metadb_index_hash hash)
	{
		mem_block_container_impl temp;
		theAPI()->get_user_data(guid_metadb_index, hash, temp);
		if (temp.get_size() > 0)
		{
			try
			{
				stream_reader_formatter_simple_ref<false> reader(temp.get_ptr(), temp.get_size());
				fields f;
				reader >> f.front_cover_width;
				reader >> f.front_cover_height;
				reader >> f.front_cover_bytes;
				reader >> f.last_modified;
				return f;
			}
			catch (exception_io_data) {}
		}
		return fields();
	}

	metadb_index_manager::ptr theAPI()
	{
		auto ret = g_cachedAPI;
		if (ret.is_empty()) ret = metadb_index_manager::get();
		return ret;
	}

	void get_hashes(metadb_handle_list_cref handles, hash_set& hashes)
	{
		const size_t count = handles.get_count();
		for (size_t i = 0; i < count; ++i)
		{
			metadb_index_hash hash;
			if (hashHandle(handles[i], hash))
			{
				hashes.emplace(hash);
			}
		}
	}

	void refresh(const pfc::list_t<metadb_index_hash>& hashes)
	{
		fb2k::inMainThread([hashes]
			{
				theAPI()->dispatch_refresh(guid_metadb_index, hashes);
			});
	}

	void reset(metadb_handle_list_cref handles)
	{
		hash_set hashes;
		get_hashes(handles, hashes);
		pfc::list_t<metadb_index_hash> to_refresh;

		for (const auto hash : hashes)
		{
			set(hash, fields());
			to_refresh += hash;
		}
		refresh(to_refresh);
	}

	void set(metadb_index_hash hash, fields f)
	{
		stream_writer_formatter_simple<false> writer;
		writer << f.front_cover_width;
		writer << f.front_cover_height;
		writer << f.front_cover_bytes;
		writer << f.last_modified;
		theAPI()->set_user_data(guid_metadb_index, hash, writer.m_buffer.get_ptr(), writer.m_buffer.get_size());
	}
}
