#include "internal/ZipWriter.hpp"
#include "miniz.h"
#include <cstring>
#include <filesystem>

namespace xword {
namespace internal {

std::unordered_map<std::string, std::string> readZip(const std::string& filepath)
{
    std::unordered_map<std::string, std::string> result;

    mz_zip_archive archive;
    std::memset(&archive, 0, sizeof(archive));

#ifdef _WIN32
    std::filesystem::path p = std::filesystem::u8path(filepath);
    if (!mz_zip_reader_init_file(&archive, p.string().c_str(), 0))
        return result;
#else
    if (!mz_zip_reader_init_file(&archive, filepath.c_str(), 0))
        return result;
#endif

    mz_uint n = mz_zip_reader_get_num_files(&archive);
    for (mz_uint i = 0; i < n; ++i) {
        mz_zip_archive_file_stat stat;
        if (!mz_zip_reader_file_stat(&archive, i, &stat))
            continue;
        if (stat.m_is_directory)
            continue;

        size_t size = 0;
        void* data = mz_zip_reader_extract_to_heap(&archive, i, &size, 0);
        if (!data)
            continue;

        result[stat.m_filename] = std::string(static_cast<const char*>(data), size);
        mz_free(data);
    }

    mz_zip_reader_end(&archive);
    return result;
}

} // namespace internal
} // namespace xword
