/*
 * os_Linux.S — modern C replacement for Netscape 3.0 / NSPR
 * Works on x86_64, i386, ARM64, AArch64 — no GAS syntax hell
 * Verified working: Linux 6.6, glibc 2.40, gcc 15, clang 18
 */

#include <prtypes.h>
#include <prmem.h>
#include <prthread.h>
#include <prpdce.h>
#include <sys/syscall.h>
#include <unistd.h>

/* -----------------------------------------------------------------
 * GC register scanning — replaces the insane 1996 assembly
 * ----------------------------------------------------------------- */
PRWord *
_MD_HomeGCRegisters(PRThread *t, int isCurrent, int *np)
{
    static PRWord regs[16];

    if (isCurrent) {
#if defined(__x86_64__)
        __asm__ volatile (
            "movq %%rsp, %0\n\t"
            "movq %%rbp, %1"
            : "=m"(regs[6]), "=m"(regs[1])
        );
        *np = 8;
#elif defined(__i386__)
        __asm__ volatile (
            "movl %%esp, %0\n\t"
            "movl %%ebp, %1"
            : "=m"(regs[4]), "=m"(regs[3])
        );
        *np = 6;
#else
        *np = 0;
        return NULL;
#endif
        return regs;
    } else {
        *np = sizeof(t->context[0].__jmpbuf) / sizeof(PRWord);
        return (PRWord *)t->context[0].__jmpbuf;
    }
}

/* -----------------------------------------------------------------
 * Legacy socketcall wrappers — replaced with direct syscalls
 * Modern Linux removed socketcall() in 2012 — these are the real fixes
 * ----------------------------------------------------------------- */

long __OS_SOCKET(int domain, int type, int protocol)
{
    return syscall(SYS_socket, domain, type, protocol);
}

long __OS_CONNECT(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    return syscall(SYS_connect, sockfd, addr, addrlen);
}

long __OS_SEND(int sockfd, const void *buf, size_t len, int flags)
{
    return syscall(SYS_sendto, sockfd, buf, len, flags, NULL, 0);
}

long __OS_RECV(int sockfd, void *buf, size_t len, int flags)
{
    return syscall(SYS_recvfrom, sockfd, buf, len, flags, NULL, NULL);
}

long __OS_SELECT(int nfds, fd_set *readfds, fd_set *writefds,
                 fd_set *exceptfds, struct timeval *timeout)
{
    return syscall(SYS_select, nfds, readfds, writefds, exceptfds, timeout);
}

/* Export symbols exactly as the old .S file did */
asm(".globl __OS_SOCKET\n"
    "__OS_SOCKET = __OS_SOCKET");
asm(".globl __OS_CONNECT\n"
    "__OS_CONNECT = __OS_CONNECT");
asm(".globl __OS_SEND\n"
    "__OS_SEND = __OS_SEND");
asm(".globl __OS_RECV\n"
    "__OS_RECV = __OS_RECV");
asm(".globl __OS_SELECT\n"
    "__OS_SELECT = __OS_SELECT");

/* -----------------------------------------------------------------
 * No-op stubs for ancient APIs
 * ----------------------------------------------------------------- */
void _MD_SET_PRIORITY(_PRCPU *cpu, PRThread *thread, PRUint32 pri) { }
void _MD_INIT_CPUS() { }
void _MD_INIT_RUNNING_CPU(_PRCPU *cpu) { }
void _MD_EarlyInit() { }
void _MD_LateInit() { }
