/* victimA.c - victimA */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  victimA  - Lab 4, Section 4.2. This is victim process. 
 *------------------------------------------------------------------------
 */
void victimA(void) {
	int x;

	x = 5;
	kprintf("\nbefore funcA: %d %d\n", getpid(), x);

	struct procent *prptr;

        prptr = &proctab[currpid]; // Process table pointer for victim's process

	sleepms(300);

	kprintf("\nafter funcA: %d %d\n", getpid(), x);


}
