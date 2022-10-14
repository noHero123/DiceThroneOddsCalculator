#pragma once
/// FILE ZIPPER https://stackoverflow.com/questions/27529570/simple-zlib-c-string-compression-and-decompression
#include "base64.h"
#include <sstream>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/zlib.hpp>

class Zipper
{
public:
    static std::string string_compress_encode(const std::string& data)
    {
        std::stringstream compressed;
        std::stringstream original;
        original << data;
        boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
        out.push(boost::iostreams::zlib_compressor());
        out.push(original);
        boost::iostreams::copy(out, compressed);

        std::string comp = compressed.str();
        /**need to encode here **/
        std::string compressed_encoded = base64_encode(comp);

        return compressed_encoded;
    }

    static std::string string_decompress_decode(const std::string& data)
    {
        std::stringstream compressed_encoded;
        std::stringstream decompressed;


        /** first decode  then decompress **/
        std::string compressed = base64_decode(data);
        compressed_encoded << compressed;

        boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
        in.push(boost::iostreams::zlib_decompressor());
        in.push(compressed_encoded);
        boost::iostreams::copy(in, decompressed);
        return decompressed.str();
    }

};
