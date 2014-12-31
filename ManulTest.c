#include "mylist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

typedef struct Params {
	linked_list_t** list;
	int n1;
	int n2;
	int n3;
	int(*test)(void);
	char* name;
}Params;

typedef struct Data {
	int x;
	int y;
	int num_changes;
}Data;

int success_counter;
int fail_counter;

void run(int(*test)(void),char* name){
	printf("%s\t", name);
	if(test()){
		printf("\r\t\t\t\t\t\t\t\t\x1b[1;4;42m[PASS]\x1b[0m\n");
	}else{
		printf("   \x1b[1;4;41m[FAIL]\x1b[0m\n");
		exit(1);
	}
}

void abolutely_crazy_run(int(*test)(void), char* name){
	if(test()){
		printf("\r\t%s\t",name);
		printf("\r\t\t\t\t\t\t\x1b[1;4;42m[PASS]\x1b[0m\n");
	}else{
		printf("%s:\t",name);
		printf("   \x1b[1;4;41m[FAIL]\x1b[0m\n");
		exit(1);
	}
}

int insert_test(){
	//printf("INSERT TEST:\t");
	linked_list_t** list = list_alloc();
	char* n1 = "test";
	if(list_insert(list, 5, n1)){
		printf("insert 5 error");
		return 0;
	}	
	if(list_insert(list, 10, n1)){
		printf("insert 10 error");
		return 0;
	}
	if(list_insert(list, 15 , n1)){
		printf("insert 15 error");
		return 0;
	}
	
	if( list_insert(list, 1, n1)){
		printf("insert 1 error");
		return 0;
	}

	if( list_insert(list, 7, n1)){
		printf("insert 7 error");
		return 0;
	}	

	if( list_insert(list, 20, n1)){
		printf("insert 20 error");
		return 0;
	}

	if( ! list_insert(list, 5, n1)){
		printf("should return error but didnt!");
		return 0;
	}

	list_free(&list);

	return 1;
}

void* thread_insert_test_aux(void* args){
	linked_list_t** list = ((Params*)args)->list;
	int n1 = ((Params*)args)->n1;
	int n2 = ((Params*)args)->n2;
	int n3 = ((Params*)args)->n3;

	//printf("thread %d params: %d , %d , %d \n",pthread_self(),n1,n2,n3);

	if( list_insert(list, n1, "test")){
		fail_counter++;
		//printf("%d insert %d error\n", pthread_self(),n1);	
	}else{
		success_counter++;
		//printf("%d insert %d success\n", pthread_self(),n1);
	}

	if( list_insert(list, n2, "test")){
		fail_counter++;
		//printf("%d insert %d error\n", pthread_self(),n2);
	}else{
		success_counter++;
		//printf("%d insert %d success\n", pthread_self(),n2);
	}

	if( list_insert(list, n3, "test")){
		fail_counter++;
		//printf("%d insert %d error\n", pthread_self(),n3);
	}else{
		success_counter++;;
		//printf("%d insert %d success\n", pthread_self(),n3);
	}

	pthread_exit(NULL);

}


int thread_insert_test(){
	// printf("THREAD_INSERT TEST:\t");
	pthread_t t1,t2,t3;
	Params* args1 = (Params*)(malloc(sizeof(Params)));
	Params* args2 = (Params*)(malloc(sizeof(Params)));
	Params* args3 = (Params*)(malloc(sizeof(Params)));

	success_counter = 0;
	fail_counter = 0;

	linked_list_t** list = list_alloc();

	args1->list = list;
	args1->n1 = -5;
	args1->n2 = 2;
	args1->n3 = 3;

	args2->list = list;
	args2->n1 = -5;
	args2->n2 = 3;
	args2->n3 = 2;

	args3->list = list;
	args3->n1 = 3;
	args3->n2 = 2;
	args3->n3 = 1;


	list_insert(list, 1, "test");
	list_insert(list, 5, "test");

	if(pthread_create(&t1, NULL, thread_insert_test_aux, args1) ){
		free(args1);
		printf("pthread_create 1 error");
		return 0;
	}	
	if(pthread_create(&t2, NULL, thread_insert_test_aux, args2) ){
		free(args2);
		printf("pthread_create 2 error");
		return 0;
	}
	if(pthread_create(&t3, NULL, thread_insert_test_aux, args3) ){
		free(args3);
		printf("pthread_create 3 error");
		return 0;
	}


	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	pthread_join(t3,NULL);

	free(args1);
	free(args2);
	free(args3);

	//print_list(list);

	if(success_counter != 3 || fail_counter != 6){
		printf("succes/fail counters error = [%d,%d]", success_counter, fail_counter);
		
		list_free(&list);
		return 0;
	}

	list_free(&list);
	return 1;
}


