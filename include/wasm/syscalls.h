#pragma once

#include <sys/socket.h>

namespace wasm {
    const char *FAKE_NAME = "faasm";
    const char *FAKE_PASSWORD = "foobar123";
    const char *FAKE_HOME = "/";
    const int FAKE_PID = 23;
    const int FAKE_UID = 1;
    const int FAKE_GID = 1;
    const char *FALSE_ROOT = "/usr/local/faasm/runtime_root";
    const char *HOSTS_FILE = "/usr/local/faasm/net/hosts";
    const char *RESOLV_FILE = "/usr/local/faasm/net/resolv.conf";

    void getBytesFromWasm(I32 dataPtr, I32 dataLen, uint8_t *buffer);

    std::vector<uint8_t> getBytesFromWasm(I32 dataPtr, I32 dataLen);

    std::string getStringFromWasm(I32 strPtr);

    std::pair<std::string, std::string> getUserKeyPairFromWasm(I32 keyPtr);

    std::string maskPath(std::string originalPath);

    std::string getMaskedPathFromWasm(I32 strPtr);

    // ---------------------------
    // System-related structs
    // ---------------------------

    // Timing
    struct wasm_timespec {
        I32 tv_sec;
        I32 tv_nsec;
    };

    // Timing
    struct wasm_timeval {
        I32 tv_sec;
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

    // Taken from arch/wasm32/bits/stat.h in the musl port
    struct wasm_stat {
        U32 st_dev;
        I32 __st_dev_padding;
        I32 __st_ino_truncated;
        U32 st_mode;
        U32 st_nlink;
        U32 st_uid;
        U32 st_gid;
        U32 st_rdev;
        I32 __st_rdev_padding;
        I32 st_size;
        I32 st_blksize;
        I32 st_blocks;
        struct wasm_timespec st_atim;
        struct wasm_timespec st_mtim;
        struct wasm_timespec st_ctim;
        U32 st_ino;
    };

    /**
     * Note that char pointers are U32
     */
    struct wasm_passwd {
        U32 pw_name;
        U32 pw_passwd;
        U32 pw_uid;
        U32 pw_gid;
        U32 pw_gecos;
        U32 pw_dir;
        U32 pw_shell;
    };

    /**
     * Note, this struct type works with emscripten, but may not work with musl implementations
     */
    struct wasm_dirent64 {
        U32 d_ino;
        I32 d_off;
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

    // List of file descriptors held by the curren thread
    static thread_local std::set<int> openFds;

    void checkThreadOwnsFd(int fd);

    sockaddr getSockAddr(I32 addrPtr);

    void writeNativeStatToWasmStat(struct stat64 *nativeStatPtr, I32 wasmStatPtr);

    int executeSyscall(int syscallNumber, int a, int b, int c, int d, int e, int f, int g);

    iovec *wasmIovecsToNativeIovecs(I32 wasmIovecPtr, I32 wasmIovecCount);

    I32 s__exit(I32 a, I32 b); // 1

    I32 s__getpid(); // 20

    I32 s__getuid32(); //199

    I32 s__getgid32(); //200

    I32 s__geteuid32(); //201

    I32 s__getegid32(); //202

    I32 s__open(I32 pathPtr, I32 flags, I32 mode);
    I32 s__dup(I32 oldFd);
    I32 s__fcntl64(I32 fd, I32 cmd, I32 c);
    I32 s__getdents64(I32 fd, I32 wasmDirentBuf, I32 wasmDirentBufLen);
    I32 s__getcwd(I32 bufPtr, I32 bufLen);
    I32 s__read(I32 fd, I32 bufPtr, I32 bufLen);
    I32 s__readlink(I32 pathPtr, I32 bufPtr, I32 bufLen);

}