# kernel-synchronization
## Description
Implement a new kernel synchronization primitive that will allow multiple processes to block on an event until some other process signals the event. When a process signals the event, all processes that are blocked on the event are unblocked. If no processes are blocked on an event when it is signaled, then the signal has no effect. 
</br></br>
Events are global identifiers; processes that did not create an event (and are not children of the process that created an event) can still signal, wait on, and close the event as long as they know the event’s unique ID number and have appropriate permission. Access control is needed to prevent different users from using or deleting other users’ events.

## Event Attribute
* UID: A numeric user ID that will be equal to the effective user ID (UID) of the process that created the event.

* GID: A numeric group ID that will be equal to the effective group ID (GID) of the process that created the event.
* User Signal Enable Bit: A flag that indicates whether a process with a matching effective UID can signal or close the event. Upon creation, the flag should be set to true.
* Group Signal Enable Bit: A flag that indicates whether a process with a matching effective GID can signal or close the event. Upon creation, the flag should be set to true.

## System Calls
* long doeventopen(); Creates a new event, returning event ID on success, -1 on failure.

* long doeventclose(int eventID); Destroy the event with the given event ID and signal any processes waiting on the event to leave the event. Return number of processes signaled on success and -1 on failure.
* long doeventwait(int eventID); Blocks process until the event is signaled. Return 1 on success and -1 on failure.
* long doeventsig(int eventID); Unblocks all waiting processes; ignored if no processes are blocked. Return number of processes signaled on success and -1 on failure.
* long doeventinfo(int num, int * eventIDs); Fills in the array of integers pointed to by eventIDs with the current set of active event IDs. num is the number of integers which the memory pointed to by eventIDs can hold. eventIDs can be NULL, in which case, doeventinfo() returns the number of active event IDs. On success, doeventinfo() returns the number of active events; otherwise, it returns -1 on failure. If num is smaller than the number of active event IDs, then -1 should be returned.
* long doeventchown(int eventID, uid t UID, gid t GID); Change the UID and GID of the event to the specified values; returns -1 on failure.
* long doeventchmod (int eventID, int UIDFlag, int GIDFlag); Change the User Signal Enable Bit to UIDFlag and the Group Signal Enable Bit to GIDFlag; returns -1 on failure.
* long doeventstat (int eventID, uid t * UID, gid t * GID, int * UIDFlag, int * GIDFlag); Place the UID, GID, User Signal Enable Bit, and Group Signal Enable Bit into the memory pointed to by UID, GID, UIDFlag, and GIDFlag, respectively; returns -1 on failure.

## Modified and added files to linux source code

### Modified File 1: /linux-3.18.77/arch/x86/syscalls/syscall_64.tbl
Assign number to each of the new syscalls
</br>181	common	doeventopen     sys_doeventopen
</br>182	common	doeventclose    sys_doeventclose
</br>183	common	doeventwait     sys_doeventwait
</br>184	common	doeventsig      sys_doeventsig
</br>185	common	doeventinfo     sys_doeventinfo
</br>211	common  doeventchmod    sys_doeventchmod
</br>214	common  doeventstat     sys_doeventstat
</br>205	common  doeventchown    sys_doeventchown

### Modified File2: /linux-3.18.77/include/linux/syscalls.h
Edit syscall head file for doeventXXX to append function prototype

### Modified File 3: /linux-3.18.77/kernel/sys.c
Implement doeventXXX() system calls

### Modified File 4: /linux-3.18.77/init/main.c b/linux-3.18.77/init/main.c
Add doevent_init() to kernel boost

extern void doevent_init(void);
</br>doevent_init();

### Added File: /linux-3.18.77/include/linux/event.h
Define the event structure
