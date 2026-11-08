all: awale


awale: main.o plateau.o joueur.o
	gcc main.o plateau.o joueur.o -o awale

main.o: main.c
	gcc -c main.c -o main.o

plateau.o: plateau.c
	gcc -c plateau.c -o plateau.o

joueur.o: joueur.c
	gcc -c joueur.c -o joueur.o


clean:
	rm -rf *.o awale

