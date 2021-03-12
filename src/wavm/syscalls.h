#pragma once

#include <WAVM/WASI/WASIABI.h>
#include <faabric/scheduler/MpiContext.h>
#include <sys/socket.h>

#define FAKE_NAME "faasm"
#define FAKE_PASSWORD "foobar123"
#define FAKE_HOME "/home/faasm/"
#define FAKE_WORKING_DIR "/bin"
#define FAKE_HOSTNAME "faasm"

#define FAKE_PID 23
#define FAKE_TID 66
#define FAKE_UID 1000
#define FAKE_GID 1000
#define FAKE_N_PROCESSORS 4

#define MAIN_MODULE_DYNLINK_HANDLE 999

namespace wasm {
void getBytesFromWasm(int32_t dataPtr, int32_t dataLen, uint8_t* buffer);

std::vector<uint8_t> getBytesFromWasm(int32_t dataPtr, int32_t dataLen);

std::string getStringFromWasm(int32_t strPtr);

std::pair<std::string, std::string> getUserKeyPairFromWasm(int32_t keyPtr);

std::string getMaskedPathFromWasm(int32_t strPtr);

int spawnChainedThread(const std::string& snapshotKey,
                       size_t snapshotSize,
                       int funcPtr,
                       int argsPtr);

// ---------------------------
// System-related structs
// ---------------------------

/**
 * Any structs passed as arguments must be re-implemented here with the
 * following mappings (respecting signed/ unsigned):
 *
 * int64_t = int64_t/uint64_t
 * short = int16_t/uint16_t
 * long = int32_t/uint32_t
 * int = int32_t/uint32_t
 * char = uint8_t
 * pointers = Uptr
 * size_t = int32_t
 * time_t = int64_t
 *
 * You need to look at include/bits/alltypes.h in the relevant sysroot to get a
 * lot of the types
 */

// Timing
struct wasm_timespec
{
    int64_t tv_sec;
    int32_t tv_nsec;
};

// Timing
struct wasm_timeval
{
    int64_t tv_sec;
    int32_t tv_usec;
};

// I/O
struct wasm_iovec
{
    uint32_t iov_base;
    uint32_t iov_len;
};

/* MPI-related interfacing structs and calls.
 */
struct wasm_faabric_win_t
{
    uint32_t worldId;
    uint32_t rank;
    uint32_t size;
    uint32_t wasmPtr;
    uint32_t dispUnit;
};

/** Socket-related struct (see
 * https://beej.us/guide/bgnet/html/multi/sockaddr_inman.html) */
struct wasm_sockaddr
{
    uint16_t sa_family;
    uint8_t sa_data[14];
};

// Taken from arch/wasm32/bits/stat.h
struct wasm_stat
{
    uint64_t st_dev;
    uint64_t st_ino;
    uint64_t st_nlink;

    uint32_t st_mode;
    uint32_t st_uid;
    uint32_t st_gid;
    uint32_t __pad0;
    uint64_t st_rdev;
    int64_t st_size;

    int32_t st_blksize;
    int64_t st_blocks;

    struct wasm_timespec st_atim;
    struct wasm_timespec st_mtim;
    struct wasm_timespec st_ctim;

