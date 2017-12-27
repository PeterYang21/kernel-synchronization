#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>


void test_no_event(void);
void test_no_task_waiting(void);
void test_multiTask_eventSignaled(void);
void test_multiEvents_open(void);
void test_waitingPs_eventClosed(void);
void test_eventchown(void);
void test_eventchmod(void);

void *openEvent(void *);
void *closeEvent(void *);

int events[100];

int main(int argc, char **argv){

    printf("\n******************************\n");
    printf("Test 1 - No event exists, but call: \n");
    test_no_event();
    
    printf("\n******************************\n");
    printf("Test 2 - Create 1 event and immediately sig.\n");
    test_no_task_waiting();

    printf("\n******************************\n");
    printf("Test 3 - Create 20 events simultaneously.\n");
	test_multiEvents_open();

    printf("\n******************************\n");
    printf("Test 4 - Multitask waiting while signaled.\n");
	test_multiTask_eventSignaled();


    printf("\n******************************\n");
    printf("Test 5 - Processes waiting when event closed.\n");
	test_waitingPs_eventClosed();

    printf("\n******************************\n");
    printf("Test 6 - Change event owner.\n");
	test_eventchown();

    printf("\n******************************\n");
    printf("Test 7 - Change event mod.\n");
	test_eventchmod();

	return 0;
}

/* Test 1 - No event while calling other operation.*/
void test_no_event(){

    int ret;
    uid_t *UID;
    gid_t *GID;
    int *UID_flag;
    int *GID_flag;

    printf("    close - ");
    ret = syscall(182, 0);
    if(ret==-1)
        printf("Correct\n");
    else
        printf("Incorrect\n");

    printf("    wait - ");
    ret = syscall(183, 0);
    if(ret==-1)
        printf("Correct\n");
    else
        printf("Incorrect\n");

    printf("    sig - ");
    ret = syscall(184, 0);
    if(ret==-1)
        printf("Correct\n");
    else
        printf("Incorrect\n");

    printf("    info - ");
    ret = syscall(185, 0);
    if(ret==0)
        printf("Correct\n");
    else
        printf("Incorrect\n");

    printf("    chown - ");
    ret = syscall(205, 0, 3, 4);
    if(ret==-1)
        printf("Correct\n");
    else
        printf("Incorrect\n");

    printf("    chmod - ");
    ret = syscall(211, 0, 0, 1);
    if(ret==-1)
        printf("Correct\n");
    else
        printf("Incorrect\n");

    printf("    stat - ");
    ret = syscall(214, 0, UID, GID, UID_flag, GID_flag);
    if(ret==-1)
        printf("Correct\n");
    else
        printf("Incorrect\n");
}

/* Test 2 - No process waiting while signaled. */
void test_no_task_waiting(){
	int eventID;
	int ret;

	eventID = syscall(181);
	if(eventID == -1){
		printf("Error in doeventopen()\n");
		return;
	}
	printf("Create event #%d. \n", eventID);
	
    ret = syscall(184, eventID);
    printf("%d processes have been signaled.\n",ret);
    if(ret==0)
        printf("Correct\n");
    else
        printf("Incorrect\n");
    
    ret = syscall(182, eventID);
    if(ret==0)
        printf("Event #%d is closed.\n", eventID);
    else
        printf("Event #%d is not closed.\n", eventID);

    return;
}

/* Test 3 - create multiple events simultaneously*/
void test_multiEvents_open(){
	int n = 20, num_active;
    int info[20];
	pthread_t threads[n];
	long i, j;
	// open events
	for(i = 0;i < n;i++){
		pthread_create(&threads[i], NULL, openEvent, (void*)i);
	}
	for(j = 0;j < n;j++){
		pthread_join(threads[j], NULL);
	}
    
    // info of the active events
    num_active = syscall(185, n+10, info);
    printf("Call info - %d active events, their eventIDs: \n", num_active);
    for(i = 0; i < num_active; i++){
        printf("%d ", info[i]);
    }
    printf("\n");
    
    printf("\nClose half of the current events. \n");

	// close events
	for(i = 0;i < n/2;i++){
		pthread_create(&threads[i], NULL, closeEvent, (void*)i);
	}
	for(j = 0;j < n/2;j++){
		pthread_join(threads[j], NULL);
	}

    // info of the remaining active events
    num_active = syscall(185, n+10, info);
    printf("Call info - %d active events left, their eventIDs: \n", num_active);
    for(i = 0; i < num_active; i++){
        printf("%d ", info[i]);
    }
    printf("\n");
}

/* Test 4 - multiple tasks waiting when doeventsig() is called. */
void test_multiTask_eventSignaled(){

    int ret_chld, ret_prt;
    int eventID;
	
    eventID = syscall(181);
	if(eventID == -1){
		printf("Error in doeventopen()\n");
		return;
	}
	printf("Event #%d is created.\n", eventID);

	if(fork() == 0){ 
        fork();
        fork();
        fork();
        ret_chld = syscall(183, eventID);
        if(ret_chld == -1)
            printf("Error in doeventwait(). \n");
        else
            printf("New process waiting on event #%d. \n", eventID);
        exit(0);// important!! Otherwise, the children will implement all the rest codes.
    }
	else{
		sleep(1);
		ret_prt = syscall(184, eventID);
		if(ret_prt == -1){
			printf("Error in doeventwait(). \n");
            return;
		}
        printf("Unblock %d processes. \n", ret_prt);
	}
}


void *openEvent(void *ptr){
    long i = (long)ptr;
	int ret;
    ret = syscall(181);
    if(ret == -1){
        printf("Error in opening event\n");
        pthread_exit(NULL);
    }
	else{
		events[i] = ret;
		printf("Opened Event: %d\n",events[i]);
	}
}

void *closeEvent(void *ptr){
    long i;
	int ret;
    i = (long)ptr;
    ret = syscall(182,events[i]);
    if(ret == -1){
        printf("Error in closing event\n");
        pthread_exit(NULL);
    }
    printf("Closed Event: %d\n",events[i]);
}

/* Test 5 - processes waiting when doeventclose() is called */
void test_waitingPs_eventClosed(){
	int ret_chld, ret_prt;
	int eventID = syscall(181);

	if(eventID == -1){
		printf("Error in doeventopen()\n");
        exit(0);
	}
	printf("Event #%d is created. \n", eventID);
	
	if(fork() == 0){ // child process
        ret_chld = syscall(183, eventID);
        if(ret_chld == -1){
			printf("Error in doeventwait(). \n");
		} 
        exit(0);
    }
	else{
        sleep(1);
        ret_prt = syscall(182, eventID);
        if(ret_prt == -1){
            printf("Error in doeventclose(). \n");
            exit(0);
        }
        printf("Close event #%d, and unblock %d processes. \n", eventID, ret_prt);
    }

}

/* Test 6 - Change event owner.*/
void test_eventchown(){
	/* get stats of an event and place it into specified memory location*/
	uid_t *UID = malloc(sizeof(uid_t));
	gid_t *GID = malloc(sizeof(gid_t));
	int *UIDFlag = malloc(sizeof(int));
    int *GIDFlag = malloc(sizeof(int));

	int eventID = syscall(181);
	if(eventID == -1){
		printf("Error in doeventopen()\n");
		return;
	}
	printf("Newly created event with eventID = %d \n", eventID);
	
	int ret = syscall(214, eventID, UID, GID, UIDFlag, GIDFlag);
	if(ret == -1){
		printf("Error in doeventstat()\n");
		return;
	}
	printf("Event effective UID: %d\n",*UID);
	printf("Event effective GID: %d\n",*GID);

    printf("Process is root can change everthing.\n");
	ret = syscall(205, eventID, *UID+200, *GID+200);
	if(ret == -1){
		printf("Fail to doeventchown()\n");
	}
	else{
		printf("Successfully change owner of event: %d\n",eventID);
		printf("Event new effective UID: %d\n",*UID+200);
		printf("Event new effective GID: %d\n",*GID+200);
        printf("Correct\n");
	}

	ret = syscall(214, eventID, UID, GID, UIDFlag, GIDFlag);
	if(ret == -1){
		printf("Error in doeventstat()\n");
		return;
	}
	printf("Get update of stats by calling doeventstat()\n");
	printf("Event effective UID: %d\n",*UID);
	printf("Event effective GID: %d\n",*GID);

    seteuid(10);
    printf("Process UID = 10, not equal to event UID.");
	ret = syscall(205, eventID, *UID+200, *GID+200);
	if(ret == -1){
		printf("Fail to doeventchown()\n");
        printf("Correct\n");
	}
	else{
		printf("Successfully change owner of event: %d\n",eventID);
		printf("Event new effective UID: %d\n",*UID+200);
		printf("Event new effective GID: %d\n",*GID+200);
	}

	return;
}

/* Test 7 - Change event owner.*/
void test_eventchmod(){
	/* get stats of an event and place it into specified memory location*/
	uid_t *UID = malloc(sizeof(uid_t));
	gid_t *GID = malloc(sizeof(gid_t));
	int *UIDFlag = malloc(sizeof(int));
    int *GIDFlag = malloc(sizeof(int));

	int eventID = syscall(181);
	if(eventID == -1){
		printf("Error in doeventopen()\n");
		return;
	}
	printf("Newly created event with eventID = %d \n", eventID);
	
	int ret = syscall(214, eventID, UID, GID, UIDFlag, GIDFlag);
	if(ret == -1){
		printf("Error in doeventstat()\n");
		return;
	}

	printf("Event effective UIDFlag: %d\n",*UIDFlag);
	printf("Event effective GIDFlag: %d\n",*GIDFlag);
	
    printf("Process is root can change everthing.\n");
	ret = syscall(211, eventID, 0, 0);
	if(ret == -1){
		printf("Fail to doeventchmod()\n");
	}
	else{
		printf("Successfully change mode of event: %d\n",eventID);
        printf("Event effective UID: %d\n",*UID);
        printf("Event effective GID: %d\n",*GID);
		printf("Event new effective UIDFlag: %d\n", 0);
		printf("Event new effective GIDFlag: %d\n", 0);
        printf("Correct\n");
	}

	ret = syscall(214, eventID, UID, GID, UIDFlag, GIDFlag);
	if(ret == -1){
		printf("Error in doeventstat()\n");
		return;
	}
	printf("Get update of stats by calling doeventstat()\n");
	printf("Event effective UID: %d\n",*UID);
	printf("Event effective GID: %d\n",*GID);
    printf("Event new effective UIDFlag: %d\n", 0);
    printf("Event new effective GIDFlag: %d\n", 0);

}
