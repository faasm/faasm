(module
  (type $0 (func (param i32 i32 i32) (result i32)))
  (type $1 (func (param i32)))
  (type $2 (func (param i32) (result i32)))
  (type $3 (func (param i32 i32 i32 i32 i32 i32) (result i32)))
  (type $4 (func (param i32 i32 i32 i32 i32) (result i32)))
  (type $5 (func))
  (type $6 (func (param i32 i32) (result i32)))
  (type $7 (func (param i32 i64 i32) (result i64)))
  (type $8 (func (result i32)))
  (type $9 (func (param i64 i64) (result i32)))
  (type $10 (func (param i32 i64 i64 i32)))
  (type $11 (func (param i32 i32 i32)))
  (type $12 (func (param i32 i32 i32 i32 i32)))
  (type $13 (func (param i64 i32) (result i32)))
  (type $14 (func (param i64 i64 i64 i64) (result i32)))
  (type $15 (func (param i32 f64)))
  (type $16 (func (param i32 i32)))
  (type $17 (func (param i32 i64 i64 i64 i64)))
  (import "env" "exit" (func $exit (param i32)))
  (import "env" "__syscall_close" (func $__syscall_close (param i32) (result i32)))
  (import "env" "__syscall_writev" (func $__syscall_writev (param i32 i32 i32) (result i32)))
  (import "env" "__syscall_ioctl" (func $__syscall_ioctl (param i32 i32 i32 i32 i32 i32) (result i32)))
  (import "env" "__syscall_llseek" (func $__syscall_llseek (param i32 i32 i32 i32 i32) (result i32)))
  (import "env" "__syscall_futex" (func $__syscall_futex (param i32 i32 i32 i32 i32 i32) (result i32)))
  (export "memory" (memory $19))
  (export "__heap_base" (global $21))
  (export "__data_end" (global $22))
  (export "_start" (func $_start))
  (memory $19  2)
  (table $18  5 5 anyfunc)
  (global $20  (mut i32) (i32.const 70224))
  (global $21  i32 (i32.const 70224))
  (global $22  i32 (i32.const 4680))
  (elem $18 (i32.const 1)
    $__stdio_write $__stdio_close $__stdout_write $__stdio_seek)
  (data $19 (i32.const 1024)
    "f: %f\n\00d: %f\n\00i: %i\n\00s: %i\n\00\a8\0d\00\00T!\"\19\0d\01\02\03\11K\1c\0c\10\04\0b\1d\12\1e'hnopqb \05\06\0f\13\14\15"
    "\1a\08\16\07($\17\18\09\n\0e\1b\1f%#\83\82}&*+<=>?CGJMXYZ[\\]^_`acdefgijklrstyz{|\00\00\00\00\00\00\00\00\00"
    "Illegal byte sequence\00Domain error\00Result not representable\00Not "
    "a tty\00Permission denied\00Operation not permitted\00No such file or "
    "directory\00No such process\00File exists\00Value too large for data t"
    "ype\00No space left on device\00Out of memory\00Resource busy\00Interrup"
    "ted system call\00Resource temporarily unavailable\00Invalid seek\00Cr"
    "oss-device link\00Read-only file system\00Directory not empty\00Connec"
    "tion reset by peer\00Operation timed out\00Connection refused\00Host i"
    "s down\00Host is unreachable\00Address in use\00Broken pipe\00I/O error\00"
    "No such device or address\00Block device required\00No such device\00N"
    "ot a directory\00Is a directory\00Text file busy\00Exec format error\00I"
    "nvalid argument\00Argument list too long\00Symbolic link loop\00Filena"
    "me too long\00Too many open files in system\00No file descriptors av"
    "ailable\00Bad file descriptor\00No child process\00Bad address\00File to"
    "o large\00Too many links\00No locks available\00Resource deadlock woul"
    "d occur\00State not recoverable\00Previous owner died\00Operation canc"
    "eled\00Function not implemented\00No message of desired type\00Identif"
    "ier removed\00Device not a stream\00No data available\00Device timeout"
    "\00Out of streams resources\00Link has been severed\00Protocol error\00B"
    "ad message\00File descriptor in bad state\00Not a socket\00Destination"
    " address required\00Message too large\00Protocol wrong type for sock"
    "et\00Protocol not available\00Protocol not supported\00Socket type not"
    " supported\00Not supported\00Protocol family not supported\00Address f"
    "amily not supported by protocol\00Address not available\00Network is"
    " down\00Network unreachable\00Connection reset by network\00Connection"
    " aborted\00No buffer space available\00Socket is connected\00Socket no"
    "t connected\00Cannot send after socket shutdown\00Operation already "
    "in progress\00Operation in progress\00Stale file handle\00Remote I/O e"
    "rror\00Quota exceeded\00No medium found\00Wrong medium type\00No error i"
    "nformation\00\00-+   0X0x\00(null)\00\00\00\00\19\00\n\00\19\19\19\00\00\00\00\05\00\00\00\00\00\00\09\00\00\00\00\0b\00\00\00\00\00\00\00\00"
    "\19\00\11\n\19\19\19\03\n\07\00\01\1b\09\0b\18\00\00\09\06\0b\00\00\0b\00\06\19\00\00\00\19\19\19\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\0e\00\00\00\00\00\00\00\00\19\00\n\0d\19\19"
    "\19\00\0d\00\00\02\00\09\0e\00\00\00\09\00\0e\00\00\0e\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\0c\00\00\00\00\00\00\00\00\00\00\00\13\00\00\00\00\13\00\00\00"
    "\00\09\0c\00\00\00\00\00\0c\00\00\0c\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\10\00\00\00\00\00\00\00\00\00\00\00\0f\00\00\00\04\0f\00\00\00\00\09\10\00\00\00"
    "\00\00\10\00\00\10\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\12\00\00\00\00\00\00\00\00\00\00\00\11\00\00\00\00\11\00\00\00\00\09\12\00\00\00\00\00\12\00\00\12"
    "\00\00\1a\00\00\00\1a\1a\1a\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\1a\00\00\00\1a\1a\1a\00\00\00\00\00\00\09\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\14\00\00\00\00\00\00\00\00\00\00\00\17\00\00\00\00\17\00\00\00\00\09\14\00\00\00\00\00\14\00\00\14\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\16\00\00\00\00\00\00\00\00\00\00\00\15\00\00\00\00\15\00\00\00\00\09\16\00\00\00\00\00\16\00\00\16\00\000123456789ABCDEF"
    "-0X+0X 0X-0x+0x 0x\00inf\00INF\00nan\00NAN\00.\00")
  (data $19 (i32.const 3496)
    "\05\00\00\00\00\00\00\00\00\00\00\00\02\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\03\00\00\00\04\00\00\00H\0e\00\00\00\04\00\00\00\00\00\00\00\00\00\00\01\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\n\ff\ff\ff\ff\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
  (data $19 (i32.const 3648)
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00")
  
  (func $__wasm_call_ctors (type $5)
    )
  
  (func $_start (type $5)
    (local $0 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $0
    set_global $20
    get_local $0
    i32.const 0
    i32.store offset=12
    get_local $0
    i32.const 12
    i32.add
    call $_start_c
    unreachable
    )
  
  (func $_start_c (type $1)
    (param $0 i32)
    get_local $0
    i32.load
    get_local $0
    i32.const 4
    i32.add
    call $main
    call $exit
    unreachable
    )
  
  (func $main (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    (local $2 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    get_local $2
    i32.const 8
    i32.add
    call $faasm::FaasmMemory::FaasmMemory_lparen__rparen_
    drop
    get_local $2
    call $faasm::exec_lparen_faasm::FaasmMemory*_rparen_
    drop
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    i32.const 0
    )
  
  (func $faasm::exec_lparen_faasm::FaasmMemory*_rparen_ (type $2)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    get_global $20
    i32.const 64
    i32.sub
    tee_local $1
    set_global $20
    get_local $1
    i64.const 4620569368542052352
    i64.store offset=48
    i32.const 1024
    get_local $1
    i32.const 48
    i32.add
    call $printf
    drop
    get_local $1
    i64.const 4616820122002590269
    i64.store offset=32
    i32.const 1031
    get_local $1
    i32.const 32
    i32.add
    call $printf
    drop
    get_local $1
    i32.const 7
    i32.store offset=16
    i32.const 1038
    get_local $1
    i32.const 16
    i32.add
    call $printf
    drop
    get_local $1
    i32.const 8
    i32.store
    i32.const 1045
    get_local $1
    call $printf
    drop
    get_local $1
    i32.const 64
    i32.add
    set_global $20
    i32.const 0
    )
  
  (func $faasm::FaasmMemory::FaasmMemory_lparen__rparen_ (type $2)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    get_global $20
    set_local $1
    i32.const 16
    set_local $2
    get_local $1
    get_local $2
    i32.sub
    set_local $3
    get_local $3
    get_local $0
    i32.store offset=12
    get_local $3
    i32.load offset=12
    set_local $4
    get_local $4
    return
    )
  
  (func $printf (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    (local $2 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    get_local $2
    get_local $1
    i32.store offset=12
    i32.const 0
    i32.load offset=1052
    get_local $0
    get_local $1
    call $vfprintf
    set_local $1
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    get_local $1
    )
  
  (func $dummy (type $2)
    (param $0 i32)
    (result i32)
    get_local $0
    )
  
  (func $__stdio_close (type $2)
    (param $0 i32)
    (result i32)
    get_local $0
    i32.load offset=60
    call $dummy
    call $__syscall_close
    call $__syscall_ret
    )
  
  (func $__stdio_write (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $3
    set_global $20
    get_local $3
    get_local $2
    i32.store offset=12
    get_local $3
    get_local $1
    i32.store offset=8
    get_local $3
    get_local $0
    i32.load offset=28
    tee_local $1
    i32.store
    get_local $3
    get_local $0
    i32.load offset=20
    get_local $1
    i32.sub
    tee_local $1
    i32.store offset=4
    i32.const 2
    set_local $4
    block $block
      block $block_0
        block $block_1
          get_local $1
          get_local $2
          i32.add
          tee_local $5
          get_local $0
          i32.load offset=60
          get_local $3
          i32.const 2
          call $__syscall_writev
          call $__syscall_ret
          tee_local $6
          i32.eq
          br_if $block_1
          get_local $3
          set_local $1
          get_local $0
          i32.const 60
          i32.add
          set_local $7
          loop $loop
            get_local $6
            i32.const -1
            i32.le_s
            br_if $block_0
            get_local $1
            i32.const 8
            i32.add
            get_local $1
            get_local $6
            get_local $1
            i32.load offset=4
            tee_local $8
            i32.gt_u
            tee_local $9
            select
            tee_local $1
            get_local $1
            i32.load
            get_local $6
            get_local $8
            i32.const 0
            get_local $9
            select
            i32.sub
            tee_local $8
            i32.add
            i32.store
            get_local $1
            get_local $1
            i32.load offset=4
            get_local $8
            i32.sub
            i32.store offset=4
            get_local $5
            get_local $6
            i32.sub
            set_local $5
            get_local $7
            i32.load
            get_local $1
            get_local $4
            get_local $9
            i32.sub
            tee_local $4
            call $__syscall_writev
            call $__syscall_ret
            tee_local $9
            set_local $6
            get_local $5
            get_local $9
            i32.ne
            br_if $loop
          end ;; $loop
        end ;; $block_1
        get_local $0
        i32.const 28
        i32.add
        get_local $0
        i32.load offset=44
        tee_local $1
        i32.store
        get_local $0
        i32.const 20
        i32.add
        get_local $1
        i32.store
        get_local $0
        get_local $1
        get_local $0
        i32.load offset=48
        i32.add
        i32.store offset=16
        get_local $2
        set_local $6
        br $block
      end ;; $block_0
      get_local $0
      i64.const 0
      i64.store offset=16
      i32.const 0
      set_local $6
      get_local $0
      i32.const 28
      i32.add
      i32.const 0
      i32.store
      get_local $0
      get_local $0
      i32.load
      i32.const 32
      i32.or
      i32.store
      get_local $4
      i32.const 2
      i32.eq
      br_if $block
      get_local $2
      get_local $1
      i32.load offset=4
      i32.sub
      set_local $6
    end ;; $block
    get_local $3
    i32.const 16
    i32.add
    set_global $20
    get_local $6
    )
  
  (func $__stdout_write (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $3
    set_global $20
    get_local $0
    i32.const 1
    i32.store offset=36
    block $block
      get_local $0
      i32.load8_u
      i32.const 64
      i32.and
      br_if $block
      get_local $0
      i32.load offset=60
      i32.const 21523
      get_local $3
      i32.const 8
      i32.add
      i32.const 0
      i32.const 0
      i32.const 0
      call $__syscall_ioctl
      i32.eqz
      br_if $block
      get_local $0
      i32.const 255
      i32.store8 offset=75
    end ;; $block
    get_local $0
    get_local $1
    get_local $2
    call $__stdio_write
    set_local $0
    get_local $3
    i32.const 16
    i32.add
    set_global $20
    get_local $0
    )
  
  (func $__stdio_seek (type $7)
    (param $0 i32)
    (param $1 i64)
    (param $2 i32)
    (result i64)
    (local $3 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $3
    set_global $20
    block $block
      block $block_0
        get_local $0
        i32.load offset=60
        get_local $1
        i64.const 32
        i64.shr_u
        i32.wrap/i64
        get_local $1
        i32.wrap/i64
        get_local $3
        i32.const 8
        i32.add
        get_local $2
        call $__syscall_llseek
        call $__syscall_ret
        i32.const 0
        i32.lt_s
        br_if $block_0
        get_local $3
        i64.load offset=8
        set_local $1
        br $block
      end ;; $block_0
      i64.const -1
      set_local $1
      get_local $3
      i64.const -1
      i64.store offset=8
    end ;; $block
    get_local $3
    i32.const 16
    i32.add
    set_global $20
    get_local $1
    )
  
  (func $__errno_location (type $8)
    (result i32)
    i32.const 31
    )
  
  (func $__syscall_ret (type $2)
    (param $0 i32)
    (result i32)
    block $block
      get_local $0
      i32.const -4095
      i32.lt_u
      br_if $block
      call $__errno_location
      i32.const 0
      get_local $0
      i32.sub
      i32.store
      i32.const -1
      set_local $0
    end ;; $block
    get_local $0
    )
  
  (func $memset (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    block $block
      get_local $2
      i32.eqz
      br_if $block
      get_local $0
      get_local $1
      i32.store8
      get_local $0
      get_local $2
      i32.add
      tee_local $3
      i32.const -1
      i32.add
      get_local $1
      i32.store8
      get_local $2
      i32.const 3
      i32.lt_u
      br_if $block
      get_local $0
      get_local $1
      i32.store8 offset=2
      get_local $0
      get_local $1
      i32.store8 offset=1
      get_local $3
      i32.const -3
      i32.add
      get_local $1
      i32.store8
      get_local $3
      i32.const -2
      i32.add
      get_local $1
      i32.store8
      get_local $2
      i32.const 7
      i32.lt_u
      br_if $block
      get_local $0
      get_local $1
      i32.store8 offset=3
      get_local $3
      i32.const -4
      i32.add
      get_local $1
      i32.store8
      get_local $2
      i32.const 9
      i32.lt_u
      br_if $block
      get_local $0
      i32.const 0
      get_local $0
      i32.sub
      i32.const 3
      i32.and
      tee_local $4
      i32.add
      tee_local $3
      get_local $1
      i32.const 255
      i32.and
      i32.const 16843009
      i32.mul
      tee_local $1
      i32.store
      get_local $3
      get_local $2
      get_local $4
      i32.sub
      i32.const -4
      i32.and
      tee_local $4
      i32.add
      tee_local $2
      i32.const -4
      i32.add
      get_local $1
      i32.store
      get_local $4
      i32.const 9
      i32.lt_u
      br_if $block
      get_local $3
      get_local $1
      i32.store offset=8
      get_local $3
      get_local $1
      i32.store offset=4
      get_local $2
      i32.const -8
      i32.add
      get_local $1
      i32.store
      get_local $2
      i32.const -12
      i32.add
      get_local $1
      i32.store
      get_local $4
      i32.const 25
      i32.lt_u
      br_if $block
      get_local $3
      get_local $1
      i32.store offset=24
      get_local $3
      get_local $1
      i32.store offset=20
      get_local $3
      get_local $1
      i32.store offset=16
      get_local $3
      get_local $1
      i32.store offset=12
      get_local $2
      i32.const -16
      i32.add
      get_local $1
      i32.store
      get_local $2
      i32.const -20
      i32.add
      get_local $1
      i32.store
      get_local $2
      i32.const -24
      i32.add
      get_local $1
      i32.store
      get_local $2
      i32.const -28
      i32.add
      get_local $1
      i32.store
      get_local $4
      get_local $3
      i32.const 4
      i32.and
      i32.const 24
      i32.or
      tee_local $5
      i32.sub
      tee_local $2
      i32.const 32
      i32.lt_u
      br_if $block
      get_local $1
      i64.extend_u/i32
      tee_local $6
      i64.const 32
      i64.shl
      get_local $6
      i64.or
      set_local $6
      get_local $3
      get_local $5
      i32.add
      set_local $1
      loop $loop
        get_local $1
        get_local $6
        i64.store
        get_local $1
        i32.const 24
        i32.add
        get_local $6
        i64.store
        get_local $1
        i32.const 16
        i32.add
        get_local $6
        i64.store
        get_local $1
        i32.const 8
        i32.add
        get_local $6
        i64.store
        get_local $1
        i32.const 32
        i32.add
        set_local $1
        get_local $2
        i32.const -32
        i32.add
        tee_local $2
        i32.const 31
        i32.gt_u
        br_if $loop
      end ;; $loop
    end ;; $block
    get_local $0
    )
  
  (func $__lockfile (type $2)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    (local $2 i32)
    i32.const 0
    set_local $1
    block $block
      get_local $0
      i32.load offset=76
      i32.const -1073741825
      i32.and
      i32.const 0
      i32.load offset=27
      tee_local $2
      i32.eq
      br_if $block
      get_local $0
      i32.const 76
      i32.add
      set_local $0
      block $block_0
        loop $loop
          get_local $0
          i32.load
          tee_local $1
          i32.eqz
          br_if $block_0
          get_local $0
          i32.load
          get_local $1
          i32.ne
          br_if $loop
        end ;; $loop
        get_local $0
        get_local $1
        i32.const 1073741824
        i32.or
        i32.store
        block $block_1
          get_local $0
          i32.load
          tee_local $1
          i32.eqz
          br_if $block_1
          loop $loop_0
            block $block_2
              get_local $0
              i32.const 128
              get_local $1
              i32.const 0
              i32.const 0
              i32.const 0
              call $__syscall_futex
              i32.const -38
              i32.ne
              br_if $block_2
              get_local $0
              i32.const 0
              get_local $1
              i32.const 0
              i32.const 0
              i32.const 0
              call $__syscall_futex
              drop
            end ;; $block_2
            get_local $0
            i32.load
            tee_local $1
            br_if $loop_0
          end ;; $loop_0
        end ;; $block_1
        get_local $2
        i32.const 1073741824
        i32.or
        set_local $2
      end ;; $block_0
      get_local $0
      get_local $2
      i32.store
      i32.const 1
      set_local $1
    end ;; $block
    get_local $1
    )
  
  (func $__unlockfile (type $1)
    (param $0 i32)
    (local $1 i32)
    get_local $0
    i32.const 76
    i32.add
    set_local $0
    loop $loop
      get_local $0
      i32.load
      tee_local $1
      get_local $0
      i32.load
      i32.ne
      br_if $loop
    end ;; $loop
    get_local $0
    i32.const 0
    i32.store
    block $block
      get_local $1
      i32.const 1073741824
      i32.and
      i32.eqz
      br_if $block
      get_local $0
      i32.const 129
      i32.const 1
      i32.const 0
      i32.const 0
      i32.const 0
      call $__syscall_futex
      i32.const -38
      i32.ne
      br_if $block
      get_local $0
      i32.const 1
      i32.const 1
      i32.const 0
      i32.const 0
      i32.const 0
      call $__syscall_futex
      drop
    end ;; $block
    )
  
  (func $__towrite (type $2)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    get_local $0
    get_local $0
    i32.load8_u offset=74
    tee_local $1
    i32.const -1
    i32.add
    get_local $1
    i32.or
    i32.store8 offset=74
    block $block
      get_local $0
      i32.load
      tee_local $1
      i32.const 8
      i32.and
      br_if $block
      get_local $0
      i64.const 0
      i64.store offset=4 align=4
      get_local $0
      get_local $0
      i32.load offset=44
      tee_local $1
      i32.store offset=28
      get_local $0
      get_local $1
      i32.store offset=20
      get_local $0
      get_local $1
      get_local $0
      i32.load offset=48
      i32.add
      i32.store offset=16
      i32.const 0
      return
    end ;; $block
    get_local $0
    get_local $1
    i32.const 32
    i32.or
    i32.store
    i32.const -1
    )
  
  (func $memcpy (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    (local $10 i32)
    block $block
      block $block_0
        get_local $2
        i32.eqz
        br_if $block_0
        get_local $1
        i32.const 3
        i32.and
        i32.eqz
        br_if $block_0
        get_local $0
        set_local $3
        loop $loop
          get_local $3
          get_local $1
          i32.load8_u
          i32.store8
          get_local $2
          i32.const -1
          i32.add
          set_local $4
          get_local $3
          i32.const 1
          i32.add
          set_local $3
          get_local $1
          i32.const 1
          i32.add
          set_local $1
          get_local $2
          i32.const 1
          i32.eq
          br_if $block
          get_local $4
          set_local $2
          get_local $1
          i32.const 3
          i32.and
          br_if $loop
          br $block
        end ;; $loop
      end ;; $block_0
      get_local $2
      set_local $4
      get_local $0
      set_local $3
    end ;; $block
    block $block_1
      block $block_2
        block $block_3
          block $block_4
            block $block_5
              get_local $3
              i32.const 3
              i32.and
              tee_local $2
              i32.eqz
              br_if $block_5
              get_local $4
              i32.const 32
              i32.lt_u
              br_if $block_2
              get_local $2
              i32.const 3
              i32.eq
              br_if $block_4
              get_local $2
              i32.const 2
              i32.eq
              br_if $block_3
              get_local $2
              i32.const 1
              i32.ne
              br_if $block_2
              get_local $3
              get_local $1
              i32.load8_u offset=1
              i32.store8 offset=1
              get_local $3
              get_local $1
              i32.load
              tee_local $5
              i32.store8
              get_local $3
              get_local $1
              i32.load8_u offset=2
              i32.store8 offset=2
              get_local $4
              i32.const -3
              i32.add
              set_local $6
              get_local $3
              i32.const 3
              i32.add
              set_local $7
              get_local $4
              i32.const -20
              i32.add
              i32.const -16
              i32.and
              set_local $8
              i32.const 0
              set_local $2
              loop $loop_0
                get_local $7
                get_local $2
                i32.add
                tee_local $3
                get_local $1
                get_local $2
                i32.add
                tee_local $9
                i32.const 4
                i32.add
                i32.load
                tee_local $10
                i32.const 8
                i32.shl
                get_local $5
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $3
                i32.const 4
                i32.add
                get_local $9
                i32.const 8
                i32.add
                i32.load
                tee_local $5
                i32.const 8
                i32.shl
                get_local $10
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $3
                i32.const 8
                i32.add
                get_local $9
                i32.const 12
                i32.add
                i32.load
                tee_local $10
                i32.const 8
                i32.shl
                get_local $5
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $3
                i32.const 12
                i32.add
                get_local $9
                i32.const 16
                i32.add
                i32.load
                tee_local $5
                i32.const 8
                i32.shl
                get_local $10
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $2
                i32.const 16
                i32.add
                set_local $2
                get_local $6
                i32.const -16
                i32.add
                tee_local $6
                i32.const 16
                i32.gt_u
                br_if $loop_0
              end ;; $loop_0
              get_local $7
              get_local $2
              i32.add
              set_local $3
              get_local $1
              get_local $2
              i32.add
              i32.const 3
              i32.add
              set_local $1
              get_local $4
              i32.const -19
              i32.add
              get_local $8
              i32.sub
              set_local $4
              br $block_2
            end ;; $block_5
            block $block_6
              block $block_7
                get_local $4
                i32.const 16
                i32.lt_u
                br_if $block_7
                get_local $4
                i32.const -16
                i32.add
                set_local $2
                loop $loop_1
                  get_local $3
                  get_local $1
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 4
                  i32.add
                  get_local $1
                  i32.const 4
                  i32.add
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 8
                  i32.add
                  get_local $1
                  i32.const 8
                  i32.add
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 12
                  i32.add
                  get_local $1
                  i32.const 12
                  i32.add
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 16
                  i32.add
                  set_local $3
                  get_local $1
                  i32.const 16
                  i32.add
                  set_local $1
                  get_local $4
                  i32.const -16
                  i32.add
                  tee_local $4
                  i32.const 15
                  i32.gt_u
                  br_if $loop_1
                  br $block_6
                end ;; $loop_1
              end ;; $block_7
              get_local $4
              set_local $2
            end ;; $block_6
            block $block_8
              get_local $2
              i32.const 8
              i32.and
              i32.eqz
              br_if $block_8
              get_local $3
              get_local $1
              i64.load align=4
              i64.store align=4
              get_local $1
              i32.const 8
              i32.add
              set_local $1
              get_local $3
              i32.const 8
              i32.add
              set_local $3
            end ;; $block_8
            block $block_9
              get_local $2
              i32.const 4
              i32.and
              i32.eqz
              br_if $block_9
              get_local $3
              get_local $1
              i32.load
              i32.store
              get_local $1
              i32.const 4
              i32.add
              set_local $1
              get_local $3
              i32.const 4
              i32.add
              set_local $3
            end ;; $block_9
            block $block_10
              get_local $2
              i32.const 2
              i32.and
              i32.eqz
              br_if $block_10
              get_local $3
              get_local $1
              i32.load8_u
              i32.store8
              get_local $3
              get_local $1
              i32.load8_u offset=1
              i32.store8 offset=1
              get_local $3
              i32.const 2
              i32.add
              set_local $3
              get_local $1
              i32.const 2
              i32.add
              set_local $1
            end ;; $block_10
            get_local $2
            i32.const 1
            i32.and
            i32.eqz
            br_if $block_1
            get_local $3
            get_local $1
            i32.load8_u
            i32.store8
            get_local $0
            return
          end ;; $block_4
          get_local $3
          get_local $1
          i32.load
          tee_local $5
          i32.store8
          get_local $4
          i32.const -1
          i32.add
          set_local $6
          get_local $3
          i32.const 1
          i32.add
          set_local $7
          get_local $4
          i32.const -20
          i32.add
          i32.const -16
          i32.and
          set_local $8
          i32.const 0
          set_local $2
          loop $loop_2
            get_local $7
            get_local $2
            i32.add
            tee_local $3
            get_local $1
            get_local $2
            i32.add
            tee_local $9
            i32.const 4
            i32.add
            i32.load
            tee_local $10
            i32.const 24
            i32.shl
            get_local $5
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $3
            i32.const 4
            i32.add
            get_local $9
            i32.const 8
            i32.add
            i32.load
            tee_local $5
            i32.const 24
            i32.shl
            get_local $10
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $3
            i32.const 8
            i32.add
            get_local $9
            i32.const 12
            i32.add
            i32.load
            tee_local $10
            i32.const 24
            i32.shl
            get_local $5
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $3
            i32.const 12
            i32.add
            get_local $9
            i32.const 16
            i32.add
            i32.load
            tee_local $5
            i32.const 24
            i32.shl
            get_local $10
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $2
            i32.const 16
            i32.add
            set_local $2
            get_local $6
            i32.const -16
            i32.add
            tee_local $6
            i32.const 18
            i32.gt_u
            br_if $loop_2
          end ;; $loop_2
          get_local $7
          get_local $2
          i32.add
          set_local $3
          get_local $1
          get_local $2
          i32.add
          i32.const 1
          i32.add
          set_local $1
          get_local $4
          i32.const -17
          i32.add
          get_local $8
          i32.sub
          set_local $4
          br $block_2
        end ;; $block_3
        get_local $3
        get_local $1
        i32.load
        tee_local $5
        i32.store8
        get_local $3
        get_local $1
        i32.load8_u offset=1
        i32.store8 offset=1
        get_local $4
        i32.const -2
        i32.add
        set_local $6
        get_local $3
        i32.const 2
        i32.add
        set_local $7
        get_local $4
        i32.const -20
        i32.add
        i32.const -16
        i32.and
        set_local $8
        i32.const 0
        set_local $2
        loop $loop_3
          get_local $7
          get_local $2
          i32.add
          tee_local $3
          get_local $1
          get_local $2
          i32.add
          tee_local $9
          i32.const 4
          i32.add
          i32.load
          tee_local $10
          i32.const 16
          i32.shl
          get_local $5
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $3
          i32.const 4
          i32.add
          get_local $9
          i32.const 8
          i32.add
          i32.load
          tee_local $5
          i32.const 16
          i32.shl
          get_local $10
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $3
          i32.const 8
          i32.add
          get_local $9
          i32.const 12
          i32.add
          i32.load
          tee_local $10
          i32.const 16
          i32.shl
          get_local $5
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $3
          i32.const 12
          i32.add
          get_local $9
          i32.const 16
          i32.add
          i32.load
          tee_local $5
          i32.const 16
          i32.shl
          get_local $10
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $2
          i32.const 16
          i32.add
          set_local $2
          get_local $6
          i32.const -16
          i32.add
          tee_local $6
          i32.const 17
          i32.gt_u
          br_if $loop_3
        end ;; $loop_3
        get_local $7
        get_local $2
        i32.add
        set_local $3
        get_local $1
        get_local $2
        i32.add
        i32.const 2
        i32.add
        set_local $1
        get_local $4
        i32.const -18
        i32.add
        get_local $8
        i32.sub
        set_local $4
      end ;; $block_2
      block $block_11
        get_local $4
        i32.const 16
        i32.and
        i32.eqz
        br_if $block_11
        get_local $3
        get_local $1
        i32.load16_u align=1
        i32.store16 align=1
        get_local $3
        get_local $1
        i32.load8_u offset=2
        i32.store8 offset=2
        get_local $3
        get_local $1
        i32.load8_u offset=3
        i32.store8 offset=3
        get_local $3
        get_local $1
        i32.load8_u offset=4
        i32.store8 offset=4
        get_local $3
        get_local $1
        i32.load8_u offset=5
        i32.store8 offset=5
        get_local $3
        get_local $1
        i32.load8_u offset=6
        i32.store8 offset=6
        get_local $3
        get_local $1
        i32.load8_u offset=7
        i32.store8 offset=7
        get_local $3
        get_local $1
        i32.load8_u offset=8
        i32.store8 offset=8
        get_local $3
        get_local $1
        i32.load8_u offset=9
        i32.store8 offset=9
        get_local $3
        get_local $1
        i32.load8_u offset=10
        i32.store8 offset=10
        get_local $3
        get_local $1
        i32.load8_u offset=11
        i32.store8 offset=11
        get_local $3
        get_local $1
        i32.load8_u offset=12
        i32.store8 offset=12
        get_local $3
        get_local $1
        i32.load8_u offset=13
        i32.store8 offset=13
        get_local $3
        get_local $1
        i32.load8_u offset=14
        i32.store8 offset=14
        get_local $3
        get_local $1
        i32.load8_u offset=15
        i32.store8 offset=15
        get_local $3
        i32.const 16
        i32.add
        set_local $3
        get_local $1
        i32.const 16
        i32.add
        set_local $1
      end ;; $block_11
      block $block_12
        get_local $4
        i32.const 8
        i32.and
        i32.eqz
        br_if $block_12
        get_local $3
        get_local $1
        i32.load8_u
        i32.store8
        get_local $3
        get_local $1
        i32.load8_u offset=1
        i32.store8 offset=1
        get_local $3
        get_local $1
        i32.load8_u offset=2
        i32.store8 offset=2
        get_local $3
        get_local $1
        i32.load8_u offset=3
        i32.store8 offset=3
        get_local $3
        get_local $1
        i32.load8_u offset=4
        i32.store8 offset=4
        get_local $3
        get_local $1
        i32.load8_u offset=5
        i32.store8 offset=5
        get_local $3
        get_local $1
        i32.load8_u offset=6
        i32.store8 offset=6
        get_local $3
        get_local $1
        i32.load8_u offset=7
        i32.store8 offset=7
        get_local $3
        i32.const 8
        i32.add
        set_local $3
        get_local $1
        i32.const 8
        i32.add
        set_local $1
      end ;; $block_12
      block $block_13
        get_local $4
        i32.const 4
        i32.and
        i32.eqz
        br_if $block_13
        get_local $3
        get_local $1
        i32.load8_u
        i32.store8
        get_local $3
        get_local $1
        i32.load8_u offset=1
        i32.store8 offset=1
        get_local $3
        get_local $1
        i32.load8_u offset=2
        i32.store8 offset=2
        get_local $3
        get_local $1
        i32.load8_u offset=3
        i32.store8 offset=3
        get_local $3
        i32.const 4
        i32.add
        set_local $3
        get_local $1
        i32.const 4
        i32.add
        set_local $1
      end ;; $block_13
      block $block_14
        get_local $4
        i32.const 2
        i32.and
        i32.eqz
        br_if $block_14
        get_local $3
        get_local $1
        i32.load8_u
        i32.store8
        get_local $3
        get_local $1
        i32.load8_u offset=1
        i32.store8 offset=1
        get_local $3
        i32.const 2
        i32.add
        set_local $3
        get_local $1
        i32.const 2
        i32.add
        set_local $1
      end ;; $block_14
      get_local $4
      i32.const 1
      i32.and
      i32.eqz
      br_if $block_1
      get_local $3
      get_local $1
      i32.load8_u
      i32.store8
    end ;; $block_1
    get_local $0
    )
  
  (func $__fwritex (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    block $block
      block $block_0
        get_local $2
        i32.load offset=16
        tee_local $3
        br_if $block_0
        i32.const 0
        set_local $6
        get_local $2
        call $__towrite
        br_if $block
        get_local $2
        i32.const 16
        i32.add
        i32.load
        set_local $3
      end ;; $block_0
      block $block_1
        get_local $3
        get_local $2
        i32.load offset=20
        tee_local $4
        i32.sub
        get_local $1
        i32.ge_u
        br_if $block_1
        get_local $2
        get_local $0
        get_local $1
        get_local $2
        i32.load offset=36
        call_indirect $18 (type $0)
        return
      end ;; $block_1
      i32.const 0
      set_local $5
      block $block_2
        get_local $2
        i32.load8_s offset=75
        i32.const 0
        i32.lt_s
        br_if $block_2
        i32.const 0
        set_local $5
        get_local $0
        set_local $6
        i32.const 0
        set_local $3
        loop $loop
          get_local $1
          get_local $3
          i32.eq
          br_if $block_2
          get_local $3
          i32.const 1
          i32.add
          set_local $3
          get_local $6
          get_local $1
          i32.add
          set_local $7
          get_local $6
          i32.const -1
          i32.add
          tee_local $8
          set_local $6
          get_local $7
          i32.const -1
          i32.add
          i32.load8_u
          i32.const 10
          i32.ne
          br_if $loop
        end ;; $loop
        get_local $2
        get_local $0
        get_local $1
        get_local $3
        i32.sub
        i32.const 1
        i32.add
        tee_local $5
        get_local $2
        i32.load offset=36
        call_indirect $18 (type $0)
        tee_local $6
        get_local $5
        i32.lt_u
        br_if $block
        get_local $8
        get_local $1
        i32.add
        i32.const 1
        i32.add
        set_local $0
        get_local $2
        i32.const 20
        i32.add
        i32.load
        set_local $4
        get_local $3
        i32.const -1
        i32.add
        set_local $1
      end ;; $block_2
      get_local $4
      get_local $0
      get_local $1
      call $memcpy
      drop
      get_local $2
      i32.const 20
      i32.add
      tee_local $3
      get_local $3
      i32.load
      get_local $1
      i32.add
      i32.store
      get_local $5
      get_local $1
      i32.add
      return
    end ;; $block
    get_local $6
    )
  
  (func $__signbitl (type $9)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    get_local $1
    i64.const 63
    i64.shr_u
    i32.wrap/i64
    )
  
  (func $__fpclassifyl (type $9)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i64)
    (local $3 i32)
    (local $4 i32)
    get_local $1
    i64.const 281474976710655
    i64.and
    set_local $2
    block $block
      block $block_0
        get_local $1
        i64.const 48
        i64.shr_u
        i32.wrap/i64
        i32.const 32767
        i32.and
        tee_local $3
        i32.const 32767
        i32.eq
        br_if $block_0
        i32.const 4
        set_local $4
        get_local $3
        br_if $block
        i32.const 2
        i32.const 3
        get_local $2
        get_local $0
        i64.or
        i64.eqz
        select
        return
      end ;; $block_0
      get_local $2
      get_local $0
      i64.or
      i64.eqz
      set_local $4
    end ;; $block
    get_local $4
    )
  
  (func $frexpl (type $10)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    get_global $20
    i32.const 32
    i32.sub
    tee_local $4
    set_global $20
    block $block
      get_local $2
      i64.const 48
      i64.shr_u
      i32.wrap/i64
      tee_local $5
      i32.const 32767
      i32.and
      tee_local $6
      i32.const 32767
      i32.eq
      br_if $block
      block $block_0
        block $block_1
          block $block_2
            get_local $6
            br_if $block_2
            get_local $1
            get_local $2
            i64.const 0
            i64.const 0
            call $__eqtf2
            i32.eqz
            br_if $block_1
            get_local $4
            get_local $1
            get_local $2
            i64.const 0
            i64.const 4645181540655955968
            call $__multf3
            get_local $4
            i32.const 16
            i32.add
            get_local $4
            i64.load
            get_local $4
            i32.const 8
            i32.add
            i64.load
            get_local $3
            call $frexpl
            get_local $3
            i32.load
            i32.const -120
            i32.add
            set_local $6
            get_local $4
            i64.load offset=24
            set_local $2
            get_local $4
            i64.load offset=16
            set_local $1
            br $block_0
          end ;; $block_2
          get_local $3
          get_local $6
          i32.const -16382
          i32.add
          i32.store
          get_local $5
          i32.const 32768
          i32.and
          i32.const 16382
          i32.or
          i64.extend_u/i32
          i64.const 48
          i64.shl
          get_local $2
          i64.const 281474976710655
          i64.and
          i64.or
          set_local $2
          br $block
        end ;; $block_1
        i32.const 0
        set_local $6
      end ;; $block_0
      get_local $3
      get_local $6
      i32.store
    end ;; $block
    get_local $0
    get_local $1
    i64.store
    get_local $0
    get_local $2
    i64.store offset=8
    get_local $4
    i32.const 32
    i32.add
    set_global $20
    )
  
  (func $dummy_0 (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    get_local $0
    )
  
  (func $__lctrans (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    get_local $0
    get_local $1
    call $dummy_0
    )
  
  (func $__strerror_l (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    i32.const 0
    set_local $2
    block $block
      block $block_0
        block $block_1
          block $block_2
            loop $loop
              get_local $2
              i32.const 1056
              i32.add
              i32.load8_u
              get_local $0
              i32.eq
              br_if $block_2
              i32.const 87
              set_local $3
              get_local $2
              i32.const 1
              i32.add
              tee_local $2
              i32.const 87
              i32.ne
              br_if $loop
              br $block_1
            end ;; $loop
          end ;; $block_2
          get_local $2
          set_local $3
          get_local $2
          i32.eqz
          br_if $block_0
        end ;; $block_1
        i32.const 1152
        set_local $2
        loop $loop_0
          get_local $2
          i32.load8_u
          set_local $0
          get_local $2
          i32.const 1
          i32.add
          tee_local $4
          set_local $2
          get_local $0
          br_if $loop_0
          get_local $4
          set_local $2
          get_local $3
          i32.const -1
          i32.add
          tee_local $3
          br_if $loop_0
          br $block
        end ;; $loop_0
      end ;; $block_0
      i32.const 1152
      set_local $4
    end ;; $block
    get_local $4
    get_local $1
    i32.load offset=20
    call $__lctrans
    )
  
  (func $strerror (type $2)
    (param $0 i32)
    (result i32)
    get_local $0
    i32.const 0
    i32.load offset=103
    call $__strerror_l
    )
  
  (func $memchr (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    get_local $2
    i32.const 0
    i32.ne
    set_local $3
    block $block
      block $block_0
        block $block_1
          block $block_2
            block $block_3
              get_local $2
              i32.eqz
              br_if $block_3
              get_local $0
              i32.const 3
              i32.and
              i32.eqz
              br_if $block_3
              get_local $1
              i32.const 255
              i32.and
              set_local $3
              loop $loop
                get_local $0
                i32.load8_u
                get_local $3
                i32.eq
                br_if $block_2
                get_local $2
                i32.const 1
                i32.ne
                set_local $4
                get_local $2
                i32.const -1
                i32.add
                set_local $5
                get_local $0
                i32.const 1
                i32.add
                set_local $0
                block $block_4
                  get_local $2
                  i32.const 1
                  i32.eq
                  br_if $block_4
                  get_local $5
                  set_local $2
                  get_local $0
                  i32.const 3
                  i32.and
                  br_if $loop
                end ;; $block_4
              end ;; $loop
              get_local $4
              br_if $block_1
              br $block_0
            end ;; $block_3
            get_local $2
            set_local $5
            get_local $3
            br_if $block_1
            br $block_0
          end ;; $block_2
          get_local $2
          set_local $5
        end ;; $block_1
        get_local $0
        i32.load8_u
        get_local $1
        i32.const 255
        i32.and
        i32.eq
        br_if $block
        block $block_5
          block $block_6
            get_local $5
            i32.const 4
            i32.lt_u
            br_if $block_6
            get_local $1
            i32.const 255
            i32.and
            i32.const 16843009
            i32.mul
            set_local $3
            loop $loop_0
              get_local $0
              i32.load
              get_local $3
              i32.xor
              tee_local $2
              i32.const -1
              i32.xor
              get_local $2
              i32.const -16843009
              i32.add
              i32.and
              i32.const -2139062144
              i32.and
              br_if $block_5
              get_local $0
              i32.const 4
              i32.add
              set_local $0
              get_local $5
              i32.const -4
              i32.add
              tee_local $5
              i32.const 3
              i32.gt_u
              br_if $loop_0
            end ;; $loop_0
          end ;; $block_6
          get_local $5
          i32.eqz
          br_if $block_0
        end ;; $block_5
        get_local $1
        i32.const 255
        i32.and
        set_local $2
        loop $loop_1
          get_local $0
          i32.load8_u
          get_local $2
          i32.eq
          br_if $block
          get_local $0
          i32.const 1
          i32.add
          set_local $0
          get_local $5
          i32.const -1
          i32.add
          tee_local $5
          br_if $loop_1
        end ;; $loop_1
      end ;; $block_0
      i32.const 0
      return
    end ;; $block
    get_local $0
    )
  
  (func $strnlen (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    (local $2 i32)
    get_local $0
    i32.const 0
    get_local $1
    call $memchr
    tee_local $2
    get_local $0
    i32.sub
    get_local $1
    get_local $2
    select
    )
  
  (func $vfprintf (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    get_global $20
    i32.const 288
    i32.sub
    tee_local $3
    set_global $20
    get_local $3
    get_local $2
    i32.store offset=284
    i32.const 0
    set_local $4
    get_local $3
    i32.const 240
    i32.add
    i32.const 0
    i32.const 40
    call $memset
    drop
    get_local $3
    get_local $3
    i32.load offset=284
    i32.store offset=280
    i32.const -1
    set_local $2
    block $block
      i32.const 0
      get_local $1
      get_local $3
      i32.const 280
      i32.add
      get_local $3
      i32.const 80
      i32.add
      get_local $3
      i32.const 240
      i32.add
      call $printf_core
      i32.const -1
      i32.le_s
      br_if $block
      block $block_0
        get_local $0
        i32.load offset=76
        i32.const 0
        i32.lt_s
        br_if $block_0
        get_local $0
        call $__lockfile
        set_local $4
      end ;; $block_0
      get_local $0
      i32.load
      set_local $2
      block $block_1
        get_local $0
        i32.load8_s offset=74
        i32.const 0
        i32.gt_s
        br_if $block_1
        get_local $0
        get_local $2
        i32.const -33
        i32.and
        i32.store
      end ;; $block_1
      get_local $2
      i32.const 32
      i32.and
      set_local $5
      block $block_2
        block $block_3
          get_local $0
          i32.load offset=48
          i32.eqz
          br_if $block_3
          get_local $0
          get_local $1
          get_local $3
          i32.const 280
          i32.add
          get_local $3
          i32.const 80
          i32.add
          get_local $3
          i32.const 240
          i32.add
          call $printf_core
          set_local $2
          br $block_2
        end ;; $block_3
        get_local $0
        i32.const 48
        i32.add
        tee_local $6
        i32.const 80
        i32.store
        get_local $0
        get_local $3
        i32.const 80
        i32.add
        i32.store offset=16
        get_local $0
        get_local $3
        i32.store offset=28
        get_local $0
        get_local $3
        i32.store offset=20
        get_local $0
        i32.load offset=44
        set_local $7
        get_local $0
        get_local $3
        i32.store offset=44
        get_local $0
        get_local $1
        get_local $3
        i32.const 280
        i32.add
        get_local $3
        i32.const 80
        i32.add
        get_local $3
        i32.const 240
        i32.add
        call $printf_core
        set_local $2
        get_local $7
        i32.eqz
        br_if $block_2
        get_local $0
        i32.const 0
        i32.const 0
        get_local $0
        i32.load offset=36
        call_indirect $18 (type $0)
        drop
        get_local $6
        i32.const 0
        i32.store
        get_local $0
        i32.const 44
        i32.add
        get_local $7
        i32.store
        get_local $0
        i32.const 28
        i32.add
        i32.const 0
        i32.store
        get_local $0
        i32.const 16
        i32.add
        i32.const 0
        i32.store
        get_local $0
        i32.const 20
        i32.add
        tee_local $1
        i32.load
        set_local $7
        get_local $1
        i32.const 0
        i32.store
        get_local $2
        i32.const -1
        get_local $7
        select
        set_local $2
      end ;; $block_2
      get_local $0
      get_local $0
      i32.load
      tee_local $1
      get_local $5
      i32.or
      i32.store
      get_local $1
      i32.const 32
      i32.and
      set_local $1
      block $block_4
        get_local $4
        i32.eqz
        br_if $block_4
        get_local $0
        call $__unlockfile
      end ;; $block_4
      i32.const -1
      get_local $2
      get_local $1
      select
      set_local $2
    end ;; $block
    get_local $3
    i32.const 288
    i32.add
    set_global $20
    get_local $2
    )
  
  (func $printf_core (type $4)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (param $3 i32)
    (param $4 i32)
    (result i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i32)
    (local $19 i32)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i32)
    (local $26 i32)
    (local $27 i32)
    (local $28 i32)
    (local $29 i64)
    (local $30 i64)
    (local $31 i32)
    (local $32 i32)
    (local $33 i32)
    (local $34 i64)
    (local $35 i64)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    get_global $20
    i32.const 7776
    i32.sub
    tee_local $5
    set_global $20
    get_local $5
    i32.const 352
    i32.add
    i32.const 8
    i32.or
    set_local $6
    get_local $5
    i32.const 352
    i32.add
    i32.const 9
    i32.or
    set_local $7
    i32.const -2
    get_local $5
    i32.const 352
    i32.add
    i32.sub
    set_local $8
    get_local $5
    i32.const 340
    i32.add
    i32.const 11
    i32.add
    set_local $9
    get_local $5
    i32.const 7312
    i32.add
    set_local $10
    get_local $5
    i32.const 352
    i32.add
    set_local $11
    i32.const 0
    get_local $5
    i32.const 352
    i32.add
    i32.sub
    set_local $12
    get_local $5
    i32.const 256
    i32.add
    i32.const 54
    i32.add
    set_local $13
    get_local $5
    i32.const 311
    i32.add
    set_local $14
    i32.const 0
    set_local $15
    i32.const 0
    set_local $16
    i32.const 0
    set_local $17
    block $block
      block $block_0
        block $block_1
          block $block_2
            block $block_3
              block $block_4
                block $block_5
                  loop $loop
                    get_local $1
                    set_local $18
                    get_local $17
                    i32.const 2147483647
                    get_local $16
                    i32.sub
                    i32.gt_s
                    br_if $block_5
                    get_local $17
                    get_local $16
                    i32.add
                    set_local $16
                    get_local $18
                    i32.load8_u
                    tee_local $17
                    i32.eqz
                    br_if $block_4
                    get_local $18
                    set_local $1
                    block $block_6
                      block $block_7
                        block $block_8
                          loop $loop_0
                            get_local $17
                            i32.const 255
                            i32.and
                            tee_local $17
                            i32.eqz
                            br_if $block_8
                            block $block_9
                              get_local $17
                              i32.const 37
                              i32.eq
                              br_if $block_9
                              get_local $1
                              i32.load8_u offset=1
                              set_local $17
                              get_local $1
                              i32.const 1
                              i32.add
                              set_local $1
                              br $loop_0
                            end ;; $block_9
                          end ;; $loop_0
                          get_local $1
                          set_local $19
                          get_local $1
                          set_local $17
                          loop $loop_1
                            get_local $17
                            i32.const 1
                            i32.add
                            i32.load8_u
                            i32.const 37
                            i32.ne
                            br_if $block_7
                            get_local $19
                            i32.const 1
                            i32.add
                            set_local $19
                            get_local $17
                            i32.load8_u offset=2
                            set_local $20
                            get_local $17
                            i32.const 2
                            i32.add
                            tee_local $1
                            set_local $17
                            get_local $20
                            i32.const 37
                            i32.eq
                            br_if $loop_1
                            br $block_6
                          end ;; $loop_1
                        end ;; $block_8
                        get_local $1
                        set_local $19
                        br $block_6
                      end ;; $block_7
                      get_local $17
                      set_local $1
                    end ;; $block_6
                    get_local $19
                    get_local $18
                    i32.sub
                    tee_local $17
                    i32.const 2147483647
                    get_local $16
                    i32.sub
                    tee_local $19
                    i32.gt_s
                    br_if $block_5
                    block $block_10
                      get_local $0
                      i32.eqz
                      br_if $block_10
                      get_local $0
                      i32.load8_u
                      i32.const 32
                      i32.and
                      br_if $block_10
                      get_local $18
                      get_local $17
                      get_local $0
                      call $__fwritex
                      drop
                    end ;; $block_10
                    get_local $17
                    br_if $loop
                    get_local $1
                    i32.const 1
                    i32.add
                    set_local $17
                    block $block_11
                      block $block_12
                        get_local $1
                        i32.load8_s offset=1
                        tee_local $21
                        i32.const -48
                        i32.add
                        tee_local $22
                        i32.const 9
                        i32.gt_u
                        br_if $block_12
                        get_local $1
                        i32.const 3
                        i32.add
                        get_local $17
                        get_local $1
                        i32.load8_u offset=2
                        i32.const 36
                        i32.eq
                        tee_local $20
                        select
                        set_local $17
                        get_local $22
                        i32.const -1
                        get_local $20
                        select
                        set_local $23
                        i32.const 1
                        get_local $15
                        get_local $20
                        select
                        set_local $15
                        get_local $1
                        i32.const 3
                        i32.const 1
                        get_local $20
                        select
                        i32.add
                        i32.load8_s
                        set_local $21
                        br $block_11
                      end ;; $block_12
                      i32.const -1
                      set_local $23
                    end ;; $block_11
                    i32.const 0
                    set_local $24
                    block $block_13
                      get_local $21
                      i32.const -32
                      i32.add
                      tee_local $1
                      i32.const 31
                      i32.gt_u
                      br_if $block_13
                      i32.const 1
                      get_local $1
                      i32.shl
                      tee_local $1
                      i32.const 75913
                      i32.and
                      i32.eqz
                      br_if $block_13
                      get_local $17
                      i32.const 1
                      i32.add
                      set_local $20
                      i32.const 0
                      set_local $24
                      loop $loop_2
                        get_local $1
                        get_local $24
                        i32.or
                        set_local $24
                        get_local $20
                        tee_local $17
                        i32.load8_s
                        tee_local $21
                        i32.const -32
                        i32.add
                        tee_local $1
                        i32.const 32
                        i32.ge_u
                        br_if $block_13
                        get_local $17
                        i32.const 1
                        i32.add
                        set_local $20
                        i32.const 1
                        get_local $1
                        i32.shl
                        tee_local $1
                        i32.const 75913
                        i32.and
                        br_if $loop_2
                      end ;; $loop_2
                    end ;; $block_13
                    block $block_14
                      block $block_15
                        block $block_16
                          block $block_17
                            block $block_18
                              block $block_19
                                get_local $21
                                i32.const 42
                                i32.ne
                                br_if $block_19
                                get_local $17
                                i32.load8_s offset=1
                                i32.const -48
                                i32.add
                                tee_local $1
                                i32.const 9
                                i32.gt_u
                                br_if $block_18
                                get_local $17
                                i32.load8_u offset=2
                                i32.const 36
                                i32.ne
                                br_if $block_18
                                get_local $4
                                get_local $1
                                i32.const 2
                                i32.shl
                                i32.add
                                i32.const 10
                                i32.store
                                get_local $17
                                i32.const 3
                                i32.add
                                set_local $25
                                i32.const 1
                                set_local $15
                                get_local $3
                                get_local $17
                                i32.const 1
                                i32.add
                                i32.load8_s
                                i32.const 4
                                i32.shl
                                i32.add
                                i32.const -768
                                i32.add
                                i32.load
                                set_local $22
                                br $block_17
                              end ;; $block_19
                              i32.const 0
                              set_local $22
                              get_local $21
                              i32.const -48
                              i32.add
                              tee_local $20
                              i32.const 9
                              i32.gt_u
                              br_if $block_16
                              i32.const 0
                              set_local $1
                              loop $loop_3
                                i32.const -1
                                set_local $22
                                block $block_20
                                  get_local $1
                                  i32.const 214748364
                                  i32.gt_u
                                  br_if $block_20
                                  i32.const -1
                                  get_local $1
                                  i32.const 10
                                  i32.mul
                                  tee_local $1
                                  get_local $20
                                  i32.add
                                  get_local $20
                                  i32.const 2147483647
                                  get_local $1
                                  i32.sub
                                  i32.gt_s
                                  select
                                  set_local $22
                                end ;; $block_20
                                get_local $17
                                i32.load8_s offset=1
                                set_local $20
                                get_local $17
                                i32.const 1
                                i32.add
                                tee_local $25
                                set_local $17
                                get_local $22
                                set_local $1
                                get_local $20
                                i32.const -48
                                i32.add
                                tee_local $20
                                i32.const 10
                                i32.lt_u
                                br_if $loop_3
                              end ;; $loop_3
                              get_local $22
                              i32.const 0
                              i32.ge_s
                              br_if $block_14
                              br $block_5
                            end ;; $block_18
                            get_local $15
                            br_if $block_1
                            get_local $17
                            i32.const 1
                            i32.add
                            set_local $25
                            get_local $0
                            i32.eqz
                            br_if $block_15
                            get_local $2
                            get_local $2
                            i32.load
                            tee_local $1
                            i32.const 4
                            i32.add
                            i32.store
                            get_local $1
                            i32.load
                            set_local $22
                            i32.const 0
                            set_local $15
                          end ;; $block_17
                          get_local $22
                          i32.const -1
                          i32.gt_s
                          br_if $block_14
                          i32.const 0
                          get_local $22
                          i32.sub
                          set_local $22
                          get_local $24
                          i32.const 8192
                          i32.or
                          set_local $24
                          br $block_14
                        end ;; $block_16
                        get_local $17
                        set_local $25
                        br $block_14
                      end ;; $block_15
                      i32.const 0
                      set_local $15
                      i32.const 0
                      set_local $22
                    end ;; $block_14
                    i32.const 0
                    set_local $17
                    i32.const -1
                    set_local $21
                    block $block_21
                      block $block_22
                        block $block_23
                          block $block_24
                            block $block_25
                              block $block_26
                                block $block_27
                                  get_local $25
                                  i32.load8_u
                                  i32.const 46
                                  i32.ne
                                  br_if $block_27
                                  get_local $25
                                  i32.load8_s offset=1
                                  tee_local $20
                                  i32.const 42
                                  i32.ne
                                  br_if $block_26
                                  get_local $25
                                  i32.load8_s offset=2
                                  i32.const -48
                                  i32.add
                                  tee_local $1
                                  i32.const 9
                                  i32.gt_u
                                  br_if $block_25
                                  get_local $25
                                  i32.load8_u offset=3
                                  i32.const 36
                                  i32.ne
                                  br_if $block_25
                                  get_local $4
                                  get_local $1
                                  i32.const 2
                                  i32.shl
                                  i32.add
                                  i32.const 10
                                  i32.store
                                  get_local $25
                                  i32.const 4
                                  i32.add
                                  set_local $1
                                  get_local $3
                                  get_local $25
                                  i32.const 2
                                  i32.add
                                  i32.load8_s
                                  i32.const 4
                                  i32.shl
                                  i32.add
                                  i32.const -768
                                  i32.add
                                  i32.load
                                  set_local $21
                                  br $block_22
                                end ;; $block_27
                                get_local $25
                                set_local $1
                                i32.const 0
                                set_local $26
                                br $block_21
                              end ;; $block_26
                              get_local $25
                              i32.const 1
                              i32.add
                              set_local $1
                              get_local $20
                              i32.const -48
                              i32.add
                              tee_local $27
                              i32.const 9
                              i32.gt_u
                              br_if $block_24
                              i32.const 0
                              set_local $25
                              get_local $1
                              set_local $20
                              loop $loop_4
                                i32.const -1
                                set_local $21
                                block $block_28
                                  get_local $25
                                  i32.const 214748364
                                  i32.gt_u
                                  br_if $block_28
                                  i32.const -1
                                  get_local $25
                                  i32.const 10
                                  i32.mul
                                  tee_local $1
                                  get_local $27
                                  i32.add
                                  get_local $27
                                  i32.const 2147483647
                                  get_local $1
                                  i32.sub
                                  i32.gt_s
                                  select
                                  set_local $21
                                end ;; $block_28
                                i32.const 1
                                set_local $26
                                get_local $20
                                i32.load8_s offset=1
                                set_local $27
                                get_local $20
                                i32.const 1
                                i32.add
                                tee_local $1
                                set_local $20
                                get_local $21
                                set_local $25
                                get_local $27
                                i32.const -48
                                i32.add
                                tee_local $27
                                i32.const 10
                                i32.lt_u
                                br_if $loop_4
                                br $block_21
                              end ;; $loop_4
                            end ;; $block_25
                            get_local $15
                            br_if $block_1
                            get_local $25
                            i32.const 2
                            i32.add
                            set_local $1
                            get_local $0
                            i32.eqz
                            br_if $block_23
                            get_local $2
                            get_local $2
                            i32.load
                            tee_local $20
                            i32.const 4
                            i32.add
                            i32.store
                            get_local $20
                            i32.load
                            set_local $21
                            br $block_22
                          end ;; $block_24
                          i32.const 1
                          set_local $26
                          i32.const 0
                          set_local $21
                          br $block_21
                        end ;; $block_23
                        i32.const 0
                        set_local $21
                      end ;; $block_22
                      get_local $21
                      i32.const 31
                      i32.shr_u
                      i32.const 1
                      i32.xor
                      set_local $26
                    end ;; $block_21
                    loop $loop_5
                      get_local $17
                      set_local $20
                      get_local $1
                      i32.load8_s
                      i32.const -65
                      i32.add
                      tee_local $17
                      i32.const 57
                      i32.gt_u
                      br_if $block_1
                      get_local $1
                      i32.const 1
                      i32.add
                      set_local $1
                      get_local $20
                      i32.const 58
                      i32.mul
                      get_local $17
                      i32.add
                      i32.const 2976
                      i32.add
                      i32.load8_u
                      tee_local $17
                      i32.const -1
                      i32.add
                      i32.const 8
                      i32.lt_u
                      br_if $loop_5
                    end ;; $loop_5
                    get_local $17
                    i32.eqz
                    br_if $block_1
                    block $block_29
                      block $block_30
                        block $block_31
                          block $block_32
                            get_local $17
                            i32.const 27
                            i32.ne
                            br_if $block_32
                            get_local $23
                            i32.const -1
                            i32.le_s
                            br_if $block_31
                            br $block_1
                          end ;; $block_32
                          get_local $23
                          i32.const 0
                          i32.lt_s
                          br_if $block_30
                          get_local $4
                          get_local $23
                          i32.const 2
                          i32.shl
                          i32.add
                          get_local $17
                          i32.store
                          get_local $5
                          get_local $3
                          get_local $23
                          i32.const 4
                          i32.shl
                          i32.add
                          tee_local $17
                          i32.const 8
                          i32.add
                          i64.load
                          i64.store offset=328
                          get_local $5
                          get_local $17
                          i64.load
                          i64.store offset=320
                        end ;; $block_31
                        i32.const 0
                        set_local $17
                        get_local $0
                        i32.eqz
                        br_if $loop
                        br $block_29
                      end ;; $block_30
                      get_local $0
                      i32.eqz
                      br_if $block_3
                      get_local $5
                      i32.const 320
                      i32.add
                      get_local $17
                      get_local $2
                      call $pop_arg
                    end ;; $block_29
                    get_local $24
                    i32.const -65537
                    i32.and
                    tee_local $25
                    get_local $24
                    get_local $24
                    i32.const 8192
                    i32.and
                    select
                    set_local $23
                    block $block_33
                      block $block_34
                        block $block_35
                          block $block_36
                            block $block_37
                              block $block_38
                                block $block_39
                                  block $block_40
                                    block $block_41
                                      block $block_42
                                        block $block_43
                                          block $block_44
                                            block $block_45
                                              block $block_46
                                                block $block_47
                                                  block $block_48
                                                    block $block_49
                                                      block $block_50
                                                        block $block_51
                                                          block $block_52
                                                            block $block_53
                                                              block $block_54
                                                                block $block_55
                                                                  block $block_56
                                                                    block $block_57
                                                                      block $block_58
                                                                        block $block_59
                                                                          block $block_60
                                                                            block $block_61
                                                                              block $block_62
                                                                                block $block_63
                                                                                  block $block_64
                                                                                    block $block_65
                                                                                      block $block_66
                                                                                        block $block_67
                                                                                          block $block_68
                                                                                            block $block_69
                                                                                              block $block_70
                                                                                                block $block_71
                                                                                                  block $block_72
                                                                                                    get_local $1
                                                                                                    i32.const -1
                                                                                                    i32.add
                                                                                                    i32.load8_s
                                                                                                    tee_local $17
                                                                                                    i32.const -33
                                                                                                    i32.and
                                                                                                    get_local $17
                                                                                                    get_local $17
                                                                                                    i32.const 15
                                                                                                    i32.and
                                                                                                    i32.const 3
                                                                                                    i32.eq
                                                                                                    select
                                                                                                    get_local $17
                                                                                                    get_local $20
                                                                                                    select
                                                                                                    tee_local $28
                                                                                                    i32.const 82
                                                                                                    i32.le_s
                                                                                                    br_if $block_72
                                                                                                    get_local $28
                                                                                                    i32.const -83
                                                                                                    i32.add
                                                                                                    tee_local $17
                                                                                                    i32.const 37
                                                                                                    i32.gt_u
                                                                                                    br_if $block_59
                                                                                                    block $block_73
                                                                                                      get_local $17
                                                                                                      br_table
                                                                                                        $block_70 $block_59 $block_59 $block_59 $block_59 $block_63 $block_59 $block_59 $block_59 $block_59 $block_59 $block_59 $block_59 $block_59 $block_71 $block_59
                                                                                                        $block_68 $block_73 $block_71 $block_71 $block_71 $block_59 $block_73 $block_59 $block_59 $block_59 $block_67 $block_66 $block_65 $block_64 $block_59 $block_59
                                                                                                        $block_62 $block_59 $block_60 $block_59 $block_59 $block_63
                                                                                                        $block_70 ;; default
                                                                                                    end ;; $block_73
                                                                                                    get_local $5
                                                                                                    i64.load offset=320
                                                                                                    tee_local $29
                                                                                                    i64.const -1
                                                                                                    i64.le_s
                                                                                                    br_if $block_58
                                                                                                    get_local $23
                                                                                                    i32.const 2048
                                                                                                    i32.and
                                                                                                    br_if $block_54
                                                                                                    i32.const 2958
                                                                                                    i32.const 2956
                                                                                                    get_local $23
                                                                                                    i32.const 1
                                                                                                    i32.and
                                                                                                    tee_local $24
                                                                                                    select
                                                                                                    set_local $27
                                                                                                    br $block_53
                                                                                                  end ;; $block_72
                                                                                                  get_local $28
                                                                                                  i32.const -69
                                                                                                  i32.add
                                                                                                  i32.const 3
                                                                                                  i32.lt_u
                                                                                                  br_if $block_71
                                                                                                  get_local $28
                                                                                                  i32.const 65
                                                                                                  i32.eq
                                                                                                  br_if $block_71
                                                                                                  get_local $28
                                                                                                  i32.const 67
                                                                                                  i32.ne
                                                                                                  br_if $block_59
                                                                                                  get_local $5
                                                                                                  i32.const 248
                                                                                                  i32.add
                                                                                                  i32.const 4
                                                                                                  i32.add
                                                                                                  i32.const 0
                                                                                                  i32.store
                                                                                                  get_local $5
                                                                                                  get_local $5
                                                                                                  i64.load offset=320
                                                                                                  i64.store32 offset=248
                                                                                                  get_local $5
                                                                                                  get_local $5
                                                                                                  i32.const 248
                                                                                                  i32.add
                                                                                                  i32.store offset=320
                                                                                                  i32.const -1
                                                                                                  set_local $21
                                                                                                  get_local $5
                                                                                                  i32.const 248
                                                                                                  i32.add
                                                                                                  set_local $18
                                                                                                  br $block_69
                                                                                                end ;; $block_71
                                                                                                block $block_74
                                                                                                  get_local $21
                                                                                                  i32.const -1
                                                                                                  i32.gt_s
                                                                                                  br_if $block_74
                                                                                                  get_local $26
                                                                                                  br_if $block_5
                                                                                                end ;; $block_74
                                                                                                get_local $5
                                                                                                i64.load offset=328
                                                                                                set_local $29
                                                                                                get_local $5
                                                                                                i64.load offset=320
                                                                                                set_local $30
                                                                                                get_local $5
                                                                                                i32.const 0
                                                                                                i32.store offset=396
                                                                                                block $block_75
                                                                                                  block $block_76
                                                                                                    get_local $30
                                                                                                    get_local $29
                                                                                                    call $__signbitl
                                                                                                    i32.eqz
                                                                                                    br_if $block_76
                                                                                                    get_local $29
                                                                                                    i64.const -9223372036854775808
                                                                                                    i64.xor
                                                                                                    set_local $29
                                                                                                    i32.const 1
                                                                                                    set_local $31
                                                                                                    i32.const 3456
                                                                                                    set_local $32
                                                                                                    br $block_75
                                                                                                  end ;; $block_76
                                                                                                  block $block_77
                                                                                                    get_local $23
                                                                                                    i32.const 2048
                                                                                                    i32.and
                                                                                                    br_if $block_77
                                                                                                    i32.const 3462
                                                                                                    i32.const 3457
                                                                                                    get_local $23
                                                                                                    i32.const 1
                                                                                                    i32.and
                                                                                                    tee_local $31
                                                                                                    select
                                                                                                    set_local $32
                                                                                                    br $block_75
                                                                                                  end ;; $block_77
                                                                                                  i32.const 1
                                                                                                  set_local $31
                                                                                                  i32.const 3459
                                                                                                  set_local $32
                                                                                                end ;; $block_75
                                                                                                block $block_78
                                                                                                  block $block_79
                                                                                                    get_local $30
                                                                                                    get_local $29
                                                                                                    call $__fpclassifyl
                                                                                                    i32.const 1
                                                                                                    i32.le_s
                                                                                                    br_if $block_79
                                                                                                    get_local $5
                                                                                                    i32.const 224
                                                                                                    i32.add
                                                                                                    get_local $30
                                                                                                    get_local $29
                                                                                                    get_local $5
                                                                                                    i32.const 396
                                                                                                    i32.add
                                                                                                    call $frexpl
                                                                                                    get_local $5
                                                                                                    i32.const 208
                                                                                                    i32.add
                                                                                                    get_local $5
                                                                                                    i64.load offset=224
                                                                                                    tee_local $29
                                                                                                    get_local $5
                                                                                                    i64.load offset=232
                                                                                                    tee_local $30
                                                                                                    get_local $29
                                                                                                    get_local $30
                                                                                                    call $__addtf3
                                                                                                    block $block_80
                                                                                                      get_local $5
                                                                                                      i64.load offset=208
                                                                                                      tee_local $29
                                                                                                      get_local $5
                                                                                                      i32.const 208
                                                                                                      i32.add
                                                                                                      i32.const 8
                                                                                                      i32.add
                                                                                                      i64.load
                                                                                                      tee_local $30
                                                                                                      i64.const 0
                                                                                                      i64.const 0
                                                                                                      call $__eqtf2
                                                                                                      i32.eqz
                                                                                                      br_if $block_80
                                                                                                      get_local $5
                                                                                                      get_local $5
                                                                                                      i32.load offset=396
                                                                                                      i32.const -1
                                                                                                      i32.add
                                                                                                      i32.store offset=396
                                                                                                    end ;; $block_80
                                                                                                    get_local $28
                                                                                                    i32.const 32
                                                                                                    i32.or
                                                                                                    tee_local $33
                                                                                                    i32.const 97
                                                                                                    i32.ne
                                                                                                    br_if $block_78
                                                                                                    get_local $32
                                                                                                    i32.const 9
                                                                                                    i32.add
                                                                                                    get_local $32
                                                                                                    get_local $28
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    tee_local $18
                                                                                                    select
                                                                                                    set_local $26
                                                                                                    get_local $21
                                                                                                    i32.const 26
                                                                                                    i32.gt_u
                                                                                                    br_if $block_44
                                                                                                    i32.const 27
                                                                                                    get_local $21
                                                                                                    i32.sub
                                                                                                    i32.eqz
                                                                                                    br_if $block_44
                                                                                                    get_local $21
                                                                                                    i32.const -27
                                                                                                    i32.add
                                                                                                    set_local $17
                                                                                                    i64.const 4612248968380809216
                                                                                                    set_local $34
                                                                                                    i64.const 0
                                                                                                    set_local $35
                                                                                                    loop $loop_6
                                                                                                      get_local $5
                                                                                                      i32.const 112
                                                                                                      i32.add
                                                                                                      get_local $35
                                                                                                      get_local $34
                                                                                                      i64.const 0
                                                                                                      i64.const 4612530443357519872
                                                                                                      call $__multf3
                                                                                                      get_local $5
                                                                                                      i32.const 112
                                                                                                      i32.add
                                                                                                      i32.const 8
                                                                                                      i32.add
                                                                                                      i64.load
                                                                                                      set_local $34
                                                                                                      get_local $5
                                                                                                      i64.load offset=112
                                                                                                      set_local $35
                                                                                                      get_local $17
                                                                                                      i32.const 1
                                                                                                      i32.add
                                                                                                      tee_local $17
                                                                                                      br_if $loop_6
                                                                                                    end ;; $loop_6
                                                                                                    get_local $26
                                                                                                    i32.load8_u
                                                                                                    i32.const 45
                                                                                                    i32.ne
                                                                                                    br_if $block_45
                                                                                                    get_local $5
                                                                                                    i32.const 64
                                                                                                    i32.add
                                                                                                    get_local $29
                                                                                                    get_local $30
                                                                                                    i64.const -9223372036854775808
                                                                                                    i64.xor
                                                                                                    get_local $35
                                                                                                    get_local $34
                                                                                                    call $__subtf3
                                                                                                    get_local $5
                                                                                                    i32.const 48
                                                                                                    i32.add
                                                                                                    get_local $35
                                                                                                    get_local $34
                                                                                                    get_local $5
                                                                                                    i64.load offset=64
                                                                                                    get_local $5
                                                                                                    i32.const 64
                                                                                                    i32.add
                                                                                                    i32.const 8
                                                                                                    i32.add
                                                                                                    i64.load
                                                                                                    call $__addtf3
                                                                                                    get_local $5
                                                                                                    i32.const 48
                                                                                                    i32.add
                                                                                                    i32.const 8
                                                                                                    i32.add
                                                                                                    i64.load
                                                                                                    i64.const -9223372036854775808
                                                                                                    i64.xor
                                                                                                    set_local $30
                                                                                                    get_local $5
                                                                                                    i64.load offset=48
                                                                                                    set_local $29
                                                                                                    br $block_44
                                                                                                  end ;; $block_79
                                                                                                  get_local $0
                                                                                                  i32.const 32
                                                                                                  get_local $22
                                                                                                  get_local $31
                                                                                                  i32.const 3
                                                                                                  i32.add
                                                                                                  tee_local $17
                                                                                                  get_local $25
                                                                                                  call $pad
                                                                                                  get_local $30
                                                                                                  get_local $29
                                                                                                  get_local $30
                                                                                                  get_local $29
                                                                                                  call $__unordtf2
                                                                                                  set_local $20
                                                                                                  block $block_81
                                                                                                    get_local $0
                                                                                                    i32.load
                                                                                                    tee_local $19
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    br_if $block_81
                                                                                                    get_local $32
                                                                                                    get_local $31
                                                                                                    get_local $0
                                                                                                    call $__fwritex
                                                                                                    drop
                                                                                                    get_local $0
                                                                                                    i32.load
                                                                                                    set_local $19
                                                                                                  end ;; $block_81
                                                                                                  block $block_82
                                                                                                    get_local $19
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    br_if $block_82
                                                                                                    i32.const 3483
                                                                                                    i32.const 3487
                                                                                                    get_local $28
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    i32.const 5
                                                                                                    i32.shr_u
                                                                                                    tee_local $19
                                                                                                    select
                                                                                                    i32.const 3475
                                                                                                    i32.const 3479
                                                                                                    get_local $19
                                                                                                    select
                                                                                                    get_local $20
                                                                                                    select
                                                                                                    i32.const 3
                                                                                                    get_local $0
                                                                                                    call $__fwritex
                                                                                                    drop
                                                                                                  end ;; $block_82
                                                                                                  get_local $0
                                                                                                  i32.const 32
                                                                                                  get_local $22
                                                                                                  get_local $17
                                                                                                  get_local $23
                                                                                                  i32.const 8192
                                                                                                  i32.xor
                                                                                                  call $pad
                                                                                                  get_local $22
                                                                                                  get_local $17
                                                                                                  get_local $17
                                                                                                  get_local $22
                                                                                                  i32.lt_s
                                                                                                  select
                                                                                                  set_local $17
                                                                                                  br $block_39
                                                                                                end ;; $block_78
                                                                                                get_local $21
                                                                                                i32.const 0
                                                                                                i32.lt_s
                                                                                                set_local $17
                                                                                                get_local $29
                                                                                                get_local $30
                                                                                                i64.const 0
                                                                                                i64.const 0
                                                                                                call $__netf2
                                                                                                i32.eqz
                                                                                                br_if $block_56
                                                                                                get_local $5
                                                                                                i32.const 192
                                                                                                i32.add
                                                                                                get_local $29
                                                                                                get_local $30
                                                                                                i64.const 0
                                                                                                i64.const 4619285842798575616
                                                                                                call $__multf3
                                                                                                get_local $5
                                                                                                get_local $5
                                                                                                i32.load offset=396
                                                                                                i32.const -28
                                                                                                i32.add
                                                                                                tee_local $18
                                                                                                i32.store offset=396
                                                                                                get_local $5
                                                                                                i32.const 192
                                                                                                i32.add
                                                                                                i32.const 8
                                                                                                i32.add
                                                                                                i64.load
                                                                                                set_local $30
                                                                                                get_local $5
                                                                                                i64.load offset=192
                                                                                                set_local $29
                                                                                                br $block_55
                                                                                              end ;; $block_70
                                                                                              get_local $21
                                                                                              i32.eqz
                                                                                              br_if $block_43
                                                                                              get_local $5
                                                                                              i32.load offset=320
                                                                                              set_local $18
                                                                                            end ;; $block_69
                                                                                            i32.const 0
                                                                                            set_local $17
                                                                                            get_local $18
                                                                                            set_local $19
                                                                                            loop $loop_7
                                                                                              get_local $19
                                                                                              i32.load
                                                                                              tee_local $20
                                                                                              i32.eqz
                                                                                              br_if $block_46
                                                                                              get_local $5
                                                                                              i32.const 244
                                                                                              i32.add
                                                                                              get_local $20
                                                                                              call $wctomb
                                                                                              tee_local $20
                                                                                              i32.const 0
                                                                                              i32.lt_s
                                                                                              tee_local $24
                                                                                              br_if $block_47
                                                                                              get_local $20
                                                                                              get_local $21
                                                                                              get_local $17
                                                                                              i32.sub
                                                                                              i32.gt_u
                                                                                              br_if $block_47
                                                                                              get_local $19
                                                                                              i32.const 4
                                                                                              i32.add
                                                                                              set_local $19
                                                                                              get_local $21
                                                                                              get_local $20
                                                                                              get_local $17
                                                                                              i32.add
                                                                                              tee_local $17
                                                                                              i32.gt_u
                                                                                              br_if $loop_7
                                                                                              br $block_46
                                                                                            end ;; $loop_7
                                                                                          end ;; $block_68
                                                                                          get_local $5
                                                                                          i32.const 256
                                                                                          i32.add
                                                                                          i32.const 54
                                                                                          i32.add
                                                                                          get_local $5
                                                                                          i64.load offset=320
                                                                                          i64.store8
                                                                                          i32.const 0
                                                                                          set_local $24
                                                                                          i32.const 2956
                                                                                          set_local $27
                                                                                          i32.const 1
                                                                                          set_local $21
                                                                                          get_local $13
                                                                                          set_local $18
                                                                                          get_local $14
                                                                                          set_local $17
                                                                                          get_local $25
                                                                                          set_local $23
                                                                                          br $block_48
                                                                                        end ;; $block_67
                                                                                        call $__errno_location
                                                                                        i32.load
                                                                                        call $strerror
                                                                                        set_local $18
                                                                                        br $block_61
                                                                                      end ;; $block_66
                                                                                      i32.const 0
                                                                                      set_local $17
                                                                                      get_local $20
                                                                                      i32.const 255
                                                                                      i32.and
                                                                                      tee_local $19
                                                                                      i32.const 7
                                                                                      i32.gt_u
                                                                                      br_if $loop
                                                                                      block $block_83
                                                                                        get_local $19
                                                                                        br_table
                                                                                          $block_83 $block_38 $block_37 $block_36 $block_35 $loop $block_34 $block_33
                                                                                          $block_83 ;; default
                                                                                      end ;; $block_83
                                                                                      get_local $5
                                                                                      i32.load offset=320
                                                                                      get_local $16
                                                                                      i32.store
                                                                                      br $loop
                                                                                    end ;; $block_65
                                                                                    get_local $14
                                                                                    set_local $18
                                                                                    block $block_84
                                                                                      get_local $5
                                                                                      i64.load offset=320
                                                                                      tee_local $29
                                                                                      i64.eqz
                                                                                      br_if $block_84
                                                                                      get_local $14
                                                                                      set_local $18
                                                                                      loop $loop_8
                                                                                        get_local $18
                                                                                        i32.const -1
                                                                                        i32.add
                                                                                        tee_local $18
                                                                                        get_local $29
                                                                                        i32.wrap/i64
                                                                                        i32.const 7
                                                                                        i32.and
                                                                                        i32.const 48
                                                                                        i32.or
                                                                                        i32.store8
                                                                                        get_local $29
                                                                                        i64.const 3
                                                                                        i64.shr_u
                                                                                        tee_local $29
                                                                                        i64.const 0
                                                                                        i64.ne
                                                                                        br_if $loop_8
                                                                                      end ;; $loop_8
                                                                                    end ;; $block_84
                                                                                    i32.const 0
                                                                                    set_local $24
                                                                                    i32.const 2956
                                                                                    set_local $27
                                                                                    get_local $23
                                                                                    i32.const 8
                                                                                    i32.and
                                                                                    i32.eqz
                                                                                    br_if $block_52
                                                                                    get_local $21
                                                                                    get_local $14
                                                                                    get_local $18
                                                                                    i32.sub
                                                                                    tee_local $17
                                                                                    i32.const 1
                                                                                    i32.add
                                                                                    get_local $21
                                                                                    get_local $17
                                                                                    i32.gt_s
                                                                                    select
                                                                                    set_local $21
                                                                                    get_local $26
                                                                                    br_if $block_51
                                                                                    br $block_50
                                                                                  end ;; $block_64
                                                                                  get_local $21
                                                                                  i32.const 8
                                                                                  get_local $21
                                                                                  i32.const 8
                                                                                  i32.gt_u
                                                                                  select
                                                                                  set_local $21
                                                                                  get_local $23
                                                                                  i32.const 8
                                                                                  i32.or
                                                                                  set_local $23
                                                                                  i32.const 120
                                                                                  set_local $28
                                                                                end ;; $block_63
                                                                                i32.const 0
                                                                                set_local $24
                                                                                i32.const 2956
                                                                                set_local $27
                                                                                block $block_85
                                                                                  get_local $5
                                                                                  i64.load offset=320
                                                                                  tee_local $29
                                                                                  i64.eqz
                                                                                  br_if $block_85
                                                                                  get_local $28
                                                                                  i32.const 32
                                                                                  i32.and
                                                                                  set_local $17
                                                                                  get_local $14
                                                                                  set_local $18
                                                                                  loop $loop_9
                                                                                    get_local $18
                                                                                    i32.const -1
                                                                                    i32.add
                                                                                    tee_local $18
                                                                                    get_local $29
                                                                                    i32.wrap/i64
                                                                                    i32.const 15
                                                                                    i32.and
                                                                                    i32.const 3440
                                                                                    i32.add
                                                                                    i32.load8_u
                                                                                    get_local $17
                                                                                    i32.or
                                                                                    i32.store8
                                                                                    get_local $29
                                                                                    i64.const 4
                                                                                    i64.shr_u
                                                                                    tee_local $29
                                                                                    i64.const 0
                                                                                    i64.ne
                                                                                    br_if $loop_9
                                                                                  end ;; $loop_9
                                                                                  get_local $23
                                                                                  i32.const 8
                                                                                  i32.and
                                                                                  i32.eqz
                                                                                  br_if $block_52
                                                                                  get_local $5
                                                                                  i64.load offset=320
                                                                                  i64.eqz
                                                                                  br_if $block_52
                                                                                  get_local $28
                                                                                  i32.const 4
                                                                                  i32.shr_s
                                                                                  i32.const 2956
                                                                                  i32.add
                                                                                  set_local $27
                                                                                  i32.const 2
                                                                                  set_local $24
                                                                                  get_local $26
                                                                                  br_if $block_51
                                                                                  br $block_50
                                                                                end ;; $block_85
                                                                                get_local $14
                                                                                set_local $18
                                                                                get_local $26
                                                                                br_if $block_51
                                                                                br $block_50
                                                                              end ;; $block_62
                                                                              get_local $5
                                                                              i32.load offset=320
                                                                              tee_local $17
                                                                              i32.const 2966
                                                                              get_local $17
                                                                              select
                                                                              set_local $18
                                                                            end ;; $block_61
                                                                            i32.const 0
                                                                            set_local $24
                                                                            get_local $18
                                                                            get_local $18
                                                                            i32.const 2147483647
                                                                            get_local $21
                                                                            get_local $21
                                                                            i32.const 0
                                                                            i32.lt_s
                                                                            select
                                                                            call $strnlen
                                                                            tee_local $20
                                                                            i32.add
                                                                            set_local $17
                                                                            i32.const 2956
                                                                            set_local $27
                                                                            get_local $21
                                                                            i32.const -1
                                                                            i32.le_s
                                                                            br_if $block_57
                                                                            get_local $25
                                                                            set_local $23
                                                                            get_local $20
                                                                            set_local $21
                                                                            br $block_48
                                                                          end ;; $block_60
                                                                          i32.const 0
                                                                          set_local $24
                                                                          i32.const 2956
                                                                          set_local $27
                                                                          get_local $5
                                                                          i64.load offset=320
                                                                          set_local $29
                                                                          br $block_53
                                                                        end ;; $block_59
                                                                        i32.const 0
                                                                        set_local $24
                                                                        i32.const 2956
                                                                        set_local $27
                                                                        br $block_49
                                                                      end ;; $block_58
                                                                      get_local $5
                                                                      i64.const 0
                                                                      get_local $29
                                                                      i64.sub
                                                                      tee_local $29
                                                                      i64.store offset=320
                                                                      i32.const 1
                                                                      set_local $24
                                                                      i32.const 2956
                                                                      set_local $27
                                                                      br $block_53
                                                                    end ;; $block_57
                                                                    get_local $25
                                                                    set_local $23
                                                                    get_local $20
                                                                    set_local $21
                                                                    get_local $17
                                                                    i32.load8_u
                                                                    i32.eqz
                                                                    br_if $block_48
                                                                    br $block_5
                                                                  end ;; $block_56
                                                                  get_local $5
                                                                  i32.load offset=396
                                                                  set_local $18
                                                                end ;; $block_55
                                                                i32.const 6
                                                                get_local $21
                                                                get_local $17
                                                                select
                                                                set_local $36
                                                                get_local $5
                                                                i32.const 400
                                                                i32.add
                                                                get_local $10
                                                                get_local $18
                                                                i32.const 0
                                                                i32.lt_s
                                                                select
                                                                tee_local $37
                                                                set_local $20
                                                                loop $loop_10
                                                                  get_local $5
                                                                  i32.const 176
                                                                  i32.add
                                                                  get_local $29
                                                                  get_local $30
                                                                  call $__fixunstfsi
                                                                  tee_local $17
                                                                  call $__floatunsitf
                                                                  get_local $5
                                                                  i32.const 160
                                                                  i32.add
                                                                  get_local $29
                                                                  get_local $30
                                                                  get_local $5
                                                                  i64.load offset=176
                                                                  get_local $5
                                                                  i32.const 176
                                                                  i32.add
                                                                  i32.const 8
                                                                  i32.add
                                                                  i64.load
                                                                  call $__subtf3
                                                                  get_local $5
                                                                  i32.const 144
                                                                  i32.add
                                                                  get_local $5
                                                                  i64.load offset=160
                                                                  get_local $5
                                                                  i32.const 160
                                                                  i32.add
                                                                  i32.const 8
                                                                  i32.add
                                                                  i64.load
                                                                  i64.const 0
                                                                  i64.const 4619810130798575616
                                                                  call $__multf3
                                                                  get_local $20
                                                                  get_local $17
                                                                  i32.store
                                                                  get_local $20
                                                                  i32.const 4
                                                                  i32.add
                                                                  set_local $20
                                                                  get_local $5
                                                                  i64.load offset=144
                                                                  tee_local $29
                                                                  get_local $5
                                                                  i32.const 144
                                                                  i32.add
                                                                  i32.const 8
                                                                  i32.add
                                                                  i64.load
                                                                  tee_local $30
                                                                  i64.const 0
                                                                  i64.const 0
                                                                  call $__netf2
                                                                  br_if $loop_10
                                                                end ;; $loop_10
                                                                block $block_86
                                                                  block $block_87
                                                                    get_local $18
                                                                    i32.const 1
                                                                    i32.lt_s
                                                                    br_if $block_87
                                                                    get_local $37
                                                                    set_local $19
                                                                    loop $loop_11
                                                                      get_local $18
                                                                      i32.const 29
                                                                      get_local $18
                                                                      i32.const 29
                                                                      i32.lt_s
                                                                      select
                                                                      set_local $18
                                                                      block $block_88
                                                                        get_local $20
                                                                        i32.const -4
                                                                        i32.add
                                                                        tee_local $17
                                                                        get_local $19
                                                                        i32.lt_u
                                                                        br_if $block_88
                                                                        get_local $18
                                                                        i64.extend_u/i32
                                                                        set_local $30
                                                                        i64.const 0
                                                                        set_local $29
                                                                        loop $loop_12
                                                                          get_local $17
                                                                          get_local $17
                                                                          i64.load32_u
                                                                          get_local $30
                                                                          i64.shl
                                                                          get_local $29
                                                                          i64.const 4294967295
                                                                          i64.and
                                                                          i64.add
                                                                          tee_local $29
                                                                          get_local $29
                                                                          i64.const 1000000000
                                                                          i64.div_u
                                                                          tee_local $29
                                                                          i64.const 1000000000
                                                                          i64.mul
                                                                          i64.sub
                                                                          i64.store32
                                                                          get_local $17
                                                                          i32.const -4
                                                                          i32.add
                                                                          tee_local $17
                                                                          get_local $19
                                                                          i32.ge_u
                                                                          br_if $loop_12
                                                                        end ;; $loop_12
                                                                        get_local $29
                                                                        i32.wrap/i64
                                                                        tee_local $17
                                                                        i32.eqz
                                                                        br_if $block_88
                                                                        get_local $19
                                                                        i32.const -4
                                                                        i32.add
                                                                        tee_local $19
                                                                        get_local $17
                                                                        i32.store
                                                                      end ;; $block_88
                                                                      block $block_89
                                                                        loop $loop_13
                                                                          get_local $20
                                                                          tee_local $17
                                                                          get_local $19
                                                                          i32.le_u
                                                                          br_if $block_89
                                                                          get_local $17
                                                                          i32.const -4
                                                                          i32.add
                                                                          tee_local $20
                                                                          i32.load
                                                                          i32.eqz
                                                                          br_if $loop_13
                                                                        end ;; $loop_13
                                                                      end ;; $block_89
                                                                      get_local $5
                                                                      get_local $5
                                                                      i32.load offset=396
                                                                      get_local $18
                                                                      i32.sub
                                                                      tee_local $18
                                                                      i32.store offset=396
                                                                      get_local $17
                                                                      set_local $20
                                                                      get_local $18
                                                                      i32.const 0
                                                                      i32.gt_s
                                                                      br_if $loop_11
                                                                      br $block_86
                                                                    end ;; $loop_11
                                                                  end ;; $block_87
                                                                  get_local $20
                                                                  set_local $17
                                                                  get_local $37
                                                                  set_local $19
                                                                end ;; $block_86
                                                                block $block_90
                                                                  get_local $18
                                                                  i32.const -1
                                                                  i32.gt_s
                                                                  br_if $block_90
                                                                  get_local $36
                                                                  i32.const 45
                                                                  i32.add
                                                                  i32.const 9
                                                                  i32.div_u
                                                                  i32.const 1
                                                                  i32.add
                                                                  set_local $26
                                                                  loop $loop_14
                                                                    i32.const 0
                                                                    get_local $18
                                                                    i32.sub
                                                                    tee_local $20
                                                                    i32.const 9
                                                                    get_local $20
                                                                    i32.const 9
                                                                    i32.lt_s
                                                                    select
                                                                    set_local $24
                                                                    block $block_91
                                                                      block $block_92
                                                                        get_local $19
                                                                        get_local $17
                                                                        i32.ge_u
                                                                        br_if $block_92
                                                                        i32.const 1000000000
                                                                        get_local $24
                                                                        i32.shr_u
                                                                        set_local $25
                                                                        i32.const -1
                                                                        get_local $24
                                                                        i32.shl
                                                                        i32.const -1
                                                                        i32.xor
                                                                        set_local $27
                                                                        i32.const 0
                                                                        set_local $18
                                                                        get_local $19
                                                                        set_local $20
                                                                        loop $loop_15
                                                                          get_local $20
                                                                          get_local $20
                                                                          i32.load
                                                                          tee_local $21
                                                                          get_local $24
                                                                          i32.shr_u
                                                                          get_local $18
                                                                          i32.add
                                                                          i32.store
                                                                          get_local $21
                                                                          get_local $27
                                                                          i32.and
                                                                          get_local $25
                                                                          i32.mul
                                                                          set_local $18
                                                                          get_local $20
                                                                          i32.const 4
                                                                          i32.add
                                                                          tee_local $20
                                                                          get_local $17
                                                                          i32.lt_u
                                                                          br_if $loop_15
                                                                        end ;; $loop_15
                                                                        get_local $19
                                                                        get_local $19
                                                                        i32.const 4
                                                                        i32.add
                                                                        get_local $19
                                                                        i32.load
                                                                        select
                                                                        set_local $19
                                                                        get_local $18
                                                                        i32.eqz
                                                                        br_if $block_91
                                                                        get_local $17
                                                                        get_local $18
                                                                        i32.store
                                                                        get_local $17
                                                                        i32.const 4
                                                                        i32.add
                                                                        set_local $17
                                                                        br $block_91
                                                                      end ;; $block_92
                                                                      get_local $19
                                                                      get_local $19
                                                                      i32.const 4
                                                                      i32.add
                                                                      get_local $19
                                                                      i32.load
                                                                      select
                                                                      set_local $19
                                                                    end ;; $block_91
                                                                    get_local $5
                                                                    get_local $5
                                                                    i32.load offset=396
                                                                    get_local $24
                                                                    i32.add
                                                                    tee_local $18
                                                                    i32.store offset=396
                                                                    get_local $37
                                                                    get_local $19
                                                                    get_local $33
                                                                    i32.const 102
                                                                    i32.eq
                                                                    select
                                                                    tee_local $20
                                                                    get_local $26
                                                                    i32.const 2
                                                                    i32.shl
                                                                    i32.add
                                                                    get_local $17
                                                                    get_local $17
                                                                    get_local $20
                                                                    i32.sub
                                                                    i32.const 2
                                                                    i32.shr_s
                                                                    get_local $26
                                                                    i32.gt_s
                                                                    select
                                                                    set_local $17
                                                                    get_local $18
                                                                    i32.const 0
                                                                    i32.lt_s
                                                                    br_if $loop_14
                                                                  end ;; $loop_14
                                                                end ;; $block_90
                                                                i32.const 0
                                                                set_local $18
                                                                block $block_93
                                                                  get_local $19
                                                                  get_local $17
                                                                  i32.ge_u
                                                                  br_if $block_93
                                                                  get_local $37
                                                                  get_local $19
                                                                  i32.sub
                                                                  i32.const 2
                                                                  i32.shr_s
                                                                  i32.const 9
                                                                  i32.mul
                                                                  set_local $18
                                                                  get_local $19
                                                                  i32.load
                                                                  tee_local $21
                                                                  i32.const 10
                                                                  i32.lt_u
                                                                  br_if $block_93
                                                                  i32.const 10
                                                                  set_local $20
                                                                  loop $loop_16
                                                                    get_local $18
                                                                    i32.const 1
                                                                    i32.add
                                                                    set_local $18
                                                                    get_local $21
                                                                    get_local $20
                                                                    i32.const 10
                                                                    i32.mul
                                                                    tee_local $20
                                                                    i32.ge_u
                                                                    br_if $loop_16
                                                                  end ;; $loop_16
                                                                end ;; $block_93
                                                                block $block_94
                                                                  get_local $36
                                                                  i32.const 0
                                                                  get_local $18
                                                                  get_local $33
                                                                  i32.const 102
                                                                  i32.eq
                                                                  select
                                                                  tee_local $21
                                                                  i32.sub
                                                                  get_local $36
                                                                  i32.const 0
                                                                  i32.ne
                                                                  get_local $33
                                                                  i32.const 103
                                                                  i32.eq
                                                                  tee_local $25
                                                                  i32.and
                                                                  tee_local $27
                                                                  i32.sub
                                                                  tee_local $20
                                                                  get_local $17
                                                                  get_local $37
                                                                  i32.sub
                                                                  i32.const 2
                                                                  i32.shr_s
                                                                  i32.const 9
                                                                  i32.mul
                                                                  i32.const -9
                                                                  i32.add
                                                                  i32.ge_s
                                                                  br_if $block_94
                                                                  get_local $37
                                                                  get_local $20
                                                                  i32.const 147456
                                                                  i32.add
                                                                  tee_local $26
                                                                  i32.const 9
                                                                  i32.div_s
                                                                  tee_local $33
                                                                  i32.const 2
                                                                  i32.shl
                                                                  i32.add
                                                                  tee_local $38
                                                                  i32.const -65532
                                                                  i32.add
                                                                  set_local $24
                                                                  i32.const 10
                                                                  set_local $20
                                                                  block $block_95
                                                                    get_local $26
                                                                    get_local $33
                                                                    i32.const 9
                                                                    i32.mul
                                                                    tee_local $33
                                                                    i32.sub
                                                                    i32.const 1
                                                                    i32.add
                                                                    i32.const 8
                                                                    i32.gt_s
                                                                    br_if $block_95
                                                                    get_local $21
                                                                    get_local $33
                                                                    i32.add
                                                                    i32.const -147448
                                                                    i32.add
                                                                    get_local $36
                                                                    i32.sub
                                                                    i32.const 0
                                                                    get_local $27
                                                                    i32.sub
                                                                    i32.sub
                                                                    set_local $21
                                                                    i32.const 10
                                                                    set_local $20
                                                                    loop $loop_17
                                                                      get_local $20
                                                                      i32.const 10
                                                                      i32.mul
                                                                      set_local $20
                                                                      get_local $21
                                                                      i32.const -1
                                                                      i32.add
                                                                      tee_local $21
                                                                      br_if $loop_17
                                                                    end ;; $loop_17
                                                                  end ;; $block_95
                                                                  get_local $24
                                                                  i32.load
                                                                  tee_local $27
                                                                  get_local $27
                                                                  get_local $20
                                                                  i32.div_u
                                                                  tee_local $26
                                                                  get_local $20
                                                                  i32.mul
                                                                  i32.sub
                                                                  set_local $21
                                                                  block $block_96
                                                                    block $block_97
                                                                      get_local $24
                                                                      i32.const 4
                                                                      i32.add
                                                                      tee_local $33
                                                                      get_local $17
                                                                      i32.ne
                                                                      br_if $block_97
                                                                      get_local $21
                                                                      i32.eqz
                                                                      br_if $block_96
                                                                    end ;; $block_97
                                                                    block $block_98
                                                                      block $block_99
                                                                        get_local $26
                                                                        i32.const 1
                                                                        i32.and
                                                                        br_if $block_99
                                                                        i64.const 4643211215818981376
                                                                        set_local $29
                                                                        i64.const 0
                                                                        set_local $30
                                                                        get_local $24
                                                                        get_local $19
                                                                        i32.le_u
                                                                        br_if $block_98
                                                                        get_local $20
                                                                        i32.const 1000000000
                                                                        i32.ne
                                                                        br_if $block_98
                                                                        get_local $24
                                                                        i32.const -4
                                                                        i32.add
                                                                        i32.load8_u
                                                                        i32.const 1
                                                                        i32.and
                                                                        i32.eqz
                                                                        br_if $block_98
                                                                      end ;; $block_99
                                                                      i64.const 4643211215818981376
                                                                      set_local $29
                                                                      i64.const 1
                                                                      set_local $30
                                                                    end ;; $block_98
                                                                    i64.const 4611123068473966592
                                                                    set_local $34
                                                                    block $block_100
                                                                      get_local $21
                                                                      get_local $20
                                                                      i32.const 1
                                                                      i32.shr_u
                                                                      tee_local $26
                                                                      i32.lt_u
                                                                      br_if $block_100
                                                                      i64.const 4611404543450677248
                                                                      i64.const 4611545280939032576
                                                                      get_local $21
                                                                      get_local $26
                                                                      i32.eq
                                                                      select
                                                                      i64.const 4611545280939032576
                                                                      get_local $33
                                                                      get_local $17
                                                                      i32.eq
                                                                      select
                                                                      set_local $34
                                                                    end ;; $block_100
                                                                    block $block_101
                                                                      get_local $31
                                                                      i32.eqz
                                                                      br_if $block_101
                                                                      get_local $32
                                                                      i32.load8_u
                                                                      i32.const 45
                                                                      i32.ne
                                                                      br_if $block_101
                                                                      get_local $34
                                                                      i64.const -9223372036854775808
                                                                      i64.xor
                                                                      set_local $34
                                                                      get_local $29
                                                                      i64.const -9223372036854775808
                                                                      i64.xor
                                                                      set_local $29
                                                                    end ;; $block_101
                                                                    get_local $5
                                                                    i32.const 128
                                                                    i32.add
                                                                    get_local $30
                                                                    get_local $29
                                                                    i64.const 0
                                                                    get_local $34
                                                                    call $__addtf3
                                                                    get_local $24
                                                                    get_local $27
                                                                    get_local $21
                                                                    i32.sub
                                                                    tee_local $21
                                                                    i32.store
                                                                    get_local $5
                                                                    i64.load offset=128
                                                                    get_local $5
                                                                    i32.const 128
                                                                    i32.add
                                                                    i32.const 8
                                                                    i32.add
                                                                    i64.load
                                                                    get_local $30
                                                                    get_local $29
                                                                    call $__eqtf2
                                                                    i32.eqz
                                                                    br_if $block_96
                                                                    get_local $24
                                                                    get_local $21
                                                                    get_local $20
                                                                    i32.add
                                                                    tee_local $20
                                                                    i32.store
                                                                    block $block_102
                                                                      get_local $20
                                                                      i32.const 1000000000
                                                                      i32.lt_u
                                                                      br_if $block_102
                                                                      get_local $38
                                                                      i32.const -65536
                                                                      i32.add
                                                                      set_local $20
                                                                      loop $loop_18
                                                                        get_local $20
                                                                        i32.const 4
                                                                        i32.add
                                                                        i32.const 0
                                                                        i32.store
                                                                        block $block_103
                                                                          get_local $20
                                                                          get_local $19
                                                                          i32.ge_u
                                                                          br_if $block_103
                                                                          get_local $19
                                                                          i32.const -4
                                                                          i32.add
                                                                          tee_local $19
                                                                          i32.const 0
                                                                          i32.store
                                                                        end ;; $block_103
                                                                        get_local $20
                                                                        get_local $20
                                                                        i32.load
                                                                        i32.const 1
                                                                        i32.add
                                                                        tee_local $18
                                                                        i32.store
                                                                        get_local $20
                                                                        i32.const -4
                                                                        i32.add
                                                                        set_local $20
                                                                        get_local $18
                                                                        i32.const 999999999
                                                                        i32.gt_u
                                                                        br_if $loop_18
                                                                      end ;; $loop_18
                                                                      get_local $20
                                                                      i32.const 4
                                                                      i32.add
                                                                      set_local $24
                                                                    end ;; $block_102
                                                                    get_local $37
                                                                    get_local $19
                                                                    i32.sub
                                                                    i32.const 2
                                                                    i32.shr_s
                                                                    i32.const 9
                                                                    i32.mul
                                                                    set_local $18
                                                                    get_local $19
                                                                    i32.load
                                                                    tee_local $21
                                                                    i32.const 10
                                                                    i32.lt_u
                                                                    br_if $block_96
                                                                    i32.const 10
                                                                    set_local $20
                                                                    loop $loop_19
                                                                      get_local $18
                                                                      i32.const 1
                                                                      i32.add
                                                                      set_local $18
                                                                      get_local $21
                                                                      get_local $20
                                                                      i32.const 10
                                                                      i32.mul
                                                                      tee_local $20
                                                                      i32.ge_u
                                                                      br_if $loop_19
                                                                    end ;; $loop_19
                                                                  end ;; $block_96
                                                                  get_local $24
                                                                  i32.const 4
                                                                  i32.add
                                                                  tee_local $20
                                                                  get_local $17
                                                                  get_local $17
                                                                  get_local $20
                                                                  i32.gt_u
                                                                  select
                                                                  set_local $17
                                                                end ;; $block_94
                                                                block $block_104
                                                                  block $block_105
                                                                    block $block_106
                                                                      loop $loop_20
                                                                        get_local $17
                                                                        tee_local $20
                                                                        get_local $19
                                                                        i32.le_u
                                                                        br_if $block_106
                                                                        get_local $20
                                                                        i32.const -4
                                                                        i32.add
                                                                        tee_local $17
                                                                        i32.load
                                                                        i32.eqz
                                                                        br_if $loop_20
                                                                      end ;; $loop_20
                                                                      i32.const 1
                                                                      set_local $26
                                                                      get_local $25
                                                                      br_if $block_104
                                                                      br $block_105
                                                                    end ;; $block_106
                                                                    i32.const 0
                                                                    set_local $26
                                                                    get_local $25
                                                                    br_if $block_104
                                                                  end ;; $block_105
                                                                  get_local $23
                                                                  i32.const 8
                                                                  i32.and
                                                                  set_local $25
                                                                  br $block_40
                                                                end ;; $block_104
                                                                get_local $18
                                                                i32.const -1
                                                                i32.xor
                                                                i32.const -1
                                                                get_local $36
                                                                i32.const 1
                                                                get_local $36
                                                                select
                                                                tee_local $17
                                                                get_local $18
                                                                i32.gt_s
                                                                get_local $18
                                                                i32.const -5
                                                                i32.gt_s
                                                                i32.and
                                                                tee_local $21
                                                                select
                                                                get_local $17
                                                                i32.add
                                                                set_local $36
                                                                i32.const -1
                                                                i32.const -2
                                                                get_local $21
                                                                select
                                                                get_local $28
                                                                i32.add
                                                                set_local $28
                                                                get_local $23
                                                                i32.const 8
                                                                i32.and
                                                                tee_local $25
                                                                br_if $block_40
                                                                i32.const 9
                                                                set_local $17
                                                                block $block_107
                                                                  get_local $26
                                                                  i32.eqz
                                                                  br_if $block_107
                                                                  get_local $20
                                                                  i32.const -4
                                                                  i32.add
                                                                  i32.load
                                                                  tee_local $24
                                                                  i32.eqz
                                                                  br_if $block_107
                                                                  i32.const 0
                                                                  set_local $17
                                                                  get_local $24
                                                                  i32.const 10
                                                                  i32.rem_u
                                                                  br_if $block_107
                                                                  i32.const 10
                                                                  set_local $21
                                                                  i32.const 0
                                                                  set_local $17
                                                                  loop $loop_21
                                                                    get_local $17
                                                                    i32.const 1
                                                                    i32.add
                                                                    set_local $17
                                                                    get_local $24
                                                                    get_local $21
                                                                    i32.const 10
                                                                    i32.mul
                                                                    tee_local $21
                                                                    i32.rem_u
                                                                    i32.eqz
                                                                    br_if $loop_21
                                                                  end ;; $loop_21
                                                                end ;; $block_107
                                                                get_local $20
                                                                get_local $37
                                                                i32.sub
                                                                i32.const 2
                                                                i32.shr_s
                                                                i32.const 9
                                                                i32.mul
                                                                i32.const -9
                                                                i32.add
                                                                set_local $21
                                                                get_local $28
                                                                i32.const 32
                                                                i32.or
                                                                i32.const 102
                                                                i32.ne
                                                                br_if $block_41
                                                                i32.const 0
                                                                set_local $25
                                                                get_local $36
                                                                get_local $21
                                                                get_local $17
                                                                i32.sub
                                                                tee_local $17
                                                                i32.const 0
                                                                get_local $17
                                                                i32.const 0
                                                                i32.gt_s
                                                                select
                                                                tee_local $17
                                                                get_local $36
                                                                get_local $17
                                                                i32.lt_s
                                                                select
                                                                set_local $36
                                                                br $block_40
                                                              end ;; $block_54
                                                              i32.const 1
                                                              set_local $24
                                                              i32.const 2957
                                                              set_local $27
                                                            end ;; $block_53
                                                            get_local $29
                                                            get_local $14
                                                            call $fmt_u
                                                            set_local $18
                                                          end ;; $block_52
                                                          get_local $26
                                                          i32.eqz
                                                          br_if $block_50
                                                        end ;; $block_51
                                                        get_local $21
                                                        i32.const 0
                                                        i32.lt_s
                                                        br_if $block_5
                                                      end ;; $block_50
                                                      get_local $23
                                                      i32.const -65537
                                                      i32.and
                                                      get_local $23
                                                      get_local $26
                                                      select
                                                      set_local $23
                                                      get_local $5
                                                      i64.load offset=320
                                                      set_local $29
                                                      block $block_108
                                                        get_local $21
                                                        br_if $block_108
                                                        get_local $29
                                                        i64.eqz
                                                        i32.eqz
                                                        br_if $block_108
                                                        get_local $14
                                                        set_local $18
                                                        get_local $14
                                                        set_local $17
                                                        i32.const 0
                                                        set_local $21
                                                        br $block_48
                                                      end ;; $block_108
                                                      get_local $21
                                                      get_local $14
                                                      get_local $18
                                                      i32.sub
                                                      get_local $29
                                                      i64.eqz
                                                      i32.add
                                                      tee_local $17
                                                      get_local $21
                                                      get_local $17
                                                      i32.gt_s
                                                      select
                                                      set_local $21
                                                    end ;; $block_49
                                                    get_local $14
                                                    set_local $17
                                                  end ;; $block_48
                                                  get_local $17
                                                  get_local $18
                                                  i32.sub
                                                  tee_local $25
                                                  get_local $21
                                                  get_local $21
                                                  get_local $25
                                                  i32.lt_s
                                                  select
                                                  tee_local $21
                                                  i32.const 2147483647
                                                  get_local $24
                                                  i32.sub
                                                  i32.gt_s
                                                  br_if $block_5
                                                  get_local $24
                                                  get_local $21
                                                  i32.add
                                                  tee_local $20
                                                  get_local $22
                                                  get_local $22
                                                  get_local $20
                                                  i32.lt_s
                                                  select
                                                  tee_local $17
                                                  get_local $19
                                                  i32.gt_s
                                                  br_if $block_5
                                                  get_local $0
                                                  i32.const 32
                                                  get_local $17
                                                  get_local $20
                                                  get_local $23
                                                  call $pad
                                                  block $block_109
                                                    get_local $0
                                                    i32.load8_u
                                                    i32.const 32
                                                    i32.and
                                                    br_if $block_109
                                                    get_local $27
                                                    get_local $24
                                                    get_local $0
                                                    call $__fwritex
                                                    drop
                                                  end ;; $block_109
                                                  get_local $0
                                                  i32.const 48
                                                  get_local $17
                                                  get_local $20
                                                  get_local $23
                                                  i32.const 65536
                                                  i32.xor
                                                  call $pad
                                                  get_local $0
                                                  i32.const 48
                                                  get_local $21
                                                  get_local $25
                                                  i32.const 0
                                                  call $pad
                                                  block $block_110
                                                    get_local $0
                                                    i32.load8_u
                                                    i32.const 32
                                                    i32.and
                                                    br_if $block_110
                                                    get_local $18
                                                    get_local $25
                                                    get_local $0
                                                    call $__fwritex
                                                    drop
                                                  end ;; $block_110
                                                  get_local $0
                                                  i32.const 32
                                                  get_local $17
                                                  get_local $20
                                                  get_local $23
                                                  i32.const 8192
                                                  i32.xor
                                                  call $pad
                                                  br $loop
                                                end ;; $block_47
                                                get_local $24
                                                br_if $block_0
                                              end ;; $block_46
                                              get_local $17
                                              i32.const 0
                                              i32.lt_s
                                              br_if $block_5
                                              get_local $0
                                              i32.const 32
                                              get_local $22
                                              get_local $17
                                              get_local $23
                                              call $pad
                                              block $block_111
                                                get_local $17
                                                i32.eqz
                                                br_if $block_111
                                                i32.const 0
                                                set_local $19
                                                loop $loop_22
                                                  get_local $18
                                                  i32.load
                                                  tee_local $20
                                                  i32.eqz
                                                  br_if $block_42
                                                  get_local $5
                                                  i32.const 244
                                                  i32.add
                                                  get_local $20
                                                  call $wctomb
                                                  tee_local $20
                                                  get_local $19
                                                  i32.add
                                                  tee_local $19
                                                  get_local $17
                                                  i32.gt_u
                                                  br_if $block_42
                                                  block $block_112
                                                    get_local $0
                                                    i32.load8_u
                                                    i32.const 32
                                                    i32.and
                                                    br_if $block_112
                                                    get_local $5
                                                    i32.const 244
                                                    i32.add
                                                    get_local $20
                                                    get_local $0
                                                    call $__fwritex
                                                    drop
                                                  end ;; $block_112
                                                  get_local $18
                                                  i32.const 4
                                                  i32.add
                                                  set_local $18
                                                  get_local $19
                                                  get_local $17
                                                  i32.lt_u
                                                  br_if $loop_22
                                                  br $block_42
                                                end ;; $loop_22
                                              end ;; $block_111
                                              i32.const 0
                                              set_local $17
                                              br $block_42
                                            end ;; $block_45
                                            get_local $5
                                            i32.const 96
                                            i32.add
                                            get_local $29
                                            get_local $30
                                            get_local $35
                                            get_local $34
                                            call $__addtf3
                                            get_local $5
                                            i32.const 80
                                            i32.add
                                            get_local $5
                                            i64.load offset=96
                                            get_local $5
                                            i32.const 96
                                            i32.add
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            get_local $35
                                            get_local $34
                                            call $__subtf3
                                            get_local $5
                                            i32.const 80
                                            i32.add
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            set_local $30
                                            get_local $5
                                            i64.load offset=80
                                            set_local $29
                                          end ;; $block_44
                                          block $block_113
                                            get_local $5
                                            i32.load offset=396
                                            tee_local $19
                                            get_local $19
                                            i32.const 31
                                            i32.shr_s
                                            tee_local $17
                                            i32.add
                                            get_local $17
                                            i32.xor
                                            i64.extend_u/i32
                                            get_local $11
                                            call $fmt_u
                                            tee_local $17
                                            get_local $11
                                            i32.ne
                                            br_if $block_113
                                            get_local $5
                                            i32.const 340
                                            i32.add
                                            i32.const 11
                                            i32.add
                                            i32.const 48
                                            i32.store8
                                            get_local $9
                                            set_local $17
                                          end ;; $block_113
                                          get_local $31
                                          i32.const 2
                                          i32.or
                                          set_local $25
                                          get_local $17
                                          i32.const -2
                                          i32.add
                                          tee_local $27
                                          get_local $28
                                          i32.const 15
                                          i32.add
                                          i32.store8
                                          get_local $17
                                          i32.const -1
                                          i32.add
                                          i32.const 45
                                          i32.const 43
                                          get_local $19
                                          i32.const 0
                                          i32.lt_s
                                          select
                                          i32.store8
                                          get_local $23
                                          i32.const 8
                                          i32.and
                                          set_local $24
                                          get_local $5
                                          i32.const 352
                                          i32.add
                                          set_local $19
                                          loop $loop_23
                                            get_local $5
                                            i32.const 32
                                            i32.add
                                            get_local $29
                                            get_local $30
                                            call $__fixtfsi
                                            tee_local $20
                                            call $__floatsitf
                                            get_local $5
                                            i32.const 16
                                            i32.add
                                            get_local $29
                                            get_local $30
                                            get_local $5
                                            i64.load offset=32
                                            get_local $5
                                            i32.const 32
                                            i32.add
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            call $__subtf3
                                            get_local $5
                                            get_local $5
                                            i64.load offset=16
                                            get_local $5
                                            i32.const 16
                                            i32.add
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            i64.const 0
                                            i64.const 4612530443357519872
                                            call $__multf3
                                            get_local $19
                                            tee_local $17
                                            get_local $20
                                            i32.const 3440
                                            i32.add
                                            i32.load8_u
                                            get_local $18
                                            i32.or
                                            i32.store8
                                            get_local $5
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            set_local $30
                                            get_local $5
                                            i64.load
                                            set_local $29
                                            block $block_114
                                              get_local $17
                                              i32.const 1
                                              i32.add
                                              tee_local $19
                                              get_local $5
                                              i32.const 352
                                              i32.add
                                              i32.sub
                                              i32.const 1
                                              i32.ne
                                              br_if $block_114
                                              block $block_115
                                                get_local $24
                                                br_if $block_115
                                                get_local $21
                                                i32.const 0
                                                i32.gt_s
                                                br_if $block_115
                                                get_local $29
                                                get_local $30
                                                i64.const 0
                                                i64.const 0
                                                call $__eqtf2
                                                i32.eqz
                                                br_if $block_114
                                              end ;; $block_115
                                              get_local $17
                                              i32.const 1
                                              i32.add
                                              i32.const 46
                                              i32.store8
                                              get_local $17
                                              i32.const 2
                                              i32.add
                                              set_local $19
                                            end ;; $block_114
                                            get_local $29
                                            get_local $30
                                            i64.const 0
                                            i64.const 0
                                            call $__netf2
                                            br_if $loop_23
                                          end ;; $loop_23
                                          i32.const -1
                                          set_local $17
                                          i32.const 2147483645
                                          get_local $25
                                          i32.sub
                                          get_local $11
                                          get_local $27
                                          i32.sub
                                          tee_local $20
                                          i32.sub
                                          get_local $21
                                          i32.lt_s
                                          br_if $block_39
                                          get_local $0
                                          i32.const 32
                                          get_local $22
                                          get_local $20
                                          get_local $25
                                          i32.add
                                          get_local $21
                                          i32.const 2
                                          i32.add
                                          get_local $19
                                          get_local $5
                                          i32.const 352
                                          i32.add
                                          i32.sub
                                          tee_local $18
                                          get_local $8
                                          get_local $19
                                          i32.add
                                          get_local $21
                                          i32.lt_s
                                          select
                                          get_local $18
                                          get_local $21
                                          select
                                          tee_local $19
                                          i32.add
                                          tee_local $17
                                          get_local $23
                                          call $pad
                                          block $block_116
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_116
                                            get_local $26
                                            get_local $25
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_116
                                          get_local $0
                                          i32.const 48
                                          get_local $22
                                          get_local $17
                                          get_local $23
                                          i32.const 65536
                                          i32.xor
                                          call $pad
                                          block $block_117
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_117
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            get_local $18
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_117
                                          get_local $0
                                          i32.const 48
                                          get_local $19
                                          get_local $18
                                          i32.sub
                                          i32.const 0
                                          i32.const 0
                                          call $pad
                                          block $block_118
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_118
                                            get_local $27
                                            get_local $20
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_118
                                          get_local $0
                                          i32.const 32
                                          get_local $22
                                          get_local $17
                                          get_local $23
                                          i32.const 8192
                                          i32.xor
                                          call $pad
                                          get_local $22
                                          get_local $17
                                          get_local $17
                                          get_local $22
                                          i32.lt_s
                                          select
                                          set_local $17
                                          br $block_39
                                        end ;; $block_43
                                        i32.const 0
                                        set_local $17
                                        get_local $0
                                        i32.const 32
                                        get_local $22
                                        i32.const 0
                                        get_local $23
                                        call $pad
                                      end ;; $block_42
                                      get_local $0
                                      i32.const 32
                                      get_local $22
                                      get_local $17
                                      get_local $23
                                      i32.const 8192
                                      i32.xor
                                      call $pad
                                      get_local $22
                                      get_local $17
                                      get_local $22
                                      get_local $17
                                      i32.gt_s
                                      select
                                      set_local $17
                                      br $loop
                                    end ;; $block_41
                                    i32.const 0
                                    set_local $25
                                    get_local $36
                                    get_local $21
                                    get_local $18
                                    i32.add
                                    get_local $17
                                    i32.sub
                                    tee_local $17
                                    i32.const 0
                                    get_local $17
                                    i32.const 0
                                    i32.gt_s
                                    select
                                    tee_local $17
                                    get_local $36
                                    get_local $17
                                    i32.lt_s
                                    select
                                    set_local $36
                                  end ;; $block_40
                                  i32.const -1
                                  set_local $17
                                  get_local $36
                                  i32.const 2147483645
                                  i32.const 2147483646
                                  get_local $36
                                  get_local $25
                                  i32.or
                                  tee_local $27
                                  select
                                  i32.gt_s
                                  br_if $block_39
                                  get_local $36
                                  get_local $27
                                  i32.const 0
                                  i32.ne
                                  i32.add
                                  i32.const 1
                                  i32.add
                                  set_local $33
                                  block $block_119
                                    block $block_120
                                      get_local $28
                                      i32.const 32
                                      i32.or
                                      i32.const 102
                                      i32.ne
                                      tee_local $38
                                      br_if $block_120
                                      get_local $18
                                      i32.const 2147483647
                                      get_local $33
                                      i32.sub
                                      i32.gt_s
                                      br_if $block_39
                                      get_local $18
                                      i32.const 0
                                      get_local $18
                                      i32.const 0
                                      i32.gt_s
                                      select
                                      set_local $18
                                      br $block_119
                                    end ;; $block_120
                                    block $block_121
                                      get_local $11
                                      get_local $18
                                      get_local $18
                                      i32.const 31
                                      i32.shr_s
                                      tee_local $17
                                      i32.add
                                      get_local $17
                                      i32.xor
                                      i64.extend_u/i32
                                      get_local $11
                                      call $fmt_u
                                      tee_local $21
                                      i32.sub
                                      i32.const 1
                                      i32.gt_s
                                      br_if $block_121
                                      get_local $21
                                      i32.const -1
                                      i32.add
                                      set_local $17
                                      loop $loop_24
                                        get_local $17
                                        i32.const 48
                                        i32.store8
                                        get_local $11
                                        get_local $17
                                        i32.sub
                                        set_local $21
                                        get_local $17
                                        i32.const -1
                                        i32.add
                                        tee_local $24
                                        set_local $17
                                        get_local $21
                                        i32.const 2
                                        i32.lt_s
                                        br_if $loop_24
                                      end ;; $loop_24
                                      get_local $24
                                      i32.const 1
                                      i32.add
                                      set_local $21
                                    end ;; $block_121
                                    get_local $21
                                    i32.const -2
                                    i32.add
                                    tee_local $24
                                    get_local $28
                                    i32.store8
                                    i32.const -1
                                    set_local $17
                                    get_local $21
                                    i32.const -1
                                    i32.add
                                    i32.const 45
                                    i32.const 43
                                    get_local $18
                                    i32.const 0
                                    i32.lt_s
                                    select
                                    i32.store8
                                    get_local $11
                                    get_local $24
                                    i32.sub
                                    tee_local $18
                                    i32.const 2147483647
                                    get_local $33
                                    i32.sub
                                    i32.gt_s
                                    br_if $block_39
                                  end ;; $block_119
                                  i32.const -1
                                  set_local $17
                                  get_local $18
                                  get_local $33
                                  i32.add
                                  tee_local $18
                                  get_local $31
                                  i32.const 2147483647
                                  i32.xor
                                  i32.gt_s
                                  br_if $block_39
                                  get_local $0
                                  i32.const 32
                                  get_local $22
                                  get_local $18
                                  get_local $31
                                  i32.add
                                  tee_local $28
                                  get_local $23
                                  call $pad
                                  block $block_122
                                    get_local $0
                                    i32.load8_u
                                    i32.const 32
                                    i32.and
                                    br_if $block_122
                                    get_local $32
                                    get_local $31
                                    get_local $0
                                    call $__fwritex
                                    drop
                                  end ;; $block_122
                                  get_local $0
                                  i32.const 48
                                  get_local $22
                                  get_local $28
                                  get_local $23
                                  i32.const 65536
                                  i32.xor
                                  call $pad
                                  block $block_123
                                    block $block_124
                                      get_local $38
                                      br_if $block_124
                                      get_local $37
                                      get_local $19
                                      get_local $19
                                      get_local $37
                                      i32.gt_u
                                      select
                                      tee_local $18
                                      set_local $19
                                      loop $loop_25
                                        get_local $19
                                        i64.load32_u
                                        get_local $7
                                        call $fmt_u
                                        set_local $17
                                        block $block_125
                                          block $block_126
                                            get_local $19
                                            get_local $18
                                            i32.eq
                                            br_if $block_126
                                            get_local $17
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.le_u
                                            br_if $block_125
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.const 48
                                            get_local $17
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.sub
                                            call $memset
                                            drop
                                            loop $loop_26
                                              get_local $17
                                              i32.const -1
                                              i32.add
                                              tee_local $17
                                              get_local $5
                                              i32.const 352
                                              i32.add
                                              i32.gt_u
                                              br_if $loop_26
                                              br $block_125
                                            end ;; $loop_26
                                          end ;; $block_126
                                          get_local $17
                                          get_local $7
                                          i32.ne
                                          br_if $block_125
                                          get_local $5
                                          i32.const 48
                                          i32.store8 offset=360
                                          get_local $6
                                          set_local $17
                                        end ;; $block_125
                                        block $block_127
                                          get_local $0
                                          i32.load8_u
                                          i32.const 32
                                          i32.and
                                          br_if $block_127
                                          get_local $17
                                          get_local $7
                                          get_local $17
                                          i32.sub
                                          get_local $0
                                          call $__fwritex
                                          drop
                                        end ;; $block_127
                                        get_local $19
                                        i32.const 4
                                        i32.add
                                        tee_local $19
                                        get_local $37
                                        i32.le_u
                                        br_if $loop_25
                                      end ;; $loop_25
                                      block $block_128
                                        get_local $27
                                        i32.eqz
                                        br_if $block_128
                                        get_local $0
                                        i32.load8_u
                                        i32.const 32
                                        i32.and
                                        br_if $block_128
                                        i32.const 3491
                                        i32.const 1
                                        get_local $0
                                        call $__fwritex
                                        drop
                                      end ;; $block_128
                                      block $block_129
                                        get_local $36
                                        i32.const 1
                                        i32.lt_s
                                        br_if $block_129
                                        get_local $19
                                        get_local $20
                                        i32.ge_u
                                        br_if $block_129
                                        loop $loop_27
                                          block $block_130
                                            get_local $19
                                            i64.load32_u
                                            get_local $7
                                            call $fmt_u
                                            tee_local $17
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.le_u
                                            br_if $block_130
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.const 48
                                            get_local $17
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.sub
                                            call $memset
                                            drop
                                            loop $loop_28
                                              get_local $17
                                              i32.const -1
                                              i32.add
                                              tee_local $17
                                              get_local $5
                                              i32.const 352
                                              i32.add
                                              i32.gt_u
                                              br_if $loop_28
                                            end ;; $loop_28
                                          end ;; $block_130
                                          block $block_131
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_131
                                            get_local $17
                                            get_local $36
                                            i32.const 9
                                            get_local $36
                                            i32.const 9
                                            i32.lt_s
                                            select
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_131
                                          get_local $36
                                          i32.const -9
                                          i32.add
                                          tee_local $36
                                          i32.const 1
                                          i32.lt_s
                                          br_if $block_129
                                          get_local $19
                                          i32.const 4
                                          i32.add
                                          tee_local $19
                                          get_local $20
                                          i32.lt_u
                                          br_if $loop_27
                                        end ;; $loop_27
                                      end ;; $block_129
                                      get_local $0
                                      i32.const 48
                                      get_local $36
                                      i32.const 9
                                      i32.add
                                      i32.const 9
                                      i32.const 0
                                      call $pad
                                      br $block_123
                                    end ;; $block_124
                                    block $block_132
                                      get_local $36
                                      i32.const 0
                                      i32.lt_s
                                      br_if $block_132
                                      get_local $20
                                      get_local $19
                                      i32.const 4
                                      i32.add
                                      get_local $26
                                      select
                                      set_local $21
                                      get_local $19
                                      set_local $20
                                      loop $loop_29
                                        block $block_133
                                          get_local $20
                                          i64.load32_u
                                          get_local $7
                                          call $fmt_u
                                          tee_local $17
                                          get_local $7
                                          i32.ne
                                          br_if $block_133
                                          get_local $5
                                          i32.const 48
                                          i32.store8 offset=360
                                          get_local $6
                                          set_local $17
                                        end ;; $block_133
                                        block $block_134
                                          block $block_135
                                            get_local $20
                                            get_local $19
                                            i32.eq
                                            br_if $block_135
                                            get_local $17
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.le_u
                                            br_if $block_134
                                            get_local $5
                                            i32.const 352
                                            i32.add
                                            i32.const 48
                                            get_local $17
                                            get_local $12
                                            i32.add
                                            call $memset
                                            drop
                                            loop $loop_30
                                              get_local $17
                                              i32.const -1
                                              i32.add
                                              tee_local $17
                                              get_local $5
                                              i32.const 352
                                              i32.add
                                              i32.gt_u
                                              br_if $loop_30
                                              br $block_134
                                            end ;; $loop_30
                                          end ;; $block_135
                                          block $block_136
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_136
                                            get_local $17
                                            i32.const 1
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_136
                                          get_local $17
                                          i32.const 1
                                          i32.add
                                          set_local $17
                                          block $block_137
                                            get_local $25
                                            br_if $block_137
                                            get_local $36
                                            i32.const 1
                                            i32.lt_s
                                            br_if $block_134
                                          end ;; $block_137
                                          get_local $0
                                          i32.load8_u
                                          i32.const 32
                                          i32.and
                                          br_if $block_134
                                          i32.const 3491
                                          i32.const 1
                                          get_local $0
                                          call $__fwritex
                                          drop
                                        end ;; $block_134
                                        get_local $7
                                        get_local $17
                                        i32.sub
                                        set_local $18
                                        block $block_138
                                          get_local $0
                                          i32.load8_u
                                          i32.const 32
                                          i32.and
                                          br_if $block_138
                                          get_local $17
                                          get_local $18
                                          get_local $36
                                          get_local $36
                                          get_local $18
                                          i32.gt_s
                                          select
                                          get_local $0
                                          call $__fwritex
                                          drop
                                        end ;; $block_138
                                        get_local $36
                                        get_local $18
                                        i32.sub
                                        set_local $36
                                        get_local $20
                                        i32.const 4
                                        i32.add
                                        tee_local $20
                                        get_local $21
                                        i32.ge_u
                                        br_if $block_132
                                        get_local $36
                                        i32.const -1
                                        i32.gt_s
                                        br_if $loop_29
                                      end ;; $loop_29
                                    end ;; $block_132
                                    get_local $0
                                    i32.const 48
                                    get_local $36
                                    i32.const 18
                                    i32.add
                                    i32.const 18
                                    i32.const 0
                                    call $pad
                                    get_local $0
                                    i32.load8_u
                                    i32.const 32
                                    i32.and
                                    br_if $block_123
                                    get_local $24
                                    get_local $11
                                    get_local $24
                                    i32.sub
                                    get_local $0
                                    call $__fwritex
                                    drop
                                  end ;; $block_123
                                  get_local $0
                                  i32.const 32
                                  get_local $22
                                  get_local $28
                                  get_local $23
                                  i32.const 8192
                                  i32.xor
                                  call $pad
                                  get_local $22
                                  get_local $28
                                  get_local $28
                                  get_local $22
                                  i32.lt_s
                                  select
                                  set_local $17
                                end ;; $block_39
                                get_local $17
                                i32.const 0
                                i32.ge_s
                                br_if $loop
                                br $block_5
                              end ;; $block_38
                              get_local $5
                              i32.load offset=320
                              get_local $16
                              i32.store
                              br $loop
                            end ;; $block_37
                            get_local $5
                            i32.load offset=320
                            get_local $16
                            i64.extend_s/i32
                            i64.store
                            br $loop
                          end ;; $block_36
                          get_local $5
                          i32.load offset=320
                          get_local $16
                          i32.store16
                          br $loop
                        end ;; $block_35
                        get_local $5
                        i32.load offset=320
                        get_local $16
                        i32.store8
                        br $loop
                      end ;; $block_34
                      get_local $5
                      i32.load offset=320
                      get_local $16
                      i32.store
                      br $loop
                    end ;; $block_33
                    get_local $5
                    i32.load offset=320
                    get_local $16
                    i64.extend_s/i32
                    i64.store
                    br $loop
                  end ;; $loop
                end ;; $block_5
                call $__errno_location
                i32.const 75
                i32.store
                br $block_0
              end ;; $block_4
              get_local $0
              br_if $block
              get_local $15
              i32.eqz
              br_if $block_3
              get_local $3
              i32.const 16
              i32.add
              set_local $19
              get_local $4
              i32.const 4
              i32.add
              set_local $1
              i32.const 1
              set_local $17
              loop $loop_31
                get_local $1
                i32.load
                tee_local $20
                i32.eqz
                br_if $block_2
                get_local $19
                get_local $20
                get_local $2
                call $pop_arg
                get_local $19
                i32.const 16
                i32.add
                set_local $19
                get_local $1
                i32.const 4
                i32.add
                set_local $1
                i32.const 1
                set_local $16
                get_local $17
                i32.const 1
                i32.add
                tee_local $17
                i32.const 10
                i32.lt_u
                br_if $loop_31
                br $block
              end ;; $loop_31
            end ;; $block_3
            i32.const 0
            set_local $16
            br $block
          end ;; $block_2
          loop $loop_32
            get_local $1
            i32.load
            br_if $block_1
            get_local $1
            i32.const 4
            i32.add
            set_local $1
            i32.const 1
            set_local $16
            get_local $17
            i32.const 1
            i32.add
            tee_local $17
            i32.const 9
            i32.le_u
            br_if $loop_32
            br $block
          end ;; $loop_32
        end ;; $block_1
        call $__errno_location
        i32.const 22
        i32.store
      end ;; $block_0
      i32.const -1
      set_local $16
    end ;; $block
    get_local $5
    i32.const 7776
    i32.add
    set_global $20
    get_local $16
    )
  
  (func $pop_arg (type $11)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (local $3 i32)
    (local $4 i64)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $3
    set_global $20
    block $block
      get_local $1
      i32.const -9
      i32.add
      tee_local $1
      i32.const 17
      i32.gt_u
      br_if $block
      block $block_0
        block $block_1
          block $block_2
            block $block_3
              block $block_4
                block $block_5
                  block $block_6
                    block $block_7
                      block $block_8
                        block $block_9
                          block $block_10
                            block $block_11
                              block $block_12
                                block $block_13
                                  block $block_14
                                    block $block_15
                                      block $block_16
                                        block $block_17
                                          get_local $1
                                          br_table
                                            $block_17 $block_16 $block_15 $block_14 $block_13 $block_12 $block_11 $block_10 $block_9 $block_8 $block_7 $block_6 $block_5 $block_4 $block_3 $block_2
                                            $block_1 $block_0
                                            $block_17 ;; default
                                        end ;; $block_17
                                        get_local $2
                                        get_local $2
                                        i32.load
                                        tee_local $1
                                        i32.const 4
                                        i32.add
                                        i32.store
                                        get_local $0
                                        get_local $1
                                        i32.load
                                        i32.store
                                        br $block
                                      end ;; $block_16
                                      get_local $2
                                      get_local $2
                                      i32.load
                                      tee_local $1
                                      i32.const 4
                                      i32.add
                                      i32.store
                                      get_local $0
                                      get_local $1
                                      i64.load32_s
                                      i64.store
                                      br $block
                                    end ;; $block_15
                                    get_local $2
                                    get_local $2
                                    i32.load
                                    tee_local $1
                                    i32.const 4
                                    i32.add
                                    i32.store
                                    get_local $0
                                    get_local $1
                                    i64.load32_u
                                    i64.store
                                    br $block
                                  end ;; $block_14
                                  get_local $2
                                  get_local $2
                                  i32.load
                                  i32.const 7
                                  i32.add
                                  i32.const -8
                                  i32.and
                                  tee_local $1
                                  i32.const 8
                                  i32.add
                                  i32.store
                                  get_local $0
                                  get_local $1
                                  i64.load
                                  i64.store
                                  br $block
                                end ;; $block_13
                                get_local $2
                                get_local $2
                                i32.load
                                tee_local $1
                                i32.const 4
                                i32.add
                                i32.store
                                get_local $0
                                get_local $1
                                i64.load32_s
                                i64.store
                                br $block
                              end ;; $block_12
                              get_local $2
                              get_local $2
                              i32.load
                              tee_local $1
                              i32.const 4
                              i32.add
                              i32.store
                              get_local $0
                              get_local $1
                              i64.load32_u
                              i64.store
                              br $block
                            end ;; $block_11
                            get_local $2
                            get_local $2
                            i32.load
                            tee_local $1
                            i32.const 4
                            i32.add
                            i32.store
                            get_local $0
                            get_local $1
                            i64.load16_s
                            i64.store
                            br $block
                          end ;; $block_10
                          get_local $2
                          get_local $2
                          i32.load
                          tee_local $1
                          i32.const 4
                          i32.add
                          i32.store
                          get_local $0
                          get_local $1
                          i64.load16_u
                          i64.store
                          br $block
                        end ;; $block_9
                        get_local $2
                        get_local $2
                        i32.load
                        tee_local $1
                        i32.const 4
                        i32.add
                        i32.store
                        get_local $0
                        get_local $1
                        i64.load8_s
                        i64.store
                        br $block
                      end ;; $block_8
                      get_local $2
                      get_local $2
                      i32.load
                      tee_local $1
                      i32.const 4
                      i32.add
                      i32.store
                      get_local $0
                      get_local $1
                      i64.load8_u
                      i64.store
                      br $block
                    end ;; $block_7
                    get_local $2
                    get_local $2
                    i32.load
                    i32.const 7
                    i32.add
                    i32.const -8
                    i32.and
                    tee_local $1
                    i32.const 8
                    i32.add
                    i32.store
                    get_local $0
                    get_local $1
                    i64.load
                    i64.store
                    br $block
                  end ;; $block_6
                  get_local $2
                  get_local $2
                  i32.load
                  tee_local $1
                  i32.const 4
                  i32.add
                  i32.store
                  get_local $0
                  get_local $1
                  i64.load32_u
                  i64.store
                  br $block
                end ;; $block_5
                get_local $2
                get_local $2
                i32.load
                i32.const 7
                i32.add
                i32.const -8
                i32.and
                tee_local $1
                i32.const 8
                i32.add
                i32.store
                get_local $0
                get_local $1
                i64.load
                i64.store
                br $block
              end ;; $block_4
              get_local $2
              get_local $2
              i32.load
              i32.const 7
              i32.add
              i32.const -8
              i32.and
              tee_local $1
              i32.const 8
              i32.add
              i32.store
              get_local $0
              get_local $1
              i64.load
              i64.store
              br $block
            end ;; $block_3
            get_local $2
            get_local $2
            i32.load
            tee_local $1
            i32.const 4
            i32.add
            i32.store
            get_local $0
            get_local $1
            i64.load32_s
            i64.store
            br $block
          end ;; $block_2
          get_local $2
          get_local $2
          i32.load
          tee_local $1
          i32.const 4
          i32.add
          i32.store
          get_local $0
          get_local $1
          i64.load32_u
          i64.store
          br $block
        end ;; $block_1
        get_local $2
        get_local $2
        i32.load
        i32.const 7
        i32.add
        i32.const -8
        i32.and
        tee_local $1
        i32.const 8
        i32.add
        i32.store
        get_local $3
        get_local $1
        f64.load
        call $__extenddftf2
        get_local $0
        get_local $3
        i32.const 8
        i32.add
        i64.load
        i64.store offset=8
        get_local $0
        get_local $3
        i64.load
        i64.store
        br $block
      end ;; $block_0
      get_local $2
      get_local $2
      i32.load
      i32.const 15
      i32.add
      i32.const -16
      i32.and
      tee_local $1
      i32.const 16
      i32.add
      i32.store
      get_local $1
      i64.load
      set_local $4
      get_local $0
      get_local $1
      i64.load offset=8
      i64.store offset=8
      get_local $0
      get_local $4
      i64.store
    end ;; $block
    get_local $3
    i32.const 16
    i32.add
    set_global $20
    )
  
  (func $pad (type $12)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (param $3 i32)
    (param $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    get_global $20
    i32.const 256
    i32.sub
    tee_local $5
    set_global $20
    block $block
      get_local $2
      get_local $3
      i32.le_s
      br_if $block
      get_local $4
      i32.const 73728
      i32.and
      br_if $block
      get_local $5
      get_local $1
      get_local $2
      get_local $3
      i32.sub
      tee_local $4
      i32.const 256
      get_local $4
      i32.const 256
      i32.lt_u
      tee_local $6
      select
      call $memset
      set_local $7
      get_local $0
      i32.load
      tee_local $8
      i32.const 32
      i32.and
      set_local $1
      block $block_0
        block $block_1
          get_local $6
          br_if $block_1
          get_local $1
          i32.eqz
          set_local $1
          get_local $2
          get_local $3
          i32.sub
          set_local $3
          loop $loop
            block $block_2
              get_local $1
              i32.const 1
              i32.and
              i32.eqz
              br_if $block_2
              get_local $7
              i32.const 256
              get_local $0
              call $__fwritex
              drop
              get_local $0
              i32.load
              set_local $8
            end ;; $block_2
            get_local $8
            i32.const 32
            i32.and
            tee_local $2
            i32.eqz
            set_local $1
            get_local $4
            i32.const -256
            i32.add
            tee_local $4
            i32.const 255
            i32.gt_u
            br_if $loop
          end ;; $loop
          get_local $2
          br_if $block
          get_local $3
          i32.const 255
          i32.and
          set_local $4
          br $block_0
        end ;; $block_1
        get_local $1
        br_if $block
      end ;; $block_0
      get_local $7
      get_local $4
      get_local $0
      call $__fwritex
      drop
    end ;; $block
    get_local $5
    i32.const 256
    i32.add
    set_global $20
    )
  
  (func $fmt_u (type $13)
    (param $0 i64)
    (param $1 i32)
    (result i32)
    (local $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    block $block
      block $block_0
        get_local $0
        i64.const 4294967296
        i64.lt_u
        br_if $block_0
        loop $loop
          get_local $1
          i32.const -1
          i32.add
          tee_local $1
          get_local $0
          get_local $0
          i64.const 10
          i64.div_u
          tee_local $2
          i64.const 10
          i64.mul
          i64.sub
          i32.wrap/i64
          i32.const 48
          i32.or
          i32.store8
          get_local $0
          i64.const 42949672959
          i64.gt_u
          set_local $3
          get_local $2
          set_local $0
          get_local $3
          br_if $loop
          br $block
        end ;; $loop
      end ;; $block_0
      get_local $0
      set_local $2
    end ;; $block
    block $block_1
      get_local $2
      i32.wrap/i64
      tee_local $3
      i32.eqz
      br_if $block_1
      loop $loop_0
        get_local $1
        i32.const -1
        i32.add
        tee_local $1
        get_local $3
        get_local $3
        i32.const 10
        i32.div_u
        tee_local $4
        i32.const 10
        i32.mul
        i32.sub
        i32.const 48
        i32.or
        i32.store8
        get_local $3
        i32.const 9
        i32.gt_u
        set_local $5
        get_local $4
        set_local $3
        get_local $5
        br_if $loop_0
      end ;; $loop_0
    end ;; $block_1
    get_local $1
    )
  
  (func $wcrtomb (type $0)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    i32.const 1
    set_local $3
    block $block
      block $block_0
        get_local $0
        i32.eqz
        br_if $block_0
        get_local $1
        i32.const 127
        i32.le_u
        br_if $block
        block $block_1
          block $block_2
            block $block_3
              i32.const 0
              i32.load offset=103
              i32.load
              i32.eqz
              br_if $block_3
              get_local $1
              i32.const 2047
              i32.gt_u
              br_if $block_2
              get_local $0
              get_local $1
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=1
              get_local $0
              get_local $1
              i32.const 6
              i32.shr_u
              i32.const 192
              i32.or
              i32.store8
              i32.const 2
              return
            end ;; $block_3
            get_local $1
            i32.const -128
            i32.and
            i32.const 57216
            i32.eq
            br_if $block
            call $__errno_location
            i32.const 84
            i32.store
            br $block_1
          end ;; $block_2
          block $block_4
            block $block_5
              get_local $1
              i32.const 55296
              i32.lt_u
              br_if $block_5
              get_local $1
              i32.const -8192
              i32.and
              i32.const 57344
              i32.eq
              br_if $block_5
              get_local $1
              i32.const -65536
              i32.add
              i32.const 1048575
              i32.gt_u
              br_if $block_4
              get_local $0
              get_local $1
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=3
              get_local $0
              get_local $1
              i32.const 18
              i32.shr_u
              i32.const 240
              i32.or
              i32.store8
              get_local $0
              get_local $1
              i32.const 6
              i32.shr_u
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=2
              get_local $0
              get_local $1
              i32.const 12
              i32.shr_u
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=1
              i32.const 4
              return
            end ;; $block_5
            get_local $0
            get_local $1
            i32.const 63
            i32.and
            i32.const 128
            i32.or
            i32.store8 offset=2
            get_local $0
            get_local $1
            i32.const 12
            i32.shr_u
            i32.const 224
            i32.or
            i32.store8
            get_local $0
            get_local $1
            i32.const 6
            i32.shr_u
            i32.const 63
            i32.and
            i32.const 128
            i32.or
            i32.store8 offset=1
            i32.const 3
            return
          end ;; $block_4
          call $__errno_location
          i32.const 84
          i32.store
        end ;; $block_1
        i32.const -1
        set_local $3
      end ;; $block_0
      get_local $3
      return
    end ;; $block
    get_local $0
    get_local $1
    i32.store8
    i32.const 1
    )
  
  (func $wctomb (type $6)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    block $block
      get_local $0
      i32.eqz
      br_if $block
      get_local $0
      get_local $1
      i32.const 0
      call $wcrtomb
      return
    end ;; $block
    i32.const 0
    )
  
  (func $__unordtf2 (type $14)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    get_local $0
    i64.const 0
    i64.ne
    get_local $1
    i64.const 9223372036854775807
    i64.and
    tee_local $1
    i64.const 9223090561878065152
    i64.gt_u
    get_local $1
    i64.const 9223090561878065152
    i64.eq
    select
    get_local $2
    i64.const 0
    i64.ne
    get_local $3
    i64.const 9223372036854775807
    i64.and
    tee_local $1
    i64.const 9223090561878065152
    i64.gt_u
    get_local $1
    i64.const 9223090561878065152
    i64.eq
    select
    i32.or
    )
  
  (func $__eqtf2 (type $14)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i64)
    i32.const 1
    set_local $4
    block $block
      get_local $0
      i64.const 0
      i64.ne
      get_local $1
      i64.const 9223372036854775807
      i64.and
      tee_local $5
      i64.const 9223090561878065152
      i64.gt_u
      get_local $5
      i64.const 9223090561878065152
      i64.eq
      select
      br_if $block
      get_local $2
      i64.const 0
      i64.ne
      get_local $3
      i64.const 9223372036854775807
      i64.and
      tee_local $6
      i64.const 9223090561878065152
      i64.gt_u
      get_local $6
      i64.const 9223090561878065152
      i64.eq
      select
      br_if $block
      block $block_0
        block $block_1
          get_local $2
          get_local $0
          i64.or
          get_local $6
          get_local $5
          i64.or
          i64.or
          i64.eqz
          br_if $block_1
          get_local $3
          get_local $1
          i64.and
          i64.const 0
          i64.lt_s
          br_if $block_0
          i32.const -1
          set_local $4
          get_local $0
          get_local $2
          i64.lt_u
          get_local $1
          get_local $3
          i64.lt_s
          get_local $1
          get_local $3
          i64.eq
          select
          br_if $block
          get_local $0
          get_local $2
          i64.xor
          get_local $1
          get_local $3
          i64.xor
          i64.or
          i64.const 0
          i64.ne
          return
        end ;; $block_1
        i32.const 0
        return
      end ;; $block_0
      i32.const -1
      set_local $4
      get_local $0
      get_local $2
      i64.gt_u
      get_local $1
      get_local $3
      i64.gt_s
      get_local $1
      get_local $3
      i64.eq
      select
      br_if $block
      get_local $0
      get_local $2
      i64.xor
      get_local $1
      get_local $3
      i64.xor
      i64.or
      i64.const 0
      i64.ne
      set_local $4
    end ;; $block
    get_local $4
    )
  
  (func $__netf2 (type $14)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i64)
    i32.const 1
    set_local $4
    block $block
      get_local $0
      i64.const 0
      i64.ne
      get_local $1
      i64.const 9223372036854775807
      i64.and
      tee_local $5
      i64.const 9223090561878065152
      i64.gt_u
      get_local $5
      i64.const 9223090561878065152
      i64.eq
      select
      br_if $block
      get_local $2
      i64.const 0
      i64.ne
      get_local $3
      i64.const 9223372036854775807
      i64.and
      tee_local $6
      i64.const 9223090561878065152
      i64.gt_u
      get_local $6
      i64.const 9223090561878065152
      i64.eq
      select
      br_if $block
      block $block_0
        block $block_1
          get_local $2
          get_local $0
          i64.or
          get_local $6
          get_local $5
          i64.or
          i64.or
          i64.eqz
          br_if $block_1
          get_local $3
          get_local $1
          i64.and
          i64.const 0
          i64.lt_s
          br_if $block_0
          i32.const -1
          set_local $4
          get_local $0
          get_local $2
          i64.lt_u
          get_local $1
          get_local $3
          i64.lt_s
          get_local $1
          get_local $3
          i64.eq
          select
          br_if $block
          get_local $0
          get_local $2
          i64.xor
          get_local $1
          get_local $3
          i64.xor
          i64.or
          i64.const 0
          i64.ne
          return
        end ;; $block_1
        i32.const 0
        return
      end ;; $block_0
      i32.const -1
      set_local $4
      get_local $0
      get_local $2
      i64.gt_u
      get_local $1
      get_local $3
      i64.gt_s
      get_local $1
      get_local $3
      i64.eq
      select
      br_if $block
      get_local $0
      get_local $2
      i64.xor
      get_local $1
      get_local $3
      i64.xor
      i64.or
      i64.const 0
      i64.ne
      set_local $4
    end ;; $block
    get_local $4
    )
  
  (func $__extenddftf2 (type $15)
    (param $0 i32)
    (param $1 f64)
    (local $2 i32)
    (local $3 i64)
    (local $4 i64)
    (local $5 i64)
    (local $6 i64)
    (local $7 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    get_local $1
    i64.reinterpret/f64
    tee_local $3
    i64.const -9223372036854775808
    i64.and
    set_local $4
    block $block
      block $block_0
        get_local $3
        i64.const 9223372036854775807
        i64.and
        tee_local $5
        i64.const -4503599627370496
        i64.add
        i64.const 9214364837600034815
        i64.gt_u
        br_if $block_0
        get_local $5
        i64.const 60
        i64.shl
        set_local $6
        get_local $5
        i64.const 4
        i64.shr_u
        i64.const 4323455642275676160
        i64.add
        set_local $5
        br $block
      end ;; $block_0
      block $block_1
        get_local $5
        i64.const 9218868437227405312
        i64.lt_u
        br_if $block_1
        get_local $3
        i64.const 60
        i64.shl
        set_local $6
        get_local $3
        i64.const 4
        i64.shr_u
        i64.const 9223090561878065152
        i64.or
        set_local $5
        br $block
      end ;; $block_1
      block $block_2
        block $block_3
          block $block_4
            get_local $5
            i64.eqz
            br_if $block_4
            get_local $5
            i64.const 4294967296
            i64.ge_u
            br_if $block_3
            get_local $3
            i32.wrap/i64
            i32.clz
            i32.const 32
            i32.add
            set_local $7
            br $block_2
          end ;; $block_4
          i64.const 0
          set_local $6
          i64.const 0
          set_local $5
          br $block
        end ;; $block_3
        get_local $5
        i64.const 32
        i64.shr_u
        i32.wrap/i64
        i32.clz
        set_local $7
      end ;; $block_2
      get_local $2
      get_local $5
      i64.const 0
      get_local $7
      i32.const 49
      i32.add
      call $__ashlti3
      get_local $2
      i32.const 8
      i32.add
      i64.load
      i64.const 281474976710656
      i64.xor
      i32.const 15372
      get_local $7
      i32.sub
      i64.extend_u/i32
      i64.const 48
      i64.shl
      i64.or
      set_local $5
      get_local $2
      i64.load
      set_local $6
    end ;; $block
    get_local $0
    get_local $6
    i64.store
    get_local $0
    get_local $5
    get_local $4
    i64.or
    i64.store offset=8
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    )
  
  (func $__fixtfsi (type $9)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    i32.const 0
    set_local $3
    block $block
      block $block_0
        get_local $1
        i64.const 48
        i64.shr_u
        i32.wrap/i64
        i32.const 32767
        i32.and
        tee_local $4
        i32.const -16383
        i32.add
        tee_local $5
        i32.const 0
        i32.lt_s
        br_if $block_0
        get_local $5
        i32.const 32
        i32.lt_u
        br_if $block
        i32.const -2147483648
        i32.const 2147483647
        get_local $1
        i64.const 0
        i64.lt_s
        select
        set_local $3
      end ;; $block_0
      get_local $2
      i32.const 16
      i32.add
      set_global $20
      get_local $3
      return
    end ;; $block
    get_local $2
    get_local $0
    get_local $1
    i64.const 281474976710655
    i64.and
    i64.const 281474976710656
    i64.or
    i32.const 16495
    get_local $4
    i32.sub
    call $__lshrti3
    get_local $2
    i32.load
    set_local $3
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    get_local $3
    i32.const -1
    i32.const 1
    get_local $1
    i64.const 0
    i64.lt_s
    select
    i32.mul
    )
  
  (func $__fixunstfsi (type $9)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    i32.const 0
    set_local $3
    block $block
      get_local $1
      i64.const 0
      i64.lt_s
      br_if $block
      get_local $1
      i64.const 48
      i64.shr_u
      i32.wrap/i64
      i32.const 32767
      i32.and
      tee_local $4
      i32.const -16383
      i32.add
      tee_local $5
      i32.const 0
      i32.lt_s
      br_if $block
      i32.const -1
      set_local $3
      get_local $5
      i32.const 31
      i32.gt_u
      br_if $block
      get_local $2
      get_local $0
      get_local $1
      i64.const 281474976710655
      i64.and
      i64.const 281474976710656
      i64.or
      i32.const 16495
      get_local $4
      i32.sub
      call $__lshrti3
      get_local $2
      i32.load
      set_local $3
    end ;; $block
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    get_local $3
    )
  
  (func $__floatsitf (type $16)
    (param $0 i32)
    (param $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i64)
    (local $5 i64)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    block $block
      block $block_0
        get_local $1
        i32.eqz
        br_if $block_0
        get_local $2
        get_local $1
        get_local $1
        i32.const 31
        i32.shr_s
        tee_local $3
        i32.add
        get_local $3
        i32.xor
        tee_local $3
        i64.extend_u/i32
        i64.const 0
        get_local $3
        i32.clz
        tee_local $3
        i32.const 81
        i32.add
        call $__ashlti3
        get_local $2
        i32.const 8
        i32.add
        i64.load
        i64.const 281474976710656
        i64.xor
        i32.const 16414
        get_local $3
        i32.sub
        i64.extend_u/i32
        i64.const 48
        i64.shl
        i64.add
        get_local $1
        i32.const -2147483648
        i32.and
        i64.extend_u/i32
        i64.const 32
        i64.shl
        i64.or
        set_local $4
        get_local $2
        i64.load
        set_local $5
        br $block
      end ;; $block_0
      i64.const 0
      set_local $5
      i64.const 0
      set_local $4
    end ;; $block
    get_local $0
    get_local $5
    i64.store
    get_local $0
    get_local $4
    i64.store offset=8
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    )
  
  (func $__floatunsitf (type $16)
    (param $0 i32)
    (param $1 i32)
    (local $2 i32)
    (local $3 i64)
    (local $4 i64)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $2
    set_global $20
    block $block
      block $block_0
        get_local $1
        i32.eqz
        br_if $block_0
        get_local $2
        get_local $1
        i64.extend_u/i32
        i64.const 0
        get_local $1
        i32.clz
        tee_local $1
        i32.const 81
        i32.add
        call $__ashlti3
        get_local $2
        i32.const 8
        i32.add
        i64.load
        i64.const 281474976710656
        i64.xor
        i32.const 16414
        get_local $1
        i32.sub
        i64.extend_u/i32
        i64.const 48
        i64.shl
        i64.add
        set_local $3
        get_local $2
        i64.load
        set_local $4
        br $block
      end ;; $block_0
      i64.const 0
      set_local $4
      i64.const 0
      set_local $3
    end ;; $block
    get_local $0
    get_local $4
    i64.store
    get_local $0
    get_local $3
    i64.store offset=8
    get_local $2
    i32.const 16
    i32.add
    set_global $20
    )
  
  (func $__addtf3 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i32)
    (local $10 i32)
    (local $11 i32)
    (local $12 i64)
    get_global $20
    i32.const 112
    i32.sub
    tee_local $5
    set_global $20
    get_local $4
    i64.const 9223372036854775807
    i64.and
    set_local $6
    block $block
      block $block_0
        block $block_1
          get_local $1
          i64.const -1
          i64.add
          tee_local $7
          i64.const -1
          i64.eq
          get_local $2
          i64.const 9223372036854775807
          i64.and
          tee_local $8
          get_local $7
          get_local $1
          i64.lt_u
          i64.extend_u/i32
          i64.add
          i64.const -1
          i64.add
          tee_local $7
          i64.const 9223090561878065151
          i64.gt_u
          get_local $7
          i64.const 9223090561878065151
          i64.eq
          select
          br_if $block_1
          get_local $3
          i64.const -1
          i64.add
          tee_local $7
          i64.const -1
          i64.ne
          get_local $6
          get_local $7
          get_local $3
          i64.lt_u
          i64.extend_u/i32
          i64.add
          i64.const -1
          i64.add
          tee_local $7
          i64.const 9223090561878065151
          i64.lt_u
          get_local $7
          i64.const 9223090561878065151
          i64.eq
          select
          br_if $block_0
        end ;; $block_1
        block $block_2
          get_local $1
          i64.eqz
          get_local $8
          i64.const 9223090561878065152
          i64.lt_u
          get_local $8
          i64.const 9223090561878065152
          i64.eq
          select
          br_if $block_2
          get_local $2
          i64.const 140737488355328
          i64.or
          set_local $4
          get_local $1
          set_local $3
          br $block
        end ;; $block_2
        block $block_3
          get_local $3
          i64.eqz
          get_local $6
          i64.const 9223090561878065152
          i64.lt_u
          get_local $6
          i64.const 9223090561878065152
          i64.eq
          select
          br_if $block_3
          get_local $4
          i64.const 140737488355328
          i64.or
          set_local $4
          br $block
        end ;; $block_3
        block $block_4
          get_local $1
          get_local $8
          i64.const 9223090561878065152
          i64.xor
          i64.or
          i64.const 0
          i64.ne
          br_if $block_4
          i64.const 9223231299366420480
          get_local $2
          get_local $3
          get_local $1
          i64.xor
          get_local $4
          get_local $2
          i64.xor
          i64.const -9223372036854775808
          i64.xor
          i64.or
          i64.eqz
          tee_local $11
          select
          set_local $4
          i64.const 0
          get_local $1
          get_local $11
          select
          set_local $3
          br $block
        end ;; $block_4
        get_local $3
        get_local $6
        i64.const 9223090561878065152
        i64.xor
        i64.or
        i64.eqz
        br_if $block
        block $block_5
          get_local $1
          get_local $8
          i64.or
          i64.const 0
          i64.eq
          br_if $block_5
          get_local $3
          get_local $6
          i64.or
          i64.eqz
          i32.eqz
          br_if $block_0
          get_local $1
          set_local $3
          get_local $2
          set_local $4
          br $block
        end ;; $block_5
        get_local $3
        get_local $6
        i64.or
        i64.const 0
        i64.ne
        br_if $block
        get_local $3
        get_local $1
        i64.and
        set_local $3
        get_local $4
        get_local $2
        i64.and
        set_local $4
        br $block
      end ;; $block_0
      get_local $3
      get_local $1
      get_local $3
      get_local $1
      i64.gt_u
      get_local $6
      get_local $8
      i64.gt_u
      get_local $6
      get_local $8
      i64.eq
      select
      tee_local $9
      select
      set_local $6
      get_local $4
      get_local $2
      get_local $9
      select
      tee_local $8
      i64.const 281474976710655
      i64.and
      set_local $7
      get_local $2
      get_local $4
      get_local $9
      select
      tee_local $2
      i64.const 48
      i64.shr_u
      i32.wrap/i64
      i32.const 32767
      i32.and
      set_local $10
      block $block_6
        block $block_7
          block $block_8
            get_local $8
            i64.const 48
            i64.shr_u
            i32.wrap/i64
            i32.const 32767
            i32.and
            tee_local $11
            i32.eqz
            br_if $block_8
            get_local $1
            get_local $3
            get_local $9
            select
            set_local $3
            get_local $2
            i64.const 281474976710655
            i64.and
            set_local $1
            get_local $10
            i32.eqz
            br_if $block_7
            br $block_6
          end ;; $block_8
          get_local $5
          i32.const 96
          i32.add
          get_local $6
          get_local $7
          get_local $6
          get_local $7
          get_local $7
          i64.eqz
          tee_local $11
          select
          i64.clz
          get_local $11
          i32.const 6
          i32.shl
          i64.extend_u/i32
          i64.add
          i32.wrap/i64
          tee_local $11
          i32.const -15
          i32.add
          call $__ashlti3
          i32.const 16
          get_local $11
          i32.sub
          set_local $11
          get_local $5
          i32.const 104
          i32.add
          i64.load
          set_local $7
          get_local $5
          i64.load offset=96
          set_local $6
          get_local $1
          get_local $3
          get_local $9
          select
          set_local $3
          get_local $2
          i64.const 281474976710655
          i64.and
          set_local $1
          get_local $10
          br_if $block_6
        end ;; $block_7
        get_local $5
        i32.const 80
        i32.add
        get_local $3
        get_local $1
        get_local $3
        get_local $1
        get_local $1
        i64.eqz
        tee_local $9
        select
        i64.clz
        get_local $9
        i32.const 6
        i32.shl
        i64.extend_u/i32
        i64.add
        i32.wrap/i64
        tee_local $9
        i32.const -15
        i32.add
        call $__ashlti3
        i32.const 16
        get_local $9
        i32.sub
        set_local $10
        get_local $5
        i32.const 88
        i32.add
        i64.load
        set_local $1
        get_local $5
        i64.load offset=80
        set_local $3
      end ;; $block_6
      get_local $1
      i64.const 3
      i64.shl
      get_local $3
      i64.const 61
      i64.shr_u
      i64.or
      i64.const 2251799813685248
      i64.or
      set_local $4
      get_local $7
      i64.const 3
      i64.shl
      get_local $6
      i64.const 61
      i64.shr_u
      i64.or
      set_local $1
      get_local $3
      i64.const 3
      i64.shl
      set_local $3
      get_local $8
      get_local $2
      i64.xor
      set_local $7
      block $block_9
        get_local $11
        get_local $10
        i32.sub
        tee_local $9
        i32.eqz
        br_if $block_9
        block $block_10
          get_local $9
          i32.const 127
          i32.gt_u
          br_if $block_10
          get_local $5
          i32.const 64
          i32.add
          get_local $3
          get_local $4
          i32.const 128
          get_local $9
          i32.sub
          call $__ashlti3
          get_local $5
          i32.const 48
          i32.add
          get_local $3
          get_local $4
          get_local $9
          call $__lshrti3
          get_local $5
          i64.load offset=48
          get_local $5
          i64.load offset=64
          get_local $5
          i32.const 64
          i32.add
          i32.const 8
          i32.add
          i64.load
          i64.or
          i64.const 0
          i64.ne
          i64.extend_u/i32
          i64.or
          set_local $3
          get_local $5
          i32.const 48
          i32.add
          i32.const 8
          i32.add
          i64.load
          set_local $4
          br $block_9
        end ;; $block_10
        i64.const 0
        set_local $4
        i64.const 1
        set_local $3
      end ;; $block_9
      get_local $1
      i64.const 2251799813685248
      i64.or
      set_local $12
      get_local $6
      i64.const 3
      i64.shl
      set_local $2
      block $block_11
        block $block_12
          block $block_13
            get_local $7
            i64.const -1
            i64.le_s
            br_if $block_13
            get_local $4
            get_local $12
            i64.add
            get_local $3
            get_local $2
            i64.add
            tee_local $1
            get_local $3
            i64.lt_u
            i64.extend_u/i32
            i64.add
            tee_local $3
            i64.const 4503599627370496
            i64.and
            i64.eqz
            br_if $block_12
            get_local $1
            i64.const 1
            i64.shr_u
            get_local $3
            i64.const 63
            i64.shl
            i64.or
            get_local $1
            i64.const 1
            i64.and
            i64.or
            set_local $1
            get_local $11
            i32.const 1
            i32.add
            set_local $11
            get_local $3
            i64.const 1
            i64.shr_u
            set_local $3
            br $block_12
          end ;; $block_13
          get_local $2
          get_local $3
          i64.sub
          tee_local $1
          get_local $12
          get_local $4
          i64.sub
          get_local $2
          get_local $3
          i64.lt_u
          i64.extend_u/i32
          i64.sub
          tee_local $3
          i64.or
          i64.eqz
          br_if $block_11
          get_local $3
          i64.const 2251799813685247
          i64.gt_u
          br_if $block_12
          get_local $5
          i32.const 32
          i32.add
          get_local $1
          get_local $3
          get_local $1
          get_local $3
          get_local $3
          i64.eqz
          tee_local $9
          select
          i64.clz
          get_local $9
          i32.const 6
          i32.shl
          i64.extend_u/i32
          i64.add
          i32.wrap/i64
          i32.const -12
          i32.add
          tee_local $9
          call $__ashlti3
          get_local $11
          get_local $9
          i32.sub
          set_local $11
          get_local $5
          i32.const 40
          i32.add
          i64.load
          set_local $3
          get_local $5
          i64.load offset=32
          set_local $1
        end ;; $block_12
        get_local $8
        i64.const -9223372036854775808
        i64.and
        set_local $4
        block $block_14
          get_local $11
          i32.const 32767
          i32.lt_s
          br_if $block_14
          get_local $4
          i64.const 9223090561878065152
          i64.or
          set_local $4
          i64.const 0
          set_local $3
          br $block
        end ;; $block_14
        i32.const 0
        set_local $9
        block $block_15
          block $block_16
            get_local $11
            i32.const 0
            i32.le_s
            br_if $block_16
            get_local $11
            set_local $9
            br $block_15
          end ;; $block_16
          get_local $5
          i32.const 16
          i32.add
          get_local $1
          get_local $3
          i32.const 128
          i32.const 1
          get_local $11
          i32.sub
          tee_local $11
          i32.sub
          call $__ashlti3
          get_local $5
          get_local $1
          get_local $3
          get_local $11
          call $__lshrti3
          get_local $5
          i64.load
          get_local $5
          i64.load offset=16
          get_local $5
          i32.const 16
          i32.add
          i32.const 8
          i32.add
          i64.load
          i64.or
          i64.const 0
          i64.ne
          i64.extend_u/i32
          i64.or
          set_local $1
          get_local $5
          i32.const 8
          i32.add
          i64.load
          set_local $3
        end ;; $block_15
        get_local $3
        i64.const 3
        i64.shr_u
        i64.const 281474976710655
        i64.and
        get_local $4
        i64.or
        get_local $9
        i64.extend_u/i32
        i64.const 48
        i64.shl
        i64.or
        get_local $1
        i64.const 3
        i64.shr_u
        get_local $3
        i64.const 61
        i64.shl
        i64.or
        tee_local $4
        get_local $1
        i32.wrap/i64
        i32.const 7
        i32.and
        tee_local $11
        i32.const 4
        i32.gt_u
        i64.extend_u/i32
        i64.add
        tee_local $3
        get_local $4
        i64.lt_u
        i64.extend_u/i32
        i64.add
        get_local $3
        i64.const 1
        i64.and
        i64.const 0
        get_local $11
        i32.const 4
        i32.eq
        select
        tee_local $1
        get_local $3
        i64.add
        tee_local $3
        get_local $1
        i64.lt_u
        i64.extend_u/i32
        i64.add
        set_local $4
        br $block
      end ;; $block_11
      i64.const 0
      set_local $3
      i64.const 0
      set_local $4
    end ;; $block
    get_local $0
    get_local $3
    i64.store
    get_local $0
    get_local $4
    i64.store offset=8
    get_local $5
    i32.const 112
    i32.add
    set_global $20
    )
  
  (func $__ashlti3 (type $10)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i32)
    (local $4 i64)
    block $block
      block $block_0
        block $block_1
          get_local $3
          i32.const 64
          i32.and
          br_if $block_1
          get_local $3
          i32.eqz
          br_if $block
          get_local $1
          i32.const 64
          get_local $3
          i32.sub
          i64.extend_u/i32
          i64.shr_u
          get_local $2
          get_local $3
          i64.extend_u/i32
          tee_local $4
          i64.shl
          i64.or
          set_local $2
          get_local $1
          get_local $4
          i64.shl
          set_local $1
          br $block_0
        end ;; $block_1
        get_local $1
        get_local $3
        i32.const -64
        i32.add
        i64.extend_u/i32
        i64.shl
        set_local $2
        i64.const 0
        set_local $1
      end ;; $block_0
      get_local $2
      i64.const 0
      i64.or
      set_local $2
    end ;; $block
    get_local $0
    get_local $1
    i64.store
    get_local $0
    get_local $2
    i64.store offset=8
    )
  
  (func $__lshrti3 (type $10)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i32)
    (local $4 i64)
    block $block
      block $block_0
        block $block_1
          get_local $3
          i32.const 64
          i32.and
          br_if $block_1
          get_local $3
          i32.eqz
          br_if $block
          get_local $2
          i32.const 64
          get_local $3
          i32.sub
          i64.extend_u/i32
          i64.shl
          get_local $1
          get_local $3
          i64.extend_u/i32
          tee_local $4
          i64.shr_u
          i64.or
          set_local $1
          get_local $2
          get_local $4
          i64.shr_u
          set_local $2
          i64.const 0
          set_local $4
          br $block_0
        end ;; $block_1
        get_local $2
        get_local $3
        i32.const -64
        i32.add
        i64.extend_u/i32
        i64.shr_u
        set_local $1
        i64.const 0
        set_local $4
        i64.const 0
        set_local $2
      end ;; $block_0
      get_local $4
      get_local $1
      i64.or
      set_local $1
    end ;; $block
    get_local $0
    get_local $1
    i64.store
    get_local $0
    get_local $2
    i64.store offset=8
    )
  
  (func $__multf3 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i32)
    get_global $20
    i32.const 96
    i32.sub
    tee_local $5
    set_global $20
    get_local $3
    i64.const 17
    i64.shr_u
    get_local $4
    i64.const 47
    i64.shl
    i64.or
    set_local $6
    get_local $1
    i64.const 32
    i64.shr_u
    get_local $2
    i64.const 32
    i64.shl
    i64.or
    set_local $7
    get_local $3
    i64.const 49
    i64.shr_u
    get_local $4
    i64.const 281474976710655
    i64.and
    tee_local $8
    i64.const 15
    i64.shl
    i64.or
    set_local $9
    get_local $4
    get_local $2
    i64.xor
    i64.const -9223372036854775808
    i64.and
    set_local $10
    get_local $8
    i64.const 17
    i64.shr_u
    set_local $11
    get_local $2
    i64.const 281474976710655
    i64.and
    tee_local $12
    i64.const 32
    i64.shr_u
    set_local $13
    get_local $4
    i64.const 48
    i64.shr_u
    i32.wrap/i64
    i32.const 32767
    i32.and
    set_local $14
    block $block
      block $block_0
        block $block_1
          block $block_2
            get_local $2
            i64.const 48
            i64.shr_u
            i32.wrap/i64
            i32.const 32767
            i32.and
            tee_local $15
            i32.const -1
            i32.add
            i32.const 32765
            i32.gt_u
            br_if $block_2
            i32.const 0
            set_local $16
            get_local $14
            i32.const -1
            i32.add
            i32.const 32766
            i32.lt_u
            br_if $block_1
          end ;; $block_2
          block $block_3
            get_local $1
            i64.eqz
            get_local $2
            i64.const 9223372036854775807
            i64.and
            tee_local $19
            i64.const 9223090561878065152
            i64.lt_u
            get_local $19
            i64.const 9223090561878065152
            i64.eq
            select
            br_if $block_3
            get_local $2
            i64.const 140737488355328
            i64.or
            set_local $10
            br $block
          end ;; $block_3
          block $block_4
            get_local $3
            i64.eqz
            get_local $4
            i64.const 9223372036854775807
            i64.and
            tee_local $2
            i64.const 9223090561878065152
            i64.lt_u
            get_local $2
            i64.const 9223090561878065152
            i64.eq
            select
            br_if $block_4
            get_local $4
            i64.const 140737488355328
            i64.or
            set_local $10
            get_local $3
            set_local $1
            br $block
          end ;; $block_4
          block $block_5
            block $block_6
              block $block_7
                get_local $1
                get_local $19
                i64.const 9223090561878065152
                i64.xor
                i64.or
                i64.const 0
                i64.ne
                br_if $block_7
                get_local $3
                get_local $2
                i64.or
                i64.eqz
                br_if $block_6
                get_local $10
                i64.const 9223090561878065152
                i64.or
                set_local $10
                br $block_0
              end ;; $block_7
              block $block_8
                get_local $3
                get_local $2
                i64.const 9223090561878065152
                i64.xor
                i64.or
                i64.const 0
                i64.ne
                br_if $block_8
                get_local $1
                get_local $19
                i64.or
                set_local $2
                i64.const 0
                set_local $1
                get_local $2
                i64.eqz
                br_if $block_5
                get_local $10
                i64.const 9223090561878065152
                i64.or
                set_local $10
                br $block
              end ;; $block_8
              get_local $1
              get_local $19
              i64.or
              i64.const 0
              i64.eq
              br_if $block_0
              get_local $3
              get_local $2
              i64.or
              i64.const 0
              i64.eq
              br_if $block_0
              i32.const 0
              set_local $16
              block $block_9
                get_local $19
                i64.const 281474976710655
                i64.gt_u
                br_if $block_9
                get_local $5
                i32.const 80
                i32.add
                get_local $1
                get_local $12
                get_local $1
                get_local $12
                get_local $12
                i64.eqz
                tee_local $16
                select
                i64.clz
                get_local $16
                i32.const 6
                i32.shl
                i64.extend_u/i32
                i64.add
                i32.wrap/i64
                tee_local $16
                i32.const -15
                i32.add
                call $__ashlti3
                i32.const 16
                get_local $16
                i32.sub
                set_local $16
                get_local $5
                i64.load offset=80
                tee_local $1
                i64.const 32
                i64.shr_u
                get_local $5
                i32.const 88
                i32.add
                i64.load
                tee_local $12
                i64.const 32
                i64.shl
                i64.or
                set_local $7
                get_local $12
                i64.const 32
                i64.shr_u
                set_local $13
              end ;; $block_9
              get_local $2
              i64.const 281474976710655
              i64.gt_u
              br_if $block_1
              get_local $5
              i32.const 64
              i32.add
              get_local $3
              get_local $8
              get_local $3
              get_local $8
              get_local $8
              i64.eqz
              tee_local $24
              select
              i64.clz
              get_local $24
              i32.const 6
              i32.shl
              i64.extend_u/i32
              i64.add
              i32.wrap/i64
              tee_local $24
              i32.const -15
              i32.add
              call $__ashlti3
              i32.const 16
              get_local $24
              i32.sub
              get_local $16
              i32.add
              set_local $16
              get_local $5
              i64.load offset=64
              tee_local $3
              i64.const 49
              i64.shr_u
              get_local $5
              i32.const 72
              i32.add
              i64.load
              tee_local $2
              i64.const 15
              i64.shl
              i64.or
              set_local $9
              get_local $3
              i64.const 17
              i64.shr_u
              get_local $2
              i64.const 47
              i64.shl
              i64.or
              set_local $6
              get_local $2
              i64.const 17
              i64.shr_u
              set_local $11
              br $block_1
            end ;; $block_6
            i64.const 9223231299366420480
            set_local $10
            br $block_0
          end ;; $block_5
          i64.const 9223231299366420480
          set_local $10
          br $block
        end ;; $block_1
        get_local $6
        i64.const 4294967295
        i64.and
        tee_local $2
        get_local $1
        i64.const 4294967295
        i64.and
        tee_local $4
        i64.mul
        tee_local $17
        get_local $3
        i64.const 15
        i64.shl
        i64.const 4294934528
        i64.and
        tee_local $1
        get_local $7
        i64.const 4294967295
        i64.and
        tee_local $3
        i64.mul
        i64.add
        tee_local $6
        i64.const 32
        i64.shl
        tee_local $8
        get_local $1
        get_local $4
        i64.mul
        i64.add
        tee_local $7
        get_local $8
        i64.lt_u
        i64.extend_u/i32
        get_local $2
        get_local $3
        i64.mul
        tee_local $18
        get_local $1
        get_local $12
        i64.const 4294967295
        i64.and
        tee_local $8
        i64.mul
        i64.add
        tee_local $19
        get_local $9
        i64.const 4294967295
        i64.and
        tee_local $12
        get_local $4
        i64.mul
        i64.add
        tee_local $9
        get_local $6
        i64.const 32
        i64.shr_u
        get_local $6
        get_local $17
        i64.lt_u
        i64.extend_u/i32
        i64.const 32
        i64.shl
        i64.or
        i64.add
        tee_local $17
        get_local $2
        get_local $8
        i64.mul
        tee_local $20
        get_local $1
        get_local $13
        i64.const 65536
        i64.or
        tee_local $6
        i64.mul
        i64.add
        tee_local $13
        get_local $12
        get_local $3
        i64.mul
        i64.add
        tee_local $21
        get_local $11
        i64.const 2147483647
        i64.and
        i64.const 2147483648
        i64.or
        tee_local $1
        get_local $4
        i64.mul
        i64.add
        tee_local $11
        i64.const 32
        i64.shl
        i64.add
        tee_local $22
        i64.add
        set_local $4
        get_local $15
        get_local $14
        i32.add
        get_local $16
        i32.add
        i32.const -16383
        i32.add
        set_local $14
        block $block_10
          block $block_11
            block $block_12
              get_local $12
              get_local $8
              i64.mul
              tee_local $23
              get_local $2
              get_local $6
              i64.mul
              i64.add
              tee_local $2
              get_local $23
              i64.lt_u
              i64.extend_u/i32
              get_local $2
              get_local $1
              get_local $3
              i64.mul
              i64.add
              tee_local $3
              get_local $2
              i64.lt_u
              i64.extend_u/i32
              i64.add
              get_local $3
              get_local $19
              get_local $18
              i64.lt_u
              i64.extend_u/i32
              get_local $9
              get_local $19
              i64.lt_u
              i64.extend_u/i32
              i64.add
              i64.add
              tee_local $2
              get_local $3
              i64.lt_u
              i64.extend_u/i32
              i64.add
              get_local $1
              get_local $6
              i64.mul
              i64.add
              get_local $1
              get_local $8
              i64.mul
              tee_local $3
              get_local $12
              get_local $6
              i64.mul
              i64.add
              tee_local $1
              get_local $3
              i64.lt_u
              i64.extend_u/i32
              i64.const 32
              i64.shl
              get_local $1
              i64.const 32
              i64.shr_u
              i64.or
              i64.add
              get_local $2
              get_local $1
              i64.const 32
              i64.shl
              i64.add
              tee_local $1
              get_local $2
              i64.lt_u
              i64.extend_u/i32
              i64.add
              get_local $1
              get_local $11
              i64.const 32
              i64.shr_u
              get_local $13
              get_local $20
              i64.lt_u
              i64.extend_u/i32
              get_local $21
              get_local $13
              i64.lt_u
              i64.extend_u/i32
              i64.add
              get_local $11
              get_local $21
              i64.lt_u
              i64.extend_u/i32
              i64.add
              i64.const 32
              i64.shl
              i64.or
              i64.add
              tee_local $3
              get_local $1
              i64.lt_u
              i64.extend_u/i32
              i64.add
              get_local $3
              get_local $17
              get_local $9
              i64.lt_u
              i64.extend_u/i32
              get_local $22
              get_local $17
              i64.lt_u
              i64.extend_u/i32
              i64.add
              i64.add
              tee_local $2
              get_local $3
              i64.lt_u
              i64.extend_u/i32
              i64.add
              tee_local $1
              i64.const 281474976710656
              i64.and
              i64.eqz
              i32.eqz
              br_if $block_12
              get_local $7
              i64.const 63
              i64.shr_u
              set_local $3
              get_local $1
              i64.const 1
              i64.shl
              get_local $2
              i64.const 63
              i64.shr_u
              i64.or
              set_local $1
              get_local $4
              i64.const 63
              i64.shr_u
              get_local $2
              i64.const 1
              i64.shl
              i64.or
              set_local $2
              get_local $7
              i64.const 1
              i64.shl
              set_local $7
              get_local $3
              get_local $4
              i64.const 1
              i64.shl
              i64.or
              set_local $4
              get_local $14
              i32.const 32767
              i32.ge_s
              br_if $block_11
              br $block_10
            end ;; $block_12
            get_local $14
            i32.const 1
            i32.add
            tee_local $14
            i32.const 32767
            i32.lt_s
            br_if $block_10
          end ;; $block_11
          get_local $10
          i64.const 9223090561878065152
          i64.or
          set_local $10
          br $block_0
        end ;; $block_10
        block $block_13
          block $block_14
            get_local $14
            i32.const 0
            i32.le_s
            br_if $block_14
            get_local $14
            i64.extend_u/i32
            i64.const 48
            i64.shl
            get_local $1
            i64.const 281474976710655
            i64.and
            i64.or
            set_local $1
            br $block_13
          end ;; $block_14
          i32.const 1
          get_local $14
          i32.sub
          tee_local $14
          i32.const 127
          i32.gt_u
          br_if $block_0
          get_local $5
          i32.const 32
          i32.add
          get_local $7
          get_local $4
          get_local $14
          call $__lshrti3
          get_local $5
          i32.const 16
          i32.add
          get_local $2
          get_local $1
          i32.const 128
          get_local $14
          i32.sub
          tee_local $15
          call $__ashlti3
          get_local $5
          i32.const 48
          i32.add
          get_local $7
          get_local $4
          get_local $15
          call $__ashlti3
          get_local $5
          get_local $2
          get_local $1
          get_local $14
          call $__lshrti3
          get_local $5
          i64.load offset=16
          get_local $5
          i64.load offset=32
          i64.or
          get_local $5
          i64.load offset=48
          get_local $5
          i32.const 48
          i32.add
          i32.const 8
          i32.add
          i64.load
          i64.or
          i64.const 0
          i64.ne
          i64.extend_u/i32
          i64.or
          set_local $7
          get_local $5
          i32.const 16
          i32.add
          i32.const 8
          i32.add
          i64.load
          get_local $5
          i32.const 32
          i32.add
          i32.const 8
          i32.add
          i64.load
          i64.or
          set_local $4
          get_local $5
          i32.const 8
          i32.add
          i64.load
          set_local $1
          get_local $5
          i64.load
          set_local $2
        end ;; $block_13
        get_local $1
        get_local $10
        i64.or
        set_local $10
        block $block_15
          get_local $7
          i64.eqz
          get_local $4
          i64.const -1
          i64.gt_s
          get_local $4
          i64.const -9223372036854775808
          i64.eq
          select
          br_if $block_15
          get_local $10
          get_local $2
          i64.const 1
          i64.add
          tee_local $1
          get_local $2
          i64.lt_u
          i64.extend_u/i32
          i64.add
          set_local $10
          br $block
        end ;; $block_15
        block $block_16
          get_local $7
          get_local $4
          i64.const -9223372036854775808
          i64.xor
          i64.or
          i64.const 0
          i64.ne
          br_if $block_16
          get_local $10
          get_local $2
          get_local $2
          i64.const 1
          i64.and
          i64.add
          tee_local $1
          get_local $2
          i64.lt_u
          i64.extend_u/i32
          i64.add
          set_local $10
          br $block
        end ;; $block_16
        get_local $2
        set_local $1
        br $block
      end ;; $block_0
      i64.const 0
      set_local $1
    end ;; $block
    get_local $0
    get_local $1
    i64.store
    get_local $0
    get_local $10
    i64.store offset=8
    get_local $5
    i32.const 96
    i32.add
    set_global $20
    )
  
  (func $__subtf3 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    get_global $20
    i32.const 16
    i32.sub
    tee_local $5
    set_global $20
    get_local $5
    get_local $1
    get_local $2
    get_local $3
    get_local $4
    i64.const -9223372036854775808
    i64.xor
    call $__addtf3
    get_local $5
    i64.load
    set_local $1
    get_local $0
    get_local $5
    i64.load offset=8
    i64.store offset=8
    get_local $0
    get_local $1
    i64.store
    get_local $5
    i32.const 16
    i32.add
    set_global $20
    ))