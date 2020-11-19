#ifndef SYNC_H
#define SYNC_H
#include "types.h"
#include "interrupt.h"
#include "threads.h"

//-------some tool function and struct-------
typedef struct sync_tool_sem_queue_node{
    TCB_t * thread;
    struct sync_tool_sem_queue_node * next;
}sync_tool_sem_queue_node_t;

typedef struct sync_tool_sem_queue {
    sync_tool_sem_queue_node_t * head;
    //no lock
} sync_tool_sem_queue_t;

void sync_tool_sem_init_queue(sync_tool_sem_queue_t* queue);
sync_tool_sem_queue_node_t * sync_tool_get(sync_tool_sem_queue_t *queue);
void sync_tool_add(sync_tool_sem_queue_t * queue,sync_tool_sem_queue_node_t * node);
bool sync_tool_is_empty(sync_tool_sem_queue_t * queue);
//-------some tool function and struct-------


//--------------Semaphore And Lock-----------------
typedef struct sem{
    uint32_t val;
    sync_tool_sem_queue_t wait_queue;
} sem_t;

typedef struct lock{
    TCB_t * holder;     //holder thread
    sem_t sem;       //the sem for lock(val = 1)
} lock_t;

void sem_init(sem_t * sem,uint32_t val);
//signal P (down and wake up)
void sem_P(sem_t* sem);
//signal V (up and sleep)
void sem_V(sem_t* sem);
void lock_init(lock_t * lock);
void lock_acquire(lock_t * lock);
void lock_release(lock_t * lock);
#endif