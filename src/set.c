

#include <stdlib.h>
#include <stdarg.h>
#include "set.h"

symbol_set unite_set(symbol_set s1, symbol_set s2)
{
	symbol_set s;
	set_node* p;
	
	s1 = s1->next;
	s2 = s2->next;
	
	s = p = (set_node*) malloc(sizeof(set_node));
	while (s1 && s2)
	{
		p->next = (set_node*) malloc(sizeof(set_node));
		p = p->next;
		if (s1->elem < s2->elem)
		{
			p->elem = s1->elem;
			s1 = s1->next;
		}
		else
		{
			p->elem = s2->elem;
			s2 = s2->next;
		}
	}

	while (s1)
	{
		p->next = (set_node*) malloc(sizeof(set_node));
		p = p->next;
		p->elem = s1->elem;
		s1 = s1->next;
		
	}

	while (s2)
	{
		p->next = (set_node*) malloc(sizeof(set_node));
		p = p->next;
		p->elem = s2->elem;
		s2 = s2->next;
	}

	p->next = NULL;

	return s;
}

/**
 * @brief Private function, insert an element into a symbol set.
 *
 * @param s Symbol set to be inserted in.
 * @param elem Element to be inserted.
 */
void set_insert(symbol_set s, int elem)
{
	set_node* p = s;
	set_node* q;

	while (p->next && p->next->elem < elem)
	{
		p = p->next;
	}
	
	q = (set_node*) malloc(sizeof(set_node));
	q->elem = elem;
	q->next = p->next;
	p->next = q;
}

symbol_set create_set(int data, ...)
{
	va_list list;
	symbol_set s;

	s = (set_node*) malloc(sizeof(set_node));
	s->next = NULL;

	va_start(list, data);
	while (data)
	{
		set_insert(s, data);
		data = va_arg(list, int);
	}
	va_end(list);
	return s;
}

void destroy_set(symbol_set s)
{
	set_node* p;

	while (s)
	{
		p = s;
		p->elem = -1000000;
		s = s->next;
		free(p);
	}
}

int in_set(int elem, symbol_set s)
{
	s = s->next;
	while (s && s->elem < elem)
		s = s->next;

	if (s && s->elem == elem)
		return 1;
	else
		return 0;
}