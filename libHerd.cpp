#include "libHerd.hpp"

int global_connection_count = 0;

const std::string SERVER_VERSION = "Saiga Server 0.0.3";

pthread_mutex_t queue_mutex 	= PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	queue_cond   	= PTHREAD_COND_INITIALIZER;


std::deque< int > client_queue;

void queue_add( int sock ){
	size_t size = 0;
	pthread_mutex_lock( &queue_mutex );
	size = client_queue.size();
	client_queue.push_back( sock );
	if( size == 0 ){
		pthread_cond_signal( &queue_cond ); 
		}
	pthread_mutex_unlock( &queue_mutex );
	
	}

int queue_get(){
	int out = -1;
	
	pthread_mutex_lock( &queue_mutex );

	while( client_queue.size() == 0 ){
		pthread_cond_wait( &queue_cond, &queue_mutex );
		}
	
	out = client_queue.front();
	client_queue.pop_front();
		
	pthread_mutex_unlock( &queue_mutex );
	
	return out;
	}


CachedEntry :: CachedEntry(){
	expires = std::numeric_limits<double>::infinity();
	}

CachedEntry :: CachedEntry( HttpMessage payload, double ttl ){
	expires = get_time() + ttl;
	content = payload;
	}

bool CachedEntry :: is_valid(){
	return expires > get_time();
	}

HttpMessage CachedEntry :: get_content(){ return content; } 


std::map< std::string, handler_function > RequestHandler :: handlers;
std::unordered_map< uint64_t, CachedEntry > RequestHandler :: cache;

pthread_rwlock_t RequestHandler :: cache_lock;

bool RequestHandler :: running = true;
hasher_function RequestHandler :: cache_hash_function = NULL;

void RequestHandler :: set_hash_function( hasher_function func ){
	cache_hash_function = func;
	}

uint64_t RequestHandler :: compute_hash( HttpMessage& request ){
	if( cache_hash_function != NULL ){
		return cache_hash_function( request );
		}
	else{
		std::string ckey = request.get_method() + ":" + request.get_path();
		return fnv1a_hash( ckey );
		}
	}

void RequestHandler :: init(){
	pthread_rwlock_init( &cache_lock, NULL );
	}

void RequestHandler :: add_handler( std::string pattern, handler_function func ){
	handlers[ pattern ] = func;
	}

void RequestHandler :: add_cache( uint64_t key, HttpMessage msg, double expires ){
	pthread_rwlock_wrlock( &cache_lock );
	cache[ key ] = CachedEntry( msg, expires );
	pthread_rwlock_unlock( &cache_lock );
	}
		

void* RequestHandler :: workerThread( void *arg ){
	
	while( RequestHandler::running ){
		int fd = queue_get();
		if( fd > 0 ){
			RequestHandler :: handleClient( fd );
			}
		}
	
	
	pthread_exit( arg );
	}

void* RequestHandler :: cleanerThread( void *arg ){
	double t0 = get_time() + 5.0;
	while( RequestHandler::running ){
		if( get_time() < t0 ){
			pthread_yield();
			}
		else{
			//std::cerr << "Zoink from cleanerThread" << std::endl;
			std::unordered_map< uint64_t, CachedEntry >::iterator it;
			//for( it = cache.begin() ; it != cache.end(); 
			it = cache.begin();
			while( it != cache.end() ){
				if( !(it->second).is_valid() ){
					pthread_rwlock_wrlock( &cache_lock );
					if( !(it->second).is_valid() ){
						cache.erase( it );
						//std::cerr << "Cleared cache entry..." << std::endl;
						}
					pthread_rwlock_unlock( &cache_lock );
					it = cache.begin();
					}
				
				}
			
			t0 = get_time() + 5.0;
			}
		
		}
	pthread_exit( arg );
	}


void RequestHandler :: handleClient( int fd ){
	Connection conn( fd );
	HttpMessage request, response;
	
	std::string content;
	std::vector< std::string > parts;
	std::string body;
	
	
	bool done = false;
	
	size_t request_counter = 0;
	
	conn.set_timeout( 15 );
	
	while( !done ){
		
		try{
			//std::cerr << "Handler " << *ptr_fd << " receiving..." << std::endl;
			while( content.find( "\r\n\r\n" ) == std::string::npos ){
				content += conn.receive( 1024 );
				}
			} catch( std::string error ){
				//std::cerr << "RH: ERROR: " << error << std::endl;
				done = true;
				continue;
				}
		//std::cerr << "Handler " << *ptr_fd << " done receiving..." << std::endl;
			
		//std::cerr << "content: '" << content << "'" << std::endl;
		parts = split_string( content, "\r\n\r\n" );
		if( parts.size() > 1 ){
			for( size_t i = 1 ; i < parts.size() - 1 ; ++i ){
				body += parts[i] + std::string( "\r\n\r\n" );
				}
			body += parts[parts.size()-1];
			}
		content.clear();
		std::vector<std::string> header_lines = split_string( parts[0], "\r\n" );
		std::vector< std::string > request_line = split_string( header_lines[0], " " );
		//std::cerr << "request_line: '" << header_lines[0] << "' and header_lines.size() = " << header_lines.size() << "request_line.size() = " <<  << std::endl;
		if( header_lines.size() < 2 || request_line.size() != 3 ){
			// return error response
			//conn.disconnect();
			std::cerr << "RH: ERROR: " << "Invalid request line!" << std::endl;
			done = true;
			continue;
			}
		
		request.set_method( request_line[0] );
		request.set_path( request_line[1] );
		request.set_version( request_line[2] );

		for( size_t i = 1 ; i < header_lines.size() ; ++i ){
			std::vector<std::string> tmp = split_string( header_lines[i], ":" );
			if( tmp.size() > 2 ){
				std::string str;
				for( size_t i = 1 ; i < tmp.size() - 1 ; ++i ){
					str += tmp[i] + std::string( ":" );
					}
				str += tmp[tmp.size()-1];
				request.set_header_field( trim(tmp[0] ), trim( str ) );
				}
			else{
				request.set_header_field( trim( tmp[0] ), trim( tmp[1] ) );
				}
			}
		
		
		if( ( request_line[0] == "PUT" ) || ( request_line[0] == "POST" ) ){
			size_t content_length = 0;
			
			if( request.has_header_field( "Content-Length" ) ){
				content_length = string_to_integer( request.get_header_field( "Content-Length" ) );
				}
			if( request.has_header_field( "Transfer-Encoding" ) ){
				if( request.get_header_field( "Transfer-Encoding" ) == "chunked" ){
					bool chunks_done = false;
					size_t chunk_size = 0, pos;
					content = body;
					body.clear();
					
					while( !chunks_done ){
						pos = content.find( "\r\n" );
						if( pos != std::string::npos ){
							std::string hex = content.substr( 0, pos );
							chunk_size = hex_string_to_integer( trim( hex ) );
							content = content.substr( pos+2 );
							if( chunk_size == 0 ){ chunks_done = true; continue; }
							}
						
						try{
							while( content.size() < chunk_size+2 ){
								content += conn.receive( chunk_size - content.size() + 2 );
								}
							} catch( std::string error ){
								//std::cerr << "RH: ERROR: " << error << std::endl;
								 
								done = true;
								continue;
								}
						body += content.substr( 0, chunk_size );
						content = content.substr( chunk_size + 2 );
						
						try{
							while( content.find( "\r\n" ) == std::string::npos ){
								content += conn.receive( 1 );
								}
							} catch( std::string error ){
								//std::cerr << "RH: ERROR:" << error << std::endl;
								done = true;
								break;
								}
						}
					
					}
				}
			else{
				try{
					while( body.size() < content_length ){
						body += conn.receive( content_length - content.size() );
						}
					} catch( std::string error ){
						std::cerr << "RH: ERROR:" << error << std::endl;
						done = true;
						continue;
						}
				}
		
			request.set_body( body );
			
			}
		
		
		//std::cerr << "Request: " << request.serialize() << std::endl;
		try{
			response = RequestHandler::handleRequest( request );
			
			conn.send( response.serialize() );
			} catch( std::string error ){
				//std::cerr << "RH: ERROR: " << error << std::endl;
				}
		
		done = true;
		if( request.has_header_field( "Connection" ) ){
			if( ( request.get_header_field( "Connection" ) == "Keep-Alive" ) || 
				( request.get_header_field( "Connection" ) == "keep-alive" ) ){
				done = false;
				}
			}
		++request_counter;
		if( request_counter > 15 ){
			done = true;
			}	
		}
	
	//printf( "Disconnecting %i \n", conn.get_fd() );
	//gconn_unmark( conn.get_fd() );
	conn.disconnect();
	
	--global_connection_count;
	
	//delete (int*)arg;
	
	//pthread_exit( NULL );
	}

HttpMessage RequestHandler :: handleRequest( HttpMessage& request ){
	HttpMessage response;
	std::string body, path, ckey;
	handler_function func = NULL;
	size_t pattern_length = 0;
	uint64_t cache_key;
	std::unordered_map<uint64_t, CachedEntry>::iterator cache_it;
	
	path = request.get_path();
	response.set_version( "HTTP/1.1" );
	response.set_header_field( "Server", SERVER_VERSION );
	response.set_header_field( "Content-Type", "text/html" );
	response.set_header_field( "Date", get_gmt_time(0) );
	
	/*
	ckey = request.get_method() + ":" + path;
	cache_key = fnv1a_hash( ckey );
	*/
	cache_key = compute_hash( request );
	
	pthread_rwlock_rdlock( &RequestHandler::cache_lock );
	cache_it = cache.find( cache_key );
	if( cache_it != cache.end() ){
		if( (cache_it->second).is_valid() ){
			pthread_rwlock_unlock( &RequestHandler::cache_lock );
			//std::cerr << "Found @ cache" << std::endl;
			return (cache_it->second).get_content();
			}
		}
	pthread_rwlock_unlock( &RequestHandler::cache_lock );
					
	for( std::map< std::string, handler_function >::iterator it = handlers.begin() ; it != handlers.end() ; it++ ){
		if( string_startswith( it->first, path ) ){
			if( (it->first).size() > pattern_length ){
				pattern_length = (it->first).size();
				func = it->second;
				}
			}
		}
	
	if( func == NULL ){
		response.set_code( 404 );
		response.set_code_string( "Not found." );
		response.set_version( "HTTP/1.1" );
		response.set_header_field( "Connection", "close" );
		response.set_header_field( "Date", get_gmt_time(0) );
		response.set_header_field( "Server", SERVER_VERSION );
		response.set_header_field( "Last-Modified", get_gmt_time(0) );
		response.set_header_field( "Content-Type", "text/plain" );
		response.set_body( "Error 404 - Not found!\n" );
		}
	else{
		response_options_t result;
		try{
			result = func( request, response );
			} catch( std::string error ){
				response.set_code( 500 );
				response.set_code_string( "Internal server error." );
				
				}
		if( !result.ok ){
			response.set_code( 404 );
			response.set_code_string( "Not found." );
			response.set_version( "HTTP/1.1" );
			response.set_header_field( "Connection", "close" );
			response.set_header_field( "Date", get_gmt_time(0) );
			response.set_header_field( "Server", SERVER_VERSION );
			response.set_header_field( "Last-Modified", get_gmt_time(0) );
			response.set_header_field( "Content-Type", "text/plain" );
			response.set_body( "Error 404 - Not found!\n" );
			}
		else{
			if( request.has_header_field( "Accept-Encoding" ) ){
				if( request.get_header_field( "Accept-Encoding" ).find( "gzip" ) != std::string::npos ){
					response.set_body( zlib_gzip_deflate( response.get_body() ) );
					response.set_header_field( "Content-Encoding", "gzip" );
					} 
				else{
					if( request.get_header_field( "Accept-Encoding" ).find( "deflate" ) != std::string::npos ){
						response.set_body( zlib_deflate( response.get_body() ) );
						response.set_header_field( "Content-Encoding", "deflate" );
						}
					}
				}
			
			if( result.cached ){
				add_cache( cache_key, response, result.expires );
				}
			
			}
		}
	
	return response;
	}




