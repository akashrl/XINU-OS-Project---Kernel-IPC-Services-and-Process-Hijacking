/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	pid32	senderspid;		/* Lab 4, 3.2, senders pid	*/

	mask = disable();
	prptr = &proctab[currpid];
	kprintf("Test: From receiver before dequeue prptr->prmsg: %s\n", prptr->prmsg);
	kprintf("Test: From receiver this is receiver pid: %d\n", currpid);

	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}

	if (prptr->prsenderflag == TRUE) { /* Lab 4, 3.2, receiving process gets sending process message */
		msg = prptr->prmsg;
		senderspid = dequeue(prptr->prblockedsenders); /* dequeues sending process from FIFO queue */
		prptr->prmsg = proctab[senderspid].prsndmsg; /* gets sending process's message */
		kprintf("Test: From receiver this is dequeued pid: %d\n", senderspid); 
		kprintf("Test: From receiver after dequeue prptr->prmsg: %s\n", prptr->prmsg);
		ready(senderspid); /* inserts process into readylist */
		prptr->prhasmsg = TRUE;
	}
	else {
		prptr->prsenderflag = FALSE;	/* Lab 4, 3.2 */
		prptr->prhasmsg = FALSE;	/* Reset message flag		*/
		msg = prptr->prmsg;
	}

	restore(mask);

	return msg;
}
