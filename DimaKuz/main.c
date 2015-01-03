#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mylist.h>

#ifndef FUZZ
#define FUZZ 1
#endif
#ifndef SIM
#define SIM 1
#endif

struct test_def {
	int (*func)();
	const char *name;
};

static int test_alloc_free_success() {
	int success = 0;
	linked_list_t **list = list_alloc();
	if (list != NULL) {
		list_free(&list);
		success = list == NULL;
	}
	return success;
}

static int test_free_invalid() {
	linked_list_t **list = NULL;
	list_free(NULL);
	list_free(&list);
	return 1;
}

static int test_insert_invalid1() {
	return list_insert(NULL, 1, NULL) > 0;
}

static int test_insert_invalid2() {
	linked_list_t *list = NULL;
	return list_insert(&list, 1, NULL) > 0;
}

int with_list(int (*func)(linked_list_t **)) {
	linked_list_t **list = list_alloc();
	int res = func(list);
	list_free(&list);
	return res;
}

static int _test_insert_success(linked_list_t **list) {
	return list_insert(list, 0, "one") == 0;
}
static int test_insert_success() {
	return with_list(_test_insert_success);
}

static int _test_insert_range_in_order(linked_list_t **list) {
	int i = 0;
	for (i = 0; i < 100; i++) {
		if (list_insert(list, i, NULL))
			return 0;
	}
	return 1;
}
static int test_insert_range_in_order() {
	return with_list(_test_insert_range_in_order);
}

static int _test_insert_range_reverse_order(linked_list_t **list) {
	int i = 0;
	for (i = 0; i < 100; i++) {
		if (list_insert(list, 100 - i, NULL))
			return 0;
	}
	return 1;
}
static int test_insert_range_reverse_order() {
	return with_list(_test_insert_range_reverse_order);
}

static int _test_insert_range_interleaved_order(linked_list_t **list) {
	int i = 0;
	for (i = 0; i < 100; i++) {
		if (list_insert(list, i * 2, NULL))
			return 0;
	}
	for (i = 0; i < 100; i++) {
		if (list_insert(list, 200 - i * 2 + 1, NULL))
			return 0;
	}
	return 1;
}
static int test_insert_range_interleaved_order() {
	return with_list(_test_insert_range_interleaved_order);
}

static int _test_insert_duplicate(linked_list_t **list) {
	list_insert(list, 1, NULL);
	return 1 == list_insert(list, 1, NULL);
}
static int test_insert_duplicate() {
	return with_list(_test_insert_duplicate);
}

static int test_remove_invalid1() {
	return list_remove(NULL, 1) > 0;
}

static int test_remove_invalid2() {
	linked_list_t *list = NULL;
	return list_remove(&list, 1) > 0;
}

static int _test_remove_success(linked_list_t **list) {
	list_insert(list, 1, NULL);
	return list_remove(list, 1) == 0;
}
static int test_remove_success() {
	return with_list(_test_remove_success);
}

static int _test_remove_failure(linked_list_t **list) {
	return list_remove(list, 1) > 0;
}
static int test_remove_failure() {
	return with_list(_test_remove_failure);
}

static int _test_remove_head(linked_list_t **list) {
	list_insert(list, 1, NULL);
	list_insert(list, 2, NULL);
	list_insert(list, 3, NULL);
	return list_remove(list, 1) == 0;
}
static int test_remove_head() {
	return with_list(_test_remove_head);
}

static int _test_remove_middle(linked_list_t **list) {
	list_insert(list, 1, NULL);
	list_insert(list, 2, NULL);
	list_insert(list, 3, NULL);
	return list_remove(list, 2) == 0;
}
static int test_remove_middle() {
	return with_list(_test_remove_middle);
}

static int _test_remove_tail(linked_list_t **list) {
	list_insert(list, 1, NULL);
	list_insert(list, 2, NULL);
	list_insert(list, 3, NULL);
	return list_remove(list, 3) == 0;
}
static int test_remove_tail() {
	return with_list(_test_remove_tail);
}

static int test_contains_invalid1() {
	return list_contains(NULL, 1) == 0;
}

