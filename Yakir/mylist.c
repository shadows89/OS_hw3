#define err(error) printf("%s\n",#error);
#include "lock.h"
#include "mylist.h"
#include <stdio.h>
typedef struct node_t {

	rwlock_t* rwlock_next;
	rwlock_t* rwlock_prev;
	rwlock_t* rwlock_data;
	rwlock_t* rwlock_index;

	struct node_t* next;
	struct node_t* previous;
	int index;
	void* data;
} node_t;

struct linked_list_t {
	rwlock_t* rwlock_size;
	rwlock_t* rwlock_head;

	struct node_t* head;
	int size;
};

typedef struct args_t {
	linked_list_t** list;
	op_t op;
} args_t;

void init_all(node_t* node) {
	node->rwlock_next = rwlock_init();
	node->rwlock_prev = rwlock_init();
	node->rwlock_data = rwlock_init();
	node->rwlock_index = rwlock_init();
}

void delete_all(node_t* node) {
	rwlock_destroy(node->rwlock_next);
	rwlock_destroy(node->rwlock_prev);
	rwlock_destroy(node->rwlock_data);
	rwlock_destroy(node->rwlock_index);
}

linked_list_t** list_alloc() {

	linked_list_t** head = malloc(sizeof(*head));
	if (!head) {
		return NULL;
	}

	*head = malloc(sizeof(**head));
	(*head)->rwlock_head = rwlock_init();
	(*head)->rwlock_size = rwlock_init();
	if (!*head) {
		return NULL;
	}
	node_t* node = malloc(sizeof(node_t));
	if (!node) {
		return NULL;
	}
	(*head)->size = 0;
	(*head)->head = node;

	node->data = NULL;
	node->index = -1;
	node->next = NULL;
	node->previous = NULL;

	init_all(node);

	return head;
}

void list_free(linked_list_t*** list) {
	if(list == NULL){
		return;
	}
	if (*list == NULL) {
		return;
	}
	node_t* current = (**list)->head;
	node_t* tmp;

	write_lock(current->rwlock_next);
	while (current != NULL) {
		tmp = current;
		current = current->next;

		if (current != NULL) {
			write_lock(current->rwlock_next);
		}

		write_unlock(tmp->rwlock_next);
		delete_all(tmp);
		free(tmp);
	}
	rwlock_destroy((**list)->rwlock_head);
	rwlock_destroy((**list)->rwlock_size);
	free(*list);
	*list = NULL;
}

int list_insert(linked_list_t** list, int index, void* data) {
	if (list == NULL || data == NULL) {
		return 1;
	}
	node_t* current = (*list)->head;
	node_t* node = malloc(sizeof(*node));
	if (!node) {
		return 1;
	}
	node->next = NULL;
	node->previous = NULL;
	node->index = index;
	node->data = data;
	init_all(node);
	write_lock(current->rwlock_next);
	while (current->next != NULL) {
		current = current->next;
		if (node->index < current->index) {
			current->previous->next = node;
			node->previous = current->previous;
			current->previous = node;
			node->next = current;
			write_lock((*list)->rwlock_size);
			(*list)->size++;
			write_unlock((*list)->rwlock_size);
			write_unlock(node->previous->rwlock_next);
			return 0;
		}
		if (node->index == current->index) {
			write_unlock(current->previous->rwlock_next);
			free(node);
			return 1;
		}

		write_lock(current->rwlock_next);
		write_unlock(current->previous->rwlock_next);
	}
	current->next = node;
	node->previous = current;
	node->next = NULL;
	write_lock((*list)->rwlock_size);
	(*list)->size++;
	write_unlock((*list)->rwlock_size);
	write_unlock(current->rwlock_next);
	return 0;
}

int list_remove(linked_list_t** list, int index) {
	if (list == NULL) {
		return 1;
	}
	node_t* current = (*list)->head;
	write_lock(current->rwlock_next);
	while (current->next != NULL) {
		current = current->next;
		write_lock(current->rwlock_next);
		if (current->index == index) {
			if (current->next != NULL) { //check if it`s not the last in the list
				current->next->previous = current->previous;
				current->previous->next = current->next;
				write_unlock(current->rwlock_next);
				write_lock((*list)->rwlock_size);
				--(*list)->size;
				write_unlock((*list)->rwlock_size);
				write_unlock(current->previous->rwlock_next);
				delete_all(current);
				free(current);
				return 0;
			}
			//than he is the last in the list
			current->previous->next = NULL;
			write_unlock(current->rwlock_next);
			write_lock((*list)->rwlock_size);
			--(*list)->size;
			write_unlock((*list)->rwlock_size);
			write_unlock(current->previous->rwlock_next);
			delete_all(current);
			free(current);
			return 0;
		}
		write_unlock(current->previous->rwlock_next);
	}
	write_unlock(current->rwlock_next);
	//no such index in the list
	return 1;
}

