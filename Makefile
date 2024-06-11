all:
	gcc plinko.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear