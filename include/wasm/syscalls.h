#pragma once

#include <sys/socket.h>

namespace wasm {
    static const char *FAKE_NAME = "faasm";
    static const char *FAKE_PASSWORD = "foobar123";
    static const char *FAKE_HOME = "/";
    static const int FAKE_PID = 23;
    static const int FAKE_UID = 1;
    static const int FAKE_GID = 1;
    static const char *FALSE_ROOT = "/usr/local/faasm/runtime_root";
    static const char *HOSTS_FILE = "/usr/local/faasm/net/hosts";
    static const char *RESOLV_FILE = "/usr/local/faasm/net/resolv.conf";

    void getBytesFromWasm(I32 dataPtr, I32 dataLen, uint8_t *buffer);

    std::vector<uint8_t> getBytesFromWasm(I32 dataPtr, I32 dataLen);

    std::string getStringFromWasm(I32 strPtr);

    std::pair<std::string, std::string> getUserKeyPairFromWasm(I32 keyPtr);

    std::string maskPath(std::string originalPath);

    std::string getMaskedPathFromWasm(I32 strPtr);

    // ---------------------------
    // System-related structs
    // ---------------------------

    /**
     * Any structs passed as arguments must be re-implemented here with the following mappings
     * (respecting signed/ unsigned):
     *
     * int64_t = I64/U64
     * short = I16/U16
     * long = I32/U32
     * int = I32/U32
     * char = U8
     * pointers = Uptr
     * size_t = I32
     *
     * You need to look at include/bits/alltypes.h in the relevant sysroot to get a lot of the types
    */

    // Timing
    struct wasm_timespec {
        I64 tv_sec;
        I32 tv_nsec;
    };

    // Timing
    struct wasm_timeval {
        I64 tv_sec;
        I32 tv_usec;
    };

    // I/O
    struct wasm_iovec {
        U32 iov_base;
        U32 iov_len;
    };

    /** Socket-related struct (see https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html) */
    struct wasm_sockaddr {
        U16 sa_family;
        U8 sa_data[14];
    };

    // Taken from arch/wasm32/bits/stat.h
    struct wasm_stat {
        U64 st_dev;
        U64 st_ino;
        U64 st_nlink;

        U32 st_mode;
        U32 st_uid;
        U32 st_gid;
        U32 __pad0;
        U64 st_rdev;
        I64 st_size;

        I32 st_blksize;
        I64 st_blocks;

        struct wasm_timespec st_atim;
        struct wasm_timespec st_mtim;
        struct wasm_timespec st_ctim;

        I64 __unused[3];
    };

    /**
     * Found in pwd.h.
     *
     * Note that char pointers are U32
     */
    struct wasm_passwd {
        U32 pw_name; // char*
        U32 pw_passwd; // char*
        U32 pw_uid;
        U32 pw_gid;
        U32 pw_gecos; // char*
        U32 pw_dir; // char*
        U32 pw_shell; // char*
    };

    /**
     * To be sure what the target dirent struct is like, you need to look in dirent.h in
     * the correct sysroot. Emscripten looks like:
     *
     * struct dirent {
     *   ino_t d_ino;  // 64-bit
     *   off_t d_off;  // 64-bit
     *   unsigned short d_reclen;
     *   unsigned char d_type;
     *   char d_name[256];
     * };
     */
    struct wasm_dirent64 {
        U64 d_ino;
        I64 d_off;
        U16 d_reclen;
        U8 d_type;
        U8 d_name[256];
    };

    // Sockets/ network
    enum SocketCalls : U32 {
        sc_socket = 1,
        sc_bind,
        sc_connect,
        sc_listen,
        sc_accept,
        sc_getsockname,
        sc_getpeername,
        sc_socketpair,
        sc_send,
        sc_recv,
        sc_sendto,
        sc_recvfrom,
        sc_shutdown,
        sc_setsockopt,
        sc_getsockopt,
        sc_sendmsg,
        sc_recvmsg,
        sc_accept4,
        sc_recvmmsg,
        sc_sendmmsg,
    };

