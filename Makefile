all:
	gcc verlet.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear
