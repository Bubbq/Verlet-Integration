all:
	gcc circle.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear