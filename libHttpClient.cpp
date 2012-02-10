#include "libHttpClient.hpp"

HttpClient :: HttpClient(){
	own_conn = true;
	conn = new Connection();
	
	use_auth = false;
	}
HttpClient :: HttpClient( Connection *aConn ){
	conn = aConn;
	own_conn = false;
	
	use_auth = false;
	}

// If this instance created a new connection object, then delete it.
HttpClient :: ~HttpClient(){
	if( own_conn ){
		delete conn;
		}
	}

HttpMessage HttpClient :: get_response(){ return payload; }

// Provide credentials for HTTP Basic Auth
void HttpClient :: authentication( std::string user, std::string passwd ){
	use_auth = true;
	auth_username = user;
	auth_password = passwd;
	}
		

bool HttpClient :: do_get( std::string url ){
	
	HttpMessage msg;
	
	// Parse url
	URI uri( url );
	
	msg.set_method( "GET" );
	msg.set_version( "HTTP/1.1" );
	msg.set_path( uri.get_path() );
	msg.set_header_field( "Host", uri.get_host() );
	
	// Ask for compressed response
	msg.set_header_field( "Accept-Encoding", "gzip, deflate" );
	
	// Set course spesific header
	msg.set_header_field( "Iam", "gronhom1" );
	
	// if we have some authentication credentials, lets just use them
	if( use_auth ){
		std::string authstr = base64_encode( auth_username + ":" + auth_password );
		msg.set_header_field( "Authorization", "Basic " + authstr );
		}
	
	if( own_conn ){
		std::cerr << "Connecting to host " << uri.get_host() << ":" << uri.get_port() << std::endl;
		
		conn->set_host( uri.get_host() );
		conn->set_service( uri.get_port() );
		
		// Wait at maximum 15 seconds
		conn->set_timeout( 15 );
		
		try{
			conn->start();
			}
			catch( std::string error ){
				std::cerr << "Error: " << error << std::endl;
				return false;
				}
		}
	
	try{
		conn->send( msg.serialize() );
		} catch( std::string error ){
			std::cerr << "Error: " << error << std::endl;
			}
	
	bool ret;
	
	try{
		ret = receive_message( true );
		} catch( std::string error ){
			std::cerr << "Error: " << error << std::endl;
			return false;
			}
			
	return ret;
	}


bool HttpClient :: do_head( std::string url ){
	
	HttpMessage msg;
	
	URI uri( url );
	
	msg.set_method( "HEAD" );
	msg.set_version( "HTTP/1.1" );
	msg.set_path( uri.get_path() );
	
	msg.set_header_field( "Host", uri.get_host() );
	
	msg.set_header_field( "Iam", "gronhom1" );
	
	// check if no external socket was provided
	if( own_conn ){
		std::cerr << "Connecting to host " << uri.get_host() << ":" << uri.get_port() << std::endl;
		conn->set_host( uri.get_host() );
		conn->set_service( uri.get_port() );

		// Wait at maximum 15 seconds
		conn->set_timeout( 15 );
		try{
			conn->start();
			}
			catch( std::string error ){
				std::cerr << "Error: " << error << std::endl;
				return false;
				}
		}
	
	try{
		conn->send( msg.serialize() );
		} catch( std::string error ){
			std::cerr << "Error: " << error << std::endl;
			}
	
	bool ret;
	
	try{
		// Receive just headers, HEAD method shouldn't get any actual content from the page.
		ret = receive_message( false );
		} catch( std::string error ){
			std::cerr << "Error: " << error << std::endl;
			return false;
			}
			
	return ret;
	}

// Parameter content is sent to the server

bool HttpClient :: do_put( std::string url, std::string content ){
	
	HttpMessage msg;
	
	URI uri( url );
	
	msg.set_method( "PUT" );
	msg.set_version( "HTTP/1.1" );
	msg.set_path( uri.get_path() );
	msg.set_header_field( "Host", uri.get_host() );
	
	// TODO: add mime type checking
	msg.set_header_field( "Content-Type", "text/plain" );
	
	msg.set_header_field( "Iam", "gronhom1" );
	
	msg.set_body( content );
	
	if( own_conn ){
		conn->set_host( uri.get_host() );
		conn->set_service( uri.get_port() );

		// Wait at maximum 15 seconds
		conn->set_timeout( 15 );
		try{
			conn->start();
			}
			catch( std::string error ){
				std::cerr << "Error: " << error << std::endl;
				return false;
				}
		}
	
	try{
		conn->send( msg.serialize() );
		} catch( std::string error ){
			std::cerr << "Error: " << error << std::endl;
			}
	
	bool ret;
	
	try{
		ret = receive_message( true );
		} catch( std::string error ){
			std::cerr << "Error: " << error << std::endl;
			return false;
			}
			
	return ret;
	}


