all:

	gcc -pedantic -O2 -Wall initial.c -o initial
	gcc -pedantic -O2 -Wall client.c -o client
	gcc -pedantic -O2 -Wall vendeur.c -o vendeur
	gcc -pedantic -O2 -Wall caissier.c -o caissier
	gcc -pedantic -O2 -Wall monitoring.c -o monitoring

clean:

	rm initial
	rm client
	rm vendeur
	rm caissier
	rm monitoring