ThreadHerd :: ThreadHerd(){
	connected = false;
	running = false;
	}

void* ThreadHerd :: _bootstap_thread( void *context ){
	((ThreadHerd *)context)->run();
	pthread_exit( NULL );
	}

bool ThreadHerd :: init( size_t aport ){
	struct sockaddr_in6	server_addr;
	int fd, n, yes = 1;
	
	RequestHandler::init();
	
	port = aport;
	
	fd = socket( AF_INET6, SOCK_STREAM, 0 );
	
	if( fd < 0 ){
		throw std::string( "Socket error: ") + std::string( strerror( errno ) );
		return false;
		}
	
	setsockopt( fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof( int ) );
	
	memset( &server_addr, 0, sizeof( server_addr ) );
	
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons( port );
	
	
	
	n = bind( fd, (struct sockaddr *) &server_addr, sizeof( server_addr ) );
	if( n < 0 ){
		perror("bind");
		throw std::string( "Bind error: ") + std::string( gai_strerror( n ) );
		return false;
		}
	
	n = listen( fd, 250 );
	if( n < 0 ){
		perror("Listen");
		throw std::string( "Listen error: ") + std::string( gai_strerror( n ) );
		return false;
		}
	
	master_socket = fd;
	
	connected = true;
	
	return true;
	}

void ThreadHerd :: start(){
	if( connected ){
		
		int rc;
		pthread_attr_t attr;
		
		running = true;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		
		for( size_t i = 0 ; i < 128 ; ++i ){
			pthread_t handler;
			rc = pthread_create( &handler, &attr, &RequestHandler::workerThread, NULL );
			if( rc != 0 ){
				throw std::string( "Unable to start RH:worker thread." );
				}
			}
		
		
		rc = pthread_create( &worker, &attr, &ThreadHerd::_bootstap_thread, this );
		if( rc != 0 ){
			throw std::string( "Unable to start worker thread." );
			}
		
		
		rc = pthread_create( &cleaner, &attr, &RequestHandler::cleanerThread, NULL );
		if( rc != 0 ){
			throw std::string( "Unable to start cleaner thread." );
			}
			
		pthread_attr_destroy(&attr);
		
		}
	
	}


void ThreadHerd :: run(){
	
	int client, fdcount;
	socklen_t len;
	struct sockaddr_in6 client_addr;
	struct timeval tv;
	fd_set read_set;
	int conncount = 0;
	
	
	printf( "Entering loop... (master_socket = %i) \n", master_socket );
	while( running ){
		FD_ZERO( &read_set );
		FD_SET( master_socket, &read_set );
		tv.tv_sec = 1;
		tv.tv_usec = 0;		
	
		fdcount = select( master_socket + 1, &read_set, NULL, NULL, &tv ); 
		//printf( "Select: fdcount = %i \n", fdcount );
		if( fdcount == 0 ){
			//std::cerr << "Global connection count " << global_connection_count << std::endl;
			}
		if( fdcount > 0 ){
			if( FD_ISSET( master_socket, &read_set ) ){
				len = sizeof( client_addr );
				client = accept( master_socket, (struct sockaddr *) &client_addr, &len );
				
				if( client < 0 ){
					std::cerr << "Accept failed: " << gai_strerror( client ) << std::endl;
					perror( "accept" );
					continue;
					} 
				++global_connection_count;
				//gconn_mark( client );
				++conncount;
				
				
				queue_add( client );
				
				}
			}
		
		
		//print_conns();
		
		
		}
	
	}

void ThreadHerd :: stop(){
	void *status;
	running = false;
	RequestHandler :: running = false;
	
	pthread_join( worker, &status );
	for( size_t i = 0 ; i < handler_threads.size() ; ++i ){
		pthread_join( handler_threads[i], &status );
		} 
	
	close( master_socket );
	}


/*
int main(){
	
	ThreadHerd herd;
	//gconn_init();
	
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
*/
