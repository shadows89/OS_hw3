
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
#define _GNU_SOURCE
//#define __USE_GNU


#include <errno.h>
#include <stdlib.h>
#include <pthread.h>



typedef struct  rwlock_t{
   int num_of_readers;
   int num_of_writers;
   pthread_cond_t readers_cond;
   pthread_cond_t writers_cond;
   pthread_mutex_t mutex;
   int starv_count;
   int waiting_writers;
   int active_threads;
   int being_destroyed;
}rwlock_t;



rwlock_t* rwlock_init();

/*
 * the calling process should not lock the rwlock
 * before destrinyg it
 */
int rwlock_destroy(rwlock_t* rwlock);

int read_lock(rwlock_t* rwlock);

int read_unlock(rwlock_t* rwlock);

int write_lock( rwlock_t* rwlock );

int write_unlock( rwlock_t* rwlock );
    
   
      
