all:
	gcc main.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear