#include "libHttpMessage.hpp"

HttpMessage :: HttpMessage(){
	
	// default to HTTP 1.1 request 
	code = 0;
	version = "HTTP/1.1";
	
	}

// Setters and getters

void HttpMessage :: set_method( std::string aMethod ){ method = aMethod; }
void HttpMessage :: set_path( std::string aPath ){ path = aPath; }
void HttpMessage :: set_code( size_t aCode ){ code = aCode; }
void HttpMessage :: set_code_string( std::string aString ){ code_string = aString; }
void HttpMessage :: set_version( std::string aVersion ){ version = aVersion; }
void HttpMessage :: set_header_field( std::string key, std::string value ){ header[ key ] = value; }
void HttpMessage :: set_body( std::string aBody ){ body = aBody; }


void HttpMessage :: append_to_body( std::string part ){ body += part; }

std::string HttpMessage :: get_body(){ return body; }
std::string HttpMessage :: get_method(){ return method; }
std::string HttpMessage :: get_path(){ return path; }

std::string HttpMessage :: get_version(){ return version; }
std::string HttpMessage :: get_code_string(){ return code_string; }
size_t HttpMessage :: get_code(){ return code; }

std::string HttpMessage :: get_header_field( std::string key ){
	
	std::string lkey = string_to_lower( key );
	
	// String case independent compare
	for( std::map<std::string, std::string>::iterator it = header.begin(); it != header.end() ; it++ ){
		if( string_to_lower(it->first) == lkey ){
			return it->second;
			}
		}
	
	return std::string();
	}

// Returns all header field names in specified in this message
std::vector< std::string > HttpMessage :: get_headers(){
	std::vector< std::string > out;
	
	// String case independent compare
	for( std::map< std::string, std::string >::iterator it = header.begin() ; it != header.end() ; it++ ){
		out.push_back( (*it).first );
		}
	
	return out;
	}

bool HttpMessage :: has_header_field( std::string key ){
	std::string lkey = string_to_lower( key );
	
	// String case independent compare
	for( std::map<std::string, std::string>::iterator it = header.begin(); it != header.end() ; it++ ){
		if( string_to_lower(it->first) == lkey ){
			return true;
			}
		}
	
	return false;
	}


// Serialize HttpMessage object to a string (very useful if you want to send a request :D )
std::string HttpMessage :: serialize(){
	std::stringstream stream;
	
	// code == 0 means that this is a HTTP request.
	if( code == 0 ){
		stream << method << " " << path << " " << version << "\r\n";
		}
	else {
		stream << version << " " << code << " " << code_string << "\r\n";
		}
		
	// If content-length header field is not provided, compute it
	if( header.find( "Content-Length") == header.end() && method != "GET" && method != "HEAD" ){
		header["Content-Length"] = integer_to_string( body.size() );
		}
	
	// Serialize all header fields into the stream, the order of the fields
	// STL implementation spesific and depends on the comparation function used in std::map
	for( std::map<std::string, std::string>::iterator it = header.begin(); it != header.end(); it++ ){
		stream << (*it).first << ": " << (*it).second << "\r\n";
		}
	stream << "\r\n";
	stream << body;
	
	
	return stream.str();
	}


URI :: URI(){}

URI :: URI( std::string url ){
	parse( url );
	}

std::string URI :: get_protocol(){ return protocol; }
std::string URI :: get_host(){ return host; }
std::string URI :: get_port(){ return port; }
std::string URI :: get_path(){ return path; }

// Parse given url
void URI :: parse( std::string url ){
	// Part url into (protocol, rest)
	std::vector< std::string > parts = split_string( url, "://" ); 
	std::string rest;
	
	// If something essential is missing....
	if( parts.size() < 2 ){
		// error handling
		throw std::string( "Invalid URI!" );
		return;
		}
	
	protocol = parts[0];
	
	// If there was more of the "://" parts in the url, parse them back together
	if( parts.size() > 2 ){
		for( size_t i = 1 ; i < parts.size()-1 ; ++i ){
			rest += parts[i] + std::string( "://" );
			}
		rest += parts[parts.size()-1];
		}
	else{
		rest = parts[1];
		}
	
	size_t pos = rest.find_first_of( "/" );

	// If path separator is not there default into "/"
	if( pos == std::string::npos ){
		port = "80";
		path = "/";
		host = rest;
		return;
		}
	host = rest.substr( 0, pos );
	path = rest.substr( pos, rest.size() - pos );
	
	// If no port provided
	if( host.find_first_of( ":" ) == std::string::npos ){
		port = "80";
		}
	else{
		// Parse port, find_last_of is used because of the ivp6-addresses
		pos = host.find_last_of( ":" );
		port = host.substr( pos+1, host.size()-pos );
		host = host.substr( 0, pos );
		}
	
	}
