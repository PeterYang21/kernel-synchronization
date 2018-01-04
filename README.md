# kernel-synchronization

Modified and added files to linux source code

Modified File 1: /linux-3.18.77/arch/x86/syscalls/syscall_64.tbl
Assign each syscall number

181	common	doeventopen     sys_doeventopen
182	common	doeventclose    sys_doeventclose

183	common	doeventwait     sys_doeventwait

184	common	doeventsig      sys_doeventsig

185	common	doeventinfo     sys_doeventinfo

211	common  doeventchmod    sys_doeventchmod

214	common  doeventstat     sys_doeventstat

205	common  doeventchown    sys_doeventchown

Modified File2: /linux-3.18.77/include/linux/syscalls.h
Edit syscall head file for doeventXXX to append function prototype

Modified File 3: /linux-3.18.77/kernel/sys.c
Implement doeventXXX() system calls

Modified File 4: /linux-3.18.77/init/main.c b/linux-3.18.77/init/main.c
Add doevent_init() to kernel boost

extern void doevent_init(void);
doevent_init();

Added File 5: /linux-3.18.77/include/linux/event.h
Define the event structure
