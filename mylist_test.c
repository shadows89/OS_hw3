#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include "mylist.h"

#define LOOPS 80
#define NTHREADS 10
#define ITEMS_PER_THREAD 100
#define RAND_SEED 0

#define NITEMS (ITEMS_PER_THREAD * (NTHREADS + 1))

#define fail(format, ...) \
	do { \
		fprintf (stderr, "*** ERROR: thread %ld: %s:%d: " format "\n", \
				pthread_self (), __FUNCTION__, __LINE__, \
				## __VA_ARGS__); \
		exit (EXIT_FAILURE); \
	} while (0)

static linked_list_t **list;
static int indices [NITEMS];

/* return a pseudo-random number between @min and @max (inclusive) */
static int xrandom (int min, int max)
{
	unsigned int range = max - min + 1;

	return (rand () % range) + min;
}

static void init_indices (void)
{
	int i;

	for (i = 0; i < NITEMS; i++)
		indices [i] = i;
}

/* randomly re-orders the values in indices[] */
static void shuffle_indices (void)
{
	int i;

	for (i = 0; i < NITEMS - 1; i++) {
		/* select an index for indices[i] */
		int x = xrandom (i, NITEMS - 1);

		if (x != i) {
			int tmp = indices [x];
			indices [x] = indices [i];
			indices [i] = tmp;
		}
	}
}

/* threads synchronization */
static int threads_sync_count;
static pthread_mutex_t thread_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t thread_sync = PTHREAD_COND_INITIALIZER;

/* wait for all other threads to reach this execution point */
static inline void synchronize (int *local_count)
{
	(*local_count)++;

	pthread_mutex_lock (&thread_lock);

	threads_sync_count++;
	pthread_cond_broadcast (&thread_sync);

	while (threads_sync_count < *local_count * NTHREADS)
		pthread_cond_wait (&thread_sync, &thread_lock);

	pthread_mutex_unlock (&thread_lock);
}

static void *thread_func (void *arg)
{
	int local_count = 0;
	int *thread_indices = arg;
	int i;

	/* first, all threads try to insert the first ITEMS_PER_THREAD
	   indices */
	for (i = 0; i < ITEMS_PER_THREAD; i++) {
		int size;

		/* this will succeed only in the first thread which gets
		   here */
		list_insert (list, indices [i], &indices [i]);

		if (!list_contains (list, indices [i]))
			fail ("ERROR: list does not contain index %d after insertion", indices [i]);

		size = list_size (list);
		if (size <= i)
			fail ("ERROR: list contains %d items after insertion of %d items", size, i + 1);
	}

	synchronize (&local_count);

	/* now all threads try to remove the first ITEMS_PER_THREAD
	   indices */
	for (i = 0; i < ITEMS_PER_THREAD; i++) {
		int size;

		/* this will succeed only in the first thread which gets
		   here */
		list_remove (list, indices [i]);

		if (list_contains (list, indices [i]))
			fail ("ERROR: list contains index %d after removal", indices [i]);

		size = list_size (list);
		if (size >= ITEMS_PER_THREAD - i)
			fail ("ERROR: list contains %d items after removal of %d items", size, i + 1);
	}

	synchronize (&local_count);

	/* now each thread tries to insert, and then remove, its own
	   block of indices */
	for (i = 0; i < ITEMS_PER_THREAD; i++) {
		int result = list_insert (list, thread_indices [i],
				&thread_indices [i]);
		if (result != 0)
			fail ("list_insert() failed: thread_indices=%d..%d, array index=%d, index=%d, result=%d",
					thread_indices - indices,
					thread_indices - indices + ITEMS_PER_THREAD - 1,
					&thread_indices [i] - indices,
					thread_indices [i], result);
		if (!list_contains (list, thread_indices [i]))
			fail ("list_contains() returned 0 after insertion: thread_indices=%d..%d, array index=%d, index=%d",
					thread_indices - indices,
					thread_indices - indices + ITEMS_PER_THREAD - 1,
					&thread_indices [i] - indices,
					thread_indices [i]);
	}

	for (i = 0; i < ITEMS_PER_THREAD; i++) {
		int result = list_remove (list, thread_indices [i]);
		if (result != 0)
			fail ("list_remove() failed: thread_indices=%d..%d, array index=%d, index=%d, result=%d",
					thread_indices - indices,
					thread_indices - indices + ITEMS_PER_THREAD - 1,
					&thread_indices [i] - indices,
					thread_indices [i], result);
		if (list_contains (list, thread_indices [i]))
			fail ("list_contains() returned 1 after removal: thread_indices=%d..%d, array index=%d, index=%d",
					thread_indices - indices,
					thread_indices - indices + ITEMS_PER_THREAD - 1,
					&thread_indices [i] - indices,
					thread_indices [i]);
	}

	return NULL;
}

int main ()
{
	pthread_t threads [NTHREADS];
	int i, j;

	list = list_alloc ();
	if (!list) {
		errno = ENOMEM;
		perror ("list_alloc");
		return EXIT_FAILURE;
	}

	srand (RAND_SEED);
	init_indices ();

	for (i = 0; i < LOOPS; i++) {
		shuffle_indices ();

		threads_sync_count = 0;
		for (j = 0; j < NTHREADS; j++) {
			int result = pthread_create (&threads [j], NULL,
					thread_func,
					&indices [ITEMS_PER_THREAD * (j + 1)]);
			if (result != 0) {
				errno = result;
				perror ("pthread_create");
				return EXIT_FAILURE;
			}
		}

		for (j = 0; j < NTHREADS; j++)
			pthread_join (threads [j], NULL);
	}

	list_free (&list);
	printf("PASS\n");
	return EXIT_SUCCESS;
}
