server: server.o parse.o respond.o
	gcc -Wall -o server server.o -g parse.o respond.o -lpthread

server.o:
	gcc -Wall -o server.o -c server.c -g

parse.o:
	gcc -Wall -o parse.o -c parse.c -g

respond.o:
	gcc -Wall -o respond.o -c respond.c -g

clean:
	rm -f *.o server