bool HttpClient :: receive_message( bool receive_body ){
	std::string content;
	std::vector< std::string > parts;
	std::string body;
	
	// First, lets receive the header
	while( content.find( "\r\n\r\n" ) == std::string::npos ){
		content += conn->receive( 1 );
		}
	
	parts = split_string( content, "\r\n\r\n" );
	
	// Check if we got also some of the body 
	if( parts.size() > 1 ){
		// Parse the body back together
		for( size_t i = 1 ; i < parts.size() - 1 ; ++i ){
			body += parts[i] + std::string( "\r\n\r\n" );
			}
		body += parts[parts.size()-1];
		}
	content.clear();
	
	// Split header into lines and first header line in order to obtain response status
	std::vector<std::string> header_lines = split_string( parts[0], "\r\n" );
	std::vector< std::string > response_line = split_string( header_lines[0], " " );
	
	// First line format <http version> <status code> <status explanation>
	payload.set_version( response_line[0] );
	payload.set_code( string_to_integer( response_line[1] ) );
	payload.set_code_string( response_line[2] );
	
	// Parse headers into a key-value pairs
	for( size_t i = 1 ; i < header_lines.size() ; ++i ){
		
		std::vector<std::string> tmp = split_string( header_lines[i], ":" );
		
		// Check if the header value contained ':'-marks
		if( tmp.size() > 2 ){
			std::string str;
			// Parse extra splits back together
			for( size_t i = 1 ; i < tmp.size() - 1 ; ++i ){
				str += tmp[i] + std::string( ":" );
				}
		
			str += tmp[ tmp.size() - 1 ];
			
			// Strip extra whitespaces from key- and value-fields
			payload.set_header_field( trim(tmp[0] ), trim( str ) );
		
			}
		
		else{
			// Strip extra whitespaces from key- and value-fields
			payload.set_header_field( trim( tmp[0] ), trim( tmp[1] ) );
			}
		}
		
	size_t content_length = 0;
	
	// Check if the Content Length header field is set and utilize it if present
	if( payload.has_header_field( "Content-Length" ) ){
		content_length = string_to_integer( payload.get_header_field( "Content-Length" ) );
		}
	
	// Receive body if we want to (e.g. when doing HEAD we don't want to do this)
	if( receive_body ){
		if( payload.has_header_field( "Transfer-Encoding" ) ){
			
			// Currently only supporting 'chunked' transfer encoding
			
			if( payload.get_header_field( "Transfer-Encoding" ) == "chunked" ){
				bool done = false;
				size_t chunk_size = 0, pos;
				content = body;
				body.clear();
				while( !done ){
					pos = content.find( "\r\n" );
					if( pos != std::string::npos ){
						// parse chunk length
						std::string hex = content.substr( 0, pos );
						
						chunk_size = hex_string_to_integer( trim( hex ) );
						
						// skip the CRLF separator
						content = content.substr( pos+2 );
						
						// Did we receive the last chunk
						if( chunk_size == 0 ){ done = true; continue; }
						}
					
					// read chunk + trailing CRLF
					while( content.size() < chunk_size + 2 ){
						content += conn->receive( chunk_size - content.size() + 2 );
						}
					if( chunk_size > 0 ){
						body += content.substr( 0, chunk_size );
						content = content.substr( chunk_size + 2 );
						}
					
					// try to find the CRLF
					while( content.find( "\r\n" ) == std::string::npos ){
						content += conn->receive( 1 );
						}
					
					}
				
				}
			
			}
		else{
			// Receive content_length amount of bytes from server
			while( body.size() < content_length ){
				body += conn->receive( content_length - content.size() );
				}
			}
		
		// Check if the content was compressed and handle it appopriately
		
		if( payload.get_header_field( "Content-Encoding" ) == "deflate" ){
			body = zlib_inflate( body );
			}
		
		if( payload.get_header_field( "Content-Encoding" ) == "gzip" ){
			body = zlib_gzip_inflate( body );
			}
		
		
		payload.set_body( body );
		
		
		}
	return true; 
	}
