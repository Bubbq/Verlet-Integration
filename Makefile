all:
	gcc cloth.c circle.c link.c physics.c spatial_partition.c timer.c -o run -lraylib -lm -Wall
clean:
	rm run
	clear