int contains_test(){
	// printf("CONTAINS TEST:\t");
	linked_list_t** list = list_alloc();

	list_insert(list, 5, "test");
	list_insert(list, 10, "test");

	if(!list_contains(list,5)){
		printf("contains 5 error");
		return 0;
	}	
	if(list_contains(list,0)){
		printf("contains 0 error");
		return 0;
	}

	list_free(&list);

	return 1;
}

int size_test(){
	// printf("SIZE TEST:\t");
	linked_list_t** list = list_alloc();
	list_insert(list,1, "test");
	list_insert(list,2, "test");
	list_insert(list,3, "test");
	list_insert(list,4, "test");

	int size = list_size(list);

	if(size != 4){
		printf("size error! %d\n", size);
		//print_list(list);
		return 0;
	}


	list_free(&list);
	return 1;
}

void* change(void* data){
	((Data*)data)->x = 7;
	((Data*)data)->y = 5;
	((Data*)data)->num_changes++;
	return NULL;
}


void* change2(void* data){
	((Data*)data)->num_changes++;
	// int* t = malloc(sizeof(int)*99999999);
	// int* k = malloc(sizeof(int)*99999999);
	// memcpy(t,k,sizeof(int)*99999999);
	// free(t);
	// free(k);
	//printf("\n%p __ %d __ %d", pthread_self(),((Data*)data)->x,((Data*)data)->num_changes);

	return NULL;
}

void* get_num_changes(void* data){
	int* result = malloc(sizeof(int));
	*result = ((Data*)data)->num_changes;
	return (void*)result;
}

void* add(void* data){
	int* result = malloc(sizeof(int));
	*result = ((Data*)data)->x + ((Data*)data)->y;
	return (void*)result;
}

void* add2(void* data){
	//printf("adding %d, %d \n", ((Data*)data)->x ,((Data*)data)->y);
	return (void*)(((Data*)data)->x + ((Data*)data)->y);
}

void* get_data_x(void* data){
	int* result = malloc(sizeof(int));
	*result = ((Data*)data)->x;
	return (void*)result;
}

void* get_data_x2(void* data){
	//printf("data_x2: %d\n",((Data*)data)->x);
	return (void*)(((Data*)data)->x);
}

int compute_test(){
	// printf("COMPUTE TEST:\t");

	linked_list_t** list = list_alloc();

	Data data;
	
	data.x = 3;
	data.y = 2;
	data.num_changes = 0;
	int* res;

	list_insert(list,1, (void*)&data);

	list_node_compute(list, 1, change, (void*)&res); // data now going to be {7,5}
	list_node_compute(list, 1, add, (void*)&res); //result going to be 5+7=12

	if(*res != 12){
		free(res);
		printf("compute error");
		return 0;
	}

	free(res);
	
	if( ! list_node_compute(list, 5, change, (void*)&res)){
		printf("compute 5 error");
		return 0;
	}


	list_free(&list);
	return 1;
}

