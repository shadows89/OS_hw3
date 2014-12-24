#include "stdlib.h"
#include "stdio.h"
#include "mylist.h"
//#include "node.h"

struct linked_list_t {
	int index;
	linked_list_t* next;
	linked_list_t* prev;
	void* data;
};

void initNewTail(linked_list_t* newTail, linked_list_t* oldTail) {
	newTail->next = NULL;
	newTail->prev = oldTail;
	oldTail->next = newTail;
}

void initNewNode(linked_list_t* newNode, linked_list_t* current) {
	newNode->prev = current;
	newNode->next = current->next;
	current->next->prev = newNode;
	current->next = newNode;
}

void initNewHead(linked_list_t* newHead, linked_list_t* oldHead) {
	newHead->prev = NULL;
	newHead->next = oldHead;
	oldHead->prev = newHead;
}

//void removeHead(linked_list_t* head,linked_list_t* newHead){
//	linked_list_t* tmp = head;
//	head = head->next;
//	if(head != NULL)
//		head->prev = NULL;
//	free(tmp);
//}
linked_list_t** list_alloc() {
	linked_list_t** p_newList = malloc(sizeof(linked_list_t*));
	*p_newList = NULL;
	return p_newList;
}

void list_free(linked_list_t** list) {
	if (list == NULL)
		return;
	linked_list_t* current = *list;
	linked_list_t* tmp = NULL;
	while (current != NULL) {
		tmp = current->next;
		free(current);
		current = tmp->next;
	}
	free(list);
}

int list_insert(linked_list_t** list, int index, void* data) {
	if (list == NULL)
		return 1;
	if (list_contains(list, index))
		return 1;
	linked_list_t* newNode = NULL;
	linked_list_t* current = *list;
	newNode = malloc(sizeof(linked_list_t));
	if (newNode == NULL)
		return 1;
	newNode->index = index;
	newNode->data = data;
	newNode->next = NULL;
	newNode->prev = NULL;
	if(current == NULL){
		*list = newNode;
		return 0;
	}
	if (current->index > index){
		initNewHead(newNode, *list);
		*list = newNode;
		return 0;
	}
	while (current != NULL) {
		if (current->next == NULL) {
			initNewTail(newNode, current);
			break;
		}
		if (current->index < index && current->next->index > index) {
			initNewNode(newNode, current);
		}
	}
	return 0;
}

int list_remove(linked_list_t** list, int index) {
	if (list == NULL)
		return 1;
	if (!list_contains(list, index))
		return 1;
	linked_list_t* current = *list;
	while(current != NULL){
		if(current->index == index){
			if(current == *list){
				if(current->next == NULL)
					*list = NULL;
				else
					*list = current->next;
				free(current);
				return 0;
			}
			if(current->next != NULL)
				current->next->prev = current->prev;
			if(current->prev != NULL)
				current->prev->next = current->next;
			free(current);
			break;
		}
		current = current->next;
	}
	return 0;
}

int list_contains(linked_list_t** list, int index) {
	if (list == NULL)
		return 0;
	linked_list_t* current = *list;
	while (current != NULL) {
		if (current->index == index)
			return 1;
		current = current->next;
	}
	return 0;
}

int list_size(linked_list_t** list) {
	if (list == NULL)
		return 0;
	int count;
	linked_list_t* current = *list;
	while (current != NULL) {
		count++;
		current = current->next;
	}
	return count;
}

void list_batch(linked_list_t** list, int num_ops, op_t* ops) {
//MIND FUCK
}

int list_update_node(linked_list_t** list, int index, void* data) {
	if (list == NULL)
		return 1;
	if (!list_contains(list, index))
		return 1;
	linked_list_t* current = *list;
	while (current != NULL) {
		if (current->index == index) {
			current->data = data;
			break;
		}
		current = current->next;
	}
	return 0;
}

int list_node_compute(linked_list_t** list, int index,
		void *(*compute_func)(void *), void** result) {
	if (list == NULL)
		return 1;
	if (!list_contains(list, index))
		return 1;
	linked_list_t* current = *list;
	while (current != NULL) {
		if (current->index == index) {
			*result = (*compute_func)(current->data);
			break;
		}
		current = current->next;
	}
	return 0;
}

