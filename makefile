CFLAGS = -pthread -o lets-talk

default: lets-talk 
	gcc -g -o lets-talk lets-talk.c list.c -pthread 


lets-talk: lets-talk.o list.o
	gcc -g $(CFLAGS) lets-talk.o list.o 

lets-talk.o: lets-talk.c 
	gcc -g -c lets-talk.c

list.o: list.c
	gcc -g -c list.c


hello: client.c
	gcc -o client client.c

val: lets-talk
	valgrind --leak-check=full ./lets-talk 8000 localhost 8001



clean:
	-rm lets-talk
	-rm *.o