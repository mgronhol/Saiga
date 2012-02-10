#include "libHttpClient.hpp"


int main( int argc, char **argv ){
	
	
	HttpClient client;
	
	
	if( argc < 2 ){
		return -1;
		}
	
	
	//client.authentication( "markus", "moikka" );
	if( !client.do_get( argv[1] ) ){
	//if( !client.do_get( "http://localhost/~markus/nwprog/auth/" ) ){
		
		std::cerr << "Phail!" << std::endl;
		return 1;
		}
	
	//std::cout << client.get_response().serialize();
	HttpMessage msg = client.get_response();
	std::vector< std::string > keys = msg.get_headers();
	for( size_t i = 0 ; i < keys.size() ; ++i ){
		std::cout << keys[i] << ": " << msg.get_header_field( keys[i] ) << std::endl;
		}
	
	std::cout << std::endl;
	
	std::cout << msg.get_body() << std::endl;
	
	return 0;
	}
