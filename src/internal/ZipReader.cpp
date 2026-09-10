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
    FILE* file = _wfopen(std::filesystem::u8path(filepath).c_str(), L"rb");
#else
    FILE* file = fopen(filepath.c_str(), "rb");
#endif
    if (!file) return result;
    if (!mz_zip_reader_init_cfile(&archive, file, 0, 0)) { fclose(file); return result; }

    mz_uint n = mz_zip_reader_get_num_files(&archive);
    bool valid = true;
    for (mz_uint i = 0; i < n; ++i) {
        mz_zip_archive_file_stat stat;
        if (!mz_zip_reader_file_stat(&archive, i, &stat)) { valid = false; break; }
        if (stat.m_is_directory)
            continue;

        size_t size = 0;
        void* data = mz_zip_reader_extract_to_heap(&archive, i, &size, 0);
        if (!data) { if (stat.m_uncomp_size == 0) { result[stat.m_filename] = ""; continue; } valid = false; break; }

        result[stat.m_filename] = std::string(static_cast<const char*>(data), size);
        mz_free(data);
    }

    mz_zip_reader_end(&archive);
    fclose(file);
    return valid ? result : std::unordered_map<std::string, std::string>();
}

} // namespace internal
} // namespace xword
