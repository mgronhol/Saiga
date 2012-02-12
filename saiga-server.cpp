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





int main( int argc, char **argv ){

	ThreadHerd herd;
	
	signal(SIGPIPE, SIG_IGN);


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