static int test_contains_invalid2() {
	linked_list_t *list = NULL;
	return list_contains(&list, 1) == 0;
}

static int _test_contains_present(linked_list_t **list) {
	list_insert(list, 1, NULL);
	return list_contains(list, 1) == 1;
}
static int test_contains_present() {
	return with_list(_test_contains_present);
}

static int _test_contains_not_present(linked_list_t **list) {
	return list_contains(list, 1) == 0;
}
static int test_contains_not_present() {
	return with_list(_test_contains_not_present);
}

static int test_size_invalid1() {
	return list_size(NULL) == -1;
}

static int test_size_invalid2() {
	linked_list_t *list = NULL;
	return list_size(&list) == -1;
}

static int _test_size_empty(linked_list_t **list) {
	return list_size(list) == 0;
}
static int test_size_empty() {
	return with_list(_test_size_empty);
}

static int _test_size_nonempty(linked_list_t **list) {
	list_insert(list, 1, NULL);
	list_insert(list, 2, NULL);
	list_insert(list, 3, NULL);
	return list_size(list) == 3;
}
static int test_size_nonempty() {
	return with_list(_test_size_nonempty);
}

static int test_node_compute_invalid1() {
	return list_node_compute(NULL, 1, NULL, NULL) > 0;
}

static int test_node_compute_invalid2() {
	linked_list_t *list = NULL;
	return list_node_compute(&list, 1, NULL, NULL) > 0;
}

void *get_ptr(void *ptr) {
	return ptr;
}
static int _test_node_compute_not_present(linked_list_t **list) {
	void *res;
	return list_node_compute(list, 1, get_ptr, &res) > 0;
}
static int test_node_compute_not_present() {
	return with_list(_test_node_compute_not_present);
}

static int _test_node_compute_present_ret_code(linked_list_t **list) {
	void *res;
	list_insert(list, 1, NULL);
	return list_node_compute(list, 1, get_ptr, &res) == 0;
}
static int test_node_compute_present_ret_code() {
	return with_list(_test_node_compute_present_ret_code);
}

static int _test_node_compute_present_result(linked_list_t **list) {
	void *res;
	const char *one = "one";
	list_insert(list, 1, (void *) one);
	list_node_compute(list, 1, get_ptr, &res);
	return one == (const char *) res;
}
static int test_node_compute_present_result() {
	return with_list(_test_node_compute_present_result);
}

static int test_update_node_invalid1() {
	return list_update_node(NULL, 1, NULL) > 0;
}
static int test_update_node_invalid2() {
	linked_list_t *list = NULL;
	return list_update_node(&list, 1, NULL) > 0;
}

static int _test_update_node_not_present(linked_list_t **list) {
	return list_update_node(list, 1, NULL) > 0;
}
static int test_update_node_not_present() {
	return with_list(_test_update_node_not_present);
}

static int _test_update_node_present_ret_code(linked_list_t **list) {
	list_insert(list, 1, NULL);
	return list_update_node(list, 1, "one") == 0;
}
static int test_update_node_present_ret_code() {
	return with_list(_test_update_node_present_ret_code);
}

static int _test_update_node_present_contents(linked_list_t **list) {
	const char *one = "one";
	void *res;
	list_insert(list, 1, NULL);
	list_update_node(list, 1, (void *) one);
	list_node_compute(list, 1, get_ptr, &res);
	return one == (const char *) res;
}
static int test_update_node_present_contents() {
	return with_list(_test_update_node_present_contents);
}

static int test_batch_invalid1() {
	list_batch(NULL, 0, NULL);
	return 1;
}
static int test_batch_invalid2() {
	linked_list_t *list = NULL;
	list_batch(&list, 0, NULL);
	return 1;
}

static int _test_batch_insert(linked_list_t **list) {
	op_t op;
	op.index = 1;
	op.data = NULL;
	op.op = INSERT;

	list_batch(list, 1, &op);
	return list_contains(list, 1) && (op.result == 0);
}
static int test_batch_insert() {
	return with_list(_test_batch_insert);
}

