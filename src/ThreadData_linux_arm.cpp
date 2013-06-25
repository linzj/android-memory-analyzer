#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <android/log.h>

struct ucontext {
	unsigned long	  uc_flags;
	struct ucontext  *uc_link;
	stack_t		  uc_stack;
	struct sigcontext_arm uc_mcontext;
	sigset_t	  uc_sigmask;
	/* Allow for uc_sigmask growth.  Glibc uses a 1024-bit sigset_t.  */
	int		  __unused[32 - (sizeof (sigset_t) / sizeof (int))];
	/* Last for extensibility.  Eight byte aligned because some
	   coprocessors require eight byte alignment.  */
 	unsigned long	  uc_regspace[128] __attribute__((__aligned__(8)));
};


void sendStackData(int fd,void ** buf,int count)
{
}

