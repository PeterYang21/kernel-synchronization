#include <stdio.h>
#include <stdlib.h>

#include <linux/unistd.h>
#include <sys/syscall.h>

/*
 * Function: main()
 *
 * Description:
 *   Entry point for this program.
 *
 * Inputs:
 *   argc - The number of argument with which this program was executed.
 *   argv - Array of pointers to strings containing the command-line arguments. 
 *
 * Return value:
 *   0 - This program terminated normally.
 */
int
main (int argc, char ** argv) {
	/* Print a friendly message */
	printf ("Hello from User Space!\n");

	/* Call our new system call */
	syscall (181, NULL);

	/* Exit the program */
	return 0;
}