int list_contains(linked_list_t** plist, int index) {
	if (plist == NULL || (*plist) == NULL) {
		return 0;
	}
	int result = 0;
	linked_list_t* list = *plist;
	read_lock(list->rwlock_head);
	node_t* current = list->head;
	read_lock(current->rwlock_next);
	read_unlock(list->rwlock_head);
	while (current->next != NULL) {
		if (current->next->index >= index) {
			result = (current->next->index == index) ? 1 : 0;
			read_unlock(current->rwlock_next);
			return result;
		}
		current = current->next;
		read_lock(current->rwlock_next);
		read_unlock(current->previous->rwlock_next);
	}
	read_unlock(current->rwlock_next);
	return 0;
}

int list_size(linked_list_t** list) {
	if (list == NULL || (*list) == NULL) {
		return 0;
	}
	int tmp_size;
	//TODO if need in RT return?
	write_lock((*list)->rwlock_size);
	tmp_size = (*list)->size;
	write_unlock((*list)->rwlock_size);
	return tmp_size;
}

int list_update_node(linked_list_t** plist, int index, void* data) {
	if (plist == NULL || (*plist) == NULL || data == NULL) {
		return 1;
	}
	linked_list_t* list = *plist;
	read_lock(list->rwlock_head);
	node_t* current = list->head;
	read_lock(current->rwlock_next);
	read_unlock(list->rwlock_head);
	while (current->next != NULL) {
		if (current->next->index > index) {
			read_unlock(current->rwlock_next);
			return 1;
		}
		if (current->next->index == index) {
			write_lock(current->next->rwlock_data);
			current->next->data = data;
			write_unlock(current->next->rwlock_data);
			read_unlock(current->rwlock_next);
			return 0;
		}
		read_lock(current->rwlock_next);
		current = current->next;
		read_unlock(current->previous->rwlock_next);
	}
	read_unlock(current->rwlock_next);
	return 1;
}

int list_node_compute(linked_list_t** list, int index,
		void *(*compute_func)(void *), void** result) {
	if (list == NULL || result == NULL || compute_func == NULL) {
		return 1;
	}
	node_t* current = (*list)->head;
	read_lock(current->rwlock_next);
	while (current->next != NULL) {
		current = current->next;
		if (current->index > index) {
			read_unlock(current->previous->rwlock_next);
			return 1;
		}
		if (current->index == index) {
			write_lock(current->rwlock_data);
			*result = compute_func(current->data);
			write_unlock(current->rwlock_data);
			read_unlock(current->previous->rwlock_next);
			return 0;
		}
		read_lock(current->rwlock_next);
		read_unlock(current->previous->rwlock_next);
	}
	read_unlock(current->rwlock_next);
	return 1;
}
//**************wrapper functions*********************************************//
void* list_insert_wp(void* args) {
	list_insert(((args_t*) args)->list, ((args_t*) args)->op.index,
			((args_t*) args)->op.data);
}
void* list_remove_wp(void* args) {
	list_remove(((args_t*) args)->list, ((args_t*) args)->op.index);
}
void* list_contains_wp(void* args) {
	list_contains(((args_t*) args)->list, ((args_t*) args)->op.index);
}
void* list_update_node_wp(void* args) {
	list_update_node(((args_t*) args)->list, ((args_t*) args)->op.index,
			((args_t*) args)->op.data);
}
void* list_node_compute_wp(void* args) {
	list_node_compute(((args_t*) args)->list, ((args_t*) args)->op.index,
			((args_t*) args)->op.compute_func,
			(void**) (&(((args_t*) args)->op.result)));
}
//****************************************************************************//
void list_batch(linked_list_t** list, int num_ops, op_t* ops) {
	if (list == NULL || ops == NULL) {
		return;
	}
	pthread_t threads[num_ops];
	args_t args[num_ops];
	int i;
	for (i = 0; i < num_ops; i++) {
		args[i].list = list;
		args[i].op = ops[i];
		switch (ops[i].op) {
		case INSERT:
			ops[i].result = pthread_create(&threads[i], NULL, list_insert_wp,
					&(args[i]));
			break;
		case REMOVE:
			ops[i].result = pthread_create(&threads[i], NULL, list_remove_wp,
					&(args[i]));
			break;
		case CONTAINS:
			ops[i].result = pthread_create(&threads[i], NULL, list_contains_wp,
					&(args[i]));
			break;
		case UPDATE:
			ops[i].result = pthread_create(&threads[i], NULL,
					list_update_node_wp, &(args[i]));
			break;
		case COMPUTE:
			pthread_create(&threads[i], NULL, list_node_compute_wp, &(args[i]));
			break;
		default:
			printf("Error in list_batch function : undefined op\n");
			return;
		}
	}
	for (i = 0; i < num_ops; i++) {
		pthread_join(threads[i], NULL);
	}
}

void list_print(linked_list_t** list) {
	node_t* current = (*list)->head;
	while (current->next != NULL) {
		current = current->next;
		printf(" %d  ",current->index);
	}
	printf("\n");
}
