#ifndef LIST_H
#define LIST_H

typedef void* elem_t;

//链表节点
typedef struct link_list_node {
    struct link_list_node* next;
    elem_t* elem;
}link_list_node_t;

elem_t get_elem(link_list_node_t* link_list_node_ptr);
elem_t get_next(link_list_node_t* link_list_node_ptr);


#endif