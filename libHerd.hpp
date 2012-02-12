#ifndef _LIBHERD_HPP_
#define _LIBHERD_HPP_

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

class RequestHandler {
	public:
		static void* workerThread( void * );
		static void handleClient( int );
		static HttpMessage handleRequest( HttpMessage& );
		static bool running;
		
	};



class ThreadHerd {
	public:
		ThreadHerd();
		
		bool init( size_t );
		
		void start();
		
		void run();
		
		void stop();
	
		static void *_bootstap_thread( void* );
		
	private:
		int master_socket;
		bool connected, running;
		size_t port;
		
		pthread_t worker;
		
		std::vector< pthread_t > handler_threads;
		
	};
































#endif
