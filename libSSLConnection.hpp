#ifndef _LIBSSLCONNECTION_HPP_
#define _LIBSSLCONNECTION_HPP_

#include <sys/select.h>
#include <sys/socket.h>
#include <strings.h>
#include <netdb.h>
#include <cstdio>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <iterator>
#include <sstream>
#include <sys/types.h>
#include <time.h>

#include <openssl/ssl.h>
#include <openssl/err.h>


class SSLConnection {
	public:
		SSLConnection();
		SSLConnection( std::string, std::string );
		SSLConnection( int );
		~SSLConnection();
		
		void set_host( std::string );
		void set_service( std::string );
		std::string get_host();
		std::string get_service();
		std::string get_address();
		
		bool start();
		void disconnect();
		
		bool is_connected();

		std::string receive( size_t );
		bool send( std::string );
		
		int get_fd();
		
		void set_timeout( int );
	
	private:
		int fd;
		bool connected, own_sock;
		std::string host, service, address;
		int timeout;
		SSL_CTX *ctx;
		SSL *ssl;
    
		SSL_CTX* init_ssl_context();
		
	};



#endif
