#!/usr/bin/env python

import libSaigaConnector as libSaiga

import libHtml as HTML

import threading

class HandlerThread( threading.Thread ):
	def __init__( self, conn, msg, response, handler ):
		threading.Thread.__init__( self )
		self.conn = conn
		self.msg = msg
		self.response = response
		self.handler = handler
	
	def run( self ):
		response = self.handler( self.msg, self.response )
		self.conn.send_message( response )


def default_page( msg, response ):
	global counter
	response.set( "status", "ok" )
	response.set( "cache", "10" ) # 10 ms cache time
	
	doc = HTML.HtmlDocument()
	with doc.html:
		with doc.head():
			doc.title( "Saiga server python connector v.0.0.1" )
		with doc.body():
			doc.h1( "Hello World!" )
			doc.div( "This is #%i request to this handler."%counter )
			doc.div( "Path of this request is '%s'."%msg.get('path' ) )
	
	response.set( "body", str( doc ) )
	return response



conn = libSaiga.SaigaConnector( "/tmp/default.saiga" )
conn.start()
print "Handler running..."

counter = 0
try:
	while True:
		msg, response = conn.get_message()
		#conn.send_message( response )
		thread = HandlerThread( conn, msg, response, default_page )
		thread.setDaemon( True )
		thread.start()
		counter += 1

except KeyboardInterrupt:
	print ""
	pass

conn.disconnect()
print "Done."
