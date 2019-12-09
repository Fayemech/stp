PG=enc_dec_example

all: $(PG) client server as

$(PG): $(PG).c aes_func.o
	gcc -g -o $(PG) $(PG).c aes_func.o -lcrypto

client: client.c aes_func.o
	gcc -g -o client client.c aes_func.o -lcrypto

server: server.c aes_func.o
	gcc -g -o server server.c aes_func.o -lcrypto

as:     as.c aes_func.o
	gcc -g -o as as.c aes_func.o -lcrypto

aes_func.o: aes_func.c
	gcc -g -c aes_func.c

clean:
	rm client server enc_dec_example as aes_func.o 