void thread_compute_test_aux(void* args){
	linked_list_t** list = ((Params*)args)->list;
	int n1 = ((Params*)args)->n1;
	int n2 = ((Params*)args)->n2;
	int n3 = ((Params*)args)->n3;

	int* res;

	//printf("I am THREAD %p\n", pthread_self());

	list_node_compute(list, n1, change2, (void*)&res); // data now going to be {7,5}
	list_node_compute(list, n2, change2, (void*)&res); // data now going to be {7,5}
	list_node_compute(list, n3, change2, (void*)&res); // data now going to be {7,5}

	pthread_exit(NULL);
}



int thread_compute_test(){
	// printf("THREAD COMPUTE TEST:\t");

	linked_list_t** list = list_alloc();

	pthread_t t1,t2,t3;
	Params args1, args2, args3;
	Data data1,data2,data3;
	data1 = (Data) {.x=1, .y=1, .num_changes=0};
	data2 = (Data) {.x=2, .y=2, .num_changes=0};
	data3 = (Data) {.x=3, .y=3, .num_changes=0};
	int* res;

	list_insert(list,1, (void*)&data1);
	list_insert(list,2, (void*)&data2);
	list_insert(list,3, (void*)&data3);

	args1 = (Params) {.list=list, .n1=1, .n2=2, .n3=3};
	args2 = (Params) {.list=list, .n1=2, .n2=3, .n3=2};
	args3 = (Params) {.list=list, .n1=2, .n2=1, .n3=1};

	if(pthread_create(&t1, NULL, thread_compute_test_aux, &args1) ){
		printf("pthread_create 1 error");
		return 0;
	}	
	if(pthread_create(&t2, NULL, thread_compute_test_aux, &args2) ){
		printf("pthread_create 2 error");
		return 0;
	}
	if(pthread_create(&t3, NULL, thread_compute_test_aux, &args3) ){
		printf("pthread_create 3 error");
		return 0;
	}

	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	pthread_join(t3,NULL);

	list_node_compute(list, 1, get_num_changes, (void*)&res);
	if(*res != 3){
		printf("node 1 num_changes error %d", *res);
		free(res);
		return 0;
	}
	free(res);

	list_node_compute(list, 2, get_num_changes, (void*)&res);
	if(*res != 4){
		printf("node 2 num_changes error %d", *res);
		free(res);
		return 0;
	}
	free(res);

	list_node_compute(list, 3, get_num_changes, (void*)&res);
	if(*res != 2){
		printf("node 3 num_changes error %d", *res);
		free(res);
		return 0;
	}
	free(res);


	list_free(&list);
	return 1;
}

int update_test(){
	// printf("UPDATE TEST:\t");

	linked_list_t** list = list_alloc();
	Data data1 = {.x = 1, .y = 1, .num_changes = 0};
	Data data2 = {.x = 2, .y = 2, .num_changes = 0};
	Data data3 = {.x = 3, .y = 3, .num_changes = 0};
	int* res;

	list_insert(list, 1, (void*)&data1);
	list_insert(list, 2, (void*)&data2);
	list_insert(list, 3, (void*)&data3);

	list_update_node(list, 1, (void*)&data3);
	list_update_node(list, 2, (void*)&data3);
	list_update_node(list, 3, (void*)&data3);	
	//now all the nodes point to data3

	list_node_compute(list, 1, get_data_x, (void*)&res);
	if(*res != 3){
		free(res);
		printf("update error 1");
		return 0;
	}
	free(res);

	list_node_compute(list, 2, get_data_x, (void*)&res);	
	if(*res != 3){
		free(res);
		printf("update error 2");
		return 0;
	}
	free(res);

	list_node_compute(list, 3, get_data_x, (void*)&res);	
	if(*res != 3){
		free(res);
		printf("update error 3");
		return 0;
	}
	free(res);

	//special check
	//this should change the data of all 3 nodes (will check only forn node 1)
	data3.x=555;
	list_node_compute(list, 1, get_data_x, (void*)&res);
	if(*res != 555){
		free(res);
		printf("update error 555");
		return 0;
	}
	free(res);

	list_free(&list);
	return 1;
}

