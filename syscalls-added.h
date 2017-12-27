/*
* The following system calls have been appended to the original 
* linux-3.18.77/include/linux/syscalls.h
*/
asmlinkage long sys_doeventopen(void);
asmlinkage long sys_doeventclose(int event_id);
asmlinkage long sys_doeventwait(int event_id);
asmlinkage long sys_doeventsig(int event_id);
asmlinkage long sys_doeventinfo(int num, int *event_ids);
asmlinkage long sys_doeventchown(int event_id, uid_t UID, gid_t GID);
asmlinkage long sys_doeventchmod(int event_id, int UID_enable, int GID_enable);
asmlinkage long sys_doeventstat(int event_id, uid_t *UID, gid_t *GID, int *UID_enable, int *GID_enable);
