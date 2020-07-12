#include "stdafx.h"

#include <atlbase.h>
#include <gdiplus.h>
#include <shlwapi.h>

namespace cinfo
{
	DECLARE_COMPONENT_VERSION(component_name, component_version, component_info);
	VALIDATE_COMPONENT_FILENAME(component_dll_name);

	cover_info::cover_info(metadb_handle_list_cref handles) : m_handles(handles) {}

	void cover_info::run(threaded_process_status& status, abort_callback& abort)
	{
		const size_t count = m_handles.get_count();
		size_t counter = 0, found = 0, skipped = 0;
		hash_list to_refresh;
		hash_set hashes;

		for (size_t i = 0; i < count; ++i)
		{
			abort.check();

			const auto handle = m_handles[i];
			const auto path = handle->get_path();

			status.set_progress(i + 1, count);
			status.set_item_path(path);

			metadb_index_hash hash;
			if (!hashHandle(handle, hash)) continue;
			if (hashes.count(hash)) continue;
			hashes.emplace(hash);

			album_art_extractor::ptr ptr;
			if (album_art_extractor::g_get_interface(ptr, path))
			{
				counter++;

				uint64_t last_modified = handle->get_filetimestamp();

				if (last_modified == get(hash).last_modified)
				{
					skipped++;
					continue;
				}

				fields f;
				f.last_modified = last_modified;

				try
				{
					album_art_data_ptr data = ptr->open(nullptr, path, abort)->query(album_art_ids::cover_front, abort);

					if (data.is_valid())
					{
						pfc::com_ptr_t<IStream> stream;
						ULONG bytes_written = 0;
						if (SUCCEEDED(CreateStreamOnHGlobal(nullptr, TRUE, stream.receive_ptr())) && SUCCEEDED(stream->Write(data->get_ptr(), data->get_size(), &bytes_written)))
						{
							pfc::ptrholder_t<Gdiplus::Bitmap> bitmap = new Gdiplus::Bitmap(stream.get_ptr());
							if (bitmap.is_valid() && bitmap->GetLastStatus() == Gdiplus::Ok)
							{
								found++;
								f.front_cover_width = bitmap->GetWidth();
								f.front_cover_height = bitmap->GetHeight();
								f.front_cover_bytes = data->get_size();
							}
						}
					}
				}
				catch (...) {}

				set(hash, f);
				to_refresh += hash;
			}
		}

		refresh(to_refresh);

		FB2K_console_formatter() << "Cover Info: " << count << " items scanned.";
		FB2K_console_formatter() << "Cover Info: " << counter << " items are capable of containing covers.";
		FB2K_console_formatter() << "Cover Info: " << skipped << " items were skipped (file modification time was same as last run).";
		FB2K_console_formatter() << "Cover Info: " << found << " new/modified items have covers.";
	}
}