int remove_test(){
	// printf("REMOVE TEST:\t");

	linked_list_t** list = list_alloc();
	int* res;

	list_insert(list, 1, "test");
	list_insert(list, 2, "test");
	list_insert(list, 3, "test");
	list_insert(list, 4, "test");
	list_insert(list, 5, "test");

	//print_list(list);

	if(list_remove(list, 1)){	//remove from head
		printf("remove 1 error");
		return 0;
	}	


	if(list_remove(list, 5)){ //remove from middle
		printf("remove 5 error");
		return 0;
	}	
	if(list_remove(list, 3)){	//remove from tail
		printf("remove 3 error");
		return 0;
	}

	int size = list_size(list);
	if(size != 2){
		printf("size error! %d\n", size);
		//print_list(list);
		return 0;
	}

	list_remove(list, 2);
	list_remove(list, 4);

	size = list_size(list);
	if(size != 0){
		printf("size error! %d\n", size);
		//print_list(list);
		return 0;
	}

	list_free(&list);
	return 1;
}

void* thread_remove_test_aux(void* args){
	linked_list_t** list = ((Params*)args)->list;
	int n1 = ((Params*)args)->n1;
	int n2 = ((Params*)args)->n2;
	int n3 = ((Params*)args)->n3;

	int* res;

	//printf("I am THREAD %p\n", pthread_self());

	list_remove(list, n1);
	// printf("THREAD %p removing %d\n", pthread_self(),n1);
	list_remove(list, n2);
	// printf("THREAD %p removing %d\n", pthread_self(),n2);
	list_remove(list, n3);
	// printf("THREAD %p removing %d\n", pthread_self(),n3);
	
	pthread_exit(NULL);
}

int thread_remove_test(){
	// printf("THREAD REMOVE TEST:\t");

	linked_list_t** list = list_alloc();

	pthread_t t1,t2,t3;
	Params args1, args2, args3;
	int* res;
	int i;

	for(i=1; i<=3; i++){
		list_insert(list, i, "test");
	}

	args1 = (Params) {.list=list, .n1=1, .n2=2, .n3=3};
	args2 = (Params) {.list=list, .n1=2, .n2=3, .n3=2};
	args3 = (Params) {.list=list, .n1=2, .n2=1, .n3=1};

	if(pthread_create(&t1, NULL, thread_remove_test_aux, &args1) ){
		printf("pthread_create 1 error");
		return 0;
	}	
	if(pthread_create(&t2, NULL, thread_remove_test_aux, &args2) ){
		printf("pthread_create 2 error");
		return 0;
	}
	if(pthread_create(&t3, NULL, thread_remove_test_aux, &args3) ){
		printf("pthread_create 3 error");
		return 0;
	}

	pthread_join(t1,NULL);
	pthread_join(t2,NULL);
	pthread_join(t3,NULL);

	int size = list_size(list);
	if(size != 0){
		printf("size error! %d\n", size);
		//print_list(list);
		return 0;
	}

	list_free(&list);
	return 1;
}

int batch_insert_remove_test(){
	// printf("BATCH INSERT_REMOVE TEST:\t");

	linked_list_t** list = list_alloc();

	int i;
	int size;

	op_t batch_list[10];

	for(i=0; i<10; i++){
		batch_list[i].index = i;
		batch_list[i].data = "test";
		batch_list[i].op = INSERT;
		batch_list[i].compute_func = NULL;
	}

	list_batch(list,10,batch_list);
	
	size = list_size(list);
	if(size != 10){
		printf("batch insert error! %d\n", size);
		//print_list(list);
		return 0;
	}

	for(i=0; i<10; i++){
		batch_list[i].index = i;
		batch_list[i].data = "test";
		batch_list[i].op = REMOVE;
		batch_list[i].compute_func = NULL;
	}

	list_batch(list,10,batch_list);
	
	size = list_size(list);
	if(size != 0){
		printf("batch remove error! %d\n", size);
		//print_list(list);
		return 0;
	}

	list_free(&list);
	return 1;
}


