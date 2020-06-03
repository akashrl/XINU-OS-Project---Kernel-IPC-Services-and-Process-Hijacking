# XINU Operating Systems Project - Kernel IPC Services and Process Hijacking

You may view this Lab Project here: https://www.cs.purdue.edu/homes/cs354/lab4/lab4.html

The modified source code is under `xinu-spring2020/system/` and `xinu-spring2020/include/`.

## Objectives

The objective of this lab is to enhance XINU's IPC services and utilize run-time stack manipulation to "hijack" a process by executing malware.

### 3. Blocking send() IPC

This problem concerns the implementation of a blocking version of send(), bsend(), which has the same function definition as send(). The difference is that if the receiver's buffer is full, bsend() blocks until the receiver's buffer frees up. This is in contrast to send() which returns immediately with SYSERR indicating that sending has failed. Use your code from lab1 as the code base. Put bsend() in system/bsend.c.

### 3.1 Kernel modifications: bsend()

For send() to behave in blocking mode means, first, if the receiver's 1-word message buffer is empty, it behaves the same way as send(). That is, send() copies the message to the receiver buffer, sets the receiver's prhasmsg flag to 1, and returns OK without blocking. Second, if the receiver's buffer is full, bsend() blocks until the receiver's buffer becomes free. To block upon attempting to send means that the sender's state changes from current to PR_SND (define PR_SND as 11 in include/process.h). The kernel records the message to be sent in a new 1-word process table field, umsg32 prsndmsg.

Before calling resched() to context-switch out the sender and context-switch in a different process, the sender is inserted into a FIFO queue of processes blocking to send to the same receiver. To do so, we need to provide each process with a private FIFO queue in case it becomes a receiver process to whom other processes attempting to send messages block. Increase NQENT in include/queue.h to accommodate per-receiver process blocked sender queues. Explain in Lab4Answers.pdf why your change of NQENT works. Confer Chapter 4 of the textbook for a discussion of how XINU implements queues. Define two additional process table fields

```
bool8 prsenderflag;            /* Set to 1 if one or more processes are blocking to send */
qid16 prblockedsenders;        /* Index to FIFO queue of blocked senders */
```
where prsenderflag is true if one or more processes are blocking to send, 0 otherwise. prblockedsenders is an index to a FIFO queue of blocked sender processes which must be initialized in create(). Use XINU's enqueue() and dequeue() functions for implementing FIFO queues for managing the per-process blocked sender queues. In XINU, a process can be resident in only one queue (e.g., sleep queue, ready list) which affords space efficient queue management using the kernel data structure queuetab[].

### 3.2 Kernel mods: receive()

