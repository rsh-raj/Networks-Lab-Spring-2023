libmsocket.a : mysocket.c mysocket.h
	gcc -Wall -c mysocket.c
	ar rcs libmsocket.a mysocket.o
	gcc  server.c -o server -lmsocket
	gcc  client.c -o client -lmsocket
install: libmsocket.a
	sudo cp mysocket.h /usr/local/include 
	sudo cp mysocket.h /usr/include
	sudo cp libmsocket.a /usr/lib
clean:
	rm libmsocket.a
	rm server
	rm client
	rm mysocket.o


