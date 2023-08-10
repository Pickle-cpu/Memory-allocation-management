#ifndef __MY_MALLOC_H__
#define __MY_MALLOC_H__

#include <stdio.h>
#include <unistd.h>

// struct for node
// element of linked list
struct single_node {
  void * user_start_address;
  size_t n_size;
  size_t free_status;
  struct single_node * prev;
  struct single_node * next;
};
typedef struct single_node single_node_t;

// useful methods
void initialnode(single_node_t * node);
single_node_t * asknewspace(size_t size);
void replacenode(size_t size, single_node_t * temp, single_node_t * replace);
void splitspace(size_t size, single_node_t * temp);
single_node_t * ffselectfree(size_t size);
single_node_t * bfselectfree(size_t size);
void deletenode(single_node_t * node);
void insertnode(single_node_t * node);
void mergenode(single_node_t * node);

unsigned long get_data_segment_size(); //in bytes
unsigned long get_data_segment_free_space_size(); //in byte

//First Fit malloc/free
void *ff_malloc(size_t size);
void ff_free(void *ptr);
//Best Fit malloc/free
void *bf_malloc(size_t size);
void bf_free(void *ptr);

#endif