int batch_contains_insert_test(){
	// printf("BATCH CONTAINS_INSERT TEST:\t");
	linked_list_t** list = list_alloc();
	
	int i;
	int size;

	op_t batch_list[5];
	op_t batch_list_1[10];

	for(i=0; i<10; i=i+2){ //only even indexes
		batch_list[i/2].index = i;
		batch_list[i/2].data = "test";
		batch_list[i/2].op = INSERT;
		batch_list[i/2].compute_func = NULL;
	}
	//insert some the even indexed nodes to the structure
	list_batch(list,5,batch_list);

	//add more inserts and also some CONTAINS operations
	for(i=0; i<10; i++){
		batch_list_1[i].index = i;
		batch_list_1[i].data = "test";
		batch_list_1[i].op = (i % 2 == 0) ? CONTAINS : INSERT;
		batch_list_1[i].compute_func = NULL;
	}
	// batch_list_1[0].index = 1;  //special test most of the time should fail but sometimes pass :)
	list_batch(list,10,batch_list_1);

	//make sure every node is in the list
	size = list_size(list);
	if(size != 10){
		printf("batch insert_contains error! %d\n", size);
		//print_list(list);
		return 0;
	}

	//make sure we really found all the even nodes
	for(i=0; i<10; i=i+2){
		if(batch_list_1[i].result != 1){
			printf("batch insert_contains (contains) error!\n");
			//print_list(list);
			return 0;
		}
	}

	list_free(&list);
	return 1;
}

int batch_compute_test(){
	// printf("BATCH COMPUTE TEST:\t");
	linked_list_t** list = list_alloc();
	
	int i;
	int size;

	Data data[5];
	for(i=0; i<5; i++)
		data[i] = (Data) {.x=i, .y=i, .num_changes=0};

	op_t batch_list[5];

	for(i=0; i<5; i++){ //only even indexes
		batch_list[i].index = i;
		batch_list[i].data = &data[i];
		batch_list[i].op = INSERT;
		batch_list[i].compute_func = NULL;
	}
	list_batch(list,5,batch_list);

	for(i=0; i<5; i++){ //only even indexes
		batch_list[i].index = i;
		batch_list[i].data = NULL;
		batch_list[i].op = COMPUTE;
		batch_list[i].compute_func = add2;
	}
	list_batch(list,5,batch_list);

	//make sure we really found all the even nodes
	for(i=0; i<5; i++){
		if(batch_list[i].result != (i+i)){
			printf("batch compute error! 1 _ %d\n", batch_list[i].result);
			//print_list(list);
			return 0;
		}

	}

	for(i=0; i<5; i++){ //only even indexes
		batch_list[i].index = i;
		batch_list[i].data = NULL;
		batch_list[i].op = COMPUTE;
		batch_list[i].compute_func = add2;
	}
	list_batch(list,5,batch_list);

	//make sure we really found all the even nodes
	for(i=0; i<5; i++){
		if(batch_list[i].result != (i+i)){
			printf("batch compute error! 2\n");
			//print_list(list);
			return 0;
		}
	}

	list_free(&list);
	return 1;
}

