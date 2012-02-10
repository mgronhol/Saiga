#include "libBase64.hpp"

std::string base64_encode( std::string str ){
	std::string out;
	size_t pos = 0;
	
	while( pos < str.size() ) {
		uint8_t parts[4];
		
		if( pos + 3 > str.size() ){
			if( pos + 1 == str.size() ){
				parts[0] = ((uint8_t)str[pos] >> 2) & 0xFF;
				parts[1] = (((uint8_t)str[pos] & 0x3) << 4 );;
				out += BASE64_TABLE[ parts[0] ];
				out += BASE64_TABLE[ parts[1] ];
				out += "==";
				}
			if( pos + 2 == str.size() ){
				parts[0] = ((uint8_t)str[pos] >> 2) & 0xFF;
				parts[1] = (((uint8_t)str[pos] & 0x3) << 4 ) | (((uint8_t)str[pos+1] >> 4) & 0x0F);
				parts[2] = (((uint8_t)str[pos+1] & 0x0F) << 2 );	
				out += BASE64_TABLE[ parts[0] ];
				out += BASE64_TABLE[ parts[1] ];
				out += BASE64_TABLE[ parts[2] ];
				out += "=";
				}
			}
		else{
			parts[0] = ((uint8_t)str[pos] >> 2) & 0xFF;
			parts[1] = (((uint8_t)str[pos] & 0x3) << 4 ) | (((uint8_t)str[pos+1] >> 4) & 0x0F);
			
			parts[2] = (((uint8_t)str[pos+1] & 0x0F) << 2 ) | ( ((uint8_t)str[pos+2] >> 6) & 0x03 );
			
			parts[3] = ((uint8_t)str[pos+2]) & 0x3f;
			
			out += BASE64_TABLE[ parts[0] ];
			out += BASE64_TABLE[ parts[1] ];
			out += BASE64_TABLE[ parts[2] ];
			out += BASE64_TABLE[ parts[3] ];
			}			
		
		pos += 3;
		}
	
	
	return out;
	}

uint8_t base64_reverse_lookup( char sym ){
	for( uint8_t i = 0 ; i < 64; ++i ){
		if( sym == BASE64_TABLE[i] ){
			return i;
			}
		}
	return 0;
	}

std::string base64_decode( std::string str ){
	std::string out;
	size_t pos = 0;
	
	
	while( pos < str.size() ){
		//std::cout << "pos: " << pos << " str.size() = " << str.size() << std::endl;
		uint8_t parts[3];
		uint8_t syms[4];
		
		for( size_t i = 0 ; i < 4 ; ++i ){
			if( str[pos+1] == '=' ){
				syms[i] = 0;
				}
			else{
				syms[i] = base64_reverse_lookup( str[pos+i] );
				}
			}
		
		
		parts[0] = (syms[0] << 2) | (syms[1] >> 4 );
		parts[1] = ( (syms[1] & 0x0F) << 4 ) | (syms[2] >> 2 );
		parts[2] = ((syms[2] & 0x3 ) << 6) | syms[3];
		
		out += (char)parts[0];
		out += (char)parts[1];
		out += (char)parts[2];
		
		
		pos += 4;
		}
	
	return out;
	}


