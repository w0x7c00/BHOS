#include "utils.h"
#include "types.h"
#include "sync.h"

static void _lock_acquire(lock_t * lock,bool flag){
    if(flag){
        lock_acquire(lock);
    }
}

static void _lock_release(lock_t * lock,bool flag){
    if(flag){
        lock_release(lock);
    }
}

static bool _remove_next_node(link_t * link,link_node_t * node){
    if(node->next == NULL){
        return False;
    }
    else{
        //remove running
        _lock_acquire(&link->lock,link->lock_init_flag);
        node->next = NULL;
        _lock_release(&link->lock,link->lock_init_flag);
        return True;
    }
}

void link_init_without_lock(link_t * link){
    link->have_lock = False;
    link->lock_init_flag = False;
}

void link_init_lock(link_t * link){
    link->have_lock = True;
    lock_init(&link->lock);
    link->lock_init_flag = True;
}

link_node_t * link_find_node_by_data(link_t * link,link_node_data_t data){
    if(link->head == NULL){
        return NULL;
    }
    else{
        for(link_node_t  * probe = link->head;probe!=NULL;probe=probe->next){
            if (probe->data ==data){
                return data;
            }
        }
        return NULL;
    }
}

void link_add_tail(link_t * link,link_node_t * node){
    if(link->head == NULL){
        _lock_acquire(&link->lock,link->lock_init_flag);
        link->head = node;
        _lock_release(&link->lock,link->lock_init_flag);
    }
    else{
        //find the tail
        link_node_t  * probe = link->head;
        for(;probe->next!=NULL;probe = probe->next);
        _lock_acquire(&link->lock,link->lock_init_flag);
        probe->next = node;
        _lock_release(&link->lock,link->lock_init_flag);
    }
}

void link_add_head(link_t * link,link_node_t * node){
    _lock_acquire(&link->lock,link->lock_init_flag);
    link_node_t * probe =link->head;
    link->head = node;
    node->next = probe;
    _lock_release(&link->lock,link->lock_init_flag);
}

//find the first eq value to remove
//return True(success remove one)
bool link_remove_by_data(link_t * link,link_node_data_t data){
    //search target
    link_node_t * probe = link->head;
    if(probe==NULL){
        return False;
    }
    for(;probe->next!=NULL;probe=probe->next){
        if(probe->next->data==data){
            return _remove_next_node(link,probe);
        }
    }
    return False;
}

bool link_remove_by_node(link_t * link,link_node_t * node){
    link_node_t * probe = link->head;
    if(probe==NULL){
        return False;
    }
    for(;probe->next!=NULL;probe=probe->next){
        if(probe->next==node){
            return _remove_next_node(link,probe);
        }
    }
    return False;
}