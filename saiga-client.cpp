#include "libHttpClient.hpp"
#include <signal.h>

int main( int argc, char **argv ){
	
	
	HttpClient client;
	std::string url, method, file;
	
	
	if( argc < 2 ){
		std::cerr << "Usage: " << argv[0] << " GET|PUT url <file if PUT>" << std::endl << std::endl;
		return -1;
		}
	
	// Ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	
	method = std::string( argv[1] );
	url = std::string( argv[2] );
	
	// Check the input for the supported methods
	if( method != "PUT" && method != "put" && method != "GET" && method != "get" ){
		std::cerr << "Method (" << method << ") not supported." << std::endl;
		}
	
	// PUT requires three params
	if( method == "PUT" || method == "put" ){
		if( argc < 3 ){
			std::cerr << "Usage: " << argv[0] << " GET|PUT url <file if PUT>" << std::endl << std::endl;
			return -1;
			}
		file = argv[3];
		} 
	
	// Check the protocol (in future, add support for HTTPS) 
	if( url.find( "http://" ) != 0 ){
		std::cerr << "Invalid or not supported protocol." << std::endl;
		}
	
	 // Append trailing slash if not present in the url
	if( url.find( "/", 8 ) == std::string::npos ){
		url += "/";
		}
	
	if( method == "GET" || method == "get" ){		
		try{
			if( !client.do_get( url ) ){
				return -1;
				}
			} catch( std::string error ){
				std::cerr << "ERROR: " << error << std::endl;
				return -1;
				}
		}
	
	if( method == "PUT" || method == "put" ){
		std::string content;
		
		if( !read_file( file, content ) ){
			std::cerr << "File (" << file << ") not found!" << std::endl;
			return -1;
			} 
			
		if( !client.do_put( url, content ) ){
			return -1;
			}
		}
	
	
	HttpMessage msg = client.get_response();
	
	// Print headers to STDERR
	
	std::cerr << msg.get_version() << " " << msg.get_code() << " " << msg.get_code_string() << std::endl;
	
	std::vector< std::string > keys = msg.get_headers();
	
	for( size_t i = 0 ; i < keys.size() ; ++i ){
		std::cerr<< keys[i] << ": " << msg.get_header_field( keys[i] ) << std::endl;
		}
	
	std::cerr << std::endl;
	
	// Print message content to STDOUT
	
	std::cout << msg.get_body() << std::endl;
	
	return 0;
	}