    int64_t __unused[3];
};

/**
 * Found in pwd.h.
 *
 * Note that char pointers are uint32_t
 */
struct wasm_passwd
{
    uint32_t pw_name;   // char*
    uint32_t pw_passwd; // char*
    uint32_t pw_uid;
    uint32_t pw_gid;
    uint32_t pw_gecos; // char*
    uint32_t pw_dir;   // char*
    uint32_t pw_shell; // char*
};

/**
 * To double check this, work out which header from the sysroot is resolved
 * Currently this is:
 *  - include/__struct_dirent.h
 *  - include/__typedef_ino_t.h
 *
 * struct dirent {
 *     ino_t d_ino;  # unsigned long long
 *     unsigned char d_type;
 *     char d_name[];
 * };
 */
struct wasm_dirent64
{
    uint64_t d_ino;
    uint8_t d_type;
    uint8_t d_name[];
};

/**
 * Found in bits/signal.h
 * stack_t is used in calls to sigaltstack, either to specify a new stack
 * or to get details about the existing
 */
struct wasm_stack_t
{
    uint32_t ss_sp; // void*
    int32_t ss_flags;
    int32_t ss_size; // size_t
};

/**
 * Found in sys/utsname.h
 * Used to convey system info via uname
 */
struct wasm_utsname
{
    char sysname[65];
    char nodename[65];
    char release[65];
    char version[65];
    char machine[65];
    char domainname[65];
};

/**
 * Found in ioctl.h
 * Used to communicate size of the window we're operating in
 */
struct wasm_winsize
{
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
};

/**
 * Found in pthread_impl.h
 * The "real" pthread struct has a lot of stuff in it. We only
 * care about a subset of the fields that appear at the start,
 * _especially_ the pointer to itself, which allows references to
 * be treated like pointers.
 */
struct wasm_pthread
{
    int32_t selfPtr;
};

// Sockets/ network
enum SocketCalls : uint32_t
{
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

// Struct conversion

sockaddr getSockAddr(int32_t addrPtr);

void writeNativeStatToWasmStat(struct stat64* nativeStatPtr,
                               int32_t wasmStatPtr);

iovec* wasmIovecsToNativeIovecs(int32_t wasmIovecPtr, int32_t wasmIovecCount);

iovec* wasiIovecsToNativeIovecs(int32_t wasiIovecPtr, int32_t wasiIovecCount);

// Faasm

std::shared_ptr<faabric::state::StateKeyValue> getStateKV(int32_t keyPtr,
                                                          size_t size);

std::shared_ptr<faabric::state::StateKeyValue> getStateKV(int32_t keyPtr);

// Syscalls

int executeSyscall(int syscallNumber,
                   int a,
                   int b,
                   int c,
                   int d,
                   int e,
                   int f,
                   int g);

int32_t s__access(int32_t pathPtr, int32_t mode);

int32_t s__brk(int32_t addr);

int32_t s__clock_gettime(int32_t clockId, int32_t timespecPtr);

int32_t s__close(int32_t fd);

int32_t s__dup(int32_t oldFd);

int32_t s__exit(int32_t a, int32_t b);

int32_t s__fcntl64(int32_t fd, int32_t cmd, int32_t c);

int32_t s__fork();

int32_t s__fstat64(int32_t fd, int32_t statBufPtr);

int32_t s__futex(int32_t uaddrPtr,
                 int32_t futex_op,
                 int32_t val,
                 int32_t timeoutPtr,
                 int32_t uaddr2Ptr,
                 int32_t other);

int32_t s__getdents64(int32_t fd,
                      int32_t wasmDirentBuf,
                      int32_t wasmDirentBufLen);

int32_t s__gettid();

int32_t s__getrandom(int32_t bufPtr, int32_t bufLen, int32_t flags);

int32_t s__gettimeofday(int32_t tvPtr, int32_t tzPtr);

int32_t s__ioctl(int32_t fd,
                 int32_t request,
                 int32_t argPtr,
                 int32_t d,
                 int32_t e,
                 int32_t f);

int32_t s__llseek(int32_t fd,
                  int32_t offsetHigh,
                  int32_t offsetLow,
                  int32_t resultPtr,
                  int32_t whence);

int32_t s__lstat64(int32_t pathPtr, int32_t statBufPtr);

int32_t s__madvise(int32_t address, int32_t numBytes, int32_t advice);

int32_t s__membarrier(int32_t a);

int32_t s__mkdir(int32_t pathPtr, int32_t mode);

int32_t s__mmap(int32_t addr,
                int32_t length,
                int32_t prot,
                int32_t flags,
                int32_t fd,
                int32_t offset);

int32_t s__mprotect(int32_t addrPtr, int32_t len, int32_t prot);

int32_t s__munmap(int32_t addr, int32_t length);

int32_t s__nanosleep(int32_t reqPtr, int32_t remPtr);

int32_t s__open(int32_t pathPtr, int32_t flags, int32_t mode);

int32_t s__poll(int32_t fdsPtr, int32_t nfds, int32_t timeout);

int32_t s__read(int32_t fd, int32_t bufPtr, int32_t bufLen);

int32_t s__readlink(int32_t pathPtr, int32_t bufPtr, int32_t bufLen);

int32_t s__readv(int32_t fd, int32_t iovecPtr, int32_t iovecCount);

int32_t s__rename(int32_t srcPtr, int32_t destPtr);

int32_t s__rt_sigprocmask(int32_t how,
                          int32_t sigSetPtr,
                          int32_t oldSetPtr,
                          int32_t sigsetsize);

int32_t s__sbrk(int32_t addr);

int32_t s__sched_getaffinity(int32_t pid, int32_t cpuSetSize, int32_t maskPtr);

int32_t s__sigaction(int32_t a, int32_t b, int32_t c);

int32_t s__sigaltstack(int32_t ssPtr, int32_t oldSsPtr);

int32_t s__sigemptyset(int32_t a);

int32_t s__siginterrupt(int32_t a, int32_t b);

int32_t s__socketcall(int32_t call, int32_t argsPtr);

int32_t s__stat64(int32_t pathPtr, int32_t statBufPtr);

int32_t s__unlink(int32_t pathPtr);

int32_t s__write(int32_t fd, int32_t bufPtr, int32_t bufLen);

int32_t s__writev(int32_t fd, int32_t iov, int32_t iovcnt);

// Hack to include other files
void chainLink();

void dynlinkLink();

void envLink();

void faasmLink();

void ioLink();

void libcxxLink();

void mathsLink();

void memoryLink();

void messagesLink();

void mpiLink();

void networkLink();

void ompLink();

void processLink();

void schedulingLink();

void signalsLink();

void threadsLink();

void timingLink();

void wasiLink();
}
