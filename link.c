#include <stdlib.h>
#include "headers/link.h"

Chain create_chain()
{
	Chain chain;

	chain.size  = 0;
	chain.capacity = sizeof(Link);
	chain.link = malloc(chain.capacity);

	return chain;
}

void draw_links(Chain* chain)
{
	for(int i = 0; i < chain->size; i++)
		DrawLine(chain->link[i].circle1->current_position.x, chain->link[i].circle1->current_position.y, chain->link[i].circle2->current_position.x, chain->link[i].circle2->current_position.y, LIGHTGRAY);
}

void resize_chain(Chain* chain)
{
	chain->capacity *= 2;
	chain->link = realloc(chain->link, chain->capacity);
}

void add_link(Chain* chain, Link link)
{
	if((chain->size * sizeof(Link)) == chain->capacity)
		resize_chain(chain);

	chain->link[chain->size++] = link;
}

void delete_link(Chain* chain, int position)
{
	for(int i = position; i < chain->size; i++)
		chain->link[i] = chain->link[i + 1];

	chain->size--;
}