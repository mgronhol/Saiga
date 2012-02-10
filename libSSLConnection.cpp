#include "libSSLConnection.hpp"


SSLConnection :: SSLConnection(){
	connected = false;
	timeout = -1;
	own_sock = true;
	}

SSLConnection :: SSLConnection( std::string aHost, std::string aService ){
	connected = false;
	host = aHost;
	service = aService;
	timeout = -1;
	own_sock = true;
	}

SSLConnection :: SSLConnection( int external_socket ){
	fd = external_socket;
	connected = true;
	timeout = -1;
	own_sock = false;
	}

SSLConnection :: ~SSLConnection(){
	if( own_sock ){
		disconnect();
		}
	}


void SSLConnection :: set_host( std::string aHost ){ host = aHost; }
void SSLConnection :: set_service( std::string aService ){ service = aService; }
void SSLConnection :: set_timeout( int aTimeout ){ timeout = aTimeout; }

std::string SSLConnection :: get_host(){ return host; }
std::string SSLConnection :: get_service(){ return service; }
std::string SSLConnection :: get_address(){ return address; }

SSL_CTX* SSLConnection :: init_ssl_context(){
	SSL_CTX *context;
    
	SSL_library_init();
    
    OpenSSL_add_all_algorithms();		/* Load cryptos, et.al. */
    
    SSL_load_error_strings();			/* Bring in and register error messages */
    
    
    context = SSL_CTX_new( SSLv23_client_method() );			/* Create new context */
    
    if ( ctx == NULL ){
        ERR_print_errors_fp(stderr);
        abort();
		}
		
    return context;
	
	}

bool SSLConnection :: start(){
	if( connected ){ return true; }
	if( host.size() < 1 || service.size() < 1 ){ return false; }
	
	int n, new_fd;
	char buffer[128];
	struct  addrinfo hints, *result, *result_saved;
	
	size_t buffer_size = sizeof( buffer );
	
	fd = -1;
	
	memset( &hints, 0, sizeof( struct addrinfo ) );
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	
	n = getaddrinfo( host.c_str(), service.c_str(), &hints, &result );
	
	if( n != 0 ){
		// error handling here
		//std::cerr << "ERROR: " << gai_strerror( n ) << std::endl;
		throw std::string( gai_strerror( n ) );
		return false;
		}
	
	ctx = init_ssl_context();
	ssl = SSL_new( ctx );
	
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
		//std::cerr << "ERROR: " << gai_strerror( fd ) << std::endl;
		throw std::string( gai_strerror( fd ) );
		return false;
		}
	else{
		struct sockaddr_in *sin = (struct sockaddr_in *)result->ai_addr;
		inet_ntop( result->ai_family, &( sin->sin_addr ), buffer, buffer_size );
		address = std::string( buffer );	
		}
	
	freeaddrinfo( result_saved );
	
	SSL_set_fd( ssl, fd );
	if( SSL_connect( ssl ) == -1 ){
		throw std::string( "SSL Failed!" );
		}
	
	
	connected = true;
	return true;
	}

void SSLConnection :: disconnect(){
	//if( connected ){
		//std::cerr << "Disconnecting..." << std::endl;
	SSL_free( ssl );
		close( fd );
	//	}
	SSL_CTX_free( ctx );
	}

bool SSLConnection :: send( std::string content ){
	size_t sent = 0, len = content.size();
	const char *ptr = content.c_str();
	int n = 0;
	fd_set writers;
	struct timeval tv;
	
	if( !connected ){ return false; }
	
	while( sent < len ){
		FD_ZERO( &writers );
		FD_SET( fd, &writers );
		if( timeout == 0 ){
			n = SSL_write( ssl, ptr + sent, len - sent );
			}
		else {
			tv.tv_sec = timeout;
			tv.tv_usec = 0;
			n = select( fd + 1, NULL, &writers, NULL, &tv );
			if( n == 0 ){
				std::cerr << "Send timeout" << std::endl;
				throw std::string( "Timeout happened." );
				return false; 
				}
			n = SSL_write( ssl, ptr + sent, len - sent );
			}
		if( n <= 0 ){
			// error handling
			//std::cerr << "ERROR: " << gai_strerror( n ) << std::endl;
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

std::string SSLConnection :: receive( size_t len ){
	int n = 0;
	std::string out;
	char *buffer;
	fd_set readers;
	struct timeval tv;
	
	FD_ZERO( &readers );
	FD_SET( fd, &readers );
	
	
	if( !connected ){
		std::cerr << "Not connected" << std::endl;
		throw std::string( "Not connected!" );
		return std::string(); 
		}
	
	buffer = new char[ len + 1 ];
	memset( buffer, 0, len + 1 );
	if( timeout < 0 ){
		n = SSL_read( ssl, buffer, len );
		}
	else{
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		n = select( fd + 1, &readers, NULL, NULL, &tv );
		if( n == 0 ){
			std::cerr << "timeout!" << std::endl;
			throw std::string( "Timeout happened." );
			return std::string(); 
			}
		n = SSL_read( ssl, buffer, len );
		}
	
	
	if( n < 0 ){
		perror("read");
		std::cerr << "read fd: " << fd << std::endl;
		throw std::string( gai_strerror( n ) );
		return std::string();
		}
	if( n == 0 ){
		std::cerr << "Nothing to read" << std::endl;
		throw std::string( "Nothing to read." );
		return std::string();
		}
	
	out = std::string( buffer, n );
	
	delete [] buffer;
	
	return out;
	}

int SSLConnection :: get_fd(){ return fd; }
