all:
	gcc cloth.c physics.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear
