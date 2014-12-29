#ifndef NODE_H_
#define NODE_H_

struct node_t;
typedef struct node_t node_t;

struct node_t{
	node_t* prev;
	node_t* next;
	int index;
	void* data;
};


#endif /* NODE_H_ */
