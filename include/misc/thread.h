#ifndef __misc_thread_h
#define __misc_thread_h

#include <iostream.h>

class Thread;
extern int _lwp_init_flags(void);
extern void _lwp_init_fpu(void);

enum ThreadState {
	THREAD_EMPTY, THREAD_DEAD, THREAD_RUNNING
};

struct _ThreadCB
{
	int stack_len;
	unsigned long *stack;
	ThreadState state;
};
typedef struct _ThreadCB ThreadCB;

class ThreadInit
{
private:
	static const int MAX_THREADS = 10;
	static int _flags;
	static char _fpu_state[108];

	static ThreadCB _tcb[MAX_THREADS + 1];
	static ThreadCB *_active;

	ThreadInit();
	static void switch_to(int pid);
	static void switch_from_to(unsigned long **old_stack,
				   unsigned long *new_stack);

	friend Thread;
};

class Thread
{
private:
	static ThreadInit _init;

	int _pid;
public:
	Thread(void);
	virtual ~Thread(void);

	void init(void);
	virtual void run(void) = 0;
	void kill(void);
	void yield(void);
	inline int getpid(void) { return _pid; }

	static void switch_to(int pid);
};

#endif /* __misc_thread_h */



