#ifndef _LIBHTTPMESSAGE_HPP_
#define _LIBHTTPMESSAGE_HPP_

#include "libConnection.hpp"
#include "libUtils.hpp"

class HttpMessage { 
	public:
		HttpMessage();
		
		void set_method( std::string );
		void set_path( std::string );
		void set_code( size_t );
		void set_code_string( std::string );
		void set_version( std::string );
		void set_header_field( std::string, std::string );
		void set_body( std::string );
		void append_to_body( std::string );

		std::string get_body();
		std::string get_method();
		std::string get_path();
		std::string get_version();
		std::string get_code_string();
		size_t get_code();
		
		std::string get_header_field( std::string );
		
		std::vector< std::string > get_headers();
		
		bool has_header_field( std::string );
		
		std::string serialize();



	private:
		std::string body;
		std::map< std::string, std::string > header;
		std::string path;
		std::string version;
		std::string method;
		size_t code;
		std::string code_string;
	};


class URI {
	public:
		URI();
		URI( std::string );
		
		void parse( std::string );
		
		std::string get_protocol();
		std::string get_host();
		std::string get_port();
		std::string get_path();
	
	private:
		std::string uri;
		std::string protocol;
		std::string host;
		std::string port;
		std::string path;

	};




#endif
