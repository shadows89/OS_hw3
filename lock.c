/*
 * lock.c
 *
 *  Created on: Dec 27, 2014
 *      Author: da
 */

#define _GNU_SOURCE 1
#include "lock.h"

/*	mutex_lock:
 *	locks the mutex (critical code) if possible
 *	returns RWLockResult;
 *	 */
RWLockResult mutex_lock(rwlock_t rwl);

rwlock_t rwl_init() {
	rwlock_t rwl;
	rwl = malloc(sizeof(struct rwlock_t));
	if (!rwl) {
		return NULL;
	}

	rwl->num_of_readers = 0;
	pthread_cond_init(&rwl->readers_cond, NULL);
	rwl->num_of_writers = 0;
	pthread_cond_init(&rwl->writers_cond, NULL);
	rwl->readers_starve_count = 0;

	/* mutexatt is used for error checking mutex */
	pthread_mutexattr_init(&rwl->mutex_attr);
	pthread_mutexattr_settype(&rwl->mutex_attr, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(&rwl->mutex_lock, &rwl->mutex_attr);

	rwl->readers_waiting = 0;
	rwl->writers_waiting = 0;
	rwl->valid_lock = true;

	return rwl;
}

RWLockResult rwl_destroy(rwlock_t rwl) {
	RWLockResult result;
	if (rwl == NULL) {
		return NULL_PARAM;
	}
	if (rwl->valid_lock == false) {
		return NOT_INIT;
	}
	result = mutex_lock(rwl);
	if (result != SUCCESS) {
		return result;
	}

	rwl->valid_lock = false; //indicates that the lock are going to be destroyed

	//waiting until the are no readers and writers active or waiting
	while (rwl->num_of_readers > 0 || rwl->num_of_writers > 0
			|| rwl->readers_waiting != 0 || rwl->writers_waiting != 0) {
	}

	//destroying process started
	pthread_cond_destroy(&rwl->writers_cond);
	pthread_cond_destroy(&rwl->readers_cond);
	pthread_mutex_unlock(&rwl->mutex_lock);
	pthread_mutex_destroy(&rwl->mutex_lock);
	pthread_mutexattr_destroy(&rwl->mutex_attr);
	free(rwl);
	return SUCCESS;
}

RWLockResult rwl_readlock(rwlock_t rwl) {
	int result;
	if (rwl == NULL) {
		return NULL_PARAM;
	}

//	checks whether the mutex is initialized or being destroyed.
//	this check is before the lock because the mutex may be not initialized
//	or in the middle of the destroying process.
	if (rwl->valid_lock == false) {
		return NOT_INIT;
	}
	result = mutex_lock(rwl);
	if (result != SUCCESS) {
		return result;
	}

	if ((rwl->num_of_writers > 0
			|| rwl->readers_starve_count == MAX_READERS_STARVE)) {
		rwl->readers_waiting++;
		while (rwl->num_of_writers > 0
				|| rwl->readers_starve_count == MAX_READERS_STARVE) {
			pthread_cond_wait(&rwl->readers_cond, &rwl->mutex_lock);
		}
		rwl->readers_waiting--;
	}

	//some threads are already readying regarding to rwl_writeunlock function
	if (rwl->writers_waiting > 0) {
		if (rwl->readers_starve_count < MAX_READERS_STARVE) { //shouldn't happen but in case
			rwl->readers_starve_count++;
		}
	}
	rwl->num_of_readers++; //increases active readers number
	pthread_mutex_unlock(&rwl->mutex_lock);
	return SUCCESS;
}

RWLockResult rwl_readunlock(rwlock_t rwl) {
	int result;
	if (rwl == NULL) {
		return NULL_PARAM;
	}
	result = mutex_lock(rwl);
	if (result != SUCCESS) {
		return result;
	}

	rwl->num_of_readers--;
	if (rwl->num_of_readers == 0 && rwl->writers_waiting > 0) {
		pthread_cond_signal(&rwl->writers_cond);
	}
	pthread_mutex_unlock(&rwl->mutex_lock);
	return SUCCESS;
}

RWLockResult rwl_writelock(rwlock_t rwl) {
	int result;
	if (rwl == NULL) {
		return NULL_PARAM;
	}

//	the explanation as rwl_readlock
	if (rwl->valid_lock == false) {
		return NOT_INIT;
	}
	result = mutex_lock(rwl);
	if (result != SUCCESS) {
		return result;
	}

	if ((rwl->num_of_writers || rwl->num_of_readers > 0)) {
		rwl->writers_waiting++;
		while (rwl->num_of_writers > 0 || rwl->num_of_readers > 0) {
			pthread_cond_wait(&rwl->writers_cond, &rwl->mutex_lock);
		}
		rwl->writers_waiting--;
	}
	rwl->num_of_writers = 1;
	rwl->readers_starve_count = 0;
	pthread_mutex_unlock(&rwl->mutex_lock);
	return SUCCESS;
}

RWLockResult rwl_writeunlock(rwlock_t rwl) {
	int result;
	if (rwl == NULL) {
		return NULL_PARAM;
	}

	result = mutex_lock(rwl);
	if (result != SUCCESS) {
		return result;
	}

	rwl->num_of_writers=0;
	if(rwl->readers_waiting>0){
		pthread_cond_broadcast(&rwl->readers_cond);
	} else if( rwl->writers_waiting){
		pthread_cond_signal(&rwl->writers_cond);
	}
	pthread_mutex_unlock(&rwl->mutex_lock);
	return SUCCESS;
}

RWLockResult mutex_lock(rwlock_t rwl) {
	int result;
	result = pthread_mutex_lock(&rwl->mutex_lock);
	if (result == EINVAL) { //the mutex has not been properly initialized
		return NOT_INIT;
	}
	if (result == EDEADLK) { //the mutex is already locked by the calling thread
		return IS_LOCKED;
	}
	if (result != 0) {
		return MUTEX_ERR;
	}

	return SUCCESS;
}

int main() {

	/* 	rwlock_t rwl = rwl_init();
	 printf("%d",rwl->num_of_readers);
	 printf("%d",rwl->num_of_writers);
	 free(rwl);*/

	return 0;
}