static int _test_batch_remove(linked_list_t **list) {
	op_t op;
	op.index = 1;
	op.data = NULL;
	op.op = REMOVE;

	list_insert(list, 1, NULL);
	list_batch(list, 1, &op);
	return list_contains(list, 1) == 0 && (op.result == 0);
}
static int test_batch_remove() {
	return with_list(_test_batch_remove);
}

static int _test_batch_update(linked_list_t **list) {
	const char *one = "one";
	void *res;
	op_t op;
	op.index = 1;
	op.data = (void *) one;
	op.op = UPDATE;

	list_insert(list, 1, NULL);
	list_batch(list, 1, &op);
	list_node_compute(list, 1, get_ptr, &res);
	return one == (const char *) res && (op.result == 0);
}
static int test_batch_update() {
	return with_list(_test_batch_update);
}

static int _test_batch_contains(linked_list_t **list) {
	op_t op[2];
	op[0].index = 1;
	op[0].op = CONTAINS;
	op[1].index = 2;
	op[1].op = CONTAINS;

	list_insert(list, 1, NULL);
	list_batch(list, 2, op);
	return (op[0].result == 1) && (op[1].result == 0);
}
static int test_batch_contains() {
	return with_list(_test_batch_contains);
}

static int _test_batch_compute(linked_list_t **list) {
	const char *one = "one";
	op_t op;
	op.op = COMPUTE;
	op.index = 1;
	op.compute_func = get_ptr;

	list_insert(list, 1, (void *) one);
	list_batch(list, 1, &op);
	return one == (const char *) (*(int **) &op.result);
}
static int test_batch_compute() {
	return with_list(_test_batch_compute);
}

static int _pipes[2];
static void *long_compute(void *data) {
	write(_pipes[1], "1", 1);
	close(_pipes[1]);
	sleep(2);
	return NULL;
}

static void *compute_thread(void *data) {
	void *res;
	int ret = list_node_compute(
		(linked_list_t **) data,
		30,
		long_compute,
		&res
	);
	return (void *) *(int **) &ret;
}

static void *free_thread(void *data) {
	linked_list_t **list = (linked_list_t **) data;
	char c;
	read(_pipes[0], &c, 1);
	close(_pipes[0]);
	list_free(&list);
	return list;
}

static int with_freeing_list(int (*func)(linked_list_t **)) {
	linked_list_t **list = list_alloc();
	pthread_t compute_thread_handle;
	pthread_t free_thread_handle;
	void *thread_ret;
	int ret = 1;
	int i;
	if (!list)
		return 0;

	if (pipe(_pipes)) {
		list_free(&list);
		return 0;
	}

	for (i = 0; i < 100; i++) {
		list_insert(list, i, NULL);
	}

	if (
		pthread_create(
			&free_thread_handle,
			NULL,
			free_thread,
			(void *) list
		)
	) {
		close(_pipes[0]);
		close(_pipes[1]);
		list_free(&list);
		return 0;
	}

	if (
		pthread_create(
			&compute_thread_handle,
			NULL,
			compute_thread,
			(void *) list
		)
	) {
		close(_pipes[0]);
		close(_pipes[1]);
		list_free(&list);
		return 0;
	}

	sleep(1);
	ret = func(list);
	pthread_join(compute_thread_handle, &thread_ret);
	if (thread_ret)
		ret = 0;
	pthread_join(free_thread_handle, &thread_ret);
	if (thread_ret)
		ret = 0;

	return ret;
}

static int do_nothing(linked_list_t **list) {
	return 1;
}

static int test_free_while_active() {
	return with_freeing_list(do_nothing);
}

static int _test_free_while_freeing(linked_list_t **list) {
	list_free(&list);
	return 1;
}
static int test_free_while_freeing() {
	return with_freeing_list(_test_free_while_freeing);
}

static int _test_size_while_freeing(linked_list_t **list) {
	return list_size(list) == -1;
}
static int test_size_while_freeing() {
	return with_freeing_list(_test_size_while_freeing);
}

static int _test_insert_while_freeing(linked_list_t **list) {
	return list_insert(list, 1000, NULL) > 0;
}
static int test_insert_while_freeing() {
	return with_freeing_list(_test_insert_while_freeing);
}

