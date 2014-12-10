#ifndef __MYLIST_H_
#define __MYLIST_H_


struct linked_list_t;
typedef struct linked_list_t linked_list_t;


typedef struct op_t
{
	int index;
	void* data;
	enum {INSERT, REMOVE, CONTAINS, UPDATE, COMPUTE} op;
	void *(*compute_func) (void *);
	int result;
} op_t;

linked_list_t** list_alloc();

void list_free(linked_list_t** list);

int list_insert(linked_list_t** list, int index, void* data);

int list_remove(linked_list_t** list, int index);

int list_contains(linked_list_t** list, int index);

int list_size(linked_list_t** list);   /*changed ask in piazza*/

void list_batch(linked_list_t** list, int num_ops, op_t* ops);

int list_update_node(linked_list_t** list, int index, void* data);

int list_node_compute(linked_list_t** list, int index, 
						void *(*compute_func) (void *), void** result);


#endif /* __MYLIST_ */
