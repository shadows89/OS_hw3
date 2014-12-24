#define _BSD_SOURCE

#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include "mylist.h"

#define MSGE(msge) fprintf(stderr,"%s\n",#msge);
#define MSG(msg)  printf("%s\n",#msg);
#define VAL(val)  printf("-%d-\n\n",(val));

// can change those parameters:
// N>254 will fail (cant open more than 254 threads)
#define N 50
#define T 50
#define SLEEP usleep(rand()%T);

#define RUN_TEST(test) do{\
   			fprintf(stderr,"\nRunning %s:\n",#test);\
			   if(test()==0){\
			   MSGE(pass)\
			}else{\
			  MSGE(fail)\
			}\
		       }while(0);\

#define INIT list = list_alloc();\
	    int i;\
   		for(i=0;i<N;i++){\
		      args_p[i] = args_init(*list,i,i,NULL,0,NULL);\
		   }

#define INIT_RAND list = list_alloc();\
	    int i;\
   		for(i=0;i<N;i++){\
		      args_p[i] = args_init(*list,rand()%N,i,NULL,0,NULL);\
		   }


#define DESTROY list_free(&list);\
        for(i=0;i<N;i++){\
          args_destroy(&(args_p[i]));\
        }

#define TEST(test) if(!(test)){\
   			fprintf(stderr,"error-%s:%d\n",__FILE__,__LINE__);\
   			result =  1;\
		}



typedef void* (*Compute)(void*);

struct args_t{
  linked_list_t* list;
  int index;
  void* data;
  Compute compute;
  int* result;
  int num_ops;
  op_t* ops;
};
typedef struct args_t args_t;


args_t* args_init(linked_list_t* list, int index, int data, Compute compute,int num_ops,op_t* ops){
   args_t* args = malloc(sizeof(args_t));
   if(args==NULL){
      return NULL;
   }
   args->list = list;
   args->index = index;
   args->data = (void*)malloc(sizeof(int));
   *((int*)(args->data)) = data;
   args->compute = compute;
   args->result = (void*)malloc(sizeof(int));
   args->num_ops = num_ops;
   args->ops = ops; 
   return args;
}

void args_destroy(args_t** args){
  free((*args)->data);
  free((*args)->result);
  free(*args);
}

#define CALL_FUNC(func)  SLEEP;\
   args_t* qargs = (args_t*)args;\
   linked_list_t* list = qargs->list;\
   int index = qargs->index;\
   void* data = qargs->data;\
   Compute compute = qargs->compute;\
   int num_ops = qargs->num_ops;\
   op_t* ops = qargs->ops;\
   *(qargs->result) = (func);
 

void* list_insert_rp(void* args){
   CALL_FUNC(list_insert(&list,index,data))
 }

void* list_remove_rp(void* args){
    CALL_FUNC(list_remove(&list,index))
}

void* list_update_node_rp(void* args){
   CALL_FUNC( list_update_node(&list,index,data))
}
// for compute - the result contains the calculation of the function on the data
// not return value (ignored)
void* list_contains_rp(void* args){
   CALL_FUNC( list_contains(&list,index))
}

void* list_node_compute_rp(void* args){
   CALL_FUNC( list_node_compute(&list,index,compute,(void**)(&qargs->result)))
}

void* list_batch_rp(void* args){
   SLEEP;
   MSGE(end of sleep)
   linked_list_t* list = ((args_t*)args)->list;
   int index = ((args_t*)args)->index;
   Compute compute = ((args_t*)args)->compute;
   int* result = ((args_t*)args)->result;
   int num_ops = ((args_t*)args)->num_ops;
   op_t* ops = ((args_t*)args)->ops;
   MSGE(ENTER batch);
   list_batch(&list,num_ops,ops);
}   

int test_simple_insert(){
   int result = 0;
   linked_list_t** list;
   args_t* args_p[N];
   INIT_RAND 
   pthread_t threads[N];
   int size = 0;
   for(i=0;i<N;i++){
      pthread_create(&(threads[i]),NULL,list_insert_rp,args_p[i]);
   }
   for(i=0;i<N;i++){
      pthread_join(threads[i],NULL);
   }
   for(i=0;i<N;i++){
      TEST(list_contains(list,args_p[i]->index)==1)
   }
   for(i=0;i<N;i++){
      size+= *(args_p[i]->result)==0 ? 1 : 0 ;
   }
   TEST(list_size(list)==size)
   DESTROY
   return result;
}

int test_double_insert(){
   srand(time(NULL));
   int result = 0;
   linked_list_t** list;
   args_t* args_p[N];
   INIT 
   pthread_t threads[N];
   for(i=0;i<N;i++){
      pthread_create(&(threads[i]),NULL,list_insert_rp,args_p[i]);
   }
   for(i=0;i<N;i++){
      pthread_join(threads[i],NULL);
   }
   for(i=0;i<N;i++){
      TEST(list_contains(list,i)==1)
   }
   TEST(list_size(list)==N)
   DESTROY
      return result;
}

int test_insert_remove(){
   int result = 0;
   linked_list_t** list; 
   args_t* args_p[N];
   INIT
   pthread_t threads[N];
   // insert even indexes
   for(i=0;i<N;i+=2){
     	 pthread_create(&(threads[i]),NULL,list_insert_rp,args_p[i]);
   }
   for(i=0;i<N;i+=2){
     	 pthread_join(threads[i],NULL);
   }
   //insert odd indexes and remove even indexes
   for(i=0;i<N;i++){
      if(i%2==0){
	 pthread_create(&(threads[i]),NULL,list_remove_rp,args_p[i]);
      }else{
	 pthread_create(&(threads[i]),NULL,list_insert_rp,args_p[i]);
      }
   }
   for(i=0;i<N;i++){
      pthread_join(threads[i],NULL);
   }
   for(i=1;i<N;i+=2){
      TEST(list_contains(list,i)==1)
   }
   TEST(list_size(list)==N/2)
   DESTROY
      return result;
}

