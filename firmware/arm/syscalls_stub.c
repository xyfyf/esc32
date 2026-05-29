/**
 * @file syscalls_stub.c
 * @brief Minimal newlib/nano syscall stubs so that the linker can resolve
 *        the C-runtime hooks pulled in by libc/libm.
 *
 * The simulation HAL routes printf() through stdio; on the embedded target
 * we don't have a real backend yet, so every call below is a no-op or
 * returns -1. Replace once UART or RTT logging is wired up.
 */
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>

extern char _end;
static char *heap_end;

void *_sbrk(int incr)
{
    char *prev_heap_end;
    if (heap_end == 0) {
        heap_end = &_end;
    }
    prev_heap_end = heap_end;
    heap_end += incr;
    return (void *)prev_heap_end;
}

int _close(int file)        { (void)file; return -1; }
int _fstat(int file, struct stat *st) { (void)file; if (st) st->st_mode = S_IFCHR; return 0; }
int _isatty(int file)       { (void)file; return 1; }
int _lseek(int file, int ptr, int dir) { (void)file; (void)ptr; (void)dir; return 0; }
int _read(int file, char *ptr, int len) { (void)file; (void)ptr; (void)len; return 0; }
int _write(int file, char *ptr, int len) { (void)file; (void)ptr; return len; }
int _getpid(void)           { return 1; }
int _kill(int pid, int sig) { (void)pid; (void)sig; errno = EINVAL; return -1; }
void _exit(int status)      { (void)status; while (1) {} }
