all: client nameserver storageserver

clean:
	rm s c n

nameserver: nameserver.c
	gcc nameserver.c -o n -pthread

storageserver: storageserver.c
	gcc storageserver.c -o s -pthread

client: client.c
	gcc client.c -o c -pthread