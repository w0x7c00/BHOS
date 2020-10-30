#include "list.h"
elem_t get_elem(link_list_node_t* link_list_node_ptr){
    return link_list_node_ptr->elem;
}

elem_t get_next(link_list_node_t* link_list_node_ptr){
    return link_list_node_ptr->next;
}