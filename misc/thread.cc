#include <stdio.h>
#include <string.h>

#include "misc/thread.h"

ThreadInit Thread::_init;

int ThreadInit::_flags = 0;
char ThreadInit::_fpu_state[108];
ThreadCB *ThreadInit::_active;
ThreadCB ThreadInit::_tcb[ThreadInit::MAX_THREADS + 1];

ThreadInit::ThreadInit()
{
	// cout << "ThreadInit()\n";
	for (int a = 0;a < MAX_THREADS;a++) {
		_tcb[a].stack_len = 0;
		_tcb[a].stack = NULL;
		_tcb[a].state = THREAD_EMPTY;
	}
	_tcb[MAX_THREADS].stack_len = 0;
	_tcb[MAX_THREADS].stack = NULL;
	_tcb[MAX_THREADS].state = THREAD_RUNNING;
	_active = &_tcb[MAX_THREADS];
	asm (
		"pushfl\n\t"
		"popl %%eax\n\t"
		: "=a" (_flags)
		: /* no input */
		);
#if 0
	asm (
		"finit\n\t"
		"fwait\n\t"
		"fsave (%%eax)\n\t"
		"fwait\n\t"
		: /* no output */
		: "a" (&_fpu_state)
		);
#endif
}

void ThreadInit::switch_to(int pid)
{
	unsigned long **__old_stack;
	unsigned long *__new_stack;

	if ((pid < 0) || (pid > MAX_THREADS)) return;
	if (_tcb[pid].state != THREAD_RUNNING) return;

	__old_stack = &_active->stack;
	if (*__old_stack == 0) {
		asm (
			"movl %%esp, %0\n\t"
			: "=g" (_active->stack)
			: /* no input */
			);
	}
	__new_stack = _tcb[pid].stack;
	_active = &_tcb[pid];
	switch_from_to(__old_stack, __new_stack);
}

Thread::Thread(void)
{
	_pid = -1;
	//cout << "Thread(): pid   = " << pid << endl;
	for (int a = 0;a < ThreadInit::MAX_THREADS;a++) {
		if (_init._tcb[a].state == THREAD_EMPTY) {
			_pid = a;
			break;
		}
	}
	if (_pid < 0) {
		cerr << "too many threads!" << endl;
		return;
	}
	_init._tcb[_pid].state = THREAD_DEAD;
}

Thread::~Thread(void)
{
	_init._tcb[_pid].state = THREAD_EMPTY;
}

void Thread::init(void)
{
	int len;
	unsigned long *stack;

	if (_pid < 0) return;

	len = 1024;
	stack = new unsigned long[len];

	//cout << "          stack = " << (void *)stack << endl;

	_init._tcb[_pid].state = THREAD_RUNNING;

	stack[--len] = 0xaffeaffe;
	stack[--len] = 0xaffeaffe;
	stack[--len] = 0xaffeaffe;
	stack[--len] = 0xaffeaffe;
	stack[--len] = (long) this;
	stack[--len] = (long) this;
	stack[--len] = (long) kill;
	stack[--len] = (long) this->run;
	//stack[--len] = 0xdeadbeef;
	stack[--len] = 0x11111111; /* eax */
	stack[--len] = 0x22222222; /* ecx */
	stack[--len] = 0x33333333; /* edx */
	stack[--len] = 0x44444444; /* ebx */
	stack[--len] = 0x55555555;
	stack[--len] = 0x66666666; /* ebp */
	stack[--len] = 0x77777777; /* esi */
	stack[--len] = (long) _init._flags; /* flags */
	//stack[--len] = 0x12345678;

	//len -= 27;
	//memcpy(stack + len, _init._fpu_state, 108);

	_init._tcb[_pid].stack = stack + len;
}

void Thread::run(void)
{
}

void Thread::kill(void)
{
	cout << _pid << ": killed" << endl;

	_init._tcb[_pid].state = THREAD_DEAD;

	yield();
}

void Thread::yield(void)
{
	_init.switch_to(ThreadInit::MAX_THREADS);
}

void Thread::switch_to(int pid)
{
	_init.switch_to(pid);
}
