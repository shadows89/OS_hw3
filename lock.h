/*
 * lock.h
 *
 *  Created on: Dec 27, 2014
 *      Author: da
 */

#ifndef LOCK_H_
#define LOCK_H_

#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

/*
 * RWLockResult (return results):
 * 		SUCCESS - in case of success
 * 		NULL_PARAM - null parameter has been passed
 * 		NOT_INIT - the lock isn't initialized or being destroyed
 * 		IS_LOCKED - the lock could not be acquired because it was already locked
 */
typedef enum RWLockResult{
	SUCCESS, NULL_PARAM, NOT_INIT, IS_LOCKED
} RWLockResult;

#define MAX_SIMUL_READERS 15

typedef struct rwlock_t  {
	int num_of_readers;
	pthread_cond_t readers_cond;

	int num_of_writers;
	pthread_cond_t writers_cond;

	pthread_mutex_t mutex_lock;

	int readers_waiting;
	int writers_waiting;
	bool valid_lock;				//the lock isn't being destroyed (is usable)
} *rwlock_t;

rwlock_t rwl_init ();

RWLockResult rwl_destroy(rwlock_t rwl);

RWLockResult rwl_readlock(rwlock_t rwl);

//RWLockResult rwl_readtrylock(rwlock_t rwl); need it?

RWLockResult rwl_readunlock(rwlock_t rwl);

RWLockResult rwl_writelock(rwlock_t rwl);

//RWLockResult rwl_writetrylock(rwlock_t rwl); need it?

RWLockResult rwl_writeunlock(rwlock_t rwl);

#endif /* LOCK_H_ */
