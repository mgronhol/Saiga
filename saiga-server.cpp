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


response_options_t handler_index( HttpMessage& request, HttpMessage& response ){
	response_options_t out;
	std::string body;
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
	return out;
	}



int main( int argc, char **argv ){

	ThreadHerd herd;
	
	signal(SIGPIPE, SIG_IGN);

	RequestHandler::add_handler( "/index", &handler_index );

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
