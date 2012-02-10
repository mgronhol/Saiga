#ifndef _LIBCOMPRESSION_HPP_
#define _LIBCOMPRESSION_HPP_ 

#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <stddef.h>
#include <unistd.h>
#include <stdint.h>

#include <zlib.h>

std::string zlib_deflate( std::string content );
std::string zlib_inflate( std::string content );

std::string zlib_gzip_deflate( std::string content );
std::string zlib_gzip_inflate( std::string content );


#endif
