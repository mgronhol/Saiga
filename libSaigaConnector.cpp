#include "libSaigaConnector.hpp"



std::string ConnectorMessage :: encode_int( uint64_t value ){
	char buffer[32];
	memset( buffer, 0, 32 );
	memcpy( buffer, &value, sizeof( uint64_t ) );
	return std::string( buffer, sizeof( uint64_t ) );
	}

uint64_t ConnectorMessage :: decode_int( std::string *str ){
	uint64_t out = 0;
	if( str->size() < sizeof( uint64_t ) ){
		return 0;
		}
	memcpy( &out, str->c_str(), sizeof( uint64_t ) );
	return out;
	}

std::string ConnectorMessage :: to_pstr( std::string str ){
	uint64_t len;
	char buffer[32];
	memset( buffer, 0, 32 );
	len = str.size();
	memcpy( buffer, &len, sizeof( uint64_t ) );
	return std::string( buffer, sizeof( uint64_t ) ) + str;
	}

ConnectorMessage :: ConnectorMessage(){}

void ConnectorMessage :: set( std::string key, std::string value ){
	content[key] = value;
	}

std::string ConnectorMessage :: get( std::string key ){
	std::map< std::string, std::string >::iterator it;
	it = content.find( key );
	if( it != content.end() ){
		return it->second;
		}
	return std::string();
	}

std::vector< std::string > ConnectorMessage :: get_keys(){
	std::map< std::string, std::string >::iterator it;
	std::vector< std::string > out;
	for( it = content.begin() ; it != content.end() ; it++ ){
		out.push_back( it->first );
		} 
	return out;
	}



std::string ConnectorMessage :: serialize(){
	std::string out;
	for( std::map< std::string, std::string >::iterator it = content.begin() ; it != content.end() ; it++ ){
		out += to_pstr( it->first );
		out += to_pstr( it->second );
		}
	
	return encode_int( out.size() ) + out;
	}


SaigaConnector :: SaigaConnector(){
	connected = false;
	}

SaigaConnector :: SaigaConnector( std::string path ){
	socket_path = path;
	connected = false;
	}

void SaigaConnector :: set_socket( std::string path ){
	socket_path = path;
	}

SaigaConnector :: ~SaigaConnector(){
	if( connected ){
		conn.disconnect();
		}
	}

bool SaigaConnector :: start(){
	struct sockaddr_un sock_addr;
	size_t addrlen;
	int sock, n;
	
	memset( &sock_addr, 0, sizeof( sock_addr ) );
	sock_addr.sun_family = AF_UNIX;
	memcpy( sock_addr.sun_path, socket_path.c_str(), socket_path.size() );
	addrlen = sizeof( sock_addr.sun_family ) + socket_path.size();
	
	sock = socket( AF_UNIX, SOCK_STREAM, 0 );
	
	if( sock < 0 ){
		return false;
		}
	
	n = connect( sock, (struct sockaddr *)&sock_addr, addrlen );
	if( n < 0 ){
		return false;
		}
	
	conn = Connection( sock );
	
	connected = true;
	return true; 
	}

ConnectorMessage SaigaConnector :: process( HttpMessage& request ){
	ConnectorMessage out, msg;
	std::vector< std::string > keys = request.get_headers();
	std::string line, tmp, key, value;
	uint64_t length, size;
	size_t pos = 0;
	
	msg.set( "method", request.get_method() );
	msg.set( "path", request.get_path() );
	msg.set( "body", request.get_body() );
	for( size_t i = 0 ; i < keys.size() ; ++i ){
		msg.set( "header." + keys[i], request.get_header_field( keys[i] ) );
		}
	
	conn.send( msg.serialize() );
	
	line.clear();
	while( line.size() < sizeof( uint64_t ) ){
		line += conn.receive( 1 );
		}
	length = ConnectorMessage :: decode_int( &line );
	line.clear();
	
	while( line.size() < length ){
		line += conn.receive( length - line.size() );
		}
	
	while( pos < length ){
		tmp = line.substr( pos, sizeof( uint64_t ) );
		pos += sizeof( uint64_t );
		size = ConnectorMessage :: decode_int( &tmp );
		key = line.substr( pos, size );
		pos += size;
		
		tmp = line.substr( pos, sizeof( uint64_t ) );
		pos += sizeof( uint64_t );
		size = ConnectorMessage :: decode_int( &tmp );
		value = line.substr( pos, size );
		pos += size;
		out.set( key, value );
		}
	
	
	return out;
	}

/*
int main(){
	HttpMessage msg;
	ConnectorMessage response;
	SaigaConnector connector( "/tmp/test1.saiga" );
	msg.set_method( "GET" );
	msg.set_path( "/index" );
	msg.set_header_field( "Host", "sai.ga" );
	
	connector.start();
	response = connector.process( msg );
	std::cout << "response.get('body') = '" << response.get( "body" ) << "'" << std::endl;
	return 0;
	}
*/

