#!/usr/bin/env python


class HtmlDocument( object ):

	TAGS = ['html', 'head', 'body', 'div', 'link', 'script', 
	'title', 'meta', 'p', 'a', 'span', 'h1', 'h2', 'h3', 'h4', 'h5', 'h6',
	'ul', 'li', 'ol', 'pre', 'code', 'footer', 'article', 'canvas', 'img', 'text' ]

	def __init__( self ):
		self.stack = []
		self.ctx_stack = []
		self.ctx = []
		self.name = None
		self.called_since = False

	def _enter_ctx( self, tag, attrs ):
		self.ctx.append( {'tag':tag, 'attrs':attrs} )
		return self

	def __enter__( self ):
		if self.called_since:
			entry = self.stack.pop()
			self._enter_ctx( entry['tag'], entry['attrs'] )
			self.called_since = False
		else:
			self._enter_ctx( self.name, {} )

		self.ctx_stack.append( self.stack )
		self.stack = []

	def __exit__( self, _type, value, tb ):
		if _type is not None:
			print "_type", _type
			return False

		entry = self.ctx.pop()
		entry['content'] = self.stack

		self.stack = self.ctx_stack.pop()
		self.stack.append( entry )

	def __getattribute__( self, name ):
		if name in HtmlDocument.TAGS:
			self.name = name
			self.called_since = False
			return self
		else:
			return object.__getattribute__( self, name )

	def __call__( self, *args, **kwargs ):
		self.called_since = True
		content = ""
		attrs = {}
		if len( args ) > 0:
			content = args[0]

		attrs = kwargs

		self.stack.append( {'tag':self.name, 'attrs': attrs, 'content': content } )
		return self

	def _serialize( self, tag, indent_level = 0 ):
		if not isinstance( tag, dict ):
			return "\t" * indent_level  + tag + "\n"

		out = ""
		if tag['tag'] == 'text':
			return "\t" * indent_level + tag['content' ] + '\n'
		else:
			sattrs = ""

			if len( tag['attrs'] ) > 0:
				sattrs = " "

			for (key, value) in tag['attrs'].items():

				if key == '_class':
					key = 'class'

				sattrs += '%s="%s" '%( key, value )

			if len( tag['content'] ) > 0:
				out =  "\t" * indent_level + '<%s%s>\n'%( tag['tag'], sattrs )

				if isinstance( tag['content'], list ):
					for entry in tag['content']:
						out += self._serialize( entry, indent_level + 1 )
				else:
					out += self._serialize( tag['content'], indent_level + 1 )

				out += "\t" * indent_level + '</%s>\n' % tag['tag']
			else:
				out += "\t" * indent_level + '<%s%s/>\n'%( tag['tag'], sattrs )

		return out

	def __str__( self ):
		out = ""
		for entry in self.stack:
			out += self._serialize( entry )
		return out




if __name__ == '__main__':

	doc = HtmlDocument()

	with doc.html:
		with doc.head:
			doc.title( 'Moi kaikki' )
			doc.link( src="style.css", type="stylesheet" )

		with doc.body( onload="init()" ):
			doc.div( ':D', id="main", _class="wrapper" )


	print doc
