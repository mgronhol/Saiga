#ifndef _LIBSAIGACONNECTOR_HPP_
#define _LIBSAIGACONNECTOR_HPP_

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/un.h>

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


class ConnectorMessage {
	public:
		ConnectorMessage();
		void set( std::string, std::string );
		std::string get( std::string );
		std::vector< std::string > get_keys();
		std::string serialize();
	
		static std::string to_pstr( std::string );
		static std::string encode_int( uint64_t );
		static uint64_t decode_int( std::string* );
		
	private:
		std::map< std::string, std::string > content;
	};


class SaigaConnector {
	public:
		SaigaConnector();
		SaigaConnector( std::string );
		~SaigaConnector();
		void set_socket( std::string );
		bool start();
		ConnectorMessage process( HttpMessage& );
	private:
		Connection conn;
		std::string socket_path;
		bool connected;
	};


#endif
