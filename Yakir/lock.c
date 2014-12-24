
/****** Read - Write Lock ********
 * readers-writers lock
 * solve starving of writers: for writer thread W0, when W0 is the next writer thread that should
 * be starts, W0 will let at most MAX_STARV readers threads to bypass it.
 * 1 point regarding that: since basic mutex/CV queue not implemented by FIFO it is unknown
 * when W0 will be the next writer to run. i.e it can be starvation of writers
 * by other writers. 
 *
 * destroy mechanism: the destroy thread first mark this lock has being_destroyed.
 * so that new threads that try to lock it will fail. after that the destroying
 * thread is waiting untill all the threads that wait for the lock or in locked the lock
 * will finish.finally it destroy both CVs and mutex and free memory.
 *
 * the lock provide error check for NULL params,safe destroy.
 * does not check: if a process calling to unlock actually locked it before, and with
 * the same lock option (r/w). and if a process that try to destroy it wasnt lock it before.
 * in both cases behavior is unknown.
 * */


/**
 *  * return values of rwlock:
 *   *  * - 0 on success
 *    *   * - 1 if param is NULL
 *     *    * - 2 if rwlock wasnt initialized or been destroyed
 *      *     * - 3 if already been locked by this process
 *       *      *- NULL from init if allocation failed(otherwise the lock)
 *        *       * */


/***
 *  points:
 *  - cond_destroy / mutext_destroy fail if there are threads waiting
 *  - cond / mutex queue arent fifo
 *  ****/
#include "lock.h"
#define NULL_CHECK if(rwlock==NULL){\
   			return 1;\
			}
			
#define LOCK_MUTEX status = pthread_mutex_lock(&(rwlock->mutex));\
			   if(status!=0){\
 			      return status==EINVAL?2:3;\
   			   }
#define CHECK_ACTIVITY if(rwlock->being_destroyed){\
			      pthread_mutex_unlock( &(rwlock->mutex) );\
			      return 2;\
			   }else{\
			      rwlock->active_threads++;\
				}			  

#define MAX_STARV (10)


#define err(error) printf("%s\n",#error);


rwlock_t* rwlock_init(){
   rwlock_t* rwlock  = malloc(sizeof(struct rwlock_t));
   if(!rwlock){
      return NULL;
   }
   rwlock->num_of_readers = 0;
   rwlock->num_of_writers = 0;
   pthread_cond_init(&(rwlock->readers_cond),NULL);
   pthread_cond_init(&(rwlock->writers_cond),NULL);
   pthread_mutexattr_t attr;
   pthread_mutexattr_init(&attr);
   pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK);
   pthread_mutex_init(&(rwlock->mutex),&attr);
   rwlock->starv_count = 0;
   rwlock->waiting_writers = 0;
   rwlock->active_threads = 0;
   rwlock->being_destroyed = 0;
   return rwlock;
}

/*
 * the calling process should not lock the rwlock
 * before destrinyg it
 */
int rwlock_destroy(rwlock_t* rwlock){
   NULL_CHECK
   int status;
   LOCK_MUTEX
   CHECK_ACTIVITY 
   rwlock->being_destroyed = 1;
   while(rwlock->active_threads>1){
      pthread_mutex_unlock( &(rwlock->mutex) );
      // maybe to add here some sleep time!
      pthread_mutex_lock( &(rwlock->mutex) );
   }
   // at this point this is the only thread that use the rwlock,
   // and there are no waiting threads 
   pthread_cond_destroy( &(rwlock->readers_cond) );
   pthread_cond_destroy( &(rwlock->writers_cond) );
   pthread_mutex_unlock( &(rwlock->mutex) );
   pthread_mutex_destroy( &(rwlock->mutex) );
   free(rwlock);
}

int read_lock(rwlock_t* rwlock){
   NULL_CHECK
   int status;
   LOCK_MUTEX
   CHECK_ACTIVITY   
   while( (rwlock->num_of_writers==1) || (rwlock->starv_count==MAX_STARV) ){
      pthread_cond_wait(&(rwlock->readers_cond),&(rwlock->mutex) );
   }
   if(rwlock->waiting_writers>0){ // here of course starv_count<MAC_STARV
      rwlock->starv_count++;
   }
   rwlock->num_of_readers++;
   pthread_mutex_unlock( &(rwlock->mutex) ); //no error can occur here
}

int read_unlock(rwlock_t* rwlock){
   NULL_CHECK 
   int status;
   LOCK_MUTEX
   rwlock->num_of_readers--;
   if(rwlock->num_of_readers == 0 ){
      pthread_cond_signal(&(rwlock->writers_cond) );
   }
   rwlock->active_threads--;
   pthread_mutex_unlock( &(rwlock->mutex) );
}

int write_lock( rwlock_t* rwlock ){

  // err(write lock)
  // printf("-%d-\n",rwlock->num_of_readers);
   NULL_CHECK 
    int status;
  // err(write lock locking mutex)
    LOCK_MUTEX
  
    rwlock->waiting_writers++;
//   err(write lock waiting in queue)
    while( (rwlock->num_of_writers==1) ||(rwlock->num_of_readers>0) ){
    //   err(write lock in loop)
      pthread_cond_wait( &(rwlock->writers_cond), &(rwlock->mutex) );
    }
    rwlock->starv_count = 0;
    rwlock->waiting_writers--;
    rwlock->num_of_writers++;
   // err(write lock unlock mutex)
    pthread_mutex_unlock( &(rwlock->mutex) );    
//printf("-%d-\n",rwlock->num_of_readers);
   // err(write lock ends)
//printf("-%d-\n",rwlock->num_of_readers);
    
    
}

int write_unlock( rwlock_t* rwlock ){
  // err(write unlock start)
// printf("-%d-\n",rwlock->num_of_readers);  
   NULL_CHECK
   int status;
   LOCK_MUTEX
   rwlock->num_of_writers=0;
   pthread_cond_broadcast( &(rwlock->readers_cond) );
   pthread_cond_signal( &(rwlock->writers_cond) );
   rwlock->active_threads--;
   pthread_mutex_unlock( &(rwlock->mutex) );
// printf("-%d-\n",rwlock->num_of_readers);
//  err(write unlock ends)
}
      
   
    
   
      
