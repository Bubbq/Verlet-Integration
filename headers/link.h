#ifndef LINK_H
#define LINK_H

#include "circle.h"

typedef struct
{
    VerletCirlce* circle1;
    VerletCirlce* circle2;
    float target_distance;
} Link;

typedef struct
{
	int size;
	Link* link;
	size_t capacity;
} Chain;

Chain create_chain();
void draw_links(Chain* chain);
void resize_chain(Chain* chain);
void add_link(Chain* chain, Link link);
void delete_link(Chain* chain, int position);

#endif