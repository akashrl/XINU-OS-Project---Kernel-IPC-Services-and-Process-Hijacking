/*  main.c  - main */

#include <xinu.h>

process	main(void)
{

	// Experimental code to trigger interrupt 0 by divide by 0
	// int x = 8/0;
	// kprintf("\nMAIN IS RUNNING\n"); 

    	kprintf("\nI'm the first XINU app and running function main() in system/main.c.\n");
    	kprintf("\nI was created by nulluser() in system/initialize.c using create().\n");
    	kprintf("\nMy creator will turn itself into the do-nothing null process.\n");
    	kprintf("\nI will create a second XINU app that runs shell() in shell/shell.c as an example.\n");
    	kprintf("\nYou can do something else, or do nothing; it's completely up to you.\n");
    	kprintf("\n...creating a shell\n");

	/* Run the Xinu shell */

	recvclr();
	void victimA(void);
	void attackerA(int y);
	void attackerB(int y);

	umsg32 msg;
	struct procent *prptr;
	
	// resume(create(shell, 8192, 50, "shell", 1, CONSOLE));


	/* Lab 4, Section 3 */
	kprintf("\nOutput from Lab 4, Section 3\n\n");
	pid32 receiverpid = create(receive, 8192, 20, "receive", 0); // Receiver process
	pid32 senderpid1 = create(bsend, 8192, 20, "send", 2, receiverpid, "hi"); // Sender process first iteration
	pid32 senderpid2 = create(bsend, 8192, 20, "send", 2, receiverpid, "hello"); // Sender process second iteration

	resume(senderpid1); // Run sender process first iteration
	resume(senderpid2); // Run sender process second iteration
	resume(receiverpid); // Run receiver process

	prptr = &proctab[receiverpid];
	msg = prptr->prmsg;
	kprintf("Message: %s\n", msg); 


	/* Lab 4, Section 4.2 and 4.3 */
	kprintf("\nSpawn victim's processes for Lab 4, 4.2 & 4.3\n\n");
	resume(create(victimA, 8192, 25, "victim 1", 0)); // Victim process 1
	resume(create(victimA, 8192, 25, "victim 2", 0)); // Victim process 2
	resume(create(victimA, 8192, 25, "victim 3", 0)); // Victim process 3
	
	/* Lab 4, Attacker process for section 4.2 */
//	resume(create(attackerA, 8192, 25, "attacker A", 1, 1)); // Attacker process for last victim's process
//	resume(create(attackerA, 8192, 25, "attacker A", 1, 2)); // Attacker process for last victim's process
	resume(create(attackerA, 8192, 25, "attacker A", 1, 3)); // Attacker process for first victim's process

	/* Lab 4, Attacker process for section 4.3 */
	resume(create(attackerB, 8192, 25, "attacker B", 1, 2)); // Attacker process for last victim's process


	/* Wait for shell to exit and recreate it */
/*
	while (TRUE) {
		receive();
		sleepms(200);
		kprintf("\n\nMain process recreating shell\n\n");
		resume(create(shell, 4096, 20, "shell", 1, CONSOLE));
	}
*/
	return OK;
    
}
