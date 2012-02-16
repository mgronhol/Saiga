#!/usr/bin/env python

import libSaigaConnector as libSaiga

import libHtml as HTML


conn = libSaiga.SaigaConnector( "/tmp/default.saiga" )
conn.start()
print "Handler running..."

counter = 0
try:
	while True:
		msg = conn.get_message()
		out = {}
		out['status'] = "ok"
		out['cache'] = "10" # 10 ms cache time
		
		doc = HTML.HtmlDocument()
		with doc.html:
			with doc.head():
				doc.title( "Saiga server python connector v.0.0.1" )
			with doc.body():
				doc.h1( "Hello World!" )
				doc.div( "This is #%i request to this handler."%counter )
				doc.div( "Path of this request is '%s'."%msg['path'] )
		
		out['body'] = str( doc )
		conn.send_message( out )
		counter += 1

except KeyboardInterrupt:
	print ""
	pass

conn.disconnect()
print "Done."