void* identity(void* data){
   return data;
}

int test_update_compute(){
   int result = 0;MSGE(aa)
   linked_list_t** list = list_alloc();
   args_t* args_p[N];
   int i;MSGE(00)
   for(i=0;i<N;i++){
      args_p[i] = args_init(*list,i,i,identity,0,NULL);
   }
   pthread_t threads[N];
   // insert all indexes and data from 0 to N-1
   for(i=0;i<N;i++){
     	 pthread_create(&(threads[i]),NULL,list_insert_rp,args_p[i]);
   }
   for(i=0;i<N;i++){
     	 pthread_join(threads[i],NULL);
   }
   for(i=0;i<N;i++){
      args_destroy(&(args_p[i]));
   }
   for(i=0;i<N;i++){
      args_p[i] = args_init(*list,i,-i,identity,0,NULL);
   }
   //update all indexes to data = -index and remove evens.
   for(i=0;i<N;i++){
      if(i%2==0){
	 TEST(pthread_create(&(threads[i]),NULL,list_remove_rp,args_p[i])==0);
      }else{
         TEST(pthread_create(&(threads[i]),NULL,list_update_node_rp,args_p[i])==0);
      }
   }MSGE(qq)
   for(i=0;i<N;i++){
      fprintf(stderr,"%d,",i);
      TEST(pthread_join(threads[i],NULL)==0)
   }MSGE(uu)
   for(i=1;i<N/2;i++){
      pthread_create(&(threads[i]),NULL,list_node_compute_rp,args_p[i]);
   }MSGE(33)
   for(i=0;i<N/2;i++){
      pthread_join(threads[i],NULL);
   }
   for(i=1;i<N;i+=2){
      TEST((*(args_p[i]->result))==-i)
   }MSGE(44)
   TEST(list_size(list)==N/2)
   DESTROY
   return result;
}

int test_list(){
   linked_list_t** list=NULL;
   int result = 0;
   list  = list_alloc();
   TEST(list!=NULL)
   int nums[5] = {0,1,2,3,4};
   list_free(NULL);
   TEST(list_insert(list,2,&(nums[2]))==0);
   TEST(list_insert(list,0,&(nums[0]))==0);
   TEST(list_insert(list,4,&(nums[4]))==0);
   TEST(list_insert(list,3,&(nums[3]))==0);
   TEST(list_remove(list,2)==0);
   TEST(list_remove(list,2)>0);
   TEST(list_update_node(list,2,&(nums[1]))>0);
   TEST(list_contains(list,2)==0);
   TEST(list_contains(list,0)==1);
   TEST(list_contains(list,4)==1);
   TEST(list_size(list)==3);  
   TEST(list_update_node(list,0,&(nums[2]))==0);
   void* ptr;
   TEST(list_node_compute(list,2,identity,&(ptr))>0 );
   TEST(list_node_compute(list,4,identity,&(ptr))==0 );
   TEST((*(int*)ptr)==4); 
   return result;
}

int test_batch(){
  linked_list_t** list = list_alloc();
  int result = 0;
  int nums[6] = {0,1,2,3,4,5};
  list_insert(list,0,&(nums[0]));
  list_insert(list,4,&(nums[2]));
  list_insert(list,2,&(nums[4]));
  int garbage = 0;
  int update[3] = {40,50,60};
 op_t ops[16] = {
    {1,&(nums[1]),INSERT,NULL,0},//0
   {3,&(nums[3]),INSERT,NULL,0} ,//1
   {-1,NULL,REMOVE,NULL,0},//2
   {6,NULL,REMOVE,NULL,0},//3
   {1,&garbage,COMPUTE,identity,0},//4
   {1,&garbage,COMPUTE,identity,0},//5
   {2,&(update[0]),UPDATE,NULL,0},//6
   {4,&(update[1]),UPDATE,NULL,0},//7
   {5,&(nums[1]),INSERT,NULL,0},//8
   {5,&(nums[3]),INSERT,NULL,0},//9
   {-1,NULL,REMOVE,NULL,0},//10
   {6,NULL,REMOVE,NULL,0},//11
   {0,NULL,COMPUTE,identity,0},//12
   {2,NULL,COMPUTE,identity,0},//13
   {0,&(update[2]),UPDATE,NULL,0},//14
   {2,&(update[0]),UPDATE,NULL,0}};//15
  pthread_t threads[N];
  args_t* args_p[N];
  int i;
  MSGE(p1)
  for(i=0;i<2;i++){
     args_p[i] = args_init(*list,i,0,NULL,8,ops+8*i);
  }MSGE(p1)
  for(i=0;i<2;i++){
    TEST(pthread_create(&(threads[i]),NULL,list_batch_rp,args_p[i])==0);
  }MSGE(p3)
  for(i=0;i<2;i++){
      TEST(pthread_join(threads[i],NULL)==0);
  }MSGE(pbet)
  for(i=0;i<2;i++){
     pthread_create(&(threads[i]),NULL,list_batch_rp,args_p[i]);
     pthread_join(threads[i],NULL);
  }MSGE(p4)
  TEST(ops[4].result == 1);
  TEST(ops[12].result == 60);
  TEST(ops[13].result == 40); 
  return result;
}

int main(){
  RUN_TEST(test_list);
   RUN_TEST(test_simple_insert)
   RUN_TEST(test_double_insert)
   RUN_TEST(test_insert_remove)
   RUN_TEST(test_update_compute) //CAUSE segmantation fault
 //  RUN_TEST(test_batch)
   return 0;
}
