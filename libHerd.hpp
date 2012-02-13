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

#include <unordered_map>
#include <limits>

#include "libConnection.hpp"
#include "libUtils.hpp"
#include "libHttpMessage.hpp"
#include "libCompression.hpp"


typedef struct ResponseOptions {
	bool ok;
	bool cached;
	double expires;
	} response_options_t;

typedef response_options_t (*handler_function)( HttpMessage&, HttpMessage& );

class CachedEntry {
	public:
		CachedEntry();
		CachedEntry( HttpMessage, double );
		bool is_valid();
		HttpMessage get_content();
	private:
		double expires;
		HttpMessage content;
	};

class RequestHandler {
	public:
		static void* workerThread( void * );
		static void handleClient( int );
		static HttpMessage handleRequest( HttpMessage& );
		static bool running;
		static void add_handler( std::string, handler_function ); 
		static void add_cache( std::string, HttpMessage, double );
		static std::map< std::string, handler_function > handlers;
		static std::unordered_map< uint64_t, CachedEntry > cache;
		static pthread_rwlock_t cache_lock;
		static void init();
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
