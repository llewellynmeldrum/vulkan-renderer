#pragma once 
#include <fstream>

#include "types.hpp"
#include "unix_helpers.hpp"
static constexpr inline std::vector<char> read_file_contents(std::string const& filename){
    std::ifstream file_stream(filename, std::ios::binary);
    ASSERT(file_stream.is_open());
    i64 sz = unix::get_file_size(filename);
    auto file_contents = std::vector<char>(sz, '\0');
    file_stream.read(file_contents.data(), sz);
    ASSERT(!file_contents.empty());
    file_stream.close();
    return file_contents;
}
