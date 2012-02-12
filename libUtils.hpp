#ifndef _LIBUTILS_HPP_
#define _LIBUTILS_HPP_

#include <sys/socket.h>
#include <strings.h>
#include <netdb.h>
#include <cstdio>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <sys/types.h>
#include <stddef.h>


#include <time.h>
#include <sys/time.h>


const std::string WHITESPACE( " \t\r\n" );
std::string integer_to_string( uint64_t N );
uint64_t string_to_integer( std::string str );
uint64_t hex_string_to_integer( std::string str );
std::string trim( std::string str );
std::vector< std::string > split_string( std::string str, std::string delim );

std::string get_gmt_time( size_t T );
std::string string_to_lower( std::string str );
bool string_startswith( std::string pattern, std::string str );

bool read_file( std::string file, std::string& out );
bool write_file( std::string file, std::string& out );

uint64_t fnv1a_hash( std::string& string );
double get_time();

#endif
