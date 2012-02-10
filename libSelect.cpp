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

#include "libConnection.hpp"
#include "libUtils.hpp"
#include "libHttpMessage.hpp"


class SocketMultiplexer {
	public:
		SocketMultiplexer();
		~SocketMultiplexer();
		
		void set_port( size_t );
		
		bool init();
		
		void start();
		
		void run();
		
		void stop();
		
		bool fetch_message( int*, HttpMessage& );
		
		bool put_message( int, HttpMessage&  );
		
		bool close_connection( int );

		static void *_bootstap_thread( void* );

	private:
		std::map< int, std::string > send_buffer, recv_buffer;
		std::map< int, std::deque< HttpMessage > > message_buffer;
		std::set<int> sockets;
		int master_socket;
		
		size_t port;
		bool connected;
		bool running;
		
		pthread_t worker;
		pthread_mutex_t lock_send, lock_recv, lock_mbuffer;
		
	};

void* SocketMultiplexer :: _bootstap_thread( void *context ){
	((SocketMultiplexer *)context)->run();
	pthread_exit( NULL );
	}


SocketMultiplexer :: SocketMultiplexer(){
	connected = false;
	port = 8000;
	running = false;
	}

SocketMultiplexer :: ~SocketMultiplexer(){
	stop();
	}

void SocketMultiplexer :: set_port( size_t aPort ){
	port = aPort;
	}

void SocketMultiplexer :: stop(){
	if( connected ){
		for( std::set<int>::iterator it = sockets.begin() ; it != sockets.end() ; it++ ){
			close( *it );
			}
		close( master_socket );
		
		pthread_mutex_destroy( &lock_mbuffer );
		pthread_mutex_destroy( &lock_recv );
		pthread_mutex_destroy( &lock_mbuffer );
		
		
		}
	
	}

bool SocketMultiplexer :: init(){
	int fd, n;
	struct sockaddr_in6	server_addr;
	
	fd = socket( AF_INET6, SOCK_STREAM, 0 );
	
	if( fd < 0 ){
		throw std::string( "Socket error: ") + std::string( gai_strerror( fd ) );
		return false;
		}
	
	memset( &server_addr, 0, sizeof( server_addr ) );
	
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons( port );
	
	n = bind( fd, (struct sockaddr *) &server_addr, sizeof( server_addr ) );
	if( n < 0 ){
		throw std::string( "Bind error: ") + std::string( gai_strerror( n ) );
		return false;
		}
	
	n = listen( fd, 32 );
	if( n < 0 ){
		throw std::string( "Listen error: ") + std::string( gai_strerror( n ) );
		}
	
	master_socket = fd;

	connected = true;
	
	pthread_mutex_init( &lock_send, NULL );
	pthread_mutex_init( &lock_recv, NULL );
	pthread_mutex_init( &lock_mbuffer, NULL );
	
	
	return true;
	}


void SocketMultiplexer :: start(){
	if( connected ){
		int rc;
		pthread_attr_t attr;
		
		running = true;
		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
		
		rc = pthread_create( &worker, &attr, &SocketMultiplexer::_bootstap_thread, this );
		if( rc != 0 ){
			throw std::string( "Unable to start worker thread." );
			}
		
		pthread_attr_destroy(&attr);
		}
	}

void SocketMultiplexer :: run(){
	
	struct timeval tv;
	fd_set read_set, write_set;
	int maxfd, fdcount;
	

	while( running ){
		maxfd = *sockets.end();
		FD_ZERO( &read_set );
		FD_ZERO( &write_set );
		maxfd = master_socket;
		
		FD_SET( master_socket, &read_set );
		for( std::set<int>::iterator it = sockets.begin(); it != sockets.end() ; it++ ){
			FD_SET( *it, &read_set );
			FD_SET( *it, &write_set );
			if( *it > maxfd ){ maxfd = *it; }
			} 
		
		tv.tv_sec = 15;
		tv.tv_usec = 0;
		
		fdcount = select( maxfd + 1, &read_set, &write_set, NULL, &tv );
		if( fdcount < 0 ){ return; }
		
		if( fdcount > 0 ){
			for( std::set<int>::iterator it = sockets.begin(); it != sockets.end() ; it++ ){
				
				if( FD_ISSET( *it, &read_set ) ){
					
					
					}
			
				
				}
			
			}
		
		}
	
	}









int main(){
	
	
	
	
	
	
	return 0;
	}
