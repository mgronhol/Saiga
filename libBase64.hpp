#ifndef _LIBBASE64_HPP_
#define _LIBBASE64_HPP_

#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <stdint.h>


const std::string BASE64_TABLE = "ABCDEFGHIJLKMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode( std::string str );
std::string base64_decode( std::string str );



#endif
