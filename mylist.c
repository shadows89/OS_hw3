#include "stdlib.h"
#include "stdio.h"
#include "mylist.h"
#include "lock.h"

typedef struct {
	linked_list_t** list;
	op_t* op;
} arg;

struct node_t;
typedef struct node_t node_t;

struct node_t {
	rwlock_t np_lock;
	rwlock_t data_lock;

	node_t* prev;
	node_t* next;
	int index;
	void* data;
};

void nodeDestroy(node_t* node) {
	if (node == NULL)
		return;
	if (node->np_lock != NULL)
		rwl_destroy(node->np_lock);
	if (node->data_lock != NULL)
		rwl_destroy(node->data_lock);
	free(node);
	node = NULL;
}

void nodeInit(int index, void* data, node_t* node) {
	if (node == NULL)
		return;
	node->index = index;
	node->data = data;
	node->next = NULL;
	node->prev = NULL;
	node->np_lock = rwl_init();
	node->data_lock = rwl_init();
	if (node->np_lock == NULL || node->data_lock == NULL) {
		nodeDestroy(node);
	}
}

struct linked_list_t {
	rwlock_t head_lock;
	rwlock_t size_lock;
	int valid;
	node_t* head;
	int numberOfElements;
};

void initNewTail(node_t* newTail, node_t* oldTail) {
	newTail->next = NULL;
	newTail->prev = oldTail;
	oldTail->next = newTail;
}

void initNewNode(node_t* newNode, node_t* current) {
	newNode->prev = current;
	newNode->next = current->next;
	current->next->prev = newNode;
	current->next = newNode;
}

void initNewHead(node_t* newHead, node_t* oldHead) {
	newHead->prev = NULL;
	newHead->next = oldHead;
	oldHead->prev = newHead;
}

void* aux_list_insert(void* args) {
	((arg*) args)->op->result = list_insert(((arg*) args)->list,
			((arg*) args)->op->index, ((arg*) args)->op->data);
	return NULL;
}

void* aux_list_remove(void* args) {
	((arg*) args)->op->result = list_remove(((arg*) args)->list,
			((arg*) args)->op->index);
	return NULL;
}

void* aux_list_contains(void* args) {
	((arg*) args)->op->result = list_contains(((arg*) args)->list,
			((arg*) args)->op->index);
	return NULL;
}

void* aux_list_update_node(void* args) {
	((arg*) args)->op->result = list_update_node(((arg*) args)->list,
			((arg*) args)->op->index, ((arg*) args)->op->data);
	return NULL;
}

void* aux_list_node_compute(void* args) {
	void* tmp;
	list_node_compute(((arg*) args)->list, ((arg*) args)->op->index,
			((arg*) args)->op->compute_func, &tmp);
	((arg*) args)->op->result = (int) tmp; // TODO check gavno!!!!
	return NULL;
}

linked_list_t** list_alloc() {
	linked_list_t** p_newList = malloc(sizeof(linked_list_t*));
	if (p_newList == NULL)
		return NULL;
	else {
		linked_list_t* newList = malloc(sizeof(linked_list_t));
		if (newList == NULL) {
			free(p_newList);
			return NULL;
		} else {
			newList->head_lock = rwl_init();
			newList->size_lock = rwl_init();
			newList->head = NULL;
			newList->numberOfElements = 0;
			newList->valid = 0;
			*p_newList = newList;
			return p_newList;
		}
	}
}

void list_free(linked_list_t*** list) {
	if (list == NULL || *list == NULL || **list == NULL)
		return;
	if ((**list)->valid !=0 )
		return;
	(**list)->valid = 1;
	linked_list_t** c_list = *list;
	*list = NULL;
	rwl_writelock((*c_list)->head_lock);
	node_t* current = (*c_list)->head;
	node_t* next = NULL;
	while (current != NULL) {
		next = current->next;
		nodeDestroy(current);
		current = next;
	}
	rwl_writelock((*c_list)->size_lock);
	(*c_list)->numberOfElements = 0;
	rwl_writeunlock((*c_list)->size_lock);
	rwl_writeunlock((*c_list)->head_lock);
	rwl_destroy((*c_list)->head_lock);
	rwl_destroy((*c_list)->size_lock);
	free(*c_list);
	free(c_list);
}

int list_insert(linked_list_t** list, int index, void* data) {
	if (list == NULL || *list == NULL)
		return 1;
	if ((*list)->valid !=0 )
		return 1;
	linked_list_t* c_list = *list;
	rwl_writelock((c_list)->head_lock);
	node_t* newNode = malloc(sizeof(node_t));
	nodeInit(index, data, newNode);
	if (newNode == NULL) { // If failed to init node (exit)
		rwl_writeunlock((c_list)->head_lock);
		return 1;
	}
	node_t* current = c_list->head;
	if (current == NULL) { // Insert new node to an empty list
		c_list->head = newNode;
		rwl_writelock((c_list)->size_lock);
		c_list->numberOfElements++;
		rwl_writeunlock((c_list)->size_lock);
		rwl_writeunlock((c_list)->head_lock);
		return 0;
	} else { // Insert new node to NOT an empty list
		rwl_writelock(current->np_lock);
	}
	if (index < current->index) { // Replace head (NOT an empty list)
		initNewHead(newNode, current);
		c_list->head = newNode;
		rwl_writelock((c_list)->size_lock);
		c_list->numberOfElements++;
		rwl_writeunlock((c_list)->size_lock);
		rwl_writeunlock((c_list)->head_lock);
		rwl_writeunlock(current->np_lock);
		return 0;
	}
	rwl_writeunlock((c_list)->head_lock);
	while (current != NULL) { // Iterating over NOT empty list
		if (current->index == index) { // Existing node with index
			rwl_writeunlock(current->np_lock);
			nodeDestroy(newNode);
			return 1;
		}
		if (current->next == NULL) { // End of list - index > current->index
			initNewTail(newNode, current);
			break;
		}
		rwl_writelock(current->next->np_lock); // We know that next exists
		if (current->index < index && index < current->next->index) { // Middle of list
			initNewNode(newNode, current);
			rwl_writeunlock(newNode->next->np_lock);
			break;
		}
		current = current->next;
		rwl_writeunlock(current->prev->np_lock);

	}
	rwl_writelock((c_list)->size_lock); // Added newNode to list
	c_list->numberOfElements++;
	rwl_writeunlock((c_list)->size_lock);
	rwl_writeunlock(current->np_lock);
	return 0;
}

int list_remove(linked_list_t** list, int index) {
	if (list == NULL || *list == NULL)
		return 1;
	if ((*list)->valid !=0 )
		return 1;
	linked_list_t* c_list = *list;
	rwl_writelock((c_list)->head_lock);
	node_t* current = (c_list)->head;
	if (current == NULL) { // Remove from empty list
		rwl_writeunlock((c_list)->head_lock);
		return 1;
	}
	rwl_writelock(current->np_lock); // Remove from NOT an empty list
	if (current->index == index) { // Node is head
		if (current->next == NULL) // Head is only node
			(*c_list).head = NULL;
		else {
			rwl_writelock(current->next->np_lock);
			(*c_list).head = current->next;
			(*c_list).head->prev = NULL;
			rwl_writeunlock(current->next->np_lock);
		}
		rwl_writelock((c_list)->size_lock); // Removed a node from list
		(c_list)->numberOfElements--;
		rwl_writeunlock((c_list)->size_lock);
		rwl_writeunlock(current->np_lock);
		rwl_writeunlock((c_list)->head_lock);
		nodeDestroy(current);
		return 0;
	}
	rwl_writeunlock((c_list)->head_lock);
	if(current->next == NULL){
		rwl_writeunlock(current->np_lock);
		return 1;
	}
	rwl_writelock(current->next->np_lock);
	current = current->next;
	while (current != NULL) {
		if (current->index == index) { // Found node with index BUT NOT HEAD
			//rwl_writelock(current->prev->np_lock); // TODO prev
			if (current->next != NULL) { // prev exist, next exist
				rwl_writelock(current->next->np_lock);
				current->next->prev = current->prev;
				current->prev->next = current->next;
				rwl_writeunlock(current->next->np_lock);
			} else { // prev exist, next does not exist
				current->prev->next = NULL;
			}
			rwl_writelock((c_list)->size_lock); // Removed a node from list
			(c_list)->numberOfElements--;
			rwl_writeunlock((c_list)->size_lock);
			rwl_writeunlock(current->prev->np_lock);
			rwl_writeunlock(current->np_lock);
			nodeDestroy(current);
			return 0;
		}
		if (current->next == NULL) { // current->index != index
			rwl_writeunlock(current->prev->np_lock);
			rwl_writeunlock(current->np_lock);
			return 1;
		}
		rwl_writelock(current->next->np_lock); // current->next != NULL
		rwl_writeunlock(current->prev->np_lock);
		current = current->next;
		//rwl_writeunlock(current->prev->np_lock);
	}
	return 1;
}

int list_contains(linked_list_t** list, int index) {
	if (list == NULL || *list == NULL)
		return 0;
	if ((*list)->valid !=0 )
		return 0;
	linked_list_t* c_list = *list;
	rwl_readlock((c_list)->head_lock);
	node_t* current = (c_list)->head;
	if (current == NULL) { // Empty list
		rwl_readunlock((c_list)->head_lock);
		return 0;
	}
	rwl_readlock(current->np_lock);
	rwl_readunlock((c_list)->head_lock);
	while (current != NULL) { // Iterating over list
		if (current->index == index) { // Found index
			rwl_readunlock(current->np_lock);
			return 1;
		}
		if (current->next == NULL) { // End of list - index not found
			rwl_readunlock(current->np_lock);
			return 0;
		}
		rwl_readlock(current->next->np_lock);
		current = current->next;
		rwl_readunlock(current->prev->np_lock);
	}
	return 0;
}

int list_size(linked_list_t** list) {
	if (list == NULL || *list == NULL)
		return -1;
	if ((*list)->valid !=0 )
		return -1;
	linked_list_t* c_list = *list;
	rwl_readlock((c_list)->head_lock);
	int tmp_size = 0;
	rwl_writelock((c_list)->size_lock);
	tmp_size = (c_list)->numberOfElements;
	rwl_writeunlock((c_list)->size_lock);
	rwl_readunlock((c_list)->head_lock);
	return tmp_size;
}

void list_batch(linked_list_t** list, int num_ops, op_t* ops) {
	if (list == NULL || *list == NULL || ops == NULL)
		return;
	if ((*list)->valid !=0 )
		return;
	int i = 0;
//	pthread_t* threads = malloc(sizeof(pthread_t) * num_ops);
//	if (threads == NULL)
//		return;
	pthread_t threads[num_ops];
	arg args[num_ops];
	for (i = 0; i < num_ops; i++) {
		args[i].list = list;
		args[i].op = ops + i;
		switch ((ops[i]).op) {
		case INSERT:
			if ((pthread_create(&threads[i], NULL, aux_list_insert, args + i)) != 0)
				return; // TODO if ( != 0) {return}
			break;
		case REMOVE:
			if ((pthread_create(&threads[i], NULL, aux_list_remove, args + i)) != 0)
				return;
			break;
		case CONTAINS:
			if ((pthread_create(&threads[i], NULL, aux_list_contains, args + i)) != 0)
				return;
			break;
		case UPDATE:
			if ((pthread_create(&threads[i], NULL, aux_list_update_node, args + i)) != 0)
				return;
			break;
		case COMPUTE:
			if ((pthread_create(&threads[i], NULL, aux_list_node_compute, args + i)) != 0)
				return;
			break;
		}
	}
	for (i = 0; i < num_ops; i++) {
		pthread_join(threads[i], NULL);
	}
//	free(threads);
}

int list_update_node(linked_list_t** list, int index, void* data) {
	if (list == NULL || *list == NULL || data == NULL)
		return 1;
	if ((*list)->valid !=0 )
		return 1;
	linked_list_t* c_list = *list;
	rwl_readlock((c_list)->head_lock);
	node_t* current = (c_list)->head;
	if (current == NULL) { // Empty list
		rwl_readunlock((c_list)->head_lock);
		return 1;
	}
	rwl_readlock(current->np_lock);
	rwl_readunlock((c_list)->head_lock);
	while (current != NULL) {
		if (current->index == index) {
			rwl_writelock(current->data_lock);
			current->data = data;
			rwl_writeunlock(current->data_lock);
			rwl_readunlock(current->np_lock);
			return 0;
		}
		if (current->next == NULL) {
			rwl_readunlock(current->np_lock);
			return 1;
		}
		rwl_readlock(current->next->np_lock);
		current = current->next;
		rwl_readunlock(current->prev->np_lock);
	}
	return 1;
}

int list_node_compute(linked_list_t** list, int index,
		void *(*compute_func)(void *), void** result) {
	if (list == NULL || *list == NULL || compute_func == NULL || result == NULL)
		return 1;
	if ((*list)->valid !=0 )
		return 1;
	linked_list_t* c_list = *list;
	rwl_readlock((c_list)->head_lock);
	node_t* current = (c_list)->head;
	if (current == NULL) { // Empty list
		rwl_readunlock((c_list)->head_lock);
		return 1;
	}
	rwl_readlock(current->np_lock);
	rwl_readunlock((c_list)->head_lock);
	while (current != NULL) {
		if (current->index == index) {
			rwl_readlock(current->data_lock);
			*result = (*compute_func)(current->data);
			rwl_readunlock(current->data_lock);
			rwl_readunlock(current->np_lock);
			return 0;
		}
		if (current->next == NULL) {
			rwl_readunlock(current->np_lock);
			return 1;
		}
		rwl_readlock(current->next->np_lock);
		current = current->next;
		rwl_readunlock(current->prev->np_lock);
	}
	return 1;
}