static int _test_remove_while_freeing(linked_list_t **list) {
	return list_remove(list, 10) > 0;
}
static int test_remove_while_freeing() {
	return with_freeing_list(_test_remove_while_freeing);
}

static int _test_contains_while_freeing(linked_list_t **list) {
	return list_contains(list, 60) == 0;
}
static int test_contains_while_freeing() {
	return with_freeing_list(_test_contains_while_freeing);
}

static int _test_update_node_while_freeing(linked_list_t **list) {
	return list_update_node(list, 2, NULL) > 0;
}
static int test_update_node_while_freeing() {
	return with_freeing_list(_test_update_node_while_freeing);
}

static int _test_node_compute_while_freeing(linked_list_t **list) {
	void *res;
	return list_node_compute(list, 2, get_ptr, &res) > 0;
}
static int test_node_compute_while_freeing() {
	return with_freeing_list(_test_node_compute_while_freeing);
}

static int _test_batch_while_freeing(linked_list_t **list) {
	op_t ops[100];
	int i, ret = 1;

	for (i = 0; i < 100; i++) {
		ops[i].op = UPDATE;
		ops[i].index = i;
		ops[i].data = NULL;
		ops[i].result = 44;
	}
	list_batch(list, 100, ops);
	for (i = 0; i < 100; i++) {
		if (ops[i].result != 44)
			ret = 0;
	}
	return ret;
}
static int test_batch_while_freeing() {
	return with_freeing_list(_test_batch_while_freeing);
}


#if FUZZ == 1
static int _test_fuzz(linked_list_t **list) {
	int i = 0;
	for (i = 0; i < 400; i++) {
		op_t ops[500];
		int j;
		for (j = 0; j < 500; j++) {
			ops[j].op = rand() % 5;
			ops[j].index = rand() % 100;
			ops[j].compute_func = get_ptr;
			ops[j].data = NULL;
		}
		list_batch(list, 500, ops);
		list_size(list);
	}
	return 1;
}
static int test_fuzz() {
	return with_list(_test_fuzz);
}

static int _test_fuzz_insert_only(linked_list_t **list) {
	int i = 0;
	for (i = 0; i < 40; i++) {
		op_t ops[500];
		int j;
		for (j = 0; j < 500; j++) {
			ops[j].op = INSERT;
			ops[j].index = rand() % 10000;
			ops[j].data = NULL;
		}
		list_batch(list, 500, ops);
		list_size(list);
	}
	return 1;
}
static int test_fuzz_insert_only() {
	return with_list(_test_fuzz_insert_only);
}
#endif //FUZZ

#if SIM == 1
#define SIM_SIZE 1000
#define SIM_ITERS 100
#define SIM_THREADS 400
static int sim_list_present[SIM_SIZE];
static void *sim_list_data[SIM_SIZE];
static int sim_size = 0;
static int _test_simulation(linked_list_t **list) {
	const char *numbers[] = {"zero", "one", "two", "three", "four"};
	int i = 0;
	memset(sim_list_present, 0, sizeof (sim_list_present));
	memset(sim_list_data, 0, sizeof (sim_list_data));

	for (i = 0; i < SIM_ITERS; i++) {
		op_t ops[SIM_THREADS];
		int j;
		for (j = 0; j < SIM_THREADS; j++) {
			int index;
			int unique;
			int op = rand() % 3;
			int data_index = rand() % 5;

			// O(huge) unique random generator
			do {
				int k;
				unique = 1;
				index = rand() % SIM_SIZE;
				for (k = 0; k < j; k++) {
					if (ops[k].index == index) {
						unique = 0;
						break;
					}
				}
			} while (!unique);

			switch (op) {
			case 0:
				ops[j].op = INSERT;
				if (!sim_list_present[index]) {
					sim_list_present[index] = 1;
					sim_list_data[index] = (void *) numbers[data_index];
					sim_size++;
				}
				break;
			case 1:
				ops[j].op = REMOVE;
				if (sim_list_present[index]) {
					sim_list_present[index] = 0;
					sim_size--;
				}
				break;

			case 2:
				ops[j].op = UPDATE;
				if (sim_list_present[index]) {
					sim_list_data[index] = (void *) numbers[data_index];
				}
			}
			ops[j].index = index;
			ops[j].compute_func = get_ptr;
			ops[j].data = (void *) numbers[data_index];
		}
		list_batch(list, SIM_THREADS, ops);
		if (list_size(list) != sim_size)
			return 0;
		for (j = 0; j < SIM_SIZE; j++) {
			if (sim_list_present[j] != list_contains(list, j))
				return 0;
			if (sim_list_present[j]) {
				void *res;
				list_node_compute(list, j, get_ptr, &res);
				if (sim_list_data[j] != (const char *) res) {
					return 0;
				}
			}
		}
	}
	return 1;
}

