#include "libConnection.hpp"

// Create a new emptry connection
Connection :: Connection(){
	connected = false;
	timeout = -1;
	own_sock = true;
	}

// Create a new connection setting both host and service (port) parameters
Connection :: Connection( std::string aHost, std::string aService ){
	connected = false;
	host = aHost;
	service = aService;
	timeout = -1;
	own_sock = true;
	}

// Create a wrapper for already opened socket connection
Connection :: Connection( int external_socket ){
	fd = external_socket;
	connected = true;
	timeout = -1;
	own_sock = false;
	}

// Connection destructor, if not external socket -> disconnect
Connection :: ~Connection(){
	if( own_sock ){
		disconnect();
		}
	}


// Setters and getters

void Connection :: set_host( std::string aHost ){ host = aHost; }

void Connection :: set_service( std::string aService ){ service = aService; }

void Connection :: set_timeout( int aTimeout ){ timeout = aTimeout; }

std::string Connection :: get_host(){ return host; }

std::string Connection :: get_service(){ return service; }

std::string Connection :: get_address(){ return address; }


// Do the actual connecting, returns true when connected

bool Connection :: start(){
	// check if already connected
	if( connected ){ return true; }
	
	// if invalid host or service then fail
	if( host.size() < 1 || service.size() < 1 ){ return false; }
	
	int n, new_fd;
	char buffer[128];
	struct  addrinfo hints, *result, *result_saved;
	
	size_t buffer_size = sizeof( buffer );
	
	fd = -1;
	
	memset( &hints, 0, sizeof( struct addrinfo ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	// get info on provided address
	n = getaddrinfo( host.c_str(), service.c_str(), &hints, &result );
	
	
	if( n != 0 ){
			throw std::string( gai_strerror( n ) );
		return false;
		}
	
	result_saved = result;
	
	while( result != NULL ){
		
		new_fd = socket( result->ai_family, result->ai_socktype, result->ai_protocol );
		if( new_fd < 0 ){
			continue;
			}
				
		n = connect( new_fd, result->ai_addr, result->ai_addrlen );
		if( n == 0 ){
			fd = new_fd;
			break;
			}
		
		result = result->ai_next;
		}
	
	if( fd < 0 ){
		// error handling
		throw std::string( gai_strerror( fd ) );
		return false;
		}
	else{
		struct sockaddr_in *sin = (struct sockaddr_in *)result->ai_addr;
		inet_ntop( result->ai_family, &( sin->sin_addr ), buffer, buffer_size );
		address = std::string( buffer );	
		}
	
	freeaddrinfo( result_saved );
	
	connected = true;
	return true;
	}

void Connection :: disconnect(){
	close( fd );
	}

bool Connection :: send( std::string content ){
	size_t sent = 0, len = content.size();
	const char *ptr = content.c_str();
	int n = 0;
	fd_set writers;
	struct timeval tv;
	
	if( !connected ){ 
		throw std::string( "Not connected!" );
		}
	
	while( sent < len ){
		
		FD_ZERO( &writers );
		FD_SET( fd, &writers );
		
		// Block if no timeout set
		if( timeout < 1 ){
			n = write( fd, ptr + sent, len - sent );
			}
		// Otherwise block at maximum timeout amount of seconds
		else {
			tv.tv_sec = timeout;
			tv.tv_usec = 0;
			n = select( fd + 1, NULL, &writers, NULL, &tv );
			if( n == 0 ){
				std::cerr << "Send timeout" << std::endl;
				throw std::string( "Timeout happened." );
				return false; 
				}
			n = write( fd, ptr + sent, len - sent );
			}
		if( n <= 0 ){
			perror( "send");
			throw std::string( gai_strerror( n ) );
			return false;
			}
		else{
			sent += n;
			}
		}
	return true;
	}

std::string Connection :: receive( size_t len ){
	int n = 0;
	std::string out;
	char *buffer;
	fd_set readers;
	struct timeval tv;
	
	FD_ZERO( &readers );
	FD_SET( fd, &readers );
	
	
	if( !connected ){
		throw std::string( "Not connected!" );
		}
	
	buffer = new char[ len + 1 ];
	memset( buffer, 0, len + 1 );
	// Block if no timeout is defined
	if( timeout < 1 ){
		n = read( fd, buffer, len );
		}
	// Otherwise block at maximum timeout amount of seconds
	else{
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		n = select( fd + 1, &readers, NULL, NULL, &tv );
		if( n == 0 ){
			std::cerr << "timeout!" << std::endl;
			throw std::string( "Timeout happened." );
			return std::string(); 
			}
		n = read( fd, buffer, len );
		}
	
	
	if( n < 0 ){
		perror("read");
		//std::cerr << "read fd: " << fd << std::endl;
		throw std::string( gai_strerror( n ) );
		return std::string();
		}
	if( n == 0 ){
		//std::cerr << "Nothing to read" << std::endl;
		throw std::string( "Nothing to read." );
		//return std::string();
		}
	
	out = std::string( buffer, n );
	
	delete [] buffer;
	
	return out;
	}

int Connection :: get_fd(){ return fd; }
