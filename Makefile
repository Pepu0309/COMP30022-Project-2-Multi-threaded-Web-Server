server: server.o
	gcc -Wall -o server server.o -g

server.o:
	gcc -Wall -o server.o -c server.c -g

clean:
	rm -f *.o server