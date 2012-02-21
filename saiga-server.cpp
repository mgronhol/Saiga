#include <sys/socket.h>
#include <sys/select.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <strings.h>

#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <iterator>
#include <deque>

#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "libConnection.hpp"
#include "libUtils.hpp"
#include "libHttpMessage.hpp"
#include "libCompression.hpp"
#include "libHerd.hpp"
#include "libSaigaConnector.hpp"

response_options_t handler_index( HttpMessage& request, HttpMessage& response ){
	response_options_t out;
	std::string body;
	
	if( request.get_path() != "/index" ){
		out.ok = false;
		return out;
		}
	
	body += "/*************************\\\n";
	body += "|                         |\n";
	body += "| Saiga Server            |\n";
	body += "|     by Alshain Oy       |\n";
	body += "|        version 0.0.3    |\n";
	body += "|                --rime   |\n";
	body += "\\*************************/\n";
	
	response.set_body( body );
	response.set_code( 200 );
	response.set_code_string( "Ok." );
	response.set_header_field( "Content-Type", "text/plain" );
	
	out.ok = true;
	out.cached = true;
	out.expires = 60;
	std::cout << "Regenerating index..." << std::endl;
	return out;
	}

response_options_t handler_static_files( HttpMessage& request, HttpMessage& response ){
	response_options_t out;
	bool file_exists;
	std::string body;
	std::vector< std::string > path_parts = split_string( request.get_path(), "/" );
	std::string file_path = "www";
	for( size_t i = 2 ; i < path_parts.size() ; ++i ){
		if( path_parts[i] != ".." && path_parts[i] != "~" ){
			file_path += "/" + path_parts[i];
			}
		}
	std::cout << "file_path: '" << file_path << "'" << std::endl;
	file_exists = read_file( file_path, body );
	if( file_exists ){
		out.ok = true;
		out.cached  = true;
		out.expires = 120;
		response.set_body( body );
		response.set_code( 200 );
		response.set_code_string( "Ok." );
		response.set_header_field( "Content-Type", "text/plain" );
		return out;
		}
	
	out.ok = false;
	return out;
	}

response_options_t handler_connector( HttpMessage& request, HttpMessage& response ){
	response_options_t out;
	ConnectorMessage msg;
	SaigaConnector connector( "/tmp/default.saiga" );
	uint64_t cache_msecs;
	
	bool connected = connector.start();
	if( !connected ){
		throw std::string( "No one to connect to!" );
		}
	
	msg = connector.process( request );
	
	if( msg.get( "status") != "ok" ){
		out.ok = false;
		return out;
		}
	
	response.set_body( msg.get( "body" ) );
	response.set_code( string_to_integer( msg.get( "code" ) ) );
	response.set_code_string( "Ok." );
	out.ok = true;
	cache_msecs = string_to_integer( msg.get( "cache" ) );
	if( cache_msecs == 0 ){
		out.cached = false;
		}
	else{
		out.cached = true;
		out.expires = cache_msecs / 1000.0;
		}

	// Header fields
	std::vector< std::string > fields = msg.get_keys();
	for( size_t i = 0 ; i < fields.size() ; ++i ){
		if( string_startswith( "header.", fields[i] ) ){
			std::string field = fields[i].substr( strlen( "header." ) );
			std::vector< std::string > parts = split_string( field, ":" );
			std::string key, value;
			key = trim( parts[0] );
			if( parts.size() > 2 ){
				value = parts[1];
				for( size_t j = 2 ; j < parts.size() ; ++j ){
					value += ":" + parts[j];
					}
				}
			value = trim( value );
			response.set_header_field( key, value );
			}
		}
	
	return out;
	}


int main( int argc, char **argv ){

	ThreadHerd herd;
	
	signal(SIGPIPE, SIG_IGN);

	RequestHandler::add_handler( "/index", &handler_index );
	RequestHandler::add_handler( "/static/", &handler_static_files );
	RequestHandler::add_handler( "/api/", &handler_connector );

	try{
		herd.init( 8000 );
		herd.start();
		
		printf( "Press enter to stop... \n" );
		getc(stdin);
		
		herd.stop();
		} catch( std::string error ){
			std::cerr << "ERROR: " << error << std::endl;
			}
	return 0;



	}
