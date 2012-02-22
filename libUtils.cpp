#include "libUtils.hpp"

// Simple function to turn integers to strings 
std::string integer_to_string( uint64_t N ){
	char buffer[80];
	sprintf( buffer, "%lu", N );
	return std::string( buffer );
	}

// Simple functions to parse a string to a integer
uint64_t string_to_integer( std::string str ){
	uint64_t out;
	sscanf( str.c_str(), "%lu", &out );
	return out;
	}

uint64_t hex_string_to_integer( std::string str ){
	uint64_t out;
	sscanf( str.c_str(), "%lX", &out );
	return out;
	}

// Remove whitepsaces from the beginning and the end of the string
std::string trim( std::string str ){
	size_t start, stop;
	
	start = str.find_first_not_of( WHITESPACE );
	stop = str.find_last_not_of( WHITESPACE );
	
	return str.substr( start, stop - start + 1 );
	}

// Split string into an array
std::vector< std::string > split_string( std::string str, std::string delim ){
	std::vector< std::string > out; 
	size_t start = 0;
	size_t pos;
	pos = str.find( delim, start );
	
	// While there is delimiters in the string, continue
	while( pos < str.size() && pos != std::string::npos ){
		out.push_back( str.substr( start, pos-start ) );
		start = pos+delim.size();
		pos = str.find( delim, start );
		}
		
	if( start < str.size() ){
		out.push_back( str.substr( start, str.size() - start ) );
		}
	return out;
	}

// Transform unix timestamp T (T == 0 -> current time ) into a string
// using the format defined in the RFC 2616
std::string get_gmt_time( size_t T ){
	char buffer[128];
	struct tm *time_ptr;
	size_t n;
	
	if( T == 0 ){
		time_t rawtime;
		time( &rawtime );
		
		time_ptr = gmtime( &rawtime );
		}
	else{
		time_t rawtime;
		rawtime = T;
		time_ptr = gmtime( &rawtime );
		}
	
	
	n = strftime( buffer, 128, "%a, %d %b %Y %H:%M:%S GMT", time_ptr ); 
	return std::string( buffer, n );
	}

// String to lower case
std::string string_to_lower( std::string str){
	std::string out = str;
	std::transform( out.begin(), out.end(), out.begin(), ::tolower );
	return out;
	}

bool string_startswith( std::string pattern, std::string str ){
	std::string test;
	if( str.size() < pattern.size() ){ return false; }
	test = str.substr( 0, pattern.size() );
	return test == pattern;
	}


// Reads file into buffer out, returns true if success
bool read_file( std::string file, std::string& out ){
	FILE *handle;
	char *buffer;
	size_t size, n;
	
	handle = fopen( file.c_str(), "rb" );
	if( handle == NULL ){ return false; }
	
	fseek( handle, 0, SEEK_END );
	size = ftell( handle );
	
	rewind( handle );
	
	buffer = new char[ size ];
	
	n = fread( buffer, 1, size, handle );
	
	fclose( handle );
	
	// If something weird happened
	if( n != size ){
		delete [] buffer;
		return false;
		}
	out = std::string( buffer, size );
	
	delete [] buffer;
		
	return true;
	}


// Writes buffer "out" to file, returns true if success 
bool write_file( std::string file, std::string& out ){
	FILE *handle;
	size_t n;
	handle = fopen( file.c_str(), "wb" );
	if( handle == NULL ){ return false; }
	
	n = fwrite( out.c_str(), 1, out.size(), handle );
	fclose( handle );
	if( n != out.size() ){
		return false;
		}
	
	return true;
	}
	
uint64_t fnv1a_hash( std::string& string ){
	uint64_t out = 0xcbf29ce484222325;
	size_t i;
	for( i = 0 ; i < string.size() ; ++i ){
		out ^= string[i];
		out *= 0x100000001b3;
		}
	return out;
	}

double get_time(){
	struct timeval tv;
	gettimeofday( &tv, NULL );
	return tv.tv_sec + tv.tv_usec * 1e-6;
	}


std::string get_mime_type( std::string& path ){
	std::string type;
	size_t pos = path.find_last_of( "." );
	if( pos == std::string::npos ){
		return std::string( "application/octet-stream" );
		}
	type = path.substr( pos + 1 );
	type = string_to_lower( type );
	
	if( type == "txt" ){
		return std::string( "text/plain" );
		}
	
	if( type == "html" ){
		return std::string( "text/html" );
		}
	
	if( type == "png" ){
		return std::string( "image/png" );
		}
	
	if( type == "jpg" || type == "jpeg" ){
		return std::string( "image/jpeg" );
		}
	
	if( type == "pdf" ){
		return std::string( "application/pdf" );
		}
	
	if( type == "gz" ){
		return std::string( "application/x-gzip" );
		}
	
	if( type == "svg" ){
		return std::string( "image/svg+xml" );
		}
	
	if( type == "css" ){
		return std::string( "text/css" );
		}
	
	if( type == "js" ){
		return std::string( "application/javascript" );
		}
	
	if( type == "xml" ){
		return std::string( "text/xml" );
		}
	
	if( type == "tar" ){
		return std::string( "application/x-tar" );
		}
		
	return std::string( "text/plain");
	}
