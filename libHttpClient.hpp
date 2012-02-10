#ifndef _LIBHTTPCLIENT_HPP_
#define _LIBHTTPCLIENT_HPP_

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
#include <stddef.h>

#include "libUtils.hpp"
#include "libCompression.hpp"
#include "libConnection.hpp"
#include "libHttpMessage.hpp"

#include "libBase64.hpp"

class HttpClient {
	
	public:
		HttpClient();
		HttpClient( Connection * );
		~HttpClient();
		void authentication( std::string, std::string );
		bool do_get( std::string );
		bool do_head( std::string );
		bool do_put( std::string, std::string );
		
		HttpMessage get_response();
	
	private:
		Connection *conn;
		bool own_conn;
		HttpMessage payload;
		bool use_auth;
		std::string auth_username, auth_password;
		
		bool receive_message( bool );
		
			
	};


#endif
