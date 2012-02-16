CC = g++
FLAGS = -O3 -Wall -Wextra -pedantic -g -std=c++0x

LIBS = libcompression.o libconnection.o libhttpmessage.o libutils.o libhttpclient.o libbase64.o libherd.o libsaigaconnector.o -lz -pthread

all: main libcompression libconnection libhttpmessage libutils libhttpclient libbase64 libherd libsaigaconnector

main: libcompression libconnection libhttpclient libhttpmessage libutils libbase64 libherd libsaigaconnector
	$(CC) $(FLAGS) saiga-client.cpp -o saiga-client $(LIBS)
	$(CC) $(FLAGS) saiga-server.cpp -o saiga-server $(LIBS)

libcompression:
	$(CC) $(FLAGS) -c libCompression.cpp -o libcompression.o

libconnection:
	$(CC) $(FLAGS) -c libConnection.cpp -o libconnection.o

libsslconnection:
	$(CC) $(FLAGS) -c libSSLConnection.cpp -o libsslconnection.o

libhttpmessage:
	$(CC) $(FLAGS) -c libHttpMessage.cpp -o libhttpmessage.o

libutils:
	$(CC) $(FLAGS) -c libUtils.cpp -o libutils.o

libhttpclient:
	$(CC) $(FLAGS) -c libHttpClient.cpp -o libhttpclient.o

libbase64:
	$(CC) $(FLAGS) -c libBase64.cpp -o libbase64.o

libherd:
	$(CC) $(FLAGS) -c libHerd.cpp -o libherd.o

libsaigaconnector:
	$(CC) $(FLAGS) -c libSaigaConnector.cpp -o libsaigaconnector.o

#lauma: libcompression libconnection libhttpmessage libhttpclient libutils libbase64
#	$(CC) $(FLAGS) libHerd.cpp -o lauma $(LIBS)

#conn: libcompression libconnection libhttpclient libhttpmessage libutils libbase64 libherd
#	$(CC) $(FLAGS) libSaigaConnector.cpp -o tmp $(LIBS)

clean:
	rm *.o saiga-client saiga-server
