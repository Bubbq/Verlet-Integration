all:
	gcc cloth.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear
