all:
	gcc link.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear
