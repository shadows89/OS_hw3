#include "stdlib.h"
#include "stdio.h"
#include "mylist.h"
#include "lock.h"

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
			*p_newList = newList;
			return p_newList;
		}
	}
}

void list_free(linked_list_t*** list) {
	if (list == NULL || *list == NULL || **list == NULL)
		return;
	rwl_writelock((**list)->head_lock);
	linked_list_t* c_list = **list;
	node_t* current = c_list->head;
	node_t* next = NULL;
	while (current != NULL) {
		next = current->next;
		nodeDestroy(current);
		current = next;
	}
	rwl_writelock(c_list->size_lock);
	c_list->numberOfElements = 0;
	rwl_writeunlock(c_list->size_lock);
	rwl_writeunlock((**list)->head_lock);
	rwl_destroy(c_list->head_lock);
	rwl_destroy(c_list->size_lock);
	free(**list);
	free(*list);
}

int list_insert(linked_list_t** list, int index, void* data) {
	if (list == NULL || *list == NULL)
		return 1;
	rwl_writelock((*list)->head_lock);
	node_t* newNode = malloc(sizeof(node_t));
	nodeInit(index, data, newNode);
	if (newNode == NULL) { // If failed to init node (exit)
		rwl_writeunlock((*list)->head_lock);
		return 1;
	}
	linked_list_t* c_list = *list;
	node_t* current = c_list->head;
	if (current == NULL) { // Insert new node to an empty list
		c_list->head = newNode;
		rwl_writeunlock((*list)->head_lock);
		rwl_writelock((*list)->size_lock);
		c_list->numberOfElements++;
		rwl_writeunlock((*list)->size_lock);
		return 0;
	} else { // Insert new node to NOT an empty list
		rwl_writelock(current->np_lock);
	}
	if (index < current->index) { // Replace head (NOT an empty list)
		initNewHead(newNode, current);
		c_list->head = newNode;
		rwl_writeunlock((*list)->head_lock);
		rwl_writeunlock(current->np_lock);
		rwl_writelock((*list)->size_lock);
		c_list->numberOfElements++;
		rwl_writeunlock((*list)->size_lock);
		return 0;
	}
	rwl_writeunlock((*list)->head_lock);
	while (current != NULL) { // Iterating over NOT empty list
		if (current->index == index) { // Existing node with index
			rwl_writeunlock(current->np_lock);
			nodeDestroy(newNode);
			return 1;
		}
		if (current->next == NULL) { // End of list - index > current->index
			initNewTail(newNode, current);
			rwl_writeunlock(current->np_lock);
			break;
		}
		rwl_writelock(current->next->np_lock); // We know that next exists
		if (current->index < index && index < current->next->index) { // Middle of list
			initNewNode(newNode, current);
			rwl_writeunlock(newNode->next->np_lock);
			rwl_writeunlock(current->np_lock);
			break;
		}
		current = current->next;
		rwl_writeunlock(current->prev->np_lock);

	}
	rwl_writelock((*list)->size_lock); // Added newNode to list
	c_list->numberOfElements++;
	rwl_writeunlock((*list)->size_lock);
	return 0;
}

int list_remove(linked_list_t** list, int index) {
	if (list == NULL || *list == NULL)
		return 1;
	rwl_writelock((*list)->head_lock);
	node_t* current = (*list)->head;
	if (current == NULL) { // Remove from empty list
		rwl_writeunlock((*list)->head_lock);
		return 1;
	}
	rwl_writelock(current->np_lock); // Remove from NOT an empty list
	if (current->index == index) { // Node is head
		if (current->next == NULL) // Head is only node
			(**list).head = NULL;
		else {
			rwl_writelock(current->next->np_lock);
			(**list).head = current->next;
			(**list).head->prev = NULL;
			rwl_writeunlock(current->next->np_lock);
		}
		rwl_writeunlock(current->np_lock);
		rwl_writeunlock((*list)->head_lock);
		nodeDestroy(current);
		rwl_writelock((*list)->size_lock); // Removed a node from list
		(*list)->numberOfElements--;
		rwl_writeunlock((*list)->size_lock);
		return 0;
	}
	rwl_writeunlock((*list)->head_lock);
	while (current != NULL) {
		if (current->index == index) { // Found node with index BUT NOT HEAD
			rwl_writelock(current->prev->np_lock);
			if (current->next != NULL) { // prev exist, next exist
				rwl_writelock(current->next->np_lock);
				current->next->prev = current->prev;
				current->prev->next = current->next;
				rwl_writeunlock(current->next->np_lock);
			} else { // prev exist, next does not exist
				current->prev->next = NULL;
			}
			rwl_writeunlock(current->prev->np_lock);
			rwl_writeunlock(current->np_lock);
			nodeDestroy(current);
			rwl_writelock((*list)->size_lock); // Removed a node from list
			(*list)->numberOfElements--;
			rwl_writeunlock((*list)->size_lock);
			return 0;
		}
		if (current->next == NULL) { // current->index != index
			rwl_writeunlock(current->np_lock);
			return 1;
		}
		rwl_writelock(current->next->np_lock); // current->next != NULL
		current = current->next;
		rwl_writeunlock(current->prev->np_lock);
	}
	return 1;
}

int list_contains(linked_list_t** list, int index) {
	if (list == NULL || *list == NULL)
		return 0;
	rwl_readlock((*list)->head_lock);
	node_t* current = (*list)->head;
	if (current == NULL) { // Empty list
		rwl_readunlock((*list)->head_lock);
		return 0;
	}
	rwl_readlock(current->np_lock);
	rwl_readunlock((*list)->head_lock);
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
	return (**list).numberOfElements;
}

void list_batch(linked_list_t** list, int num_ops, op_t* ops) {

}

int list_update_node(linked_list_t** list, int index, void* data) {
	if (list == NULL || *list == NULL || data == NULL)
		return 1;
	rwl_readlock((*list)->head_lock);
	node_t* current = (*list)->head;
	if (current == NULL) { // Empty list
		rwl_readunlock((*list)->head_lock);
		return 1;
	}
	rwl_readlock(current->np_lock);
	rwl_readunlock((*list)->head_lock);
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
	rwl_readlock((*list)->head_lock);
	node_t* current = (*list)->head;
	if (current == NULL) { // Empty list
		rwl_readunlock((*list)->head_lock);
		return 1;
	}
	rwl_readlock(current->np_lock);
	rwl_readunlock((*list)->head_lock);
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

