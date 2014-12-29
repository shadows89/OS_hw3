#include "stdlib.h"
#include "stdio.h"
#include "mylist.h"
#include "lock.h"

struct node_t;
typedef struct node_t node_t;

struct node_t{
	rwlock_t next_lock;
	rwlock_t prev_lock;
	rwlock_t data_lock;

	node_t* prev;
	node_t* next;
	int index;
	void* data;
};

void nodeDestroy(node_t* node){
	if(node == NULL)
		return;
	if(node->next_lock != NULL)
		rwl_destroy(node->next_lock);
	if(node->prev_lock != NULL)
		rwl_destroy(node->prev_lock);
	if(node->data_lock != NULL)
		rwl_destroy(node->data_lock);
	free(node);
	node = NULL;
}

void nodeInit(int index,void* data,node_t* node){
	if(node == NULL)
		return;
	node->index = index;
	node->data = data;
	node->next = NULL;
	node->prev = NULL;
	node->next_lock = rwl_init();
	node->prev_lock = rwl_init();
	node->data_lock = rwl_init();
	if(node->next_lock == NULL || node->prev_lock == NULL ||
			node->data_lock == NULL){
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
	if(list == NULL || *list == NULL || **list==NULL)
		return;
	rwl_writelock((**list)->head_lock);
	linked_list_t* c_list = **list;
	node_t* current = c_list->head;
	node_t* next = NULL;
	while(current != NULL){
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
	linked_list_t* c_list = *list;
	node_t* current = c_list->head;
	if(current != NULL){
		rwl_writelock(current->next_lock);
		rwl_writeunlock((*list)->head_lock);
	}
	node_t* newNode = malloc(sizeof(node_t));
	nodeInit(index,data,newNode);
	if (newNode == NULL){
//		if(current != NULL)
//			rwl_writeunlock(current->next_lock);
		rwl_writeunlock((*list)->head_lock);
		return 1;
	}
	if (current == NULL) {	// Empty list
		c_list->head = newNode;
		rwl_writeunlock((*list)->head_lock);
		rwl_writelock((*list)->size_lock);
		c_list->numberOfElements++;
		rwl_writeunlock((*list)->size_lock);
		return 0;
	}
	if (index < current->index) { // Insert new head
		initNewHead(newNode, current);
		c_list->head = newNode;
		rwl_writelock((*list)->size_lock);
		c_list->numberOfElements++;
		rwl_writeunlock((*list)->size_lock);
		return 0;
	}
	while (current != NULL) {
		if(current->index == index){
			nodeDestroy(newNode);
			return 1;
		}
		if (current->next == NULL) {  //index > current->index
			initNewTail(newNode, current);
			break;
		}
		if (current->index < index && index < current->next->index) {
			initNewNode(newNode, current);
			break;
		}
		current = current->next;
	}
	rwl_writelock((*list)->size_lock);
	c_list->numberOfElements++;
	rwl_writeunlock((*list)->size_lock);
	return 0;
}

int list_remove(linked_list_t** list, int index) {
	if (list == NULL || *list == NULL)
		return 1;
	node_t* current = (*list)->head;
	while(current != NULL){
			if(current->index == index){
				if(current == (**list).head){
					if(current->next == NULL)
						(**list).head = NULL;
					else
						(**list).head = current->next;
					nodeDestroy(current);
					(*list)->numberOfElements--;
					return 0;
				}
				if(current->next != NULL)
					current->next->prev = current->prev;
				if(current->prev != NULL)
					current->prev->next = current->next;
				nodeDestroy(current);
				(*list)->numberOfElements--;
				return 0;
			}
			current = current->next;
		}
	return 1;
}

int list_contains(linked_list_t** list, int index) {
	if (list == NULL || *list == NULL)
		return 0;
	node_t* currnet = (*list)->head;
	while (currnet != NULL) {
		if (currnet->index == index)
			return 1;
		currnet = currnet->next;
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
	node_t* current = (*list)->head;
	while(current != NULL){
		if(current->index == index){
			current->data = data;
			return 0;
		}
		current = current->next;
	}
	return 1;
}

int list_node_compute(linked_list_t** list, int index,
		void *(*compute_func)(void *), void** result) {
	if (list == NULL || *list == NULL)
		return 1;
	node_t* current = (*list)->head;
	while (current != NULL) {
		if (current->index == index){
			*result = (*compute_func)(current->data);
			return 0;
		}
		current = current->next;
	}
	return 1;
}