int batch_update_test(){
	// printf("BATCH UPDATE TEST:\t");
	linked_list_t** list = list_alloc();
	
	int i;
	int size;

	Data data[5];
	for(i=0; i<5; i++)
		data[i] = (Data) {.x=i, .y=i, .num_changes=0};

	op_t batch_list[5];

	for(i=0; i<5; i++){ //only even indexes
		batch_list[i].index = i;
		batch_list[i].data = &data[i];
		batch_list[i].op = INSERT;
		batch_list[i].compute_func = NULL;
	}
	list_batch(list,5,batch_list);
	//print_list(list);

	Data new_data[5];
	for(i=0; i<5; i++) 
		new_data[i] = (Data) {.x=50+i, .y=50+i, .num_changes=0};

	for(i=0; i<5; i++){
		batch_list[i].index = i;
		batch_list[i].data = &new_data[i];
		batch_list[i].op = UPDATE;
		batch_list[i].compute_func = NULL;
	}
	list_batch(list,5,batch_list);

	//make sure we really updated all the nodes
	for(i=0; i<5; i++){
		batch_list[i].index = i;
		batch_list[i].data = NULL;
		batch_list[i].op = COMPUTE;
		batch_list[i].compute_func = get_data_x2;
	}
	list_batch(list,5,batch_list);

	for(i=0; i<5; i++){
		if(batch_list[i].result != (50+i)){
			printf("batch update error! %d\n", batch_list[i].result);
			//print_list(list);
			return 0;
		}
		//printf("elem %d: x = %d\n",i,batch_list[i].result);
	}

	list_free(&list);
	return 1;
}


int batch_update_insert_test(){
	// printf("BATCH UPDATE_INSERT TEST:\t");
	linked_list_t** list = list_alloc();
	
	int i;
	int size;

	op_t batch_list[5];
	op_t batch_list_1[10];

	for(i=0; i<10; i=i+2){ //only even indexes
		batch_list[i/2].index = i;
		batch_list[i/2].data = "test";
		batch_list[i/2].op = INSERT;
		batch_list[i/2].compute_func = NULL;
	}
	//insert some the even indexed nodes to the structure
	list_batch(list,5,batch_list);

	Data data[10];
	for(i=0; i<10; i++)
		data[i] = (Data) {.x=i, .y=i, .num_changes=0};

	//add more inserts and also some UPDATE operations
	for(i=0; i<10; i++){
		batch_list_1[i].index = i;
		batch_list_1[i].data = &data[i];
		batch_list_1[i].op = (i % 2 == 0) ? UPDATE : INSERT;
		batch_list_1[i].compute_func = NULL;
	}
	// batch_list_1[0].index = 1;  //special test most of the time should fail but sometimes pass :)
	list_batch(list,10,batch_list_1);

	//printf("finished updating\n");
	//print_list(list);

	//make sure we really updated all the even nodes
	for(i=0; i<10; i++){
		batch_list_1[i].index = i;
		batch_list_1[i].data = NULL;
		batch_list_1[i].op = COMPUTE;
		batch_list_1[i].compute_func = get_data_x2;
	}
	list_batch(list,10,batch_list_1);

	for(i=0; i<10; i++){
		if(batch_list_1[i].result != i){
			printf("batch update_insert error! %d: %d\n",i, batch_list[i].result);
			//print_list(list);
			return 0;
		}
		//printf("elem %d: x = %d\n",i,batch_list[i].result);
	}

	list_free(&list);
	return 1;
}

int crazy_random_batch_test_for_deadlocks(){
	// printf("crazy_random_batch_test_for_deadlocks:\t");
	//not so crazy because of the limit on maximum threads number %)
	//not as crazy as I hoped %)
	//but can run 10000000 times to make sure it works ok :)
	linked_list_t** list = list_alloc();
	
	int i;
	int batch_size = 200;
	op_t batch_list[batch_size];
	int existing[batch_size];

	Data data[batch_size];
	
	for(i=0; i<batch_size; i++){
		data[i] = (Data) {.x=i-batch_size/2, .y=i-batch_size/2, .num_changes=0};
	}

	for(i=0; i<batch_size; i++){ //only even indexes
		batch_list[i].index = rand() % batch_size - batch_size/2;
		batch_list[i].data = &data[batch_list[i].index + batch_size/2];
		batch_list[i].op = rand() % 5;
		batch_list[i].compute_func = get_data_x2;
		//printf("BATCH: op: %d, index: %d, data: %d\n", batch_list[i].op, batch_list[i].index, ((Data*)batch_list[i].data)->x);	
	}


	list_batch(list, batch_size, batch_list);

	// printf("NO WHAAAATS AFTER THIS POINT\n");
	// make sure we have a valid list
	for(i=0; i<batch_size; i++){
		batch_list[i].index = i - batch_size/2;
		batch_list[i].data = NULL;
		batch_list[i].op = COMPUTE;
		batch_list[i].compute_func = get_data_x2;
	}
	list_batch(list, batch_size, batch_list);


	for(i=0; i<batch_size; i++){
		 if(list_contains(list,batch_list[i].index) && batch_list[i].result != data[batch_list[i].index + batch_size/2].x){
			printf("batch update_insert error! %d: %d expected: %d\n", batch_list[i].index, batch_list[i].result, data[batch_list[i].index + batch_size/2].x);
			//print_list(list);
			return 0;
		 }
		//printf("elem %d: x = %d\n",i,batch_list[i].result);
	}

	list_free(&list);
	return 1;
}

