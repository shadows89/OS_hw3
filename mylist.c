#include "stdlib.h"
#include "stdio.h"
#include "mylist.h"
#include "node.h"

struct linked_list_t {
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

//void removeHead(node_t* head) {
//	linked_list_t* tmp = head;
//	head = head->next;
//	if (head != NULL)
//		head->prev = NULL;
//	free(tmp);
//}

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
	linked_list_t c_list = ***list;
	node_t* current = c_list.head;
	node_t* next = NULL;
	while(current != NULL){
		next = current->next;
		free(current);
		current = next;
	}
	free(*list);
}

int list_insert(linked_list_t** list, int index, void* data) {
	if (list == NULL || *list == NULL)
		return 1;
	linked_list_t* c_list = *list;
	node_t* tmp = c_list->head;
	node_t* newNode = malloc(sizeof(node_t));
	if (newNode == NULL)
		return 1;
	newNode->index = index;
	newNode->data = data;
	newNode->next = NULL;
	newNode->prev = NULL;
	if (tmp == NULL) {
		c_list->head = newNode;
		c_list->numberOfElements++;
		return 0;
	}
	if (tmp->index > index) {
		initNewHead(newNode, tmp);
		c_list->head = newNode;
		c_list->numberOfElements++;
		return 0;
	}
	while (tmp != NULL) {
		if(tmp->index == index){
			free(newNode);
			return 1;
		}
		if (tmp->next == NULL) {
			initNewTail(newNode, tmp);
			break;
		}
		if (tmp->index < index && tmp->next->index > index) {
			initNewNode(newNode, tmp);
			break;
		}
		tmp = tmp->next;
	}
	c_list->numberOfElements++;
	return 0;
}

int list_remove(linked_list_t** list, int index) {
	if (list == NULL)
		return 1;
	if (*list == NULL)
		return 1;
	if (!list_contains(list, index))
		return 1;
	node_t* tmp = (**list).head;
	while(tmp != NULL){
			if(tmp->index == index){
				if(tmp == (**list).head){
					if(tmp->next == NULL)
						(**list).head = NULL;
					else
						(**list).head = tmp->next;
					free(tmp);
					(**list).numberOfElements--;
					return 0;
				}
				if(tmp->next != NULL)
					tmp->next->prev = tmp->prev;
				if(tmp->prev != NULL)
					tmp->prev->next = tmp->next;
				free(tmp);
				break;
			}
			tmp = tmp->next;
		}
	(**list).numberOfElements--;
	return 0;
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

