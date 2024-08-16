all:
	gcc playground.c circle.c link.c physics.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear