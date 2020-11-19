//
// Created by root on 2020/11/8.
// Data Struct Lib For Kernel
//
#ifndef UTILS_H
#define UTILS_H
#include "types.h"
#include "sync.h"

//----------------------Link-------------------------
//1.those action is not allowed:
//  (1)Manual change the node like:     node->data = ***
//  (2)for shared node data,change the value without independent lock(the link lock is only
//       for add node and remove node from link)
//2.You can set lock for link but don`t forget to release it when release link struct
typedef void* link_node_data_t;
//link_list_node
typedef struct link_node {
    struct link_node* next;
    link_node_data_t data;
}link_node_t;

typedef struct link{
    link_node_t * head;
    //Set lock for threads shared Link list
    bool have_lock;
    bool lock_init_flag;
    lock_t lock;
}link_t;

void link_init_without_lock(link_t * link);
void link_init_lock(link_t * link);
link_node_t * link_find_node_by_data(link_t * link,link_node_data_t data);
void link_add_tail(link_t * link,link_node_t * node);
void link_add_head(link_t * link,link_node_t * node);
bool link_remove_by_data(link_t * link,link_node_data_t data);
bool link_remove_by_node(link_t * link,link_node_t * node);
//----------------------Link-------------------------
#endif