When a process makes a receive() system call, receive() checks if it has a message in its 1-word buffer by inspecting the prhasmsg flag. If prhasmsg is 0, the receiver blocks. In the case prhasmsg is true, before the 1-word message is returned by receive(), the kernel must check if there are any blocker sender processes. If prsenderflag is 1, one or more sender processes are waiting in the receiver's blocked sender FIFO queue. Using dequeue() the process at the front of the queue is extracted and inserted into the ready list. The dequeued sender's message is copied from the sender's prsndmsg process table field to the receiver's prmsg 1-word buffer. Use of the local variable msg prevents the previous message (which hasn't been returned yet) from being overwritten. The receiver's prhasmsg flag remains 1. Lastly, the previous message stored in msg is returned by receive(). Note that in our version of receive(), the scheduler is only called when the receiver blocks because its 1-word message buffer is empty. Even in the case where an unblocked sender process has higher priority than the receiver process, the receiver process will remain current and execute the instruction following the receive() system call. That is, we do not preempt a process making a receive() system call if its message buffer is not empty. This comes at the cost of potentially delaying execution of a high priority process which just became ready after blocking in a receiver process's blocked sender queue. The benefit is reduced overhead as resched() is not called before executing ret from receive(). In the case the just unblocked sender has lower priority than the receiver, the resched() call would have been pure overhead.

### 3.3 Testing

Test your kernels mods for implementing bsend(). Describe in Lab4Answers.pdf what test scenarios you have considered to gauge correctness.

## 4. Hijacking a process by modifying its run-time stack

### 4.1 Basic idea

An important technique for modifying the run-time behavior of a process is ROP (return-oriented programming) which we utilized in Problem 3, lab3, to make a process execute code that it wasn't programmed to do. By modifying the return address pushed onto the stack of a newly created process, we induced it to make a detour to code that took time stamps to commence CPU usage measurement. In this problem, we will use the same technique to "hijack" a process by making it execute malware code. We will do so by modifying the run-time stack of a victim process while it is context-switched out so that when it eventually becomes current we induce ctxsw() to jump to the attacker's malware code. In the first attack, the attacker makes ctxsw() jump to malware function hellomalware() which prints a hello message and terminates the victim process. The attacker makes no effort to hide the attack. In the second attack, the attacker makes ctxsw() jump to its malware function quietmalware(). Instead of printing a message and terminating the victim process, quietmalware() surgically modifies the victim process's data -- a local variable in the app code -- then returns to resched() as if nothing had happened. The victim process will continue to execute, but when printing the value of the local variable, it will have been corrupted unbeknownst to the victim. Use your code base from lab1 but set QUANTUM to be 30 msec.

### 4.2 Overt attack

Spawn three app processes from main() back-to-back using create()/resume(), all running victimA(). This is followed by creation and resumption from main() of an attacker process which runs attackerA(). Assign all four processes equal priority, albeit less than that of the parent process running main().

**Victim process.** The app code victimA() in system/victimA.c is given by

```
void victimA(void) {
  int x;

  x = 5;
  kprintf("before funcA: %d %d\n", getpid(), x);
  sleepms(300);
  kprintf("after funcA: %d %d\n", getpid(), x);
}
```

Creating and resuming the three app processes will result in the expected output of a process's PID and integer value 5, twice per process. This is in the absence of interference by the attacker process. Verify normal operation when the attacker process does nothing.

**Attacker process.** The attacker process runs, void attackerA(int y), in system/attackerA.c which works as follows. The attacker chooses its victim's PID as getpid() - y. For example, if y = 1 then the victim is the third app process created by main(). To stage a successful attack, the attacker is assumed to be privy to certain knowledge such as the victim process being in sleep state. The attacker is assumed to have access to the kernel's process table which allows it to know the address of the top of the victim's run-time stack by looking up prstkptr. With knowledge of how XINU's context-switch works, the attacker finds the address at which the return address of ctxsw() (which returns to its caller resched()) has been pushed. Explain in Lab4Answers.pdf how you determine this address. The attacker then overwrites the return address with the address of void hellomalware(void) (in system/hellomalware.c). When the victim process eventually wakes up and becomes current, it will continue to execute the "second half" of ctxsw() that pops the victim's stack to restore register contents and returns to resched(). However, since the return address has been modified to contain the function pointer of hellomalware(), the victim process jumps to hellomalware() which outputs a suitable message (e.g., "successful takeover") and terminates the victim process.

**Testing.** Verify that the attacker code works as intended by running tests without the attacker as a reference point, followed by attacks with different values of argument y. For example, for y = 3, the first app process spawned by the process main() should not return from sleepms() to victimA() and, hence, not get to output the second kprintf(). The second and third app processes should run as before.

### 4.3 Stealth attack

Hackers may opt for overt attacks in some cases. In others, a more stealthy mode may be employed to leave a victim in the dark about having been attacked. Code a stealth attack, void attackerB(int y), in system/attackerB.c which works as follows. The victim code remains the same, and the process running main() spawns three app processes followed by the attacker process as in 4.2. When the attacker process run attackerB(), its goal is three-fold. First, as in 4.2, make ctxsw() jump to malware code, in this instance, void quietmalware(void), in system/quietmalware.c. Second, the code of quietmalware() finds out the address of the local variable x of victimA() and modifies its value from 5 to 9. Third, after quietmalware() is finished it jumps to the instruction of resched() that ctxsw() would have returned to. The victim process returns from resched() to sleepms() which returns to victimA() and outputs the second kprintf(). The victim process seems to execute normally except for the value of the local variable x having changed to 9. A victim that does not know what the correct value of x should be will be unaware that it has been corrupted. Explain in Lab4Answers.pdf your method for accomplishing the second and third goals. For Problem 4.3, add the option -fno-omit-frame-pointer in Makefile which forces gcc to use ebp as frame pointer. By default, gcc tries to optimize by freeing up ebp to hold other values. Test and verify that the attacker code works correctly.

In Problem 3, the possibility exists that processes are blocked trying to send to a receiver, but the receiver terminates without reading all messages. That is, one or more processes are queued in the receiver's blocked sender queue when it undergoes termination. One approach is to dequeue such processes, insert them into XINU's ready list, and have bsend() return SYSERR. Describe a solution in Lab4Answers.pdf that follows this approach and is backward compatible with the solution of Problem 3. No need to implement the solution, but the description should be sufficiently detailed. That is, all kernel functions and data structures that are required to be modified should be listed and the changes needed specified.

