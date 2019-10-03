#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>

#define xassert(var)							\
({									\
	typeof (var) _var = var;					\
									\
									\
	if (!_var) {							\
		fprintf(stderr, "%s:%d: %s: Assertion `%s' failed.\n",	\
			__FILE__, __LINE__, __func__, #var);		\
		fflush(stderr);						\
		exit(1);						\
	}								\
									\
	_var;								\
})

#define list_init(p)							\
do {									\
	typeof (p) _p;							\
									\
									\
 	_p = malloc(sizeof (typeof (*_p)));				\
	memset(&_p->_list, 0, sizeof (struct list_node));		\
									\
	_p->_list.meta = malloc(sizeof (struct list_meta));		\
	_p->_list.meta->offset = offsetof(typeof (*_p), _list);		\
	_p->_list.meta->head = &_p->_list;				\
	_p->_list.meta->tail = &_p->_list;				\
	_p->_list.prev = NULL;						\
 	_p->_list.next = NULL;						\
									\
	p = _p;								\
} while (0)

#define list_alloc_at_end(p)						\
({									\
	typeof (p) newentry, _p = xassert(p);				\
									\
									\
	newentry = malloc(sizeof (typeof (*_p)));			\
	newentry->_list.meta = _p->_list.meta;				\
	newentry->_list.prev = _p->_list.meta->tail;			\
	newentry->_list.next = NULL;					\
	newentry->_list.meta->tail = &(newentry->_list);		\
	if (newentry->_list.prev != NULL)				\
		newentry->_list.prev->next = &(newentry->_list);	\
									\
	newentry;							\
})

#define list_delete(p)							\
({									\
	typeof (p) head = NULL, _p = xassert(p);			\
									\
									\
	if (_p->_list.prev == NULL)					\
		_p->_list.meta->head = _p->_list.next;			\
	else								\
		_p->_list.prev->next = _p->_list.next;			\
									\
	if (_p->_list.next == NULL)					\
		_p->_list.meta->tail = _p->_list.prev;			\
	else								\
		_p->_list.next->prev = _p->_list.prev;			\
									\
	if (_p->_list.meta->head != NULL)				\
		head = (typeof (_p)) ((int8_t*)_p->_list.meta->head - _p->_list.meta->offset);	\
									\
	if (_p->_list.prev == NULL && _p->_list.next == NULL)		\
		free(_p->_list.meta);					\
									\
	free(_p);							\
									\
	head;								\
})

#define list_get_prev(p)						\
({									\
	typeof (p) ret = NULL, _p = xassert(p);				\
									\
									\
	if (_p->_list.prev != NULL)					\
		ret = (void*)((int8_t*)_p->_list.prev - _p->_list.meta->offset); \
									\
	ret;								\
})

#define list_get_next(p)						\
({									\
	typeof (p) ret = NULL, _p = xassert(p);				\
									\
									\
	if (_p->_list.next != NULL)					\
		ret = (void*)((int8_t*)_p->_list.next - _p->_list.meta->offset); \
									\
	ret;								\
})

#define list_get_head(p)						\
({									\
	typeof (p) ret = NULL, _p = xassert(p);				\
									\
									\
	if (_p->_list.meta->head != NULL)				\
		ret = (void*)((int8_t*)_p->_list.meta->head - _p->_list.meta->offset);	\
									\
	ret;								\
})

#define list_get_tail(p)						\
({									\
	typeof (p) ret = NULL, _p = xassert(p);				\
									\
									\
	if (_p->_list.meta->tail != NULL)				\
		ret = (void*)((int8_t*)_p->_list.meta->tail - _p->_list.meta->offset);	\
									\
	ret;								\
})

#define list_foreach(head, entry) for (entry = head; entry != NULL; entry = list_get_next(entry))


struct list_meta {
	struct list_node *head, *tail;
	int offset;
};

struct list_node {
	struct list_meta *meta;
	struct list_node *prev, *next;
};

struct somestruct {
	int x, y;
	struct list_node _list;
};


int
main(void)
{
	struct somestruct *head, *node;

	list_init(head);
	head->x = 1;
	head->y = 2;

	node = list_alloc_at_end(head);
	node->x = 3;
	node->y = 4;

	node = list_alloc_at_end(head);
	node->x = 5;
	node->y = 6;

	node = list_delete(list_delete(list_delete(list_delete(list_get_prev(node)))));

	list_foreach(node, node)
		printf("%d:%d\n", node->x, node->y);

	return 0;
}
