/// Libnosys issues warnings when you link against its version of these functions.
/// These stubs just get rid of the warnings.
///
/// For example: https://github.com/eblot/newlib/blob/master/libgloss/libnosys/close.c

#include <cerrno>

extern "C" int _close(int file) {
    static_cast<void>(file);
    errno = ENOSYS;
    return -1;
}

extern "C" int _fstat(int file, struct stat *st) {
    static_cast<void>(file);
    static_cast<void>(st);
    errno = ENOSYS;
    return -1;
}

extern "C" int _isatty(int file) {
    static_cast<void>(file);
    errno = ENOSYS;
    return 0;
}

extern "C" int _getpid() {
    errno = ENOSYS;
    return -1;
}

extern "C" int _kill(int pid, int sig) {
    static_cast<void>(pid);
    static_cast<void>(sig);
    errno = ENOSYS;
    return -1;
}

extern "C" int _lseek(int file, int ptr, int dir) {
    static_cast<void>(file);
    static_cast<void>(ptr);
    static_cast<void>(dir);
    errno = ENOSYS;
    return -1;
}