    // List of file descriptors held by the current thread
    void addFdForThisThread(int fd);

    void removeFdForThisThread(int fd);

    void checkThreadOwnsFd(int fd);

    // Struct conversion

    sockaddr getSockAddr(I32 addrPtr);

    void writeNativeStatToWasmStat(struct stat64 *nativeStatPtr, I32 wasmStatPtr);

    iovec *wasmIovecsToNativeIovecs(I32 wasmIovecPtr, I32 wasmIovecCount);

    // Faasm

    std::shared_ptr<state::StateKeyValue> getStateKV(I32 keyPtr, size_t size);

    std::shared_ptr<state::StateKeyValue> getStateKVRead(I32 keyPtr, size_t size, int async);

    // Syscalls

    int executeSyscall(int syscallNumber, int a, int b, int c, int d, int e, int f, int g);

    I32 s__brk(I32 addr);

    I32 s__clock_gettime(I32 clockId, I32 timespecPtr);

    I32 s__close(I32 fd);

    I32 s__dup(I32 oldFd);

    I32 s__exit(I32 a, I32 b);

    I32 s__fcntl64(I32 fd, I32 cmd, I32 c);

    I32 s__fork();

    I32 s__fstat64(I32 fd, I32 statBufPtr);

    I32 s__futex(I32 uaddrPtr, I32 futex_op, I32 val, I32 timeoutPtr, I32 uaddr2Ptr, I32 other);

    I32 s__getcwd(I32 bufPtr, I32 bufLen);

    I32 s__getdents64(I32 fd, I32 wasmDirentBuf, I32 wasmDirentBufLen);

    I32 s__geteuid32();

    I32 s__getegid32();

    I32 s__getgid32();

    I32 s__getpid();

    I32 s__gettimeofday(int tvPtr, int tzPtr);

    I32 s__getuid32();

    I32 s__ioctl(I32 fd, I32 request, I32 argPtr, I32 d, I32 e, I32 f);

    I32 s__llseek(I32 fd, I32 offsetHigh, I32 offsetLow, I32 resultPtr, I32 whence);

    I32 s__lstat64(I32 pathPtr, I32 statBufPtr);

    I32 s__madvise(I32 address, I32 numBytes, I32 advice);

    I32 s__membarrier(I32 a);

    I32 s__mkdir(I32 pathPtr, I32 mode);

    I32 s__mmap(I32 addr, I32 length, I32 prot, I32 flags, I32 fd, I32 offset);

    I32 s__munmap(I32 addr, I32 length);

    I32 s__open(I32 pathPtr, I32 flags, I32 mode);

    I32 s__poll(I32 fdsPtr, I32 nfds, I32 timeout);

    I32 s__read(I32 fd, I32 bufPtr, I32 bufLen);

    I32 s__readlink(I32 pathPtr, I32 bufPtr, I32 bufLen);

    I32 s__readv(I32 fd, I32 iovecPtr, I32 iovecCount);

    I32 s__rename(I32 srcPtr, I32 destPtr);

    I32 s__rt_sigprocmask(I32 how, I32 sigSetPtr, I32 oldSetPtr, I32 sigsetsize);

    I32 s__sigaction(I32 a, I32 b, I32 c);

    I32 s__sigaltstack(I32 ssPtr, I32 oldSsPtr);

    I32 s__sigemptyset(I32 a);

    I32 s__siginterrupt(I32 a, I32 b);

    I32 s__signal(I32 a, I32 b);

    I32 s__socketcall(I32 call, I32 argsPtr);

    I32 s__stat64(I32 pathPtr, I32 statBufPtr);

    I32 s__unlink(I32 pathPtr);

    I32 s__write(I32 fd, I32 bufPtr, I32 bufLen);

    I32 s__writev(I32 fd, I32 iov, I32 iovcnt);

    // Hack to include dynlink
    void dynlinkLink();
}