# kernel-synchronization

Modified(M)/Added(A) files to the kernel source
M-File 1: /linux-3.18.77/arch/x86/syscalls/syscall_64.tbl
Register doeventXXX as #181-#185

M-File 2: /linux-3.18.77/include/linux/syscalls.h
Edit syscall head file for doeventXXX to append function prototype

M-File 3: /linux-3.18.77/kernel/sys.c
Implement doeventXXX() system calls

M-File 4: /linux-3.18.77/init/main.c b/linux-3.18.77/init/main.c
Add doevent_init() to kernel boost

extern void doevent_init(void);
doevent_init();

A-File 5: /linux-3.18.77/include/linux/event.h
Define the event structure
