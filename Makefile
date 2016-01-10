build:
	gcc -g -Wall -o client client.c
	gcc -g -Wall -o server server.c

clean:
	rm -f client server
