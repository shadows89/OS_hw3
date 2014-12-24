#include "stdio.h"
#include "stdlib.h"
#include "mylist.h"

void print(int* data){
	printf("Data: %d\n",*data);
}

int main(){
	int data = 1;
	//int i = 0;
	linked_list_t** test = list_alloc();
	if(!list_insert(test,1, &data))
		printf("Insert DONE!!\n");
	if(!list_insert(test,-12, &data))
		printf("Insert DONE!!\n");
	if(list_contains(test,1))
		printf("Found It!\n");
	if(list_contains(test,-12))
		printf("Found It!\n");

	if(!list_remove(test,-12))
		printf("Remove DONE!!\n");
	list_free(test);
	return 0;
}