void* absolutely_crazy_multithreaded_test_all_at_the_same_time_aux(void* args){

	int(*test)(void) = ((Params*)args)->test;
	char* name = ((Params*)args)->name;
	// printf(" - %p -  \n",test);
	abolutely_crazy_run(test,name);

	pthread_exit(NULL);
}

int(*tests[15])(void) = {
	insert_test,
	// insert_test,
	thread_insert_test,
	contains_test,
	size_test,
	compute_test,
	thread_compute_test,
	update_test,
	remove_test,
	thread_remove_test,
	batch_insert_remove_test,
	batch_contains_insert_test,
	batch_compute_test,
	batch_update_test,
	batch_update_insert_test,
	crazy_random_batch_test_for_deadlocks
};
char* names[15]={
	"insert_test:",
	"thread_insert_test:",
	"contains_test:",
	"size_test:",
	"compute_test:",
	"thread_compute_test:",
	"update_test:",
	"remove_test:",
	"thread_remove_test:",
	"batch_insert_remove_test:",
	"batch_contains_insert_test:",
	"batch_compute_test:",
	"batch_update_test:",
	"batch_update_insert_test:",
	"crazy_random_batch_test_for_deadlock:",
};

int absolutely_crazy_multithreaded_test_all_at_the_same_time(){
	// printf("absolutely_crazy_multithreaded_test_all_at_the_same_time:\n-----------------\n");
	int num_tests = 15;

	int i;
	pthread_t threads[num_tests];
	Params* args[num_tests];

	void* res;

	for(i=0; i<num_tests; i++){
		args[i] = (Params*)(malloc(sizeof(Params)));
		args[i]->test = tests[i];
		args[i]->name = names[i];
		// printf("_%p_%d\n",tests[i],i);
		if(pthread_create(&threads[i], NULL, absolutely_crazy_multithreaded_test_all_at_the_same_time_aux, args[i]) ){
			free(args[i]);
			printf("pthread_create %d error", i);
			return 0;
		}	
	}


	for(i=0; i<num_tests; i++){
		pthread_join(threads[i], &res);
		free(args[i]);
	}


	return 1;
}

int main(){
	srand (time(NULL));

//	 run(insert_test, "INSERT_TEST");
//	 run(thread_insert_test,"THREAD_INSERT_TEST");
//	 run(contains_test,"CONTAINS_TEST");
//	 run(size_test,"SIZE_TEST");
//	 run(compute_test,"COMPUTE_TEST");
//	 run(thread_compute_test,"THREAD_COMPUTE_TEST");
//	 run(update_test);
//	 run(remove_test);
//	 run(thread_remove_test);
//
////	 run(batch_insert_remove_test);
////	 run(batch_contains_insert_test);
////	 run(batch_compute_test);
////	 run(batch_update_test);
////	 run(batch_update_insert_test);

	 //run(crazy_random_batch_test_for_deadlocks);
	int i;
	for(i=0; i<15; i++)
		run(tests[i], names[i]);

	run(absolutely_crazy_multithreaded_test_all_at_the_same_time,"absolutely_crazy_multithreaded_test_all_at_the_same_time:\n");

}
