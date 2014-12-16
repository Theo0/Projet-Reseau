all: serveur client

sock.o : sock.cc
	g++ -c sock.cc -o sock.o

sockdist.o : sockdist.cc
	g++ -c sockdist.cc -o sockdist.o

client : client.cpp sock.o sockdist.o
	g++ client.cpp sock.o sockdist.o -w -fpermissive -lpthread -o client

serveur : serveur.cpp sock.o sockdist.o
	g++ serveur.cpp sock.o sockdist.o -w -fpermissive -lpthread -o serveur
