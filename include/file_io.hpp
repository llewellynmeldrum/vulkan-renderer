#pragma once 
#include <fstream>

#include "logger.hpp"
#include "types.hpp"
#include "unix_helpers.hpp"

static constexpr inline bool DBG_FILE_READS = true;

static constexpr inline std::vector<char> read_file_contents(std::string_view filename){
    auto const filename_str = std::string(filename);
    std::ifstream file_stream(filename_str, std::ios::binary);
    ASSERT(file_stream.is_open());
    i64 sz = unix::get_file_size(filename_str);
    auto file_contents = std::vector<char>(sz, '\0');
    file_stream.read(file_contents.data(), sz);
    ASSERT(!file_contents.empty());
    file_stream.close();
    if (DBG_FILE_READS){
        std::string s{};
        s.reserve(sz);
        for (auto ch : file_contents){
            s.push_back(ch);
        }
        LOG_DBG("Printing file contents for '{}', size={}B:",filename,sz);
        LOG_DBG("{}",s);
    }
    return file_contents;
}
