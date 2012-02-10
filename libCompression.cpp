#include "libCompression.hpp"

// Applying deflate to string
std::string zlib_deflate( std::string content ){
	int ret, flush;
	z_stream stream;
	const unsigned int CHUNK = 16384;
	uint8_t in[CHUNK], out[CHUNK];
	size_t pos = 0;
	std::string str_chunk, result;
	
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	
	// Use default compression
	ret = deflateInit( &stream, Z_DEFAULT_COMPRESSION );
	
	// If failed return empty string
	if( ret != 0 ){ return std::string(); }
	
	do{
		str_chunk.clear();
		
		// take a chunk from input content
		
		if( content.size() - pos > CHUNK ){
			str_chunk = content.substr( pos, pos + CHUNK );
			pos += CHUNK;
			flush = Z_NO_FLUSH;
			}
		// if there is less than CHUNK bytes left, just take the rest
		else {
			if( pos < content.size() ){
				str_chunk = content.substr( pos );
				pos = content.size();
				
				}
			flush = Z_FINISH;
			}
		
		memcpy( in, str_chunk.c_str(), str_chunk.size() );
		
		stream.avail_in = str_chunk.size();
		stream.next_in = in;
		
		// pack chunk using deflate
		do {
			stream.avail_out = CHUNK;
			stream.next_out = out;
			ret = deflate( &stream, flush ); 
			result += std::string( (char *)out, CHUNK - stream.avail_out );
			} while( stream.avail_out == 0 );
		
		} while( str_chunk.size() > 0 );
	
	deflateEnd( &stream );
	
	return result;
	}

// Inflate a deflate-compressed string

std::string zlib_inflate( std::string content ){
	int ret;
	z_stream stream;
	const unsigned int CHUNK = 16384;
	uint8_t in[CHUNK], out[CHUNK];
	size_t pos = 0;
	std::string str_chunk, result;
	
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	
	ret = inflateInit( &stream );
	
	if( ret != 0 ){ return std::string(); }
	
	do{
		str_chunk.clear();
		
		// take a chunk from input content
		
		if( content.size() - pos > CHUNK ){
			str_chunk = content.substr( pos, pos + CHUNK );
			pos += CHUNK;
			}
		// if there is less than CHUNK bytes left, just take the rest
		else {
			if( pos < content.size() ){
				str_chunk = content.substr( pos );
				pos = content.size();
				
				}
			}
		
		memcpy( in, str_chunk.c_str(), str_chunk.size() );
		
		stream.avail_in = str_chunk.size();
		stream.next_in = in;
		// Go through the chunk and inflate it into out buffer
		do {
			stream.avail_out = CHUNK;
			stream.next_out = out;
			ret = inflate( &stream, Z_NO_FLUSH ); 
			result += std::string( (char *)out, CHUNK - stream.avail_out );
			} while( stream.avail_out == 0 );
		
		} while( str_chunk.size() > 0 );
	
	inflateEnd( &stream );
	
	return result;
	}


// Compress string using gzip
// Works the same as zlib_deflate

std::string zlib_gzip_deflate( std::string content ){
	int ret, flush;
	z_stream stream;
	const unsigned int CHUNK = 16384;
	uint8_t in[CHUNK], out[CHUNK];
	size_t pos = 0;
	std::string str_chunk, result;
	
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	
	// Use deflate algorithm and automatic gzip format creation
	ret = deflateInit2( &stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY );
	
	if( ret != 0 ){ return std::string(); }
	
	do{
		str_chunk.clear();
		
		if( content.size() - pos > CHUNK ){
			str_chunk = content.substr( pos, pos + CHUNK );
			pos += CHUNK;
			flush = Z_NO_FLUSH;
			}
		else {
			if( pos < content.size() ){
				str_chunk = content.substr( pos );
				pos = content.size();
				
				}
			flush = Z_FINISH;
			}
		
		memcpy( in, str_chunk.c_str(), str_chunk.size() );
		
		stream.avail_in = str_chunk.size();
		stream.next_in = in;
		
		do {
			stream.avail_out = CHUNK;
			stream.next_out = out;
			ret = deflate( &stream, flush ); 
			result += std::string( (char *)out, CHUNK - stream.avail_out );
			} while( stream.avail_out == 0 );
		
		} while( str_chunk.size() > 0 );
	
	deflateEnd( &stream );
	
	return result;
	}

// Inflate a gzip compressed string
// Works the same as zlib_inflate

std::string zlib_gzip_inflate( std::string content ){
	int ret;
	z_stream stream;
	const unsigned int CHUNK = 16384;
	uint8_t in[CHUNK], out[CHUNK];
	size_t pos = 0;
	std::string str_chunk, result;
	
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.next_in = Z_NULL;
	stream.avail_in = Z_NULL;
	
	// Turn on automatic format detection
	ret = inflateInit2( &stream, 32+15 );
	
	if( ret != 0 ){
		std::cerr << "inflateInit2 failed!" << std::endl;
		return std::string(); 
		}
	
	do{
		str_chunk.clear();
		if( content.size() - pos > CHUNK ){
			str_chunk = content.substr( pos, CHUNK - 1 );
			pos += CHUNK - 1;
			
			}
		else {
			if( pos < content.size() ){
				str_chunk = content.substr( pos );
				pos = content.size();
				
				}
			}
		
		memcpy( in, str_chunk.c_str(), str_chunk.size() );
		
		stream.avail_in = str_chunk.size();
		stream.next_in = in;
		do {
			stream.avail_out = CHUNK;
			stream.next_out = out;
			ret = inflate( &stream, Z_NO_FLUSH ); 
			
			result += std::string( (char *)out, CHUNK - stream.avail_out );
			
			} while( stream.avail_out == 0 );
		
		} while( str_chunk.size() > 0 );
	
	inflateEnd( &stream );
	
	return result;
	}
