#include "stdlib.h"
#include "stdio.h"
#include "mylist.h"

linked_list_t** list_alloc(){
	return NULL;
}

void list_free(linked_list_t** list){}

int list_insert(linked_list_t** list, int index, void* data){
	return 0;
}

int list_remove(linked_list_t** list, int index){
	return 0;
}

int list_contains(linked_list_t** list, int index){
	return 0;
}

int list_size(linked_list_t** list){
	return 0;
}

void list_batch(linked_list_t** list, int num_ops, op_t* ops){

}

int list_update_node(linked_list_t** list, int index, void* data){
	return 0;
}

int list_node_compute(linked_list_t** list, int index,
						void *(*compute_func) (void *), void** result){
	return 0;
}



