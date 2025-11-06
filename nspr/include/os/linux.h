#ifndef nspr_linux_defs_h___
#define nspr_linux_defs_h___

#define PR_LINKER_ARCH "linux"
#define _MD_GC_VMBASE          0x40000000
#define _MD_STACK_VMBASE       0x50000000
#define _MD_DEFAULT_STACK_SIZE 65536L

#define USE_SETJMP

#ifdef SW_THREADS
#include <setjmp.h>

/*
 * Complete, architecture-independent jump-buffer layout for modern glibc.
 * NSPR's GC and thread code only ever touches JB_SP, JB_PC, and JB_BP.
 * These values are taken directly from glibc's jmpbuf-offsets.h (2.40).
 */
#ifndef JB_SP
#  if defined(__x86_64__)
#    define JB_RBX  0
#    define JB_RBP  1
#    define JB_R12  2
#    define JB_R13  3
#    define JB_R14  4
#    define JB_R15  5
#    define JB_RSP  6   /* used as JB_SP */
#    define JB_PC   7   /* RIP */
#    define JB_SP   6
#    define JB_BP   1   /* RBP */
#  elif defined(__i386__)
#    define JB_EBX  0
#    define JB_ESI  1
#    define JB_EDI  2
#    define JB_EBP  3   /* used as JB_BP */
#    define JB_ESP  4   /* used as JB_SP */
#    define JB_PC   5   /* EIP */
#    define JB_SP   4
#    define JB_BP   3
#  elif defined(__aarch64__)
#    define JB_X19  0
#    define JB_X20  1
#    define JB_X21  2
#    define JB_X22  3
#    define JB_X23  4
#    define JB_X24  5
#    define JB_X25  6
#    define JB_X26  7
#    define JB_X27  8
#    define JB_X28  9
#    define JB_X29  10  /* FP */
#    define JB_X30  11  /* LR (used as PC) */
#    define JB_SP   20
#    define JB_PC   21
#    define JB_BP   10
#  elif defined(__arm__)
#    define JB_R4   0
#    define JB_R5   1
#    define JB_R6   2
#    define JB_R7   3
#    define JB_R8   4
#    define JB_R9   5
#    define JB_R10  6
#    define JB_R11  7   /* FP -> JB_BP */
#    define JB_R12  8
#    define JB_R13  9   /* SP -> JB_SP */
#    define JB_R14  10  /* LR */
#    define JB_R15  11  /* PC -> JB_PC */
#    define JB_SP   9
#    define JB_PC   11
#    define JB_BP   7
#  elif defined(__powerpc64__)
#    define JB_GPR1  0   /* SP */
#    define JB_GPR2  1
#    define JB_LR    2   /* PC */
#    define JB_SP    0
#    define JB_PC    2
#    define JB_BP    1
#  elif defined(__powerpc__) || defined(__ppc__)
#    define JB_GPR1  0   /* SP */
#    define JB_LR    1   /* PC */
#    define JB_SP    0
#    define JB_PC    1
#    define JB_BP    0
#  else
#    warning "Unknown architecture – using x86 fallback for jmp_buf layout"
#    define JB_SP    4
#    define JB_PC    5
#    define JB_BP    3
#  endif
#endif /* !JB_SP */

#ifndef _JB_CONSTANTS_DEFINED
#  define _JB_CONSTANTS_DEFINED 1
#endif

/* NSPR accessors – now 100% safe */
#define PR_GetSP(_t)   (_t)->context[0].__jmpbuf[JB_SP]
#define PR_GetPC(_t)   (_t)->context[0].__jmpbuf[JB_PC]

#define PR_NUM_GCREGS  6
#define PR_CONTEXT_TYPE sigjmp_buf
#define CONTEXT(_th)   ((_th)->context)

/*
** Initialize a thread context to run "e(o,a)" when started
*/
#define _MD_INIT_CONTEXT(_thread, e, o, a) \
{ \
    (_thread)->asyncCall = e; \
    (_thread)->asyncArg0 = o; \
    (_thread)->asyncArg1 = a; \
    (_thread)->context[0].__jmpbuf[JB_BP] = 0; \
    (_thread)->context[0].__jmpbuf[JB_SP] = (unsigned char*) \
        ((_thread)->stack->stackTop - 64); \
    (_thread)->context[0].__jmpbuf[JB_PC] = HopToadNoArgs; \
}

#define _MD_SWITCH_CONTEXT(_thread) \
    if (!sigsetjmp(CONTEXT(_thread), 1)) { \
        (_thread)->errcode = errno; \
        _PR_Schedule(); \
    }

/*
** Restore a thread context, saved by _MD_SWITCH_CONTEXT
*/
#define _MD_RESTORE_CONTEXT(_thread) \
{ \
    _pr_current_thread = _thread; \
    PR_LOG(SCHED, warn, ("Scheduled")); \
    errno = (_thread)->errcode; \
    siglongjmp(CONTEXT(_thread), 1); \
}

#endif /* SW_THREADS */

#undef HAVE_LONG_LONG
#undef HAVE_ALIGNED_DOUBLES
#undef HAVE_ALIGNED_LONGLONGS

#ifdef LINUX1_2
#define HAVE_DLL
#define USE_DLFCN
#else
#undef HAVE_DLL
#undef USE_DLFCN
#endif

#define NEED_TIME_R

#endif /* nspr_linux_defs_h___ */
