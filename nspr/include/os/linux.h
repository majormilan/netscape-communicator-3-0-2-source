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

/*
 * Since glibc 2.4, setjmp()/sigsetjmp() do not store raw SP/BP/PC values
 * into __jmpbuf: they XOR them with a per-process "pointer guard" cookie
 * (kept at %fs:0x30 in the TCB on x86_64) and rotate the result left by
 * 17 bits ("pointer mangling", meant to make stack-smashing attacks that
 * overwrite a jmp_buf harder). longjmp()/siglongjmp() always undo this
 * transform on the way out.
 *
 * _MD_INIT_CONTEXT below builds a jmp_buf by hand (there is no real
 * setjmp() call to prime it, since the thread hasn't run yet), and
 * PR_GetSP()/PR_GetPC() read the SP/PC back out of a jmp_buf for the
 * conservative GC (prgcapi.c). Both must apply the same mangling glibc
 * uses, or siglongjmp() restores garbage and crashes (this was the
 * cause of a SIGSEGV inside __longjmp() / _PR_Schedule() when starting
 * the very first NSPR thread). Verified empirically against this
 * system's glibc by comparing real sigsetjmp() output for known
 * SP/BP/PC values.
 */
#if defined(__x86_64__)
static __inline__ long int
_nspr_ptr_mangle(long int val)
{
    unsigned long int guard, v = (unsigned long int) val;
    __asm__ __volatile__ ("mov %%fs:0x30, %0" : "=r" (guard));
    v ^= guard;
    return (long int) ((v << 17) | (v >> (64 - 17)));
}
static __inline__ long int
_nspr_ptr_demangle(long int val)
{
    unsigned long int guard, v = (unsigned long int) val;
    __asm__ __volatile__ ("mov %%fs:0x30, %0" : "=r" (guard));
    v = (v >> 17) | (v << (64 - 17));
    return (long int) (v ^ guard);
}
#else
#  warning "NSPR SW_THREADS: jmp_buf pointer-mangling not implemented for this architecture; hand-built thread contexts may crash on glibc versions that mangle jmp_buf pointers"
#  define _nspr_ptr_mangle(v)   ((long int)(v))
#  define _nspr_ptr_demangle(v) ((long int)(v))
#endif

/* NSPR accessors – demangle the raw jmp_buf slots back into real
   pointers before returning them. */
#define PR_GetSP(_t)   _nspr_ptr_demangle((_t)->context[0].__jmpbuf[JB_SP])
#define PR_GetPC(_t)   _nspr_ptr_demangle((_t)->context[0].__jmpbuf[JB_PC])

#define PR_NUM_GCREGS  6
#define PR_CONTEXT_TYPE sigjmp_buf
#define CONTEXT(_th)   ((_th)->context)

/*
** Initialize a thread context to run "e(o,a)" when started
*/
#define _MD_INIT_CONTEXT(_thread, e, o, a) \
{ \
    unsigned long int _nspr_init_sp = (unsigned long int) \
        (unsigned char*) ((_thread)->stack->stackTop - 64); \
    /* \
     * siglongjmp() jumps directly to JB_PC with the CPU registers set \
     * from JB_SP/JB_BP -- it does not execute a "call" instruction, so \
     * no return address gets pushed. But HopToadNoArgs() is ordinary \
     * C code compiled assuming the x86_64 SysV ABI invariant that, on \
     * entry to any function reached via "call", %rsp % 16 == 8 (the \
     * call's implicit push of the 8-byte return address is what knocks \
     * a 16-aligned pre-call %rsp down to 8-aligned). Handing it a \
     * 16-aligned %rsp instead breaks that invariant one level in, and \
     * SSE (movaps) stores/loads on 16-byte-aligned stack locals further \
     * down the call chain fault with SIGSEGV. Align down to 16 and then \
     * subtract 8 so the manufactured context looks exactly like a real \
     * call site would. \
     */ \
    (_thread)->asyncCall = e; \
    (_thread)->asyncArg0 = o; \
    (_thread)->asyncArg1 = a; \
    (_thread)->context[0].__jmpbuf[JB_BP] = _nspr_ptr_mangle(0); \
    (_thread)->context[0].__jmpbuf[JB_SP] = _nspr_ptr_mangle((long int) \
        ((_nspr_init_sp & ~(unsigned long int)0xF) - 8)); \
    (_thread)->context[0].__jmpbuf[JB_PC] = _nspr_ptr_mangle((long int)HopToadNoArgs); \
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
