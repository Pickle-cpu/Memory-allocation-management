#include "my_malloc.h"

// global variables
single_node_t * free_head = NULL;
unsigned long data_segment_size = 0;

// initial a node
void initialnode(single_node_t * node){

    node->user_start_address = (void *)node + sizeof(single_node_t);
    node->prev = NULL;
    node->next = NULL;

}

// allocate a new space with size of struct + size
single_node_t * asknewspace(size_t size){

    // sbrk for allocation
    single_node_t * node = sbrk(size+sizeof(single_node_t));
    if(node == (void *)-1){
        return NULL;
    }
    // increase the useage of heap
    data_segment_size += size + sizeof(single_node_t);
    // initial
    initialnode(node);
    node->free_status = 0;
    node->n_size = size;

    return node;

}


void replacenode(size_t size, single_node_t * temp, single_node_t * replace){

    // since the remaining still works
    // change the details of replace
    replace->n_size = temp->n_size-size-sizeof(single_node_t);
    replace->free_status = 1;

    if(free_head == temp){

        single_node_t * tempnext = temp->next;

        temp->prev = NULL;
        temp->next = NULL;

        free_head = replace;
        replace->prev = NULL;
        replace->next = tempnext;
        if(tempnext != NULL){
            tempnext->prev = replace;
        }
    }else if(temp->next == NULL){

        single_node_t * tempprev = temp->prev;

        temp->prev = NULL;
        temp->next = NULL;

        tempprev->next = replace;
        replace->prev = tempprev;
        replace->next = NULL;
    }else{

        single_node_t * tempprev = temp->prev;
        single_node_t * tempnext = temp->next;

        temp->prev = NULL;
        temp->next = NULL;
        
        tempprev->next = replace;
        tempnext->prev = replace;
        replace->prev = tempprev;
        replace->next = tempnext;
    }

    // change the details of temp
    temp->n_size = size;
    temp->free_status = 0;
    

}

// splite space into two parts
// first part will be used
// second part will still be in free list and wait to be used later
void splitspace(size_t size, single_node_t * temp){
    
    // check if the remaining still can be used
    if((temp->n_size-size)<=sizeof(single_node_t)){

        // too small so just delete it
        deletenode(temp);
    
    }else{

        // create a new available node
        single_node_t * replace = (single_node_t *)(temp->user_start_address + size);
        initialnode(replace);
        // replace node in free list
        replacenode(size, temp, replace);
        
    }

}

// instead of allocating new space
// we choose to use the space in free list
// int fit will tell us which fit to go
// select free space for first fit
single_node_t * ffselectfree(size_t size){

    single_node_t * temp = free_head;
    while(temp != NULL){
        // try to find the first matched one
        if(temp->n_size >= size){
            splitspace(size,temp);
            return temp;
        }
        temp = temp->next;
    }

    return NULL;

}
// select free space for best fit
single_node_t * bfselectfree(size_t size){

    single_node_t * temp = free_head;
    single_node_t * temptouse = NULL;
    while(temp != NULL){
        // try to find the best matched one
        if(temp->n_size == size){
            temptouse = temp;
            break;
        }else if(temp->n_size > size){
            if((temptouse == NULL) || (temptouse->n_size > temp->n_size)){
                temptouse = temp;
            }
        }
        temp = temp->next;
    }

    if(temptouse != NULL){
        splitspace(size,temptouse);
    }
    
    return temptouse;

}

// delete node from linked list (free list)
void deletenode(single_node_t * node){

    if(free_head == node){
        if(free_head->next == NULL){
            free_head = NULL;
        }else{
            free_head = node->next;
            free_head->prev = NULL;
        }
    }else if(node->next == NULL){
        node->prev->next = NULL;
    }else{
        node->prev->next = node->next;
        node->next->prev = node->prev;
    }

    node->free_status = 0;
    node->prev = NULL;
    node->next = NULL;
}

// insert node to linked list (free list)
void insertnode(single_node_t * node){

    // nothing inside
    if(free_head == NULL){

        free_head = node;
        return;
    }

    // before head
    single_node_t * temp = free_head;
    if((node->user_start_address+node->n_size)<(void *)free_head){

        node->next = temp;
        temp->prev = node;
        free_head = node;
        return;
    }
    
    // at least has one in free list
    temp = free_head;
    while(temp != NULL){

        if((void *)node>=(temp->user_start_address+temp->n_size)){

            if(temp->next==NULL){

                temp->next = node;
                node->prev = temp;
                return;
            }else if((node->user_start_address+node->n_size)<=(void *)temp->next){

                single_node_t * tempnext= temp->next;
                temp->next = node;
                node->prev = temp;
                node->next = tempnext;
                tempnext->prev = node;
                return;
            }
        }

        temp = temp->next;
    }

}

// merge nodes
// situation 1, merge prev and node
// situation 2, merge node and next
// situation 3, merge prev, node, and next
void mergenode(single_node_t * node){

    // check if next is a free one
    if(node->next != NULL && node->next->free_status == 1){
        single_node_t * nodenext = node->next;

        // check if next and node are adjacent
        if((void *)nodenext == (node->user_start_address+node->n_size)){

            node->n_size = node->n_size + sizeof(single_node_t) + nodenext->n_size;
            node->next = nodenext->next;
            if(nodenext->next != NULL){
                nodenext->next->prev = node;
            }

            nodenext->next = NULL;
            nodenext->prev = NULL;

        }

    }

    // check if prev is a free one
    if(node->prev != NULL && node->prev->free_status == 1){
        single_node_t * nodeprev = node->prev;

        // check if prev and node are adjacent
        if((void *)node == (nodeprev->user_start_address+nodeprev->n_size)){

            nodeprev->n_size = nodeprev->n_size + sizeof(single_node_t) + node->n_size;
            nodeprev->next = node->next;
            if(node->next != NULL){
                node->next->prev = nodeprev;
            }
            

            node->next = NULL;
            node->prev = NULL;

        }

    }

}

// return size of entire heap memory
// (this includes memory used to save metadata)
unsigned long get_data_segment_size(){

    return data_segment_size;

}

// size of the free list = (actual
// usable free space + space occupied by metadata)
// of the blocks in free list
unsigned long get_data_segment_free_space_size(){

    
    unsigned long data_segment_free_space_size = 0;
    single_node_t * node = free_head;
    while(node != NULL){
        data_segment_free_space_size += node->n_size + sizeof(single_node_t);
        node = node->next;
    }

    return data_segment_free_space_size;

}


//First Fit malloc/free
void *ff_malloc(size_t size){

    single_node_t * node = ffselectfree(size);

    // cannot find one in free list
    // allocate a new one
    if(node == NULL){
        node = asknewspace(size);
    }
    
    return node->user_start_address;

}
void ff_free(void *ptr){
    
    single_node_t * node = (single_node_t *)(ptr - sizeof(single_node_t));
    node->free_status = 1;
    insertnode(node);
    mergenode(node);

}

//Best Fit malloc/free
void *bf_malloc(size_t size){

    single_node_t * node = bfselectfree(size);

    // cannot find one in free list
    // allocate a new one
    if(node == NULL){
        node = asknewspace(size);
    }

    return node->user_start_address;

}
void bf_free(void *ptr){
    
    single_node_t * node = (single_node_t *)(ptr - sizeof(single_node_t));
    node->free_status = 1;
    insertnode(node);
    mergenode(node);

}