static int test_simulation() {
	return with_list(_test_simulation);
}
#endif // SIM
#define DEFINE_TEST(func) { func, #func }

struct test_def tests[] = {
	DEFINE_TEST(test_alloc_free_success),
	DEFINE_TEST(test_free_invalid),
	DEFINE_TEST(test_insert_invalid1),
	DEFINE_TEST(test_insert_invalid2),
	DEFINE_TEST(test_insert_success),
	DEFINE_TEST(test_insert_range_in_order),
	DEFINE_TEST(test_insert_range_reverse_order),
	DEFINE_TEST(test_insert_range_interleaved_order),
	DEFINE_TEST(test_insert_duplicate),
	DEFINE_TEST(test_remove_invalid1),
	DEFINE_TEST(test_remove_invalid2),
	DEFINE_TEST(test_remove_success),
	DEFINE_TEST(test_remove_failure),
	DEFINE_TEST(test_remove_head),
	DEFINE_TEST(test_remove_middle),
	DEFINE_TEST(test_remove_tail),
	DEFINE_TEST(test_contains_invalid1),
	DEFINE_TEST(test_contains_invalid2),
	DEFINE_TEST(test_contains_present),
	DEFINE_TEST(test_contains_not_present),
	DEFINE_TEST(test_size_invalid1),
	DEFINE_TEST(test_size_invalid2),
	DEFINE_TEST(test_size_empty),
	DEFINE_TEST(test_size_nonempty),
	DEFINE_TEST(test_node_compute_invalid1),
	DEFINE_TEST(test_node_compute_invalid2),
	DEFINE_TEST(test_node_compute_not_present),
	DEFINE_TEST(test_node_compute_present_ret_code),
	DEFINE_TEST(test_node_compute_present_result),
	DEFINE_TEST(test_update_node_invalid1),
	DEFINE_TEST(test_update_node_invalid2),
	DEFINE_TEST(test_update_node_not_present),
	DEFINE_TEST(test_update_node_present_ret_code),
	DEFINE_TEST(test_update_node_present_contents),
	DEFINE_TEST(test_batch_invalid1),
	DEFINE_TEST(test_batch_invalid2),
	DEFINE_TEST(test_batch_insert),
	DEFINE_TEST(test_batch_remove),
	DEFINE_TEST(test_batch_update),
	DEFINE_TEST(test_batch_contains),
	DEFINE_TEST(test_batch_compute),
	DEFINE_TEST(test_free_while_active),
	DEFINE_TEST(test_free_while_freeing),
	DEFINE_TEST(test_size_while_freeing),
	DEFINE_TEST(test_insert_while_freeing),
	DEFINE_TEST(test_remove_while_freeing),
	DEFINE_TEST(test_contains_while_freeing),
	DEFINE_TEST(test_update_node_while_freeing),
	DEFINE_TEST(test_node_compute_while_freeing),
	DEFINE_TEST(test_batch_while_freeing),
#if FUZZ == 1
	DEFINE_TEST(test_fuzz),
	DEFINE_TEST(test_fuzz_insert_only),
#endif
#if SIM == 1
	DEFINE_TEST(test_simulation),
#endif
	{ NULL, "The end" },
};

int main() {
	struct test_def *current = &tests[0];
	while (current->func) {
		printf("%-35s:\t%s\n",
		       current->name,
		       (1 == current->func()) ? "PASS" : "FAIL");
		current++;
	};
	return 0;
}
