all:
	gcc circle_playground.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear
