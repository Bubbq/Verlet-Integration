all:
	gcc rps.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear
