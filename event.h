#ifndef _EVENT_H
#define _EVENT_H

#include <linux/types.h>
#include <linux/wait.h>
#include <linux/list.h>

struct event 
{
	int event_id;
	uid_t UID;			// equal to effective UID of the process creator
	gid_t GID;			// equal to effective GID of the process creator
	int UID_enable;     // (default 1) enable process of equal UID to signal/close the event
	int GID_enable;     // (default 1) enable process of equal GID to signal/close the event

	int sig;            // (born 1) indicate whether the event is signaled by any process (0-no, 1-yes)
	wait_queue_head_t wq_process;	// waiting queue for the event
					
	struct list_head list_event;	// kernel list structure -- connect all the event
};

/* event initialization function (called while boosting kernel) */
void __init doevent_init(void);

#endif
