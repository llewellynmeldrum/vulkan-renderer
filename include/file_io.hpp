#pragma once 
#include <fstream>

#include "logger.hpp"
#include "types.hpp"
#include "unix_helpers.hpp"

static constexpr inline bool DBG_FILE_READS = false;

template<typename tOutputType = std::vector<char>>
static constexpr inline auto read_file_contents(std::string_view filename)
-> tOutputType{
    using OutputType = tOutputType;

    auto const filename_str = std::string(filename);
    std::ifstream file_stream(filename_str, std::ios::binary);
    ASSERT(file_stream.is_open());
    i64 sz = unix::get_file_size(filename_str);
    auto file_buf = std::vector<char>(sz, '\0');
    file_stream.read(file_buf.data(), sz);
    ASSERT(!file_buf.empty());
    file_stream.close();
    if (DBG_FILE_READS){
        std::string s{};
        s.reserve(sz);
        for (auto ch : file_buf){
            s.push_back(ch);
        }
        LOG_DBG("Printing file contents for '{}', size={}B:",filename,sz);
        LOG_DBG("{}",s);
    }
    if constexpr(std::same_as<OutputType,std::vector<char>>){
        return file_buf;
    } else if constexpr(std::same_as<OutputType, std::string>){
        return std::string(std::from_range, file_buf);
    }else{
        static_assert(false, "Unsupported tOutputType.");
    }
}
