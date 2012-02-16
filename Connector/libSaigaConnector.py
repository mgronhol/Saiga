#!/usr/bin/env python

import socket, sys, os
import struct


class SaigaConnector( object ):
	def __init__( self, path ):
		self.path = path
		self.sock = None
		self.conn = None
	
	def start( self ):
		self.sock = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
		try:
			os.remove( self.path )
		except OSError:
			pass
		
		self.sock.bind( self.path )
		self.sock.listen( 255 )
	
	def get_message( self ):
		self.conn, addr = self.sock.accept()
		out = {}
		msg_len = ""
		lenSize = struct.calcsize( "Q" )
		while len(msg_len) < lenSize:
			msg_len += self.conn.recv( 1 )
		
		length = struct.unpack( "Q", msg_len )[0]
		content = ""
		while len(content) < length:
			content += self.conn.recv( length - len( content ) )
		
		pos = 0
		while pos < length:
			L = content[pos:pos + lenSize]
			strlen = struct.unpack( "Q", L )[0]
			pos += lenSize
			key = content[pos:pos+strlen]
			pos += strlen
			
			L = content[pos:pos + lenSize]
			strlen = struct.unpack( "Q", L )[0]
			pos += lenSize
			value = content[pos:pos+strlen]
			pos += strlen
			out[key] = value
		return out

	def send_message( self, msg ):
		out = ""
		for (key,value) in msg.items():
			lkey = struct.pack( "Q", len( key ) )
			lvalue = struct.pack( "Q", len( value ) )
			out += lkey + key + lvalue + value
		out = struct.pack( "Q", len( out ) ) + out
		
		L = 0
		while L < len( out ):
			n = self.conn.send( out[L:] )
			if n > 0:
				L += n
			else:
				break
		self.conn.close()
		self.conn = None

	def disconnect( self ):
		self.sock.close()

#conn = SaigaConnector( "/tmp/default.saiga" )
#
#conn.start()
#try:
#	while True:
#		msg = conn.get_message()
#		#print "!"
#		conn.send_message( {'body': "Hola mundi from python!", 'status':'ok', 'cache': '0' } )
#except KeyboardInterrupt:
#	pass
#	
#conn.disconnect()
