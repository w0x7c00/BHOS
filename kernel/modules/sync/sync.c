#include "sync.h"
#include "types.h"
#include "printk.h"
#include "kern_log.h"
#include "threads.h"

//-------------------tool-------------------
void sync_tool_sem_init_queue(sync_tool_sem_queue_t* queue){
    queue->head = NULL;
}

//Fail:return NULL
sync_tool_sem_queue_node_t * sync_tool_get(sync_tool_sem_queue_t * queue){
    sync_tool_sem_queue_node_t * probe = queue->head;
    if(probe==NULL){
        return NULL;
    }
    queue->head = probe->next;
    probe->next = NULL;
    return probe;
}

void sync_tool_add(sync_tool_sem_queue_t * queue,sync_tool_sem_queue_node_t * node){
    node->next = NULL;
    sync_tool_sem_queue_node_t * probe = queue->head;
    if(probe==NULL){
        queue->head = node;
    }
    else{
        while (probe->next!=NULL){
            probe = probe->next;
        }
        probe->next = node;
    }
}
bool sync_tool_is_empty(sync_tool_sem_queue_t * queue){
    if(queue->head==NULL){
        return True;
    }
    else{
        return False;
    }
}

//-------------------tool-------------------
void sem_init(sem_t * sem,uint32_t val){
    sync_tool_sem_init_queue(&(sem->wait_queue));
    sem->val =val;
}

void sem_P(sem_t * sem){
    //we should change the shared value:sem,so we have to close the interrupt(atomic action)
    bool condition = cli_condition();
    while (sem->val == 0){
        sync_tool_sem_queue_node_t node;
        node.thread = get_running_progress();
        //add to wait queue
        sync_tool_add(&sem->wait_queue,&node);
        //block self
        thread_block();
    }
    sem->val--;
    sti_condition(condition);
}

void sem_V(sem_t * sem){
    bool condition = cli_condition();
    if(sync_tool_is_empty(&sem->wait_queue)){
        // do nothing
    }
    else{
        sync_tool_sem_queue_node_t * node = sync_tool_get(&sem->wait_queue);
        if(node!=NULL){
            thread_wakeup(node->thread);
        }
    }
    sem->val++;
    sti_condition(condition);
}

void lock_init(lock_t * lock){
    sem_init(&lock->sem,1);
    lock->holder =NULL;
}

void lock_acquire(lock_t * lock){
    bool condition = cli_condition();
    if(lock->holder != get_running_progress()){
        sem_P(&lock->sem);
        lock->holder = get_running_progress();
    }
    sti_condition(condition);
}

void lock_release(lock_t * lock){
    bool condition = cli_condition();
    if(lock->holder == get_running_progress()){
        lock->holder = NULL;
        sem_V(&lock->sem);
    }
    sti_condition(condition);
}