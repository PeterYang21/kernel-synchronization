#include <linux/kernel.h>
#include <linux/events.h>
#include <linux/spinlock.h>
#include <linux/slab.h>
#include <linux/syscalls.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/mutex.h>

#define MAX_EVENTS 100000

/* event related global variables */
static int next_event_id;
static int num_event;
static LIST_HEAD(events);

/* private lock for each event */
struct mutex locks[MAX_EVENTS];

/* global lock for global variables */
static DEFINE_MUTEX(doevent_lock);

/* 
 * init event info, called when boosting kernel
 */
void __init doevent_init(void)
{
    next_event_id = 0;
    num_event = 0;
    printk("Init Events, succeed.\n");
}

/* 
 * sys_doeventopen - creates a new event
 * return event ID on success, -1 on failure. 
 */
SYSCALL_DEFINE0(doeventopen)
{
    struct event *new_event = (struct event *)kmalloc(sizeof(struct event), GFP_KERNEL);
    
    /* new event creation fails */
    if(new_event == NULL)
        return -1;

    /* assign content to new event */
    new_event->UID = sys_geteuid();
    new_event->GID = sys_getegid();
    new_event->UID_enable = 1;
    new_event->GID_enable = 1;
    new_event->sig = 1;
    init_waitqueue_head(&(new_event->wq_process));
    INIT_LIST_HEAD(&new_event->list_event);

    /* operations that use global variables should be locked */
    mutex_lock(&doevent_lock);
    if(next_event_id>MAX_EVENTS){
        printk("Create too many events!\n");
        return -1;
    }

    new_event->event_id = next_event_id;
    next_event_id++;
    list_add(&(new_event->list_event), &events);

    mutex_init(&locks[new_event->event_id]);
    num_event++;
    mutex_unlock(&doevent_lock); 

    return new_event->event_id;
}

/* 
 * helper function
 * return event that matches the specified event_id
 */
static struct event * get_event(int event_id)
{
    struct event *evt;
    list_for_each_entry(evt, &events, list_event){
        if(evt->event_id==event_id)
            return evt;
    }

    return NULL;
}

/* 
 * helper function
 * implement access control to different operations on events
 * operation id:
 * #0 close
 * #1 wait_sig
 * #2 ch
 */
static bool access_control(struct event * evt, int operate)
{
    uid_t UID_proc = sys_geteuid();
    gid_t GID_proc = sys_getegid();
    
    // rule 1-2, 7
    if(UID_proc==0)
        return true;
    
    if(UID_proc==evt->UID){
        // rule 3-4
        if(operate==0 || operate==1)
            if(evt->UID_enable==1)
                return true;
        // rule 8
        else
            return true;
    }
    else if(GID_proc==evt->GID){
        // rule 5-6
        if(operate==0 || operate==1)
            if(evt->GID_enable==1)
                return true;
    }
    else
        return false;
}

/* 
 * helper function
 * return the length of a kernel list
 */
static int get_list_length(struct list_head * lh)
{
    int len = 0;

    if(lh==NULL)
        return -1;
    
    struct list_head * travs;
    list_for_each(travs, lh)
        len++;
    return len;
} 

/* 
 * sys_doeventclose - destroy the event with the given event ID
 * return number of processes signaled on success and -1 on failure.
 */
SYSCALL_DEFINE1(doeventclose, int, event_id)
{
    bool acc;   // permission to close
    int num_wait_process;
    mutex_lock(&doevent_lock);
    
    struct event *evt = get_event(event_id);
    if(evt==NULL){
        mutex_unlock(&doevent_lock);
        return -1;
    }

    acc = access_control(evt, 0);
    if(!acc){
        mutex_unlock(&doevent_lock);
        return -1;
    }
    mutex_unlock(&doevent_lock);

    mutex_lock(&locks[event_id]);
    // count the num of process waiting
    num_wait_process = get_list_length(&(evt->wq_process.task_list));
    evt->sig = 1;
    // wake up only the processes in interruptible sleep
    wake_up_interruptible(&(evt->wq_process)); 

    list_del(&(evt->list_event));
    kfree(evt);
    num_event--;
    // mutex_unlock(&doevent_lock);
    mutex_unlock(&locks[event_id]);

    return num_wait_process;
} 

/* 
 * sys_doeventwait - blocks process until the event is signaled
 * return 1 on success and -1 on failure.
 */
SYSCALL_DEFINE1(doeventwait, int, event_id)
{
    bool acc;   // right to close

    mutex_lock(&doevent_lock);
    
    struct event *evt = get_event(event_id);
    if(evt == NULL){
        mutex_unlock(&doevent_lock);
        return -1;
    }

    acc = access_control(evt, 1);
    if(!acc){
        mutex_unlock(&doevent_lock);
        return -1;
    }
    mutex_unlock(&doevent_lock);

    mutex_lock(&locks[event_id]);
    evt->sig = 0;
    // mutex_unlock(&doevent_lock);
    mutex_unlock(&locks[event_id]);
    // add process to the wq
    wait_event_interruptible(evt->wq_process, evt->sig==1);
    
    return 1;
}

/* 
 * sys_doeventsig - unblocks all waiting processes
 * return number of processes signaled on success and -1 on failure.
 */
SYSCALL_DEFINE1(doeventsig, int, event_id)
{
    bool acc;   // right to close
    int num_wait_process;

    mutex_lock(&doevent_lock);
    
    struct event *evt = get_event(event_id);
    if(evt==NULL){
        mutex_unlock(&doevent_lock);
        return -1;
    }

    acc = access_control(evt, 1);
    if(!acc){
        mutex_unlock(&doevent_lock);
        return -1;
    }
    mutex_unlock(&doevent_lock);
    
    mutex_lock(&locks[event_id]);
    /* count the num of process waiting */
    num_wait_process = get_list_length(&(evt->wq_process.task_list));
    evt->sig = 1;
    wake_up_interruptible(&(evt->wq_process));
    // mutex_unlock(&doevent_lock);
    mutex_unlock(&locks[event_id]);

    return num_wait_process;
}

/* 
 * sys_doeventinfo - fills in the event_ids array with the current set of active event IDs
 * returns the number of active events and -1 on failure.
 */
SYSCALL_DEFINE2(doeventinfo, int, num, int *, event_ids)
{
    int ret;

    if(num < num_event)
        return -1;
    if(event_ids == NULL)
        return num_event;
    if(num_event==0)
        return num_event;

    int *event_ids_from_ker = (int *)kmalloc(num_event*sizeof(int), GFP_KERNEL);

    if(event_ids_from_ker==NULL)
        return -1;

    mutex_lock(&doevent_lock);
    /* traverse the event list */
    int idx = 0;
    struct event *evt;
    list_for_each_entry(evt, &events, list_event){
        event_ids_from_ker[idx] = evt->event_id;
        idx++;
    }
    mutex_unlock(&doevent_lock);

    /* copy data to user space */
    ret = copy_to_user(event_ids, event_ids_from_ker, num_event*sizeof(int));
    if(ret!=0)
        return -1;

    return idx;
}

/* 
 * sys_doeventchown - change the UID and GID of the event to the specified values
 * returns -1 on failure.
 */
SYSCALL_DEFINE3(doeventchown, int, event_id, uid_t, UID, gid_t, GID)
{
    bool acc;   // right to close

    mutex_lock(&doevent_lock);
    
    struct event *evt = get_event(event_id);
    if(evt==NULL){
        mutex_unlock(&doevent_lock);
        return -1;
    }

    acc = access_control(evt, 2);
    if(!acc){
        mutex_unlock(&doevent_lock);
        return -1;
    }
    mutex_unlock(&doevent_lock);

    mutex_lock(&locks[event_id]);
    evt->UID = UID;
    evt->GID = GID;
    // mutex_unlock(&doevent_lock);
    mutex_unlock(&locks[event_id]);

    return 1;
}

/* 
 * sys_doeventchmod - change the UID_enable and GID_enable
 * returns -1 on failure.
 */
SYSCALL_DEFINE3(doeventchmod, int, event_id, int, UID_enable, int, GID_enable)
{
    bool acc;   // right to close

    mutex_lock(&doevent_lock);
    
    struct event *evt = get_event(event_id);
    if(evt==NULL){
        mutex_unlock(&doevent_lock);
        return -1;
    }

    acc = access_control(evt, 2);
    if(!acc){
        mutex_unlock(&doevent_lock);
        return -1;
    }
    mutex_unlock(&doevent_lock);

    mutex_lock(&locks[event_id]);
    evt->UID_enable = UID_enable;
    evt->GID_enable = GID_enable;
    // mutex_unlock(&doevent_lock);
    mutex_unlock(&locks[event_id]);

    return 1;
}

/* 
 * sys_doeventstat - place all the event info into memory pointed by the pointers
 * returns -1 on failure.
 */
SYSCALL_DEFINE5(doeventstat, int, event_id, uid_t *, UID, gid_t *, GID, int *, UID_enable, int *, GID_enable)
{
    int ret[4];

    mutex_lock(&doevent_lock);
    struct event *evt = get_event(event_id);
    mutex_unlock(&doevent_lock);

    if(evt==NULL)
        return -1;

    ret[0] = copy_to_user(UID, &(evt->UID), sizeof(uid_t)); 
    ret[1] = copy_to_user(GID, &(evt->UID), sizeof(uid_t)); 
    ret[2] = copy_to_user(UID_enable, &(evt->UID_enable), sizeof(int)); 
    ret[3] = copy_to_user(GID_enable, &(evt->GID_enable), sizeof(int)); 

    /* all 3 should return 0 when succeeding */
    if((ret[0] + ret[1] + ret[2] + ret[3])!=0)
        return -1;

    return 1;