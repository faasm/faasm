(module
  (type $0 (func))
  (type $1 (func (param i32 i32 i32) (result i32)))
  (type $2 (func (param i32 i64 i32) (result i64)))
  (type $3 (func (param i32)))
  (type $4 (func (param i32) (result i32)))
  (type $5 (func (param i32 i32 i32 i32 i32 i32) (result i32)))
  (type $6 (func (param i32 i32 i32 i32 i32) (result i32)))
  (type $7 (func (param i32 i32 i32 i32) (result i32)))
  (type $8 (func (param i32 i32) (result i32)))
  (type $9 (func (param i32 i32)))
  (type $10 (func (result i32)))
  (type $11 (func (param i64 i64) (result i32)))
  (type $12 (func (param i32 i64 i64 i32)))
  (type $13 (func (param i32 i32 i32)))
  (type $14 (func (param i32 i32 i32 i32 i32)))
  (type $15 (func (param i64 i32) (result i32)))
  (type $16 (func (param i64 i64 i64 i64) (result i32)))
  (type $17 (func (param i32 i64 i64)))
  (type $18 (func (param i32 f64)))
  (type $19 (func (param f64) (result i64)))
  (type $20 (func (param i64) (result i32)))
  (type $21 (func (param i32 i64 i64 i64 i64)))
  (type $22 (func (param i64 i64 i64 i64 i32 i32)))
  (import "env" "__syscall_poll" (func $__syscall_poll (param i32 i32 i32) (result i32)))
  (import "env" "__syscall_open" (func $__syscall_open (param i32 i32 i32) (result i32)))
  (import "env" "abort" (func $abort))
  (import "env" "_Exit" (func $_Exit (param i32)))
  (import "env" "__syscall_close" (func $__syscall_close (param i32) (result i32)))
  (import "env" "__syscall_writev" (func $__syscall_writev (param i32 i32 i32) (result i32)))
  (import "env" "__syscall_ioctl" (func $__syscall_ioctl (param i32 i32 i32 i32 i32 i32) (result i32)))
  (import "env" "__syscall_llseek" (func $__syscall_llseek (param i32 i32 i32 i32 i32) (result i32)))
  (import "env" "__syscall_futex" (func $__syscall_futex (param i32 i32 i32 i32 i32 i32) (result i32)))
  (export "memory" (memory $24))
  (export "__heap_base" (global $26))
  (export "__data_end" (global $27))
  (export "_start" (func $_start))
  (export "_Z3runPhS_S_S_" (func $run_lparen_unsigned_char*_2c__unsigned_char*_2c__unsigned_char*_2c__unsigned_char*_rparen_))
  (memory $24  2)
  (table $23  8 8 anyfunc)
  (global $25  (mut i32) (i32.const 70384))
  (global $26  i32 (i32.const 70384))
  (global $27  i32 (i32.const 4844))
  (elem $23 (i32.const 1)
    $main $dummy $_fini $__stdio_write $__stdio_close $__stdout_write $__stdio_seek)
  (data $24 (i32.const 1024)
    "d: %.2f\n\00i: %i\n\00s: %i\n\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\01\00\00\00\00\00\00\00\02\00\00\00\00\00\00\00/dev/nul"
    "l\00\00\00X\12\00\00\00\00\00\00\00\00\00\00T!\"\19\0d\01\02\03\11K\1c\0c\10\04\0b\1d\12\1e'hnopqb \05\06\0f\13\14\15\1a\08\16\07($\17\18\09\n\0e\1b\1f%#\83"
    "\82}&*+<=>?CGJMXYZ[\\]^_`acdefgijklrstyz{|\00\00\00\00\00\00\00\00\00Illegal byte seq"
    "uence\00Domain error\00Result not representable\00Not a tty\00Permission"
    " denied\00Operation not permitted\00No such file or directory\00No suc"
    "h process\00File exists\00Value too large for data type\00No space lef"
    "t on device\00Out of memory\00Resource busy\00Interrupted system call\00"
    "Resource temporarily unavailable\00Invalid seek\00Cross-device link\00"
    "Read-only file system\00Directory not empty\00Connection reset by pe"
    "er\00Operation timed out\00Connection refused\00Host is down\00Host is u"
    "nreachable\00Address in use\00Broken pipe\00I/O error\00No such device o"
    "r address\00Block device required\00No such device\00Not a directory\00I"
    "s a directory\00Text file busy\00Exec format error\00Invalid argument\00"
    "Argument list too long\00Symbolic link loop\00Filename too long\00Too "
    "many open files in system\00No file descriptors available\00Bad file"
    " descriptor\00No child process\00Bad address\00File too large\00Too many"
    " links\00No locks available\00Resource deadlock would occur\00State no"
    "t recoverable\00Previous owner died\00Operation canceled\00Function no"
    "t implemented\00No message of desired type\00Identifier removed\00Devi"
    "ce not a stream\00No data available\00Device timeout\00Out of streams "
    "resources\00Link has been severed\00Protocol error\00Bad message\00File "
    "descriptor in bad state\00Not a socket\00Destination address require"
    "d\00Message too large\00Protocol wrong type for socket\00Protocol not "
    "available\00Protocol not supported\00Socket type not supported\00Not s"
    "upported\00Protocol family not supported\00Address family not suppor"
    "ted by protocol\00Address not available\00Network is down\00Network un"
    "reachable\00Connection reset by network\00Connection aborted\00No buff"
    "er space available\00Socket is connected\00Socket not connected\00Cann"
    "ot send after socket shutdown\00Operation already in progress\00Oper"
    "ation in progress\00Stale file handle\00Remote I/O error\00Quota excee"
    "ded\00No medium found\00Wrong medium type\00No error information\00\00-+  "
    " 0X0x\00(null)\00\00\00\00\19\00\n\00\19\19\19\00\00\00\00\05\00\00\00\00\00\00\09\00\00\00\00\0b\00\00\00\00\00\00\00\00\19\00\11\n\19\19\19\03\n\07\00\01\1b\09\0b\18"
    "\00\00\09\06\0b\00\00\0b\00\06\19\00\00\00\19\19\19\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\0e\00\00\00\00\00\00\00\00\19\00\n\0d\19\19\19\00\0d\00\00\02\00\09\0e\00\00\00\09\00\0e\00"
    "\00\0e\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\0c\00\00\00\00\00\00\00\00\00\00\00\13\00\00\00\00\13\00\00\00\00\09\0c\00\00\00\00\00\0c\00\00\0c\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\10\00\00\00\00\00\00\00\00\00\00\00\0f\00\00\00\04\0f\00\00\00\00\09\10\00\00\00\00\00\10\00\00\10\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\12\00\00\00\00\00\00\00\00\00\00\00\11\00\00\00\00\11\00\00\00\00\09\12\00\00\00\00\00\12\00\00\12\00\00\1a\00\00\00\1a\1a\1a\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\1a\00\00\00\1a\1a\1a\00\00\00\00\00\00\09\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\14\00\00\00\00\00\00\00\00\00\00\00\17\00\00\00\00\17\00\00\00\00\09\14\00\00\00\00\00\14\00\00\14\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\16\00\00"
    "\00\00\00\00\00\00\00\00\00\15\00\00\00\00\15\00\00\00\00\09\16\00\00\00\00\00\16\00\00\16\00\000123456789ABCDEF-0X+0X 0X-0x+0x "
    "0x\00inf\00INF\00nan\00NAN\00.\00")
  (data $24 (i32.const 3552)
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
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00")
  (data $24 (i32.const 4696)
    "\05\00\00\00\00\00\00\00\00\00\00\00\05\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\06\00\00\00\07\00\00\00H\0e\00\00\00\04\00\00\00\00\00\00\00\00\00\00\01\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\n\ff\ff\ff\ff\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"
    "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00X\12\00\00")
  
  (func $__wasm_call_ctors (type $0)
    )
  
  (func $_start (type $0)
    (local $0 i32)
    get_global $25
    i32.const 16
    i32.sub
    tee_local $0
    set_global $25
    get_local $0
    i32.const 0
    i32.store offset=12
    get_local $0
    i32.const 12
    i32.add
    call $_start_c
    unreachable
    )
  
  (func $_start_c (type $3)
    (param $0 i32)
    i32.const 1
    get_local $0
    i32.load
    get_local $0
    i32.const 4
    i32.add
    i32.const 2
    i32.const 3
    call $__libc_start_main
    drop
    unreachable
    )
  
  (func $run_lparen_unsigned_char*_2c__unsigned_char*_2c__unsigned_char*_2c__unsigned_char*_rparen_ (type $7)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (param $3 i32)
    (result i32)
    (local $4 i32)
    get_local $4
    call $faasm::exec_lparen_faasm::FaasmMemory*_rparen_
    drop
    i32.const 0
    )
  
  (func $faasm::exec_lparen_faasm::FaasmMemory*_rparen_ (type $4)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    get_global $25
    i32.const 48
    i32.sub
    tee_local $1
    set_global $25
    get_local $1
    i64.const 4616820122002590269
    i64.store offset=32
    i32.const 1024
    get_local $1
    i32.const 32
    i32.add
    call $printf
    drop
    get_local $1
    i32.const 7
    i32.store offset=16
    i32.const 1033
    get_local $1
    i32.const 16
    i32.add
    call $printf
    drop
    get_local $1
    i32.const 8
    i32.store
    i32.const 1040
    get_local $1
    call $printf
    drop
    get_local $1
    i32.const 48
    i32.add
    set_global $25
    i32.const 0
    )
  
  (func $main (type $8)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    (local $2 i32)
    get_local $2
    call $faasm::exec_lparen_faasm::FaasmMemory*_rparen_
    drop
    i32.const 0
    )
  
  (func $dummy (type $0)
    )
  
  (func $__init_libc (type $9)
    (param $0 i32)
    (param $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    get_global $25
    i32.const 192
    i32.sub
    tee_local $2
    set_global $25
    get_local $2
    i32.const 32
    i32.add
    i32.const 0
    i32.const 152
    call $memset
    drop
    i32.const 0
    get_local $0
    i32.store offset=3552
    loop $loop
      get_local $0
      i32.load
      set_local $3
      get_local $0
      i32.const 4
      i32.add
      tee_local $4
      set_local $0
      get_local $3
      br_if $loop
    end ;; $loop
    i32.const 0
    get_local $4
    i32.store offset=3580
    block $block
      block $block_0
        get_local $4
        i32.load
        tee_local $0
        i32.eqz
        br_if $block_0
        get_local $4
        i32.const 8
        i32.add
        set_local $3
        loop $loop_0
          block $block_1
            get_local $0
            i32.const 37
            i32.gt_u
            br_if $block_1
            get_local $2
            i32.const 32
            i32.add
            get_local $0
            i32.const 2
            i32.shl
            i32.add
            get_local $3
            i32.const -4
            i32.add
            i32.load
            i32.store
          end ;; $block_1
          get_local $3
          i32.load
          set_local $0
          get_local $3
          i32.const 8
          i32.add
          set_local $3
          get_local $0
          br_if $loop_0
        end ;; $loop_0
        get_local $2
        i32.load offset=156
        set_local $0
        get_local $2
        i32.load offset=56
        set_local $3
        get_local $2
        i32.load offset=160
        set_local $4
        get_local $2
        i32.load offset=96
        set_local $5
        br $block
      end ;; $block_0
      i32.const 0
      set_local $0
      i32.const 0
      set_local $3
      i32.const 0
      set_local $4
      i32.const 0
      set_local $5
    end ;; $block
    i32.const 0
    get_local $4
    i32.store offset=3632
    i32.const 0
    get_local $5
    i32.store offset=3628
    i32.const 0
    get_local $3
    i32.store offset=3600
    i32.const 0
    get_local $1
    get_local $0
    get_local $1
    select
    tee_local $0
    i32.const 1047
    get_local $0
    select
    tee_local $0
    i32.store offset=3560
    i32.const 0
    get_local $0
    i32.store offset=3556
    block $block_2
      loop $loop_1
        block $block_3
          block $block_4
            get_local $0
            i32.load8_u
            tee_local $3
            i32.const 47
            i32.ne
            br_if $block_4
            i32.const 0
            get_local $0
            i32.const 1
            i32.add
            i32.store offset=3556
            br $block_3
          end ;; $block_4
          get_local $3
          i32.eqz
          br_if $block_2
        end ;; $block_3
        get_local $0
        i32.const 1
        i32.add
        set_local $0
        br $loop_1
      end ;; $loop_1
    end ;; $block_2
    block $block_5
      block $block_6
        block $block_7
          get_local $2
          i32.load offset=76
          get_local $2
          i32.load offset=80
          i32.ne
          br_if $block_7
          get_local $2
          i32.load offset=84
          get_local $2
          i32.load offset=88
          i32.ne
          br_if $block_7
          get_local $2
          i32.load offset=124
          i32.eqz
          br_if $block_6
        end ;; $block_7
        get_local $2
        i32.const 16
        i32.add
        i32.const 0
        i64.load offset=1072
        i64.store
        get_local $2
        i32.const 0
        i64.load offset=1064
        i64.store offset=8
        get_local $2
        i32.const 0
        i64.load offset=1056
        i64.store
        get_local $2
        i32.const 3
        i32.const 0
        call $__syscall_poll
        i32.const 0
        i32.lt_s
        br_if $block_5
        get_local $2
        i32.const 6
        i32.or
        set_local $0
        i32.const 0
        set_local $3
        loop $loop_2
          block $block_8
            get_local $0
            i32.load8_u
            i32.const 32
            i32.and
            i32.eqz
            br_if $block_8
            i32.const 1080
            i32.const 32770
            i32.const 0
            call $__syscall_open
            i32.const -1
            i32.le_s
            br_if $block_5
          end ;; $block_8
          get_local $0
          i32.const 8
          i32.add
          set_local $0
          get_local $3
          i32.const 1
          i32.add
          tee_local $3
          i32.const 3
          i32.lt_u
          br_if $loop_2
        end ;; $loop_2
        i32.const 0
        i32.const 1
        i32.store offset=3572
      end ;; $block_6
      get_local $2
      i32.const 192
      i32.add
      set_global $25
      return
    end ;; $block_5
    call $abort
    unreachable
    )
  
  (func $libc_start_init (type $0)
    (local $0 i32)
    i32.const 0
    set_local $0
    call $dummy
    block $block
      i32.const 0
      i32.const 0
      i32.ge_u
      br_if $block
      loop $loop
        get_local $0
        i32.load
        call_indirect $23 (type $0)
        get_local $0
        i32.const 4
        i32.add
        tee_local $0
        i32.const 0
        i32.lt_u
        br_if $loop
      end ;; $loop
    end ;; $block
    )
  
  (func $__libc_start_main (type $6)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (param $3 i32)
    (param $4 i32)
    (result i32)
    (local $5 i32)
    get_local $2
    get_local $1
    i32.const 2
    i32.shl
    i32.add
    i32.const 4
    i32.add
    tee_local $5
    get_local $2
    i32.load
    call $__init_libc
    call $libc_start_init
    get_local $1
    get_local $2
    get_local $5
    get_local $0
    call_indirect $23 (type $1)
    call $exit
    unreachable
    )
  
  (func $libc_exit_fini (type $0)
    (local $0 i32)
    i32.const 0
    set_local $0
    block $block
      i32.const 0
      i32.const 0
      i32.le_u
      br_if $block
      loop $loop
        get_local $0
        i32.const -4
        i32.add
        tee_local $0
        i32.load
        call_indirect $23 (type $0)
        get_local $0
        i32.const 0
        i32.gt_u
        br_if $loop
      end ;; $loop
    end ;; $block
    call $_fini
    )
  
  (func $exit (type $3)
    (param $0 i32)
    call $_fini
    call $libc_exit_fini
    call $__stdio_exit
    get_local $0
    call $_Exit
    unreachable
    )
  
  (func $_fini (type $0)
    )
  
  (func $memset (type $1)
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
      i32.store offset=16
      get_local $3
      get_local $1
      i32.store offset=12
      get_local $3
      get_local $1
      i32.store offset=20
      get_local $3
      get_local $1
      i32.store offset=24
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
      get_local $2
      i32.const -20
      i32.add
      get_local $1
      i32.store
      get_local $2
      i32.const -16
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
        i32.const 8
        i32.add
        get_local $6
        i64.store
        get_local $1
        i32.const 16
        i32.add
        get_local $6
        i64.store
        get_local $1
        i32.const 24
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
  
  (func $printf (type $8)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    (local $2 i32)
    get_global $25
    i32.const 16
    i32.sub
    tee_local $2
    set_global $25
    get_local $2
    get_local $1
    i32.store offset=12
    i32.const 0
    i32.load offset=1092
    get_local $0
    get_local $1
    call $vfprintf
    set_local $1
    get_local $2
    i32.const 16
    i32.add
    set_global $25
    get_local $1
    )
  
  (func $dummy_0 (type $4)
    (param $0 i32)
    (result i32)
    get_local $0
    )
  
  (func $__stdio_close (type $4)
    (param $0 i32)
    (result i32)
    get_local $0
    i32.load offset=60
    call $dummy_0
    call $__syscall_close
    call $__syscall_ret
    )
  
  (func $__stdio_write (type $1)
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
    get_global $25
    i32.const 16
    i32.sub
    tee_local $3
    set_global $25
    get_local $3
    get_local $1
    i32.store offset=8
    get_local $3
    get_local $2
    i32.store offset=12
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
    set_global $25
    get_local $6
    )
  
  (func $__stdout_write (type $1)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    get_global $25
    i32.const 16
    i32.sub
    tee_local $3
    set_global $25
    get_local $0
    i32.const 4
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
    set_global $25
    get_local $0
    )
  
  (func $__stdio_seek (type $2)
    (param $0 i32)
    (param $1 i64)
    (param $2 i32)
    (result i64)
    (local $3 i32)
    get_global $25
    i32.const 16
    i32.sub
    tee_local $3
    set_global $25
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
    set_global $25
    get_local $1
    )
  
  (func $__errno_location (type $10)
    (result i32)
    i32.const 31
    )
  
  (func $__syscall_ret (type $4)
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
  
  (func $__lockfile (type $4)
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
  
  (func $__unlockfile (type $3)
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
  
  (func $__lock (type $3)
    (param $0 i32)
    (local $1 i32)
    (local $2 i32)
    (local $3 i32)
    block $block
      i32.const 0
      i32.load offset=3576
      i32.eqz
      br_if $block
      block $block_0
        block $block_1
          block $block_2
            get_local $0
            i32.load
            tee_local $1
            i32.eqz
            br_if $block_2
            i32.const 0
            set_local $2
            loop $loop
              get_local $1
              i32.const 2147483647
              i32.add
              get_local $1
              get_local $1
              i32.const 0
              i32.lt_s
              select
              tee_local $3
              get_local $0
              i32.load
              tee_local $1
              i32.eq
              br_if $block_1
              get_local $2
              i32.const 1
              i32.add
              tee_local $2
              i32.const 10
              i32.lt_u
              br_if $loop
            end ;; $loop
            loop $loop_0
              get_local $0
              i32.load
              tee_local $1
              get_local $0
              i32.load
              i32.ne
              br_if $loop_0
            end ;; $loop_0
            get_local $0
            get_local $1
            i32.const 1
            i32.add
            tee_local $1
            i32.store
            loop $loop_1
              block $block_3
                block $block_4
                  get_local $1
                  i32.const -1
                  i32.le_s
                  br_if $block_4
                  get_local $1
                  set_local $2
                  br $block_3
                end ;; $block_4
                block $block_5
                  get_local $0
                  i32.const 128
                  get_local $1
                  i32.const 0
                  i32.const 0
                  i32.const 0
                  call $__syscall_futex
                  i32.const -38
                  i32.ne
                  br_if $block_5
                  get_local $0
                  i32.const 0
                  get_local $1
                  i32.const 0
                  i32.const 0
                  i32.const 0
                  call $__syscall_futex
                  drop
                end ;; $block_5
                get_local $1
                i32.const 2147483647
                i32.add
                set_local $2
              end ;; $block_3
              get_local $0
              i32.load
              tee_local $1
              get_local $2
              i32.ne
              br_if $loop_1
            end ;; $loop_1
            get_local $2
            i32.const -2147483648
            i32.or
            set_local $1
            br $block_0
          end ;; $block_2
          i32.const -2147483647
          set_local $1
          br $block_0
        end ;; $block_1
        get_local $3
        i32.const -2147483647
        i32.add
        set_local $1
      end ;; $block_0
      get_local $0
      get_local $1
      i32.store
    end ;; $block
    )
  
  (func $__ofl_lock (type $10)
    (result i32)
    i32.const 4680
    call $__lock
    i32.const 4684
    )
  
  (func $__stdio_exit (type $0)
    (local $0 i32)
    block $block
      call $__ofl_lock
      i32.load
      tee_local $0
      i32.eqz
      br_if $block
      loop $loop
        get_local $0
        call $close_file
        get_local $0
        i32.load offset=56
        tee_local $0
        br_if $loop
      end ;; $loop
    end ;; $block
    i32.const 0
    i32.load offset=4688
    call $close_file
    i32.const 0
    i32.load offset=4840
    call $close_file
    )
  
  (func $close_file (type $3)
    (param $0 i32)
    (local $1 i32)
    (local $2 i32)
    block $block
      get_local $0
      i32.eqz
      br_if $block
      block $block_0
        get_local $0
        i32.load offset=76
        i32.const 0
        i32.lt_s
        br_if $block_0
        get_local $0
        call $__lockfile
        drop
      end ;; $block_0
      block $block_1
        get_local $0
        i32.load offset=20
        get_local $0
        i32.load offset=28
        i32.le_u
        br_if $block_1
        get_local $0
        i32.const 0
        i32.const 0
        get_local $0
        i32.load offset=36
        call_indirect $23 (type $1)
        drop
      end ;; $block_1
      get_local $0
      i32.load offset=4
      tee_local $1
      get_local $0
      i32.load offset=8
      tee_local $2
      i32.ge_u
      br_if $block
      get_local $0
      get_local $1
      get_local $2
      i32.sub
      i64.extend_s/i32
      i32.const 1
      get_local $0
      i32.load offset=40
      call_indirect $23 (type $2)
      drop
    end ;; $block
    )
  
  (func $__towrite (type $4)
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
  
  (func $memcpy (type $1)
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
              i32.eqz
              br_if $block_5
              get_local $4
              i32.const 32
              i32.lt_u
              br_if $block_2
              get_local $3
              i32.const 3
              i32.and
              tee_local $2
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
              get_local $1
              i32.const 16
              i32.add
              set_local $2
              get_local $4
              i32.const -19
              i32.add
              set_local $6
              get_local $4
              i32.const -3
              i32.add
              set_local $7
              get_local $3
              i32.const 3
              i32.add
              set_local $8
              get_local $1
              get_local $4
              i32.const -20
              i32.add
              i32.const -16
              i32.and
              tee_local $9
              i32.const 19
              i32.add
              tee_local $10
              i32.add
              set_local $1
              loop $loop_0
                get_local $8
                get_local $2
                i32.const -12
                i32.add
                i32.load
                tee_local $4
                i32.const 8
                i32.shl
                get_local $5
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $8
                i32.const 4
                i32.add
                get_local $2
                i32.const -8
                i32.add
                i32.load
                tee_local $5
                i32.const 8
                i32.shl
                get_local $4
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $8
                i32.const 8
                i32.add
                get_local $2
                i32.const -4
                i32.add
                i32.load
                tee_local $4
                i32.const 8
                i32.shl
                get_local $5
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $8
                i32.const 12
                i32.add
                get_local $2
                i32.load
                tee_local $5
                i32.const 8
                i32.shl
                get_local $4
                i32.const 24
                i32.shr_u
                i32.or
                i32.store
                get_local $8
                i32.const 16
                i32.add
                set_local $8
                get_local $2
                i32.const 16
                i32.add
                set_local $2
                get_local $7
                i32.const -16
                i32.add
                tee_local $7
                i32.const 16
                i32.gt_u
                br_if $loop_0
              end ;; $loop_0
              get_local $6
              get_local $9
              i32.sub
              set_local $4
              get_local $3
              get_local $10
              i32.add
              set_local $3
              br $block_2
            end ;; $block_5
            block $block_6
              block $block_7
                get_local $4
                i32.const 16
                i32.lt_u
                br_if $block_7
                get_local $3
                get_local $4
                i32.const -16
                i32.add
                tee_local $5
                i32.const -16
                i32.and
                tee_local $7
                i32.const 16
                i32.add
                tee_local $6
                i32.add
                set_local $8
                get_local $1
                set_local $2
                loop $loop_1
                  get_local $3
                  get_local $2
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 4
                  i32.add
                  get_local $2
                  i32.const 4
                  i32.add
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 8
                  i32.add
                  get_local $2
                  i32.const 8
                  i32.add
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 12
                  i32.add
                  get_local $2
                  i32.const 12
                  i32.add
                  i32.load
                  i32.store
                  get_local $3
                  i32.const 16
                  i32.add
                  set_local $3
                  get_local $2
                  i32.const 16
                  i32.add
                  set_local $2
                  get_local $4
                  i32.const -16
                  i32.add
                  tee_local $4
                  i32.const 15
                  i32.gt_u
                  br_if $loop_1
                end ;; $loop_1
                get_local $5
                get_local $7
                i32.sub
                set_local $4
                get_local $1
                get_local $6
                i32.add
                set_local $1
                br $block_6
              end ;; $block_7
              get_local $3
              set_local $8
            end ;; $block_6
            block $block_8
              get_local $4
              i32.const 8
              i32.and
              i32.eqz
              br_if $block_8
              get_local $8
              get_local $1
              i32.load
              i32.store
              get_local $8
              get_local $1
              i32.load offset=4
              i32.store offset=4
              get_local $1
              i32.const 8
              i32.add
              set_local $1
              get_local $8
              i32.const 8
              i32.add
              set_local $8
            end ;; $block_8
            block $block_9
              get_local $4
              i32.const 4
              i32.and
              i32.eqz
              br_if $block_9
              get_local $8
              get_local $1
              i32.load
              i32.store
              get_local $1
              i32.const 4
              i32.add
              set_local $1
              get_local $8
              i32.const 4
              i32.add
              set_local $8
            end ;; $block_9
            block $block_10
              get_local $4
              i32.const 2
              i32.and
              i32.eqz
              br_if $block_10
              get_local $8
              get_local $1
              i32.load8_u
              i32.store8
              get_local $8
              get_local $1
              i32.load8_u offset=1
              i32.store8 offset=1
              get_local $8
              i32.const 2
              i32.add
              set_local $8
              get_local $1
              i32.const 2
              i32.add
              set_local $1
            end ;; $block_10
            get_local $4
            i32.const 1
            i32.and
            i32.eqz
            br_if $block_1
            get_local $8
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
          get_local $1
          i32.const 16
          i32.add
          set_local $2
          get_local $4
          i32.const -17
          i32.add
          set_local $6
          get_local $4
          i32.const -1
          i32.add
          set_local $7
          get_local $3
          i32.const 1
          i32.add
          set_local $8
          get_local $1
          get_local $4
          i32.const -20
          i32.add
          i32.const -16
          i32.and
          tee_local $9
          i32.const 17
          i32.add
          tee_local $10
          i32.add
          set_local $1
          loop $loop_2
            get_local $8
            get_local $2
            i32.const -12
            i32.add
            i32.load
            tee_local $4
            i32.const 24
            i32.shl
            get_local $5
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $8
            i32.const 4
            i32.add
            get_local $2
            i32.const -8
            i32.add
            i32.load
            tee_local $5
            i32.const 24
            i32.shl
            get_local $4
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $8
            i32.const 8
            i32.add
            get_local $2
            i32.const -4
            i32.add
            i32.load
            tee_local $4
            i32.const 24
            i32.shl
            get_local $5
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $8
            i32.const 12
            i32.add
            get_local $2
            i32.load
            tee_local $5
            i32.const 24
            i32.shl
            get_local $4
            i32.const 8
            i32.shr_u
            i32.or
            i32.store
            get_local $8
            i32.const 16
            i32.add
            set_local $8
            get_local $2
            i32.const 16
            i32.add
            set_local $2
            get_local $7
            i32.const -16
            i32.add
            tee_local $7
            i32.const 18
            i32.gt_u
            br_if $loop_2
          end ;; $loop_2
          get_local $6
          get_local $9
          i32.sub
          set_local $4
          get_local $3
          get_local $10
          i32.add
          set_local $3
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
        get_local $1
        i32.const 16
        i32.add
        set_local $2
        get_local $4
        i32.const -18
        i32.add
        set_local $6
        get_local $4
        i32.const -2
        i32.add
        set_local $7
        get_local $3
        i32.const 2
        i32.add
        set_local $8
        get_local $1
        get_local $4
        i32.const -20
        i32.add
        i32.const -16
        i32.and
        tee_local $9
        i32.const 18
        i32.add
        tee_local $10
        i32.add
        set_local $1
        loop $loop_3
          get_local $8
          get_local $2
          i32.const -12
          i32.add
          i32.load
          tee_local $4
          i32.const 16
          i32.shl
          get_local $5
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $8
          i32.const 4
          i32.add
          get_local $2
          i32.const -8
          i32.add
          i32.load
          tee_local $5
          i32.const 16
          i32.shl
          get_local $4
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $8
          i32.const 8
          i32.add
          get_local $2
          i32.const -4
          i32.add
          i32.load
          tee_local $4
          i32.const 16
          i32.shl
          get_local $5
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $8
          i32.const 12
          i32.add
          get_local $2
          i32.load
          tee_local $5
          i32.const 16
          i32.shl
          get_local $4
          i32.const 16
          i32.shr_u
          i32.or
          i32.store
          get_local $8
          i32.const 16
          i32.add
          set_local $8
          get_local $2
          i32.const 16
          i32.add
          set_local $2
          get_local $7
          i32.const -16
          i32.add
          tee_local $7
          i32.const 17
          i32.gt_u
          br_if $loop_3
        end ;; $loop_3
        get_local $6
        get_local $9
        i32.sub
        set_local $4
        get_local $3
        get_local $10
        i32.add
        set_local $3
      end ;; $block_2
      block $block_11
        get_local $4
        i32.const 16
        i32.and
        i32.eqz
        br_if $block_11
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
        i32.load8_u
        i32.store8
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
  
  (func $__fwritex (type $1)
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
        call_indirect $23 (type $1)
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
        call_indirect $23 (type $1)
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
  
  (func $__signbitl (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    get_local $1
    i32.wrap/i64
    i32.const 15
    i32.shr_u
    i32.const 1
    i32.and
    )
  
  (func $__fpclassifyl (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    get_local $1
    i32.wrap/i64
    i32.const 32767
    i32.and
    set_local $2
    block $block
      get_local $0
      i64.const 0
      i64.lt_s
      br_if $block
      get_local $2
      br_if $block
      i32.const 2
      i32.const 3
      get_local $0
      i64.eqz
      select
      return
    end ;; $block
    block $block_0
      block $block_1
        get_local $2
        i32.const 32767
        i32.ne
        br_if $block_1
        get_local $0
        i64.const -1
        i64.le_s
        br_if $block_0
        i32.const 0
        return
      end ;; $block_1
      get_local $0
      i64.const 61
      i64.shr_u
      i32.wrap/i64
      i32.const 4
      i32.and
      return
    end ;; $block_0
    get_local $0
    i64.const 9223372036854775807
    i64.and
    i64.eqz
    )
  
  (func $frexpl (type $12)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    get_global $25
    i32.const 32
    i32.sub
    tee_local $4
    set_global $25
    block $block
      get_local $2
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
            set_local $5
            get_local $4
            i64.load offset=24
            set_local $2
            get_local $4
            i64.load offset=16
            set_local $1
            br $block_0
          end ;; $block_2
          get_local $3
          get_local $5
          i32.const 32767
          i32.and
          i32.const -16382
          i32.add
          i32.store
          get_local $5
          i32.const 16382
          i32.or
          i64.extend_u/i32
          i64.const 49150
          i64.and
          get_local $2
          i64.const -65536
          i64.and
          i64.or
          set_local $2
          br $block
        end ;; $block_1
        i32.const 0
        set_local $5
      end ;; $block_0
      get_local $3
      get_local $5
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
    set_global $25
    )
  
  (func $dummy_1 (type $8)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    get_local $0
    )
  
  (func $__lctrans (type $8)
    (param $0 i32)
    (param $1 i32)
    (result i32)
    get_local $0
    get_local $1
    call $dummy_1
    )
  
  (func $__strerror_l (type $8)
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
              i32.const 1104
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
        i32.const 1200
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
      i32.const 1200
      set_local $4
    end ;; $block
    get_local $4
    get_local $1
    i32.load offset=20
    call $__lctrans
    )
  
  (func $strerror (type $4)
    (param $0 i32)
    (result i32)
    get_local $0
    i32.const 0
    i32.load offset=103
    call $__strerror_l
    )
  
  (func $memchr (type $1)
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
  
  (func $strnlen (type $8)
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
  
  (func $vfprintf (type $1)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (result i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    get_global $25
    i32.const 288
    i32.sub
    tee_local $3
    set_global $25
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
        call_indirect $23 (type $1)
        drop
        get_local $0
        i32.const 44
        i32.add
        get_local $7
        i32.store
        get_local $6
        i32.const 0
        i32.store
        get_local $0
        i32.const 16
        i32.add
        i32.const 0
        i32.store
        get_local $0
        i32.const 28
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
    set_global $25
    get_local $2
    )
  
  (func $printf_core (type $6)
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
    (local $28 i64)
    (local $29 i64)
    (local $30 i32)
    (local $31 i32)
    (local $32 i32)
    (local $33 i64)
    (local $34 i64)
    (local $35 i32)
    (local $36 i32)
    (local $37 i32)
    get_global $25
    i32.const 7712
    i32.sub
    tee_local $5
    set_global $25
    i32.const 0
    get_local $5
    i32.const 336
    i32.add
    i32.sub
    set_local $6
    get_local $5
    i32.const 336
    i32.add
    i32.const 8
    i32.or
    set_local $7
    get_local $5
    i32.const 336
    i32.add
    i32.const 9
    i32.or
    set_local $8
    get_local $5
    i32.const 324
    i32.add
    i32.const 11
    i32.add
    set_local $9
    get_local $5
    i32.const 7448
    i32.add
    set_local $10
    get_local $5
    i32.const 336
    i32.add
    set_local $11
    get_local $5
    i32.const 256
    i32.add
    i32.const 42
    i32.add
    set_local $12
    get_local $5
    i32.const 256
    i32.add
    i32.const 43
    i32.add
    set_local $13
    i32.const 0
    set_local $14
    i32.const 0
    set_local $15
    i32.const 0
    set_local $16
    block $block
      block $block_0
        block $block_1
          block $block_2
            block $block_3
              block $block_4
                block $block_5
                  loop $loop
                    get_local $1
                    set_local $17
                    get_local $16
                    i32.const 2147483647
                    get_local $15
                    i32.sub
                    i32.gt_s
                    br_if $block_5
                    get_local $16
                    get_local $15
                    i32.add
                    set_local $15
                    get_local $17
                    i32.load8_u
                    tee_local $16
                    i32.eqz
                    br_if $block_4
                    get_local $17
                    set_local $1
                    block $block_6
                      block $block_7
                        block $block_8
                          loop $loop_0
                            get_local $16
                            i32.const 255
                            i32.and
                            tee_local $16
                            i32.eqz
                            br_if $block_8
                            block $block_9
                              get_local $16
                              i32.const 37
                              i32.eq
                              br_if $block_9
                              get_local $1
                              i32.load8_u offset=1
                              set_local $16
                              get_local $1
                              i32.const 1
                              i32.add
                              set_local $1
                              br $loop_0
                            end ;; $block_9
                          end ;; $loop_0
                          get_local $1
                          set_local $18
                          get_local $1
                          set_local $16
                          loop $loop_1
                            get_local $16
                            i32.const 1
                            i32.add
                            i32.load8_u
                            i32.const 37
                            i32.ne
                            br_if $block_7
                            get_local $18
                            i32.const 1
                            i32.add
                            set_local $18
                            get_local $16
                            i32.load8_u offset=2
                            set_local $19
                            get_local $16
                            i32.const 2
                            i32.add
                            tee_local $1
                            set_local $16
                            get_local $19
                            i32.const 37
                            i32.eq
                            br_if $loop_1
                            br $block_6
                          end ;; $loop_1
                        end ;; $block_8
                        get_local $1
                        set_local $18
                        br $block_6
                      end ;; $block_7
                      get_local $16
                      set_local $1
                    end ;; $block_6
                    get_local $18
                    get_local $17
                    i32.sub
                    tee_local $16
                    i32.const 2147483647
                    get_local $15
                    i32.sub
                    tee_local $18
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
                      get_local $17
                      get_local $16
                      get_local $0
                      call $__fwritex
                      drop
                    end ;; $block_10
                    get_local $16
                    br_if $loop
                    get_local $1
                    i32.const 1
                    i32.add
                    set_local $16
                    block $block_11
                      block $block_12
                        get_local $1
                        i32.load8_s offset=1
                        tee_local $20
                        i32.const -48
                        i32.add
                        tee_local $21
                        i32.const 9
                        i32.gt_u
                        br_if $block_12
                        get_local $1
                        i32.const 3
                        i32.add
                        get_local $16
                        get_local $1
                        i32.load8_u offset=2
                        i32.const 36
                        i32.eq
                        tee_local $19
                        select
                        set_local $16
                        get_local $21
                        i32.const -1
                        get_local $19
                        select
                        set_local $22
                        i32.const 1
                        get_local $14
                        get_local $19
                        select
                        set_local $14
                        get_local $1
                        i32.const 3
                        i32.const 1
                        get_local $19
                        select
                        i32.add
                        i32.load8_s
                        set_local $20
                        br $block_11
                      end ;; $block_12
                      i32.const -1
                      set_local $22
                    end ;; $block_11
                    i32.const 0
                    set_local $23
                    block $block_13
                      get_local $20
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
                      get_local $16
                      i32.const 1
                      i32.add
                      set_local $19
                      i32.const 0
                      set_local $23
                      loop $loop_2
                        get_local $1
                        get_local $23
                        i32.or
                        set_local $23
                        get_local $19
                        tee_local $16
                        i32.load8_s
                        tee_local $20
                        i32.const -32
                        i32.add
                        tee_local $1
                        i32.const 32
                        i32.ge_u
                        br_if $block_13
                        get_local $16
                        i32.const 1
                        i32.add
                        set_local $19
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
                                get_local $20
                                i32.const 42
                                i32.ne
                                br_if $block_19
                                get_local $16
                                i32.load8_s offset=1
                                i32.const -48
                                i32.add
                                tee_local $1
                                i32.const 9
                                i32.gt_u
                                br_if $block_18
                                get_local $16
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
                                get_local $16
                                i32.const 3
                                i32.add
                                set_local $24
                                i32.const 1
                                set_local $14
                                get_local $3
                                get_local $16
                                i32.const 1
                                i32.add
                                i32.load8_s
                                i32.const 4
                                i32.shl
                                i32.add
                                i32.const -768
                                i32.add
                                i32.load
                                set_local $21
                                br $block_17
                              end ;; $block_19
                              i32.const 0
                              set_local $21
                              get_local $20
                              i32.const -48
                              i32.add
                              tee_local $19
                              i32.const 9
                              i32.gt_u
                              br_if $block_16
                              i32.const 0
                              set_local $1
                              loop $loop_3
                                i32.const -1
                                set_local $21
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
                                  get_local $19
                                  i32.add
                                  get_local $19
                                  i32.const 2147483647
                                  get_local $1
                                  i32.sub
                                  i32.gt_s
                                  select
                                  set_local $21
                                end ;; $block_20
                                get_local $16
                                i32.load8_s offset=1
                                set_local $19
                                get_local $16
                                i32.const 1
                                i32.add
                                tee_local $24
                                set_local $16
                                get_local $21
                                set_local $1
                                get_local $19
                                i32.const -48
                                i32.add
                                tee_local $19
                                i32.const 10
                                i32.lt_u
                                br_if $loop_3
                              end ;; $loop_3
                              get_local $21
                              i32.const 0
                              i32.ge_s
                              br_if $block_14
                              br $block_5
                            end ;; $block_18
                            get_local $14
                            br_if $block_1
                            get_local $16
                            i32.const 1
                            i32.add
                            set_local $24
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
                            set_local $21
                            i32.const 0
                            set_local $14
                          end ;; $block_17
                          get_local $21
                          i32.const -1
                          i32.gt_s
                          br_if $block_14
                          i32.const 0
                          get_local $21
                          i32.sub
                          set_local $21
                          get_local $23
                          i32.const 8192
                          i32.or
                          set_local $23
                          br $block_14
                        end ;; $block_16
                        get_local $16
                        set_local $24
                        br $block_14
                      end ;; $block_15
                      i32.const 0
                      set_local $14
                      i32.const 0
                      set_local $21
                    end ;; $block_14
                    i32.const 0
                    set_local $16
                    i32.const -1
                    set_local $20
                    block $block_21
                      block $block_22
                        block $block_23
                          block $block_24
                            block $block_25
                              block $block_26
                                block $block_27
                                  get_local $24
                                  i32.load8_u
                                  i32.const 46
                                  i32.ne
                                  br_if $block_27
                                  get_local $24
                                  i32.load8_s offset=1
                                  tee_local $19
                                  i32.const 42
                                  i32.ne
                                  br_if $block_26
                                  get_local $24
                                  i32.load8_s offset=2
                                  i32.const -48
                                  i32.add
                                  tee_local $1
                                  i32.const 9
                                  i32.gt_u
                                  br_if $block_25
                                  get_local $24
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
                                  get_local $24
                                  i32.const 4
                                  i32.add
                                  set_local $1
                                  get_local $3
                                  get_local $24
                                  i32.const 2
                                  i32.add
                                  i32.load8_s
                                  i32.const 4
                                  i32.shl
                                  i32.add
                                  i32.const -768
                                  i32.add
                                  i32.load
                                  set_local $20
                                  br $block_22
                                end ;; $block_27
                                get_local $24
                                set_local $1
                                i32.const 0
                                set_local $25
                                br $block_21
                              end ;; $block_26
                              get_local $24
                              i32.const 1
                              i32.add
                              set_local $1
                              get_local $19
                              i32.const -48
                              i32.add
                              tee_local $26
                              i32.const 9
                              i32.gt_u
                              br_if $block_24
                              i32.const 0
                              set_local $24
                              get_local $1
                              set_local $19
                              loop $loop_4
                                i32.const -1
                                set_local $20
                                block $block_28
                                  get_local $24
                                  i32.const 214748364
                                  i32.gt_u
                                  br_if $block_28
                                  i32.const -1
                                  get_local $24
                                  i32.const 10
                                  i32.mul
                                  tee_local $1
                                  get_local $26
                                  i32.add
                                  get_local $26
                                  i32.const 2147483647
                                  get_local $1
                                  i32.sub
                                  i32.gt_s
                                  select
                                  set_local $20
                                end ;; $block_28
                                i32.const 1
                                set_local $25
                                get_local $19
                                i32.load8_s offset=1
                                set_local $26
                                get_local $19
                                i32.const 1
                                i32.add
                                tee_local $1
                                set_local $19
                                get_local $20
                                set_local $24
                                get_local $26
                                i32.const -48
                                i32.add
                                tee_local $26
                                i32.const 10
                                i32.lt_u
                                br_if $loop_4
                                br $block_21
                              end ;; $loop_4
                            end ;; $block_25
                            get_local $14
                            br_if $block_1
                            get_local $24
                            i32.const 2
                            i32.add
                            set_local $1
                            get_local $0
                            i32.eqz
                            br_if $block_23
                            get_local $2
                            get_local $2
                            i32.load
                            tee_local $19
                            i32.const 4
                            i32.add
                            i32.store
                            get_local $19
                            i32.load
                            set_local $20
                            br $block_22
                          end ;; $block_24
                          i32.const 1
                          set_local $25
                          i32.const 0
                          set_local $20
                          br $block_21
                        end ;; $block_23
                        i32.const 0
                        set_local $20
                      end ;; $block_22
                      get_local $20
                      i32.const 31
                      i32.shr_u
                      i32.const 1
                      i32.xor
                      set_local $25
                    end ;; $block_21
                    loop $loop_5
                      get_local $16
                      set_local $19
                      get_local $1
                      i32.load8_s
                      i32.const -65
                      i32.add
                      tee_local $16
                      i32.const 57
                      i32.gt_u
                      br_if $block_1
                      get_local $1
                      i32.const 1
                      i32.add
                      set_local $1
                      get_local $19
                      i32.const 58
                      i32.mul
                      get_local $16
                      i32.add
                      i32.const 3024
                      i32.add
                      i32.load8_u
                      tee_local $16
                      i32.const -1
                      i32.add
                      i32.const 8
                      i32.lt_u
                      br_if $loop_5
                    end ;; $loop_5
                    get_local $16
                    i32.eqz
                    br_if $block_1
                    block $block_29
                      block $block_30
                        block $block_31
                          block $block_32
                            get_local $16
                            i32.const 27
                            i32.ne
                            br_if $block_32
                            get_local $22
                            i32.const -1
                            i32.le_s
                            br_if $block_31
                            br $block_1
                          end ;; $block_32
                          get_local $22
                          i32.const 0
                          i32.lt_s
                          br_if $block_30
                          get_local $4
                          get_local $22
                          i32.const 2
                          i32.shl
                          i32.add
                          get_local $16
                          i32.store
                          get_local $5
                          get_local $3
                          get_local $22
                          i32.const 4
                          i32.shl
                          i32.add
                          tee_local $16
                          i32.const 8
                          i32.add
                          i64.load
                          i64.store offset=312
                          get_local $5
                          get_local $16
                          i64.load
                          i64.store offset=304
                        end ;; $block_31
                        i32.const 0
                        set_local $16
                        get_local $0
                        i32.eqz
                        br_if $loop
                        br $block_29
                      end ;; $block_30
                      get_local $0
                      i32.eqz
                      br_if $block_3
                      get_local $5
                      i32.const 304
                      i32.add
                      get_local $16
                      get_local $2
                      call $pop_arg
                    end ;; $block_29
                    get_local $23
                    i32.const -65537
                    i32.and
                    tee_local $24
                    get_local $23
                    get_local $23
                    i32.const 8192
                    i32.and
                    select
                    set_local $22
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
                                                                                                    tee_local $16
                                                                                                    i32.const -33
                                                                                                    i32.and
                                                                                                    get_local $16
                                                                                                    get_local $16
                                                                                                    i32.const 15
                                                                                                    i32.and
                                                                                                    i32.const 3
                                                                                                    i32.eq
                                                                                                    select
                                                                                                    get_local $16
                                                                                                    get_local $19
                                                                                                    select
                                                                                                    tee_local $27
                                                                                                    i32.const 82
                                                                                                    i32.le_s
                                                                                                    br_if $block_72
                                                                                                    get_local $27
                                                                                                    i32.const -83
                                                                                                    i32.add
                                                                                                    tee_local $16
                                                                                                    i32.const 37
                                                                                                    i32.gt_u
                                                                                                    br_if $block_59
                                                                                                    block $block_73
                                                                                                      get_local $16
                                                                                                      br_table
                                                                                                        $block_70 $block_59 $block_59 $block_59 $block_59 $block_63 $block_59 $block_59 $block_59 $block_59 $block_59 $block_59 $block_59 $block_59 $block_71 $block_59
                                                                                                        $block_68 $block_73 $block_71 $block_71 $block_71 $block_59 $block_73 $block_59 $block_59 $block_59 $block_67 $block_66 $block_65 $block_64 $block_59 $block_59
                                                                                                        $block_62 $block_59 $block_60 $block_59 $block_59 $block_63
                                                                                                        $block_70 ;; default
                                                                                                    end ;; $block_73
                                                                                                    get_local $5
                                                                                                    i64.load offset=304
                                                                                                    tee_local $28
                                                                                                    i64.const -1
                                                                                                    i64.le_s
                                                                                                    br_if $block_58
                                                                                                    get_local $22
                                                                                                    i32.const 2048
                                                                                                    i32.and
                                                                                                    br_if $block_54
                                                                                                    i32.const 3006
                                                                                                    i32.const 3004
                                                                                                    get_local $22
                                                                                                    i32.const 1
                                                                                                    i32.and
                                                                                                    tee_local $23
                                                                                                    select
                                                                                                    set_local $26
                                                                                                    br $block_53
                                                                                                  end ;; $block_72
                                                                                                  get_local $27
                                                                                                  i32.const -69
                                                                                                  i32.add
                                                                                                  i32.const 3
                                                                                                  i32.lt_u
                                                                                                  br_if $block_71
                                                                                                  get_local $27
                                                                                                  i32.const 65
                                                                                                  i32.eq
                                                                                                  br_if $block_71
                                                                                                  get_local $27
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
                                                                                                  i64.load offset=304
                                                                                                  i64.store32 offset=248
                                                                                                  get_local $5
                                                                                                  get_local $5
                                                                                                  i32.const 248
                                                                                                  i32.add
                                                                                                  i32.store offset=304
                                                                                                  i32.const -1
                                                                                                  set_local $20
                                                                                                  get_local $5
                                                                                                  i32.const 248
                                                                                                  i32.add
                                                                                                  set_local $17
                                                                                                  br $block_69
                                                                                                end ;; $block_71
                                                                                                block $block_74
                                                                                                  get_local $20
                                                                                                  i32.const -1
                                                                                                  i32.gt_s
                                                                                                  br_if $block_74
                                                                                                  get_local $25
                                                                                                  br_if $block_5
                                                                                                end ;; $block_74
                                                                                                get_local $5
                                                                                                i64.load offset=312
                                                                                                set_local $28
                                                                                                get_local $5
                                                                                                i64.load offset=304
                                                                                                set_local $29
                                                                                                get_local $5
                                                                                                i32.const 0
                                                                                                i32.store offset=364
                                                                                                block $block_75
                                                                                                  block $block_76
                                                                                                    get_local $29
                                                                                                    get_local $28
                                                                                                    call $__signbitl
                                                                                                    i32.eqz
                                                                                                    br_if $block_76
                                                                                                    get_local $28
                                                                                                    i64.const -9223372036854775808
                                                                                                    i64.xor
                                                                                                    set_local $28
                                                                                                    i32.const 1
                                                                                                    set_local $30
                                                                                                    i32.const 3504
                                                                                                    set_local $31
                                                                                                    br $block_75
                                                                                                  end ;; $block_76
                                                                                                  block $block_77
                                                                                                    get_local $22
                                                                                                    i32.const 2048
                                                                                                    i32.and
                                                                                                    br_if $block_77
                                                                                                    i32.const 3510
                                                                                                    i32.const 3505
                                                                                                    get_local $22
                                                                                                    i32.const 1
                                                                                                    i32.and
                                                                                                    tee_local $30
                                                                                                    select
                                                                                                    set_local $31
                                                                                                    br $block_75
                                                                                                  end ;; $block_77
                                                                                                  i32.const 1
                                                                                                  set_local $30
                                                                                                  i32.const 3507
                                                                                                  set_local $31
                                                                                                end ;; $block_75
                                                                                                block $block_78
                                                                                                  block $block_79
                                                                                                    get_local $29
                                                                                                    get_local $28
                                                                                                    call $__fpclassifyl
                                                                                                    i32.const 1
                                                                                                    i32.le_s
                                                                                                    br_if $block_79
                                                                                                    get_local $5
                                                                                                    i32.const 224
                                                                                                    i32.add
                                                                                                    get_local $29
                                                                                                    get_local $28
                                                                                                    get_local $5
                                                                                                    i32.const 364
                                                                                                    i32.add
                                                                                                    call $frexpl
                                                                                                    get_local $5
                                                                                                    i32.const 208
                                                                                                    i32.add
                                                                                                    get_local $5
                                                                                                    i64.load offset=224
                                                                                                    tee_local $28
                                                                                                    get_local $5
                                                                                                    i64.load offset=232
                                                                                                    tee_local $29
                                                                                                    get_local $28
                                                                                                    get_local $29
                                                                                                    call $__addtf3
                                                                                                    block $block_80
                                                                                                      get_local $5
                                                                                                      i64.load offset=208
                                                                                                      tee_local $28
                                                                                                      get_local $5
                                                                                                      i32.const 208
                                                                                                      i32.add
                                                                                                      i32.const 8
                                                                                                      i32.add
                                                                                                      i64.load
                                                                                                      tee_local $29
                                                                                                      i64.const 0
                                                                                                      i64.const 0
                                                                                                      call $__eqtf2
                                                                                                      i32.eqz
                                                                                                      br_if $block_80
                                                                                                      get_local $5
                                                                                                      get_local $5
                                                                                                      i32.load offset=364
                                                                                                      i32.const -1
                                                                                                      i32.add
                                                                                                      i32.store offset=364
                                                                                                    end ;; $block_80
                                                                                                    get_local $27
                                                                                                    i32.const 32
                                                                                                    i32.or
                                                                                                    tee_local $32
                                                                                                    i32.const 97
                                                                                                    i32.ne
                                                                                                    br_if $block_78
                                                                                                    get_local $31
                                                                                                    i32.const 9
                                                                                                    i32.add
                                                                                                    get_local $31
                                                                                                    get_local $27
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    tee_local $17
                                                                                                    select
                                                                                                    set_local $25
                                                                                                    get_local $20
                                                                                                    i32.const 14
                                                                                                    i32.gt_u
                                                                                                    br_if $block_44
                                                                                                    i32.const 15
                                                                                                    get_local $20
                                                                                                    i32.sub
                                                                                                    i32.eqz
                                                                                                    br_if $block_44
                                                                                                    get_local $20
                                                                                                    i32.const -15
                                                                                                    i32.add
                                                                                                    set_local $16
                                                                                                    i64.const 4612248968380809216
                                                                                                    set_local $33
                                                                                                    i64.const 0
                                                                                                    set_local $34
                                                                                                    loop $loop_6
                                                                                                      get_local $5
                                                                                                      i32.const 112
                                                                                                      i32.add
                                                                                                      get_local $34
                                                                                                      get_local $33
                                                                                                      i64.const 0
                                                                                                      i64.const 4612530443357519872
                                                                                                      call $__multf3
                                                                                                      get_local $5
                                                                                                      i32.const 112
                                                                                                      i32.add
                                                                                                      i32.const 8
                                                                                                      i32.add
                                                                                                      i64.load
                                                                                                      set_local $33
                                                                                                      get_local $5
                                                                                                      i64.load offset=112
                                                                                                      set_local $34
                                                                                                      get_local $16
                                                                                                      i32.const 1
                                                                                                      i32.add
                                                                                                      tee_local $16
                                                                                                      br_if $loop_6
                                                                                                    end ;; $loop_6
                                                                                                    get_local $25
                                                                                                    i32.load8_u
                                                                                                    i32.const 45
                                                                                                    i32.ne
                                                                                                    br_if $block_45
                                                                                                    get_local $5
                                                                                                    i32.const 64
                                                                                                    i32.add
                                                                                                    get_local $28
                                                                                                    get_local $29
                                                                                                    i64.const -9223372036854775808
                                                                                                    i64.xor
                                                                                                    get_local $34
                                                                                                    get_local $33
                                                                                                    call $__subtf3
                                                                                                    get_local $5
                                                                                                    i32.const 48
                                                                                                    i32.add
                                                                                                    get_local $34
                                                                                                    get_local $33
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
                                                                                                    set_local $29
                                                                                                    get_local $5
                                                                                                    i64.load offset=48
                                                                                                    set_local $28
                                                                                                    br $block_44
                                                                                                  end ;; $block_79
                                                                                                  get_local $0
                                                                                                  i32.const 32
                                                                                                  get_local $21
                                                                                                  get_local $30
                                                                                                  i32.const 3
                                                                                                  i32.add
                                                                                                  tee_local $16
                                                                                                  get_local $24
                                                                                                  call $pad
                                                                                                  get_local $29
                                                                                                  get_local $28
                                                                                                  get_local $29
                                                                                                  get_local $28
                                                                                                  call $__unordtf2
                                                                                                  set_local $19
                                                                                                  block $block_81
                                                                                                    get_local $0
                                                                                                    i32.load
                                                                                                    tee_local $18
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    br_if $block_81
                                                                                                    get_local $31
                                                                                                    get_local $30
                                                                                                    get_local $0
                                                                                                    call $__fwritex
                                                                                                    drop
                                                                                                    get_local $0
                                                                                                    i32.load
                                                                                                    set_local $18
                                                                                                  end ;; $block_81
                                                                                                  block $block_82
                                                                                                    get_local $18
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    br_if $block_82
                                                                                                    i32.const 3531
                                                                                                    i32.const 3535
                                                                                                    get_local $27
                                                                                                    i32.const 32
                                                                                                    i32.and
                                                                                                    i32.const 5
                                                                                                    i32.shr_u
                                                                                                    tee_local $18
                                                                                                    select
                                                                                                    i32.const 3523
                                                                                                    i32.const 3527
                                                                                                    get_local $18
                                                                                                    select
                                                                                                    get_local $19
                                                                                                    select
                                                                                                    i32.const 3
                                                                                                    get_local $0
                                                                                                    call $__fwritex
                                                                                                    drop
                                                                                                  end ;; $block_82
                                                                                                  get_local $0
                                                                                                  i32.const 32
                                                                                                  get_local $21
                                                                                                  get_local $16
                                                                                                  get_local $22
                                                                                                  i32.const 8192
                                                                                                  i32.xor
                                                                                                  call $pad
                                                                                                  get_local $21
                                                                                                  get_local $16
                                                                                                  get_local $16
                                                                                                  get_local $21
                                                                                                  i32.lt_s
                                                                                                  select
                                                                                                  set_local $16
                                                                                                  br $block_39
                                                                                                end ;; $block_78
                                                                                                get_local $28
                                                                                                get_local $29
                                                                                                i64.const 0
                                                                                                i64.const 0
                                                                                                call $__eqtf2
                                                                                                set_local $16
                                                                                                get_local $20
                                                                                                i32.const 0
                                                                                                i32.lt_s
                                                                                                set_local $18
                                                                                                get_local $5
                                                                                                i32.load offset=364
                                                                                                set_local $17
                                                                                                block $block_83
                                                                                                  get_local $16
                                                                                                  i32.eqz
                                                                                                  br_if $block_83
                                                                                                  get_local $5
                                                                                                  i32.const 192
                                                                                                  i32.add
                                                                                                  get_local $28
                                                                                                  get_local $29
                                                                                                  i64.const 0
                                                                                                  i64.const 4619285842798575616
                                                                                                  call $__multf3
                                                                                                  get_local $5
                                                                                                  get_local $17
                                                                                                  i32.const -28
                                                                                                  i32.add
                                                                                                  tee_local $17
                                                                                                  i32.store offset=364
                                                                                                  get_local $5
                                                                                                  i32.const 192
                                                                                                  i32.add
                                                                                                  i32.const 8
                                                                                                  i32.add
                                                                                                  i64.load
                                                                                                  set_local $29
                                                                                                  get_local $5
                                                                                                  i64.load offset=192
                                                                                                  set_local $28
                                                                                                end ;; $block_83
                                                                                                i32.const 6
                                                                                                get_local $20
                                                                                                get_local $18
                                                                                                select
                                                                                                set_local $35
                                                                                                get_local $5
                                                                                                i32.const 368
                                                                                                i32.add
                                                                                                get_local $10
                                                                                                get_local $17
                                                                                                i32.const 0
                                                                                                i32.lt_s
                                                                                                select
                                                                                                tee_local $36
                                                                                                set_local $19
                                                                                                loop $loop_7
                                                                                                  get_local $5
                                                                                                  i32.const 176
                                                                                                  i32.add
                                                                                                  get_local $28
                                                                                                  get_local $29
                                                                                                  call $__fixunstfsi
                                                                                                  tee_local $16
                                                                                                  call $__floatunsitf
                                                                                                  get_local $5
                                                                                                  i32.const 160
                                                                                                  i32.add
                                                                                                  get_local $28
                                                                                                  get_local $29
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
                                                                                                  get_local $19
                                                                                                  get_local $16
                                                                                                  i32.store
                                                                                                  get_local $19
                                                                                                  i32.const 4
                                                                                                  i32.add
                                                                                                  set_local $19
                                                                                                  get_local $5
                                                                                                  i64.load offset=144
                                                                                                  tee_local $28
                                                                                                  get_local $5
                                                                                                  i32.const 144
                                                                                                  i32.add
                                                                                                  i32.const 8
                                                                                                  i32.add
                                                                                                  i64.load
                                                                                                  tee_local $29
                                                                                                  i64.const 0
                                                                                                  i64.const 0
                                                                                                  call $__netf2
                                                                                                  br_if $loop_7
                                                                                                end ;; $loop_7
                                                                                                get_local $17
                                                                                                i32.const 1
                                                                                                i32.lt_s
                                                                                                br_if $block_56
                                                                                                get_local $36
                                                                                                set_local $18
                                                                                                loop $loop_8
                                                                                                  get_local $17
                                                                                                  i32.const 29
                                                                                                  get_local $17
                                                                                                  i32.const 29
                                                                                                  i32.lt_s
                                                                                                  select
                                                                                                  set_local $17
                                                                                                  block $block_84
                                                                                                    get_local $19
                                                                                                    i32.const -4
                                                                                                    i32.add
                                                                                                    tee_local $16
                                                                                                    get_local $18
                                                                                                    i32.lt_u
                                                                                                    br_if $block_84
                                                                                                    get_local $17
                                                                                                    i64.extend_u/i32
                                                                                                    set_local $29
                                                                                                    i64.const 0
                                                                                                    set_local $28
                                                                                                    loop $loop_9
                                                                                                      get_local $16
                                                                                                      get_local $16
                                                                                                      i64.load32_u
                                                                                                      get_local $29
                                                                                                      i64.shl
                                                                                                      get_local $28
                                                                                                      i64.const 4294967295
                                                                                                      i64.and
                                                                                                      i64.add
                                                                                                      tee_local $28
                                                                                                      get_local $28
                                                                                                      i64.const 1000000000
                                                                                                      i64.div_u
                                                                                                      tee_local $28
                                                                                                      i64.const 1000000000
                                                                                                      i64.mul
                                                                                                      i64.sub
                                                                                                      i64.store32
                                                                                                      get_local $16
                                                                                                      i32.const -4
                                                                                                      i32.add
                                                                                                      tee_local $16
                                                                                                      get_local $18
                                                                                                      i32.ge_u
                                                                                                      br_if $loop_9
                                                                                                    end ;; $loop_9
                                                                                                    get_local $28
                                                                                                    i32.wrap/i64
                                                                                                    tee_local $16
                                                                                                    i32.eqz
                                                                                                    br_if $block_84
                                                                                                    get_local $18
                                                                                                    i32.const -4
                                                                                                    i32.add
                                                                                                    tee_local $18
                                                                                                    get_local $16
                                                                                                    i32.store
                                                                                                  end ;; $block_84
                                                                                                  block $block_85
                                                                                                    loop $loop_10
                                                                                                      get_local $19
                                                                                                      tee_local $16
                                                                                                      get_local $18
                                                                                                      i32.le_u
                                                                                                      br_if $block_85
                                                                                                      get_local $16
                                                                                                      i32.const -4
                                                                                                      i32.add
                                                                                                      tee_local $19
                                                                                                      i32.load
                                                                                                      i32.eqz
                                                                                                      br_if $loop_10
                                                                                                    end ;; $loop_10
                                                                                                  end ;; $block_85
                                                                                                  get_local $5
                                                                                                  get_local $5
                                                                                                  i32.load offset=364
                                                                                                  get_local $17
                                                                                                  i32.sub
                                                                                                  tee_local $17
                                                                                                  i32.store offset=364
                                                                                                  get_local $16
                                                                                                  set_local $19
                                                                                                  get_local $17
                                                                                                  i32.const 0
                                                                                                  i32.gt_s
                                                                                                  br_if $loop_8
                                                                                                  br $block_55
                                                                                                end ;; $loop_8
                                                                                              end ;; $block_70
                                                                                              get_local $20
                                                                                              i32.eqz
                                                                                              br_if $block_43
                                                                                              get_local $5
                                                                                              i32.load offset=304
                                                                                              set_local $17
                                                                                            end ;; $block_69
                                                                                            i32.const 0
                                                                                            set_local $16
                                                                                            get_local $17
                                                                                            set_local $18
                                                                                            loop $loop_11
                                                                                              get_local $18
                                                                                              i32.load
                                                                                              tee_local $19
                                                                                              i32.eqz
                                                                                              br_if $block_46
                                                                                              get_local $5
                                                                                              i32.const 244
                                                                                              i32.add
                                                                                              get_local $19
                                                                                              call $wctomb
                                                                                              tee_local $19
                                                                                              i32.const 0
                                                                                              i32.lt_s
                                                                                              tee_local $23
                                                                                              br_if $block_47
                                                                                              get_local $19
                                                                                              get_local $20
                                                                                              get_local $16
                                                                                              i32.sub
                                                                                              i32.gt_u
                                                                                              br_if $block_47
                                                                                              get_local $18
                                                                                              i32.const 4
                                                                                              i32.add
                                                                                              set_local $18
                                                                                              get_local $20
                                                                                              get_local $19
                                                                                              get_local $16
                                                                                              i32.add
                                                                                              tee_local $16
                                                                                              i32.gt_u
                                                                                              br_if $loop_11
                                                                                              br $block_46
                                                                                            end ;; $loop_11
                                                                                          end ;; $block_68
                                                                                          get_local $5
                                                                                          i32.const 256
                                                                                          i32.add
                                                                                          i32.const 42
                                                                                          i32.add
                                                                                          get_local $5
                                                                                          i64.load offset=304
                                                                                          i64.store8
                                                                                          i32.const 0
                                                                                          set_local $23
                                                                                          i32.const 3004
                                                                                          set_local $26
                                                                                          i32.const 1
                                                                                          set_local $20
                                                                                          get_local $12
                                                                                          set_local $17
                                                                                          get_local $13
                                                                                          set_local $16
                                                                                          get_local $24
                                                                                          set_local $22
                                                                                          br $block_48
                                                                                        end ;; $block_67
                                                                                        call $__errno_location
                                                                                        i32.load
                                                                                        call $strerror
                                                                                        set_local $17
                                                                                        br $block_61
                                                                                      end ;; $block_66
                                                                                      i32.const 0
                                                                                      set_local $16
                                                                                      get_local $19
                                                                                      i32.const 255
                                                                                      i32.and
                                                                                      tee_local $18
                                                                                      i32.const 7
                                                                                      i32.gt_u
                                                                                      br_if $loop
                                                                                      block $block_86
                                                                                        get_local $18
                                                                                        br_table
                                                                                          $block_86 $block_38 $block_37 $block_36 $block_35 $loop $block_34 $block_33
                                                                                          $block_86 ;; default
                                                                                      end ;; $block_86
                                                                                      get_local $5
                                                                                      i32.load offset=304
                                                                                      get_local $15
                                                                                      i32.store
                                                                                      br $loop
                                                                                    end ;; $block_65
                                                                                    get_local $13
                                                                                    set_local $17
                                                                                    block $block_87
                                                                                      get_local $5
                                                                                      i64.load offset=304
                                                                                      tee_local $28
                                                                                      i64.eqz
                                                                                      br_if $block_87
                                                                                      get_local $13
                                                                                      set_local $17
                                                                                      loop $loop_12
                                                                                        get_local $17
                                                                                        i32.const -1
                                                                                        i32.add
                                                                                        tee_local $17
                                                                                        get_local $28
                                                                                        i32.wrap/i64
                                                                                        i32.const 7
                                                                                        i32.and
                                                                                        i32.const 48
                                                                                        i32.or
                                                                                        i32.store8
                                                                                        get_local $28
                                                                                        i64.const 3
                                                                                        i64.shr_u
                                                                                        tee_local $28
                                                                                        i64.const 0
                                                                                        i64.ne
                                                                                        br_if $loop_12
                                                                                      end ;; $loop_12
                                                                                    end ;; $block_87
                                                                                    i32.const 0
                                                                                    set_local $23
                                                                                    i32.const 3004
                                                                                    set_local $26
                                                                                    get_local $22
                                                                                    i32.const 8
                                                                                    i32.and
                                                                                    i32.eqz
                                                                                    br_if $block_52
                                                                                    get_local $20
                                                                                    get_local $13
                                                                                    get_local $17
                                                                                    i32.sub
                                                                                    tee_local $16
                                                                                    i32.const 1
                                                                                    i32.add
                                                                                    get_local $20
                                                                                    get_local $16
                                                                                    i32.gt_s
                                                                                    select
                                                                                    set_local $20
                                                                                    get_local $25
                                                                                    br_if $block_51
                                                                                    br $block_50
                                                                                  end ;; $block_64
                                                                                  get_local $20
                                                                                  i32.const 8
                                                                                  get_local $20
                                                                                  i32.const 8
                                                                                  i32.gt_u
                                                                                  select
                                                                                  set_local $20
                                                                                  get_local $22
                                                                                  i32.const 8
                                                                                  i32.or
                                                                                  set_local $22
                                                                                  i32.const 120
                                                                                  set_local $27
                                                                                end ;; $block_63
                                                                                i32.const 0
                                                                                set_local $23
                                                                                i32.const 3004
                                                                                set_local $26
                                                                                block $block_88
                                                                                  get_local $5
                                                                                  i64.load offset=304
                                                                                  tee_local $28
                                                                                  i64.eqz
                                                                                  br_if $block_88
                                                                                  get_local $27
                                                                                  i32.const 32
                                                                                  i32.and
                                                                                  set_local $16
                                                                                  get_local $13
                                                                                  set_local $17
                                                                                  loop $loop_13
                                                                                    get_local $17
                                                                                    i32.const -1
                                                                                    i32.add
                                                                                    tee_local $17
                                                                                    get_local $28
                                                                                    i32.wrap/i64
                                                                                    i32.const 15
                                                                                    i32.and
                                                                                    i32.const 3488
                                                                                    i32.add
                                                                                    i32.load8_u
                                                                                    get_local $16
                                                                                    i32.or
                                                                                    i32.store8
                                                                                    get_local $28
                                                                                    i64.const 4
                                                                                    i64.shr_u
                                                                                    tee_local $28
                                                                                    i64.const 0
                                                                                    i64.ne
                                                                                    br_if $loop_13
                                                                                  end ;; $loop_13
                                                                                  get_local $22
                                                                                  i32.const 8
                                                                                  i32.and
                                                                                  i32.eqz
                                                                                  br_if $block_52
                                                                                  get_local $5
                                                                                  i64.load offset=304
                                                                                  i64.eqz
                                                                                  br_if $block_52
                                                                                  get_local $27
                                                                                  i32.const 4
                                                                                  i32.shr_s
                                                                                  i32.const 3004
                                                                                  i32.add
                                                                                  set_local $26
                                                                                  i32.const 2
                                                                                  set_local $23
                                                                                  get_local $25
                                                                                  br_if $block_51
                                                                                  br $block_50
                                                                                end ;; $block_88
                                                                                get_local $13
                                                                                set_local $17
                                                                                get_local $25
                                                                                br_if $block_51
                                                                                br $block_50
                                                                              end ;; $block_62
                                                                              get_local $5
                                                                              i32.load offset=304
                                                                              tee_local $16
                                                                              i32.const 3014
                                                                              get_local $16
                                                                              select
                                                                              set_local $17
                                                                            end ;; $block_61
                                                                            i32.const 0
                                                                            set_local $23
                                                                            get_local $17
                                                                            get_local $17
                                                                            i32.const 2147483647
                                                                            get_local $20
                                                                            get_local $20
                                                                            i32.const 0
                                                                            i32.lt_s
                                                                            select
                                                                            call $strnlen
                                                                            tee_local $19
                                                                            i32.add
                                                                            set_local $16
                                                                            i32.const 3004
                                                                            set_local $26
                                                                            get_local $20
                                                                            i32.const -1
                                                                            i32.le_s
                                                                            br_if $block_57
                                                                            get_local $24
                                                                            set_local $22
                                                                            get_local $19
                                                                            set_local $20
                                                                            br $block_48
                                                                          end ;; $block_60
                                                                          i32.const 0
                                                                          set_local $23
                                                                          i32.const 3004
                                                                          set_local $26
                                                                          get_local $5
                                                                          i64.load offset=304
                                                                          set_local $28
                                                                          br $block_53
                                                                        end ;; $block_59
                                                                        i32.const 0
                                                                        set_local $23
                                                                        i32.const 3004
                                                                        set_local $26
                                                                        br $block_49
                                                                      end ;; $block_58
                                                                      get_local $5
                                                                      i64.const 0
                                                                      get_local $28
                                                                      i64.sub
                                                                      tee_local $28
                                                                      i64.store offset=304
                                                                      i32.const 1
                                                                      set_local $23
                                                                      i32.const 3004
                                                                      set_local $26
                                                                      br $block_53
                                                                    end ;; $block_57
                                                                    get_local $24
                                                                    set_local $22
                                                                    get_local $19
                                                                    set_local $20
                                                                    get_local $16
                                                                    i32.load8_u
                                                                    i32.eqz
                                                                    br_if $block_48
                                                                    br $block_5
                                                                  end ;; $block_56
                                                                  get_local $19
                                                                  set_local $16
                                                                  get_local $36
                                                                  set_local $18
                                                                end ;; $block_55
                                                                block $block_89
                                                                  get_local $17
                                                                  i32.const -1
                                                                  i32.gt_s
                                                                  br_if $block_89
                                                                  get_local $35
                                                                  i32.const 29
                                                                  i32.add
                                                                  i32.const 9
                                                                  i32.div_u
                                                                  i32.const 1
                                                                  i32.add
                                                                  set_local $25
                                                                  loop $loop_14
                                                                    i32.const 0
                                                                    get_local $17
                                                                    i32.sub
                                                                    tee_local $19
                                                                    i32.const 9
                                                                    get_local $19
                                                                    i32.const 9
                                                                    i32.lt_s
                                                                    select
                                                                    set_local $23
                                                                    block $block_90
                                                                      block $block_91
                                                                        get_local $18
                                                                        get_local $16
                                                                        i32.ge_u
                                                                        br_if $block_91
                                                                        i32.const 1000000000
                                                                        get_local $23
                                                                        i32.shr_u
                                                                        set_local $24
                                                                        i32.const -1
                                                                        get_local $23
                                                                        i32.shl
                                                                        i32.const -1
                                                                        i32.xor
                                                                        set_local $26
                                                                        i32.const 0
                                                                        set_local $17
                                                                        get_local $18
                                                                        set_local $19
                                                                        loop $loop_15
                                                                          get_local $19
                                                                          get_local $19
                                                                          i32.load
                                                                          tee_local $20
                                                                          get_local $23
                                                                          i32.shr_u
                                                                          get_local $17
                                                                          i32.add
                                                                          i32.store
                                                                          get_local $20
                                                                          get_local $26
                                                                          i32.and
                                                                          get_local $24
                                                                          i32.mul
                                                                          set_local $17
                                                                          get_local $19
                                                                          i32.const 4
                                                                          i32.add
                                                                          tee_local $19
                                                                          get_local $16
                                                                          i32.lt_u
                                                                          br_if $loop_15
                                                                        end ;; $loop_15
                                                                        get_local $18
                                                                        get_local $18
                                                                        i32.const 4
                                                                        i32.add
                                                                        get_local $18
                                                                        i32.load
                                                                        select
                                                                        set_local $18
                                                                        get_local $17
                                                                        i32.eqz
                                                                        br_if $block_90
                                                                        get_local $16
                                                                        get_local $17
                                                                        i32.store
                                                                        get_local $16
                                                                        i32.const 4
                                                                        i32.add
                                                                        set_local $16
                                                                        br $block_90
                                                                      end ;; $block_91
                                                                      get_local $18
                                                                      get_local $18
                                                                      i32.const 4
                                                                      i32.add
                                                                      get_local $18
                                                                      i32.load
                                                                      select
                                                                      set_local $18
                                                                    end ;; $block_90
                                                                    get_local $5
                                                                    get_local $5
                                                                    i32.load offset=364
                                                                    get_local $23
                                                                    i32.add
                                                                    tee_local $17
                                                                    i32.store offset=364
                                                                    get_local $36
                                                                    get_local $18
                                                                    get_local $32
                                                                    i32.const 102
                                                                    i32.eq
                                                                    select
                                                                    tee_local $19
                                                                    get_local $25
                                                                    i32.const 2
                                                                    i32.shl
                                                                    i32.add
                                                                    get_local $16
                                                                    get_local $16
                                                                    get_local $19
                                                                    i32.sub
                                                                    i32.const 2
                                                                    i32.shr_s
                                                                    get_local $25
                                                                    i32.gt_s
                                                                    select
                                                                    set_local $16
                                                                    get_local $17
                                                                    i32.const 0
                                                                    i32.lt_s
                                                                    br_if $loop_14
                                                                  end ;; $loop_14
                                                                end ;; $block_89
                                                                i32.const 0
                                                                set_local $17
                                                                block $block_92
                                                                  get_local $18
                                                                  get_local $16
                                                                  i32.ge_u
                                                                  br_if $block_92
                                                                  get_local $36
                                                                  get_local $18
                                                                  i32.sub
                                                                  i32.const 2
                                                                  i32.shr_s
                                                                  i32.const 9
                                                                  i32.mul
                                                                  set_local $17
                                                                  get_local $18
                                                                  i32.load
                                                                  tee_local $20
                                                                  i32.const 10
                                                                  i32.lt_u
                                                                  br_if $block_92
                                                                  i32.const 10
                                                                  set_local $19
                                                                  loop $loop_16
                                                                    get_local $17
                                                                    i32.const 1
                                                                    i32.add
                                                                    set_local $17
                                                                    get_local $20
                                                                    get_local $19
                                                                    i32.const 10
                                                                    i32.mul
                                                                    tee_local $19
                                                                    i32.ge_u
                                                                    br_if $loop_16
                                                                  end ;; $loop_16
                                                                end ;; $block_92
                                                                block $block_93
                                                                  get_local $35
                                                                  i32.const 0
                                                                  get_local $17
                                                                  get_local $32
                                                                  i32.const 102
                                                                  i32.eq
                                                                  select
                                                                  tee_local $20
                                                                  i32.sub
                                                                  get_local $35
                                                                  i32.const 0
                                                                  i32.ne
                                                                  get_local $32
                                                                  i32.const 103
                                                                  i32.eq
                                                                  i32.and
                                                                  tee_local $24
                                                                  i32.sub
                                                                  tee_local $19
                                                                  get_local $16
                                                                  get_local $36
                                                                  i32.sub
                                                                  i32.const 2
                                                                  i32.shr_s
                                                                  i32.const 9
                                                                  i32.mul
                                                                  i32.const -9
                                                                  i32.add
                                                                  i32.ge_s
                                                                  br_if $block_93
                                                                  get_local $36
                                                                  get_local $19
                                                                  i32.const 147456
                                                                  i32.add
                                                                  tee_local $26
                                                                  i32.const 9
                                                                  i32.div_s
                                                                  tee_local $25
                                                                  i32.const 2
                                                                  i32.shl
                                                                  i32.add
                                                                  tee_local $37
                                                                  i32.const -65532
                                                                  i32.add
                                                                  set_local $23
                                                                  i32.const 10
                                                                  set_local $19
                                                                  block $block_94
                                                                    get_local $26
                                                                    get_local $25
                                                                    i32.const 9
                                                                    i32.mul
                                                                    tee_local $25
                                                                    i32.sub
                                                                    i32.const 1
                                                                    i32.add
                                                                    i32.const 8
                                                                    i32.gt_s
                                                                    br_if $block_94
                                                                    get_local $20
                                                                    get_local $25
                                                                    i32.add
                                                                    i32.const -147448
                                                                    i32.add
                                                                    get_local $35
                                                                    i32.sub
                                                                    i32.const 0
                                                                    get_local $24
                                                                    i32.sub
                                                                    i32.sub
                                                                    set_local $20
                                                                    i32.const 10
                                                                    set_local $19
                                                                    loop $loop_17
                                                                      get_local $19
                                                                      i32.const 10
                                                                      i32.mul
                                                                      set_local $19
                                                                      get_local $20
                                                                      i32.const -1
                                                                      i32.add
                                                                      tee_local $20
                                                                      br_if $loop_17
                                                                    end ;; $loop_17
                                                                  end ;; $block_94
                                                                  get_local $23
                                                                  i32.load
                                                                  tee_local $24
                                                                  get_local $24
                                                                  get_local $19
                                                                  i32.div_u
                                                                  tee_local $26
                                                                  get_local $19
                                                                  i32.mul
                                                                  i32.sub
                                                                  set_local $20
                                                                  block $block_95
                                                                    block $block_96
                                                                      get_local $23
                                                                      i32.const 4
                                                                      i32.add
                                                                      tee_local $25
                                                                      get_local $16
                                                                      i32.ne
                                                                      br_if $block_96
                                                                      get_local $20
                                                                      i32.eqz
                                                                      br_if $block_95
                                                                    end ;; $block_96
                                                                    block $block_97
                                                                      block $block_98
                                                                        get_local $26
                                                                        i32.const 1
                                                                        i32.and
                                                                        br_if $block_98
                                                                        i64.const 4626322717216342015
                                                                        set_local $28
                                                                        i64.const -12798843684340
                                                                        set_local $29
                                                                        get_local $23
                                                                        get_local $18
                                                                        i32.le_u
                                                                        br_if $block_97
                                                                        get_local $19
                                                                        i32.const 1000000000
                                                                        i32.ne
                                                                        br_if $block_97
                                                                        get_local $23
                                                                        i32.const -4
                                                                        i32.add
                                                                        i32.load8_u
                                                                        i32.const 1
                                                                        i32.and
                                                                        i32.eqz
                                                                        br_if $block_97
                                                                      end ;; $block_98
                                                                      i64.const 4626322717216342016
                                                                      set_local $28
                                                                      i64.const 1152915105185004806
                                                                      set_local $29
                                                                    end ;; $block_97
                                                                    i64.const 4611123068473966592
                                                                    set_local $33
                                                                    block $block_99
                                                                      get_local $20
                                                                      get_local $19
                                                                      i32.const 1
                                                                      i32.shr_u
                                                                      tee_local $26
                                                                      i32.lt_u
                                                                      br_if $block_99
                                                                      i64.const 4611404543450677248
                                                                      i64.const 4611545280939032576
                                                                      get_local $20
                                                                      get_local $26
                                                                      i32.eq
                                                                      select
                                                                      i64.const 4611545280939032576
                                                                      get_local $25
                                                                      get_local $16
                                                                      i32.eq
                                                                      select
                                                                      set_local $33
                                                                    end ;; $block_99
                                                                    block $block_100
                                                                      get_local $30
                                                                      i32.eqz
                                                                      br_if $block_100
                                                                      get_local $31
                                                                      i32.load8_u
                                                                      i32.const 45
                                                                      i32.ne
                                                                      br_if $block_100
                                                                      get_local $33
                                                                      i64.const -9223372036854775808
                                                                      i64.xor
                                                                      set_local $33
                                                                      get_local $28
                                                                      i64.const -9223372036854775808
                                                                      i64.xor
                                                                      set_local $28
                                                                    end ;; $block_100
                                                                    get_local $5
                                                                    i32.const 128
                                                                    i32.add
                                                                    get_local $29
                                                                    get_local $28
                                                                    i64.const 0
                                                                    get_local $33
                                                                    call $__addtf3
                                                                    get_local $23
                                                                    get_local $24
                                                                    get_local $20
                                                                    i32.sub
                                                                    tee_local $20
                                                                    i32.store
                                                                    get_local $5
                                                                    i64.load offset=128
                                                                    get_local $5
                                                                    i32.const 128
                                                                    i32.add
                                                                    i32.const 8
                                                                    i32.add
                                                                    i64.load
                                                                    get_local $29
                                                                    get_local $28
                                                                    call $__eqtf2
                                                                    i32.eqz
                                                                    br_if $block_95
                                                                    get_local $23
                                                                    get_local $20
                                                                    get_local $19
                                                                    i32.add
                                                                    tee_local $19
                                                                    i32.store
                                                                    block $block_101
                                                                      get_local $19
                                                                      i32.const 1000000000
                                                                      i32.lt_u
                                                                      br_if $block_101
                                                                      get_local $37
                                                                      i32.const -65536
                                                                      i32.add
                                                                      set_local $19
                                                                      loop $loop_18
                                                                        get_local $19
                                                                        i32.const 4
                                                                        i32.add
                                                                        i32.const 0
                                                                        i32.store
                                                                        block $block_102
                                                                          get_local $19
                                                                          get_local $18
                                                                          i32.ge_u
                                                                          br_if $block_102
                                                                          get_local $18
                                                                          i32.const -4
                                                                          i32.add
                                                                          tee_local $18
                                                                          i32.const 0
                                                                          i32.store
                                                                        end ;; $block_102
                                                                        get_local $19
                                                                        get_local $19
                                                                        i32.load
                                                                        i32.const 1
                                                                        i32.add
                                                                        tee_local $17
                                                                        i32.store
                                                                        get_local $19
                                                                        i32.const -4
                                                                        i32.add
                                                                        set_local $19
                                                                        get_local $17
                                                                        i32.const 999999999
                                                                        i32.gt_u
                                                                        br_if $loop_18
                                                                      end ;; $loop_18
                                                                      get_local $19
                                                                      i32.const 4
                                                                      i32.add
                                                                      set_local $23
                                                                    end ;; $block_101
                                                                    get_local $36
                                                                    get_local $18
                                                                    i32.sub
                                                                    i32.const 2
                                                                    i32.shr_s
                                                                    i32.const 9
                                                                    i32.mul
                                                                    set_local $17
                                                                    get_local $18
                                                                    i32.load
                                                                    tee_local $20
                                                                    i32.const 10
                                                                    i32.lt_u
                                                                    br_if $block_95
                                                                    i32.const 10
                                                                    set_local $19
                                                                    loop $loop_19
                                                                      get_local $17
                                                                      i32.const 1
                                                                      i32.add
                                                                      set_local $17
                                                                      get_local $20
                                                                      get_local $19
                                                                      i32.const 10
                                                                      i32.mul
                                                                      tee_local $19
                                                                      i32.ge_u
                                                                      br_if $loop_19
                                                                    end ;; $loop_19
                                                                  end ;; $block_95
                                                                  get_local $23
                                                                  i32.const 4
                                                                  i32.add
                                                                  tee_local $19
                                                                  get_local $16
                                                                  get_local $16
                                                                  get_local $19
                                                                  i32.gt_u
                                                                  select
                                                                  set_local $16
                                                                end ;; $block_93
                                                                block $block_103
                                                                  block $block_104
                                                                    loop $loop_20
                                                                      get_local $16
                                                                      tee_local $19
                                                                      get_local $18
                                                                      i32.le_u
                                                                      br_if $block_104
                                                                      get_local $19
                                                                      i32.const -4
                                                                      i32.add
                                                                      tee_local $16
                                                                      i32.load
                                                                      i32.eqz
                                                                      br_if $loop_20
                                                                    end ;; $loop_20
                                                                    i32.const 1
                                                                    set_local $25
                                                                    br $block_103
                                                                  end ;; $block_104
                                                                  i32.const 0
                                                                  set_local $25
                                                                end ;; $block_103
                                                                get_local $22
                                                                i32.const 8
                                                                i32.and
                                                                set_local $24
                                                                get_local $32
                                                                i32.const 103
                                                                i32.ne
                                                                br_if $block_40
                                                                get_local $17
                                                                i32.const -1
                                                                i32.xor
                                                                i32.const -1
                                                                get_local $35
                                                                i32.const 1
                                                                get_local $35
                                                                select
                                                                tee_local $16
                                                                get_local $17
                                                                i32.gt_s
                                                                get_local $17
                                                                i32.const -5
                                                                i32.gt_s
                                                                i32.and
                                                                tee_local $20
                                                                select
                                                                get_local $16
                                                                i32.add
                                                                set_local $35
                                                                i32.const -1
                                                                i32.const -2
                                                                get_local $20
                                                                select
                                                                get_local $27
                                                                i32.add
                                                                set_local $27
                                                                get_local $24
                                                                br_if $block_40
                                                                i32.const 9
                                                                set_local $16
                                                                block $block_105
                                                                  get_local $25
                                                                  i32.eqz
                                                                  br_if $block_105
                                                                  get_local $19
                                                                  i32.const -4
                                                                  i32.add
                                                                  i32.load
                                                                  tee_local $23
                                                                  i32.eqz
                                                                  br_if $block_105
                                                                  i32.const 0
                                                                  set_local $16
                                                                  get_local $23
                                                                  i32.const 10
                                                                  i32.rem_u
                                                                  br_if $block_105
                                                                  i32.const 10
                                                                  set_local $20
                                                                  i32.const 0
                                                                  set_local $16
                                                                  loop $loop_21
                                                                    get_local $16
                                                                    i32.const 1
                                                                    i32.add
                                                                    set_local $16
                                                                    get_local $23
                                                                    get_local $20
                                                                    i32.const 10
                                                                    i32.mul
                                                                    tee_local $20
                                                                    i32.rem_u
                                                                    i32.eqz
                                                                    br_if $loop_21
                                                                  end ;; $loop_21
                                                                end ;; $block_105
                                                                get_local $19
                                                                get_local $36
                                                                i32.sub
                                                                i32.const 2
                                                                i32.shr_s
                                                                i32.const 9
                                                                i32.mul
                                                                i32.const -9
                                                                i32.add
                                                                set_local $20
                                                                get_local $27
                                                                i32.const 32
                                                                i32.or
                                                                i32.const 102
                                                                i32.ne
                                                                br_if $block_41
                                                                i32.const 0
                                                                set_local $24
                                                                get_local $35
                                                                get_local $20
                                                                get_local $16
                                                                i32.sub
                                                                tee_local $16
                                                                i32.const 0
                                                                get_local $16
                                                                i32.const 0
                                                                i32.gt_s
                                                                select
                                                                tee_local $16
                                                                get_local $35
                                                                get_local $16
                                                                i32.lt_s
                                                                select
                                                                set_local $35
                                                                br $block_40
                                                              end ;; $block_54
                                                              i32.const 1
                                                              set_local $23
                                                              i32.const 3005
                                                              set_local $26
                                                            end ;; $block_53
                                                            get_local $28
                                                            get_local $13
                                                            call $fmt_u
                                                            set_local $17
                                                          end ;; $block_52
                                                          get_local $25
                                                          i32.eqz
                                                          br_if $block_50
                                                        end ;; $block_51
                                                        get_local $20
                                                        i32.const 0
                                                        i32.lt_s
                                                        br_if $block_5
                                                      end ;; $block_50
                                                      get_local $22
                                                      i32.const -65537
                                                      i32.and
                                                      get_local $22
                                                      get_local $25
                                                      select
                                                      set_local $22
                                                      get_local $5
                                                      i64.load offset=304
                                                      set_local $28
                                                      block $block_106
                                                        get_local $20
                                                        br_if $block_106
                                                        get_local $28
                                                        i64.eqz
                                                        i32.eqz
                                                        br_if $block_106
                                                        get_local $13
                                                        set_local $17
                                                        get_local $13
                                                        set_local $16
                                                        i32.const 0
                                                        set_local $20
                                                        br $block_48
                                                      end ;; $block_106
                                                      get_local $20
                                                      get_local $13
                                                      get_local $17
                                                      i32.sub
                                                      get_local $28
                                                      i64.eqz
                                                      i32.add
                                                      tee_local $16
                                                      get_local $20
                                                      get_local $16
                                                      i32.gt_s
                                                      select
                                                      set_local $20
                                                    end ;; $block_49
                                                    get_local $13
                                                    set_local $16
                                                  end ;; $block_48
                                                  get_local $16
                                                  get_local $17
                                                  i32.sub
                                                  tee_local $24
                                                  get_local $20
                                                  get_local $20
                                                  get_local $24
                                                  i32.lt_s
                                                  select
                                                  tee_local $20
                                                  i32.const 2147483647
                                                  get_local $23
                                                  i32.sub
                                                  i32.gt_s
                                                  br_if $block_5
                                                  get_local $23
                                                  get_local $20
                                                  i32.add
                                                  tee_local $19
                                                  get_local $21
                                                  get_local $21
                                                  get_local $19
                                                  i32.lt_s
                                                  select
                                                  tee_local $16
                                                  get_local $18
                                                  i32.gt_s
                                                  br_if $block_5
                                                  get_local $0
                                                  i32.const 32
                                                  get_local $16
                                                  get_local $19
                                                  get_local $22
                                                  call $pad
                                                  block $block_107
                                                    get_local $0
                                                    i32.load8_u
                                                    i32.const 32
                                                    i32.and
                                                    br_if $block_107
                                                    get_local $26
                                                    get_local $23
                                                    get_local $0
                                                    call $__fwritex
                                                    drop
                                                  end ;; $block_107
                                                  get_local $0
                                                  i32.const 48
                                                  get_local $16
                                                  get_local $19
                                                  get_local $22
                                                  i32.const 65536
                                                  i32.xor
                                                  call $pad
                                                  get_local $0
                                                  i32.const 48
                                                  get_local $20
                                                  get_local $24
                                                  i32.const 0
                                                  call $pad
                                                  block $block_108
                                                    get_local $0
                                                    i32.load8_u
                                                    i32.const 32
                                                    i32.and
                                                    br_if $block_108
                                                    get_local $17
                                                    get_local $24
                                                    get_local $0
                                                    call $__fwritex
                                                    drop
                                                  end ;; $block_108
                                                  get_local $0
                                                  i32.const 32
                                                  get_local $16
                                                  get_local $19
                                                  get_local $22
                                                  i32.const 8192
                                                  i32.xor
                                                  call $pad
                                                  br $loop
                                                end ;; $block_47
                                                get_local $23
                                                br_if $block_0
                                              end ;; $block_46
                                              get_local $16
                                              i32.const 0
                                              i32.lt_s
                                              br_if $block_5
                                              get_local $0
                                              i32.const 32
                                              get_local $21
                                              get_local $16
                                              get_local $22
                                              call $pad
                                              block $block_109
                                                get_local $16
                                                i32.eqz
                                                br_if $block_109
                                                i32.const 0
                                                set_local $18
                                                loop $loop_22
                                                  get_local $17
                                                  i32.load
                                                  tee_local $19
                                                  i32.eqz
                                                  br_if $block_42
                                                  get_local $5
                                                  i32.const 244
                                                  i32.add
                                                  get_local $19
                                                  call $wctomb
                                                  tee_local $19
                                                  get_local $18
                                                  i32.add
                                                  tee_local $18
                                                  get_local $16
                                                  i32.gt_u
                                                  br_if $block_42
                                                  block $block_110
                                                    get_local $0
                                                    i32.load8_u
                                                    i32.const 32
                                                    i32.and
                                                    br_if $block_110
                                                    get_local $5
                                                    i32.const 244
                                                    i32.add
                                                    get_local $19
                                                    get_local $0
                                                    call $__fwritex
                                                    drop
                                                  end ;; $block_110
                                                  get_local $17
                                                  i32.const 4
                                                  i32.add
                                                  set_local $17
                                                  get_local $18
                                                  get_local $16
                                                  i32.lt_u
                                                  br_if $loop_22
                                                  br $block_42
                                                end ;; $loop_22
                                              end ;; $block_109
                                              i32.const 0
                                              set_local $16
                                              br $block_42
                                            end ;; $block_45
                                            get_local $5
                                            i32.const 96
                                            i32.add
                                            get_local $28
                                            get_local $29
                                            get_local $34
                                            get_local $33
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
                                            get_local $34
                                            get_local $33
                                            call $__subtf3
                                            get_local $5
                                            i32.const 80
                                            i32.add
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            set_local $29
                                            get_local $5
                                            i64.load offset=80
                                            set_local $28
                                          end ;; $block_44
                                          block $block_111
                                            get_local $5
                                            i32.load offset=364
                                            tee_local $18
                                            get_local $18
                                            i32.const 31
                                            i32.shr_s
                                            tee_local $16
                                            i32.add
                                            get_local $16
                                            i32.xor
                                            i64.extend_u/i32
                                            get_local $11
                                            call $fmt_u
                                            tee_local $16
                                            get_local $11
                                            i32.ne
                                            br_if $block_111
                                            get_local $5
                                            i32.const 324
                                            i32.add
                                            i32.const 11
                                            i32.add
                                            i32.const 48
                                            i32.store8
                                            get_local $9
                                            set_local $16
                                          end ;; $block_111
                                          get_local $30
                                          i32.const 2
                                          i32.or
                                          set_local $24
                                          get_local $16
                                          i32.const -2
                                          i32.add
                                          tee_local $26
                                          get_local $27
                                          i32.const 15
                                          i32.add
                                          i32.store8
                                          get_local $16
                                          i32.const -1
                                          i32.add
                                          i32.const 45
                                          i32.const 43
                                          get_local $18
                                          i32.const 0
                                          i32.lt_s
                                          select
                                          i32.store8
                                          get_local $22
                                          i32.const 8
                                          i32.and
                                          set_local $23
                                          get_local $5
                                          i32.const 336
                                          i32.add
                                          set_local $18
                                          loop $loop_23
                                            get_local $5
                                            i32.const 32
                                            i32.add
                                            get_local $28
                                            get_local $29
                                            call $__fixtfsi
                                            tee_local $19
                                            call $__floatsitf
                                            get_local $5
                                            i32.const 16
                                            i32.add
                                            get_local $28
                                            get_local $29
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
                                            get_local $18
                                            tee_local $16
                                            get_local $19
                                            i32.const 3488
                                            i32.add
                                            i32.load8_u
                                            get_local $17
                                            i32.or
                                            i32.store8
                                            get_local $5
                                            i32.const 8
                                            i32.add
                                            i64.load
                                            set_local $29
                                            get_local $5
                                            i64.load
                                            set_local $28
                                            block $block_112
                                              get_local $16
                                              i32.const 1
                                              i32.add
                                              tee_local $18
                                              get_local $5
                                              i32.const 336
                                              i32.add
                                              i32.sub
                                              i32.const 1
                                              i32.ne
                                              br_if $block_112
                                              block $block_113
                                                get_local $23
                                                br_if $block_113
                                                get_local $20
                                                i32.const 0
                                                i32.gt_s
                                                br_if $block_113
                                                get_local $28
                                                get_local $29
                                                i64.const 0
                                                i64.const 0
                                                call $__eqtf2
                                                i32.eqz
                                                br_if $block_112
                                              end ;; $block_113
                                              get_local $16
                                              i32.const 1
                                              i32.add
                                              i32.const 46
                                              i32.store8
                                              get_local $16
                                              i32.const 2
                                              i32.add
                                              set_local $18
                                            end ;; $block_112
                                            get_local $28
                                            get_local $29
                                            i64.const 0
                                            i64.const 0
                                            call $__netf2
                                            br_if $loop_23
                                          end ;; $loop_23
                                          i32.const -1
                                          set_local $16
                                          i32.const 2147483645
                                          get_local $24
                                          i32.sub
                                          get_local $11
                                          get_local $26
                                          i32.sub
                                          tee_local $19
                                          i32.sub
                                          get_local $20
                                          i32.lt_s
                                          br_if $block_39
                                          get_local $0
                                          i32.const 32
                                          get_local $21
                                          get_local $19
                                          get_local $24
                                          i32.add
                                          get_local $20
                                          i32.const 2
                                          i32.add
                                          get_local $18
                                          get_local $5
                                          i32.const 336
                                          i32.add
                                          i32.sub
                                          tee_local $18
                                          get_local $18
                                          i32.const -2
                                          i32.add
                                          get_local $20
                                          i32.lt_s
                                          select
                                          get_local $18
                                          get_local $20
                                          select
                                          tee_local $17
                                          i32.add
                                          tee_local $16
                                          get_local $22
                                          call $pad
                                          block $block_114
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_114
                                            get_local $25
                                            get_local $24
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_114
                                          get_local $0
                                          i32.const 48
                                          get_local $21
                                          get_local $16
                                          get_local $22
                                          i32.const 65536
                                          i32.xor
                                          call $pad
                                          block $block_115
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_115
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            get_local $18
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_115
                                          get_local $0
                                          i32.const 48
                                          get_local $17
                                          get_local $18
                                          i32.sub
                                          i32.const 0
                                          i32.const 0
                                          call $pad
                                          block $block_116
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_116
                                            get_local $26
                                            get_local $19
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_116
                                          get_local $0
                                          i32.const 32
                                          get_local $21
                                          get_local $16
                                          get_local $22
                                          i32.const 8192
                                          i32.xor
                                          call $pad
                                          get_local $21
                                          get_local $16
                                          get_local $16
                                          get_local $21
                                          i32.lt_s
                                          select
                                          set_local $16
                                          br $block_39
                                        end ;; $block_43
                                        i32.const 0
                                        set_local $16
                                        get_local $0
                                        i32.const 32
                                        get_local $21
                                        i32.const 0
                                        get_local $22
                                        call $pad
                                      end ;; $block_42
                                      get_local $0
                                      i32.const 32
                                      get_local $21
                                      get_local $16
                                      get_local $22
                                      i32.const 8192
                                      i32.xor
                                      call $pad
                                      get_local $21
                                      get_local $16
                                      get_local $21
                                      get_local $16
                                      i32.gt_s
                                      select
                                      set_local $16
                                      br $loop
                                    end ;; $block_41
                                    i32.const 0
                                    set_local $24
                                    get_local $35
                                    get_local $20
                                    get_local $17
                                    i32.add
                                    get_local $16
                                    i32.sub
                                    tee_local $16
                                    i32.const 0
                                    get_local $16
                                    i32.const 0
                                    i32.gt_s
                                    select
                                    tee_local $16
                                    get_local $35
                                    get_local $16
                                    i32.lt_s
                                    select
                                    set_local $35
                                  end ;; $block_40
                                  i32.const -1
                                  set_local $16
                                  get_local $35
                                  i32.const 2147483645
                                  i32.const 2147483646
                                  get_local $35
                                  get_local $24
                                  i32.or
                                  tee_local $26
                                  select
                                  i32.gt_s
                                  br_if $block_39
                                  get_local $35
                                  get_local $26
                                  i32.const 0
                                  i32.ne
                                  i32.add
                                  i32.const 1
                                  i32.add
                                  set_local $32
                                  block $block_117
                                    block $block_118
                                      get_local $27
                                      i32.const 32
                                      i32.or
                                      i32.const 102
                                      i32.ne
                                      tee_local $37
                                      br_if $block_118
                                      get_local $17
                                      i32.const 2147483647
                                      get_local $32
                                      i32.sub
                                      i32.gt_s
                                      br_if $block_39
                                      get_local $17
                                      i32.const 0
                                      get_local $17
                                      i32.const 0
                                      i32.gt_s
                                      select
                                      set_local $17
                                      br $block_117
                                    end ;; $block_118
                                    block $block_119
                                      get_local $11
                                      get_local $17
                                      get_local $17
                                      i32.const 31
                                      i32.shr_s
                                      tee_local $16
                                      i32.add
                                      get_local $16
                                      i32.xor
                                      i64.extend_u/i32
                                      get_local $11
                                      call $fmt_u
                                      tee_local $20
                                      i32.sub
                                      i32.const 1
                                      i32.gt_s
                                      br_if $block_119
                                      get_local $20
                                      i32.const -1
                                      i32.add
                                      set_local $16
                                      loop $loop_24
                                        get_local $16
                                        i32.const 48
                                        i32.store8
                                        get_local $11
                                        get_local $16
                                        i32.sub
                                        set_local $20
                                        get_local $16
                                        i32.const -1
                                        i32.add
                                        tee_local $23
                                        set_local $16
                                        get_local $20
                                        i32.const 2
                                        i32.lt_s
                                        br_if $loop_24
                                      end ;; $loop_24
                                      get_local $23
                                      i32.const 1
                                      i32.add
                                      set_local $20
                                    end ;; $block_119
                                    get_local $20
                                    i32.const -2
                                    i32.add
                                    tee_local $23
                                    get_local $27
                                    i32.store8
                                    i32.const -1
                                    set_local $16
                                    get_local $20
                                    i32.const -1
                                    i32.add
                                    i32.const 45
                                    i32.const 43
                                    get_local $17
                                    i32.const 0
                                    i32.lt_s
                                    select
                                    i32.store8
                                    get_local $11
                                    get_local $23
                                    i32.sub
                                    tee_local $17
                                    i32.const 2147483647
                                    get_local $32
                                    i32.sub
                                    i32.gt_s
                                    br_if $block_39
                                  end ;; $block_117
                                  i32.const -1
                                  set_local $16
                                  get_local $17
                                  get_local $32
                                  i32.add
                                  tee_local $17
                                  get_local $30
                                  i32.const 2147483647
                                  i32.xor
                                  i32.gt_s
                                  br_if $block_39
                                  get_local $0
                                  i32.const 32
                                  get_local $21
                                  get_local $17
                                  get_local $30
                                  i32.add
                                  tee_local $27
                                  get_local $22
                                  call $pad
                                  block $block_120
                                    get_local $0
                                    i32.load8_u
                                    i32.const 32
                                    i32.and
                                    br_if $block_120
                                    get_local $31
                                    get_local $30
                                    get_local $0
                                    call $__fwritex
                                    drop
                                  end ;; $block_120
                                  get_local $0
                                  i32.const 48
                                  get_local $21
                                  get_local $27
                                  get_local $22
                                  i32.const 65536
                                  i32.xor
                                  call $pad
                                  block $block_121
                                    block $block_122
                                      get_local $37
                                      br_if $block_122
                                      get_local $36
                                      get_local $18
                                      get_local $18
                                      get_local $36
                                      i32.gt_u
                                      select
                                      tee_local $17
                                      set_local $18
                                      loop $loop_25
                                        get_local $18
                                        i64.load32_u
                                        get_local $8
                                        call $fmt_u
                                        set_local $16
                                        block $block_123
                                          block $block_124
                                            get_local $18
                                            get_local $17
                                            i32.eq
                                            br_if $block_124
                                            get_local $16
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.le_u
                                            br_if $block_123
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.const 48
                                            get_local $16
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.sub
                                            call $memset
                                            drop
                                            loop $loop_26
                                              get_local $16
                                              i32.const -1
                                              i32.add
                                              tee_local $16
                                              get_local $5
                                              i32.const 336
                                              i32.add
                                              i32.gt_u
                                              br_if $loop_26
                                              br $block_123
                                            end ;; $loop_26
                                          end ;; $block_124
                                          get_local $16
                                          get_local $8
                                          i32.ne
                                          br_if $block_123
                                          get_local $5
                                          i32.const 48
                                          i32.store8 offset=344
                                          get_local $7
                                          set_local $16
                                        end ;; $block_123
                                        block $block_125
                                          get_local $0
                                          i32.load8_u
                                          i32.const 32
                                          i32.and
                                          br_if $block_125
                                          get_local $16
                                          get_local $8
                                          get_local $16
                                          i32.sub
                                          get_local $0
                                          call $__fwritex
                                          drop
                                        end ;; $block_125
                                        get_local $18
                                        i32.const 4
                                        i32.add
                                        tee_local $18
                                        get_local $36
                                        i32.le_u
                                        br_if $loop_25
                                      end ;; $loop_25
                                      block $block_126
                                        get_local $26
                                        i32.eqz
                                        br_if $block_126
                                        get_local $0
                                        i32.load8_u
                                        i32.const 32
                                        i32.and
                                        br_if $block_126
                                        i32.const 3539
                                        i32.const 1
                                        get_local $0
                                        call $__fwritex
                                        drop
                                      end ;; $block_126
                                      block $block_127
                                        get_local $35
                                        i32.const 1
                                        i32.lt_s
                                        br_if $block_127
                                        get_local $18
                                        get_local $19
                                        i32.ge_u
                                        br_if $block_127
                                        loop $loop_27
                                          block $block_128
                                            get_local $18
                                            i64.load32_u
                                            get_local $8
                                            call $fmt_u
                                            tee_local $16
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.le_u
                                            br_if $block_128
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.const 48
                                            get_local $16
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.sub
                                            call $memset
                                            drop
                                            loop $loop_28
                                              get_local $16
                                              i32.const -1
                                              i32.add
                                              tee_local $16
                                              get_local $5
                                              i32.const 336
                                              i32.add
                                              i32.gt_u
                                              br_if $loop_28
                                            end ;; $loop_28
                                          end ;; $block_128
                                          block $block_129
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_129
                                            get_local $16
                                            get_local $35
                                            i32.const 9
                                            get_local $35
                                            i32.const 9
                                            i32.lt_s
                                            select
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_129
                                          get_local $35
                                          i32.const -9
                                          i32.add
                                          tee_local $35
                                          i32.const 1
                                          i32.lt_s
                                          br_if $block_127
                                          get_local $18
                                          i32.const 4
                                          i32.add
                                          tee_local $18
                                          get_local $19
                                          i32.lt_u
                                          br_if $loop_27
                                        end ;; $loop_27
                                      end ;; $block_127
                                      get_local $0
                                      i32.const 48
                                      get_local $35
                                      i32.const 9
                                      i32.add
                                      i32.const 9
                                      i32.const 0
                                      call $pad
                                      br $block_121
                                    end ;; $block_122
                                    block $block_130
                                      get_local $35
                                      i32.const 0
                                      i32.lt_s
                                      br_if $block_130
                                      get_local $19
                                      get_local $18
                                      i32.const 4
                                      i32.add
                                      get_local $25
                                      select
                                      set_local $20
                                      get_local $18
                                      set_local $19
                                      loop $loop_29
                                        block $block_131
                                          get_local $19
                                          i64.load32_u
                                          get_local $8
                                          call $fmt_u
                                          tee_local $16
                                          get_local $8
                                          i32.ne
                                          br_if $block_131
                                          get_local $5
                                          i32.const 48
                                          i32.store8 offset=344
                                          get_local $7
                                          set_local $16
                                        end ;; $block_131
                                        block $block_132
                                          block $block_133
                                            get_local $19
                                            get_local $18
                                            i32.eq
                                            br_if $block_133
                                            get_local $16
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.le_u
                                            br_if $block_132
                                            get_local $5
                                            i32.const 336
                                            i32.add
                                            i32.const 48
                                            get_local $16
                                            get_local $6
                                            i32.add
                                            call $memset
                                            drop
                                            loop $loop_30
                                              get_local $16
                                              i32.const -1
                                              i32.add
                                              tee_local $16
                                              get_local $5
                                              i32.const 336
                                              i32.add
                                              i32.gt_u
                                              br_if $loop_30
                                              br $block_132
                                            end ;; $loop_30
                                          end ;; $block_133
                                          block $block_134
                                            get_local $0
                                            i32.load8_u
                                            i32.const 32
                                            i32.and
                                            br_if $block_134
                                            get_local $16
                                            i32.const 1
                                            get_local $0
                                            call $__fwritex
                                            drop
                                          end ;; $block_134
                                          get_local $16
                                          i32.const 1
                                          i32.add
                                          set_local $16
                                          block $block_135
                                            get_local $24
                                            br_if $block_135
                                            get_local $35
                                            i32.const 1
                                            i32.lt_s
                                            br_if $block_132
                                          end ;; $block_135
                                          get_local $0
                                          i32.load8_u
                                          i32.const 32
                                          i32.and
                                          br_if $block_132
                                          i32.const 3539
                                          i32.const 1
                                          get_local $0
                                          call $__fwritex
                                          drop
                                        end ;; $block_132
                                        get_local $8
                                        get_local $16
                                        i32.sub
                                        set_local $17
                                        block $block_136
                                          get_local $0
                                          i32.load8_u
                                          i32.const 32
                                          i32.and
                                          br_if $block_136
                                          get_local $16
                                          get_local $17
                                          get_local $35
                                          get_local $35
                                          get_local $17
                                          i32.gt_s
                                          select
                                          get_local $0
                                          call $__fwritex
                                          drop
                                        end ;; $block_136
                                        get_local $35
                                        get_local $17
                                        i32.sub
                                        set_local $35
                                        get_local $19
                                        i32.const 4
                                        i32.add
                                        tee_local $19
                                        get_local $20
                                        i32.ge_u
                                        br_if $block_130
                                        get_local $35
                                        i32.const -1
                                        i32.gt_s
                                        br_if $loop_29
                                      end ;; $loop_29
                                    end ;; $block_130
                                    get_local $0
                                    i32.const 48
                                    get_local $35
                                    i32.const 18
                                    i32.add
                                    i32.const 18
                                    i32.const 0
                                    call $pad
                                    get_local $0
                                    i32.load8_u
                                    i32.const 32
                                    i32.and
                                    br_if $block_121
                                    get_local $23
                                    get_local $11
                                    get_local $23
                                    i32.sub
                                    get_local $0
                                    call $__fwritex
                                    drop
                                  end ;; $block_121
                                  get_local $0
                                  i32.const 32
                                  get_local $21
                                  get_local $27
                                  get_local $22
                                  i32.const 8192
                                  i32.xor
                                  call $pad
                                  get_local $21
                                  get_local $27
                                  get_local $27
                                  get_local $21
                                  i32.lt_s
                                  select
                                  set_local $16
                                end ;; $block_39
                                get_local $16
                                i32.const 0
                                i32.ge_s
                                br_if $loop
                                br $block_5
                              end ;; $block_38
                              get_local $5
                              i32.load offset=304
                              get_local $15
                              i32.store
                              br $loop
                            end ;; $block_37
                            get_local $5
                            i32.load offset=304
                            get_local $15
                            i64.extend_s/i32
                            i64.store
                            br $loop
                          end ;; $block_36
                          get_local $5
                          i32.load offset=304
                          get_local $15
                          i32.store16
                          br $loop
                        end ;; $block_35
                        get_local $5
                        i32.load offset=304
                        get_local $15
                        i32.store8
                        br $loop
                      end ;; $block_34
                      get_local $5
                      i32.load offset=304
                      get_local $15
                      i32.store
                      br $loop
                    end ;; $block_33
                    get_local $5
                    i32.load offset=304
                    get_local $15
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
              get_local $14
              i32.eqz
              br_if $block_3
              get_local $3
              i32.const 16
              i32.add
              set_local $18
              get_local $4
              i32.const 4
              i32.add
              set_local $1
              i32.const 1
              set_local $16
              loop $loop_31
                get_local $1
                i32.load
                tee_local $19
                i32.eqz
                br_if $block_2
                get_local $18
                get_local $19
                get_local $2
                call $pop_arg
                get_local $18
                i32.const 16
                i32.add
                set_local $18
                get_local $1
                i32.const 4
                i32.add
                set_local $1
                i32.const 1
                set_local $15
                get_local $16
                i32.const 1
                i32.add
                tee_local $16
                i32.const 10
                i32.lt_u
                br_if $loop_31
                br $block
              end ;; $loop_31
            end ;; $block_3
            i32.const 0
            set_local $15
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
            set_local $15
            get_local $16
            i32.const 1
            i32.add
            tee_local $16
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
      set_local $15
    end ;; $block
    get_local $5
    i32.const 7712
    i32.add
    set_global $25
    get_local $15
    )
  
  (func $pop_arg (type $13)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (local $3 i32)
    (local $4 i64)
    get_global $25
    i32.const 16
    i32.sub
    tee_local $3
    set_global $25
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
    set_global $25
    )
  
  (func $pad (type $14)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (param $3 i32)
    (param $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    get_global $25
    i32.const 256
    i32.sub
    tee_local $5
    set_global $25
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
    set_global $25
    )
  
  (func $fmt_u (type $15)
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
  
  (func $wcrtomb (type $1)
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
              i32.const 18
              i32.shr_u
              i32.const 240
              i32.or
              i32.store8
              get_local $0
              get_local $1
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=3
              get_local $0
              get_local $1
              i32.const 12
              i32.shr_u
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=1
              get_local $0
              get_local $1
              i32.const 6
              i32.shr_u
              i32.const 63
              i32.and
              i32.const 128
              i32.or
              i32.store8 offset=2
              i32.const 4
              return
            end ;; $block_5
            get_local $0
            get_local $1
            i32.const 12
            i32.shr_u
            i32.const 224
            i32.or
            i32.store8
            get_local $0
            get_local $1
            i32.const 63
            i32.and
            i32.const 128
            i32.or
            i32.store8 offset=2
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
  
  (func $wctomb (type $8)
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
  
  (func $__letf2 (type $16)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i64)
    (local $8 i64)
    (local $9 i32)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i32)
    (local $20 i64)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i32)
    (local $33 i32)
    (local $34 i64)
    (local $35 i32)
    (local $36 i32)
    (local $37 i32)
    (local $38 i64)
    (local $39 i64)
    (local $40 i64)
    (local $41 i32)
    (local $42 i32)
    (local $43 i64)
    (local $44 i32)
    (local $45 i32)
    (local $46 i32)
    (local $47 i32)
    (local $48 i64)
    (local $49 i64)
    (local $50 i64)
    (local $51 i64)
    (local $52 i64)
    (local $53 i64)
    (local $54 i64)
    (local $55 i32)
    (local $56 i32)
    (local $57 i32)
    (local $58 i64)
    (local $59 i64)
    (local $60 i64)
    (local $61 i64)
    (local $62 i32)
    (local $63 i32)
    (local $64 i64)
    (local $65 i64)
    (local $66 i64)
    (local $67 i64)
    (local $68 i32)
    (local $69 i32)
    (local $70 i32)
    (local $71 i32)
    (local $72 i32)
    (local $73 i32)
    (local $74 i64)
    (local $75 i64)
    (local $76 i64)
    (local $77 i64)
    (local $78 i64)
    (local $79 i64)
    (local $80 i64)
    (local $81 i32)
    (local $82 i32)
    (local $83 i32)
    (local $84 i32)
    (local $85 i64)
    (local $86 i64)
    (local $87 i64)
    (local $88 i64)
    (local $89 i32)
    (local $90 i32)
    (local $91 i32)
    (local $92 i32)
    (local $93 i32)
    (local $94 i32)
    (local $95 i64)
    (local $96 i64)
    (local $97 i64)
    (local $98 i64)
    (local $99 i64)
    (local $100 i64)
    (local $101 i64)
    (local $102 i32)
    (local $103 i32)
    (local $104 i32)
    (local $105 i32)
    (local $106 i32)
    (local $107 i32)
    (local $108 i32)
    get_global $25
    set_local $4
    i32.const 144
    set_local $5
    get_local $4
    get_local $5
    i32.sub
    set_local $6
    get_local $6
    set_global $25
    get_local $6
    get_local $0
    i64.store offset=112
    get_local $6
    get_local $1
    i64.store offset=120
    get_local $6
    get_local $3
    i64.store offset=104
    get_local $6
    get_local $2
    i64.store offset=96
    get_local $6
    i64.load offset=120
    set_local $7
    get_local $6
    i64.load offset=112
    set_local $8
    i32.const 16
    set_local $9
    get_local $6
    get_local $9
    i32.add
    set_local $10
    get_local $10
    get_local $8
    get_local $7
    call $toRep
    i32.const 8
    set_local $11
    i32.const 16
    set_local $12
    get_local $6
    get_local $12
    i32.add
    set_local $13
    get_local $13
    get_local $11
    i32.add
    set_local $14
    get_local $14
    i64.load align=1
    set_local $15
    get_local $6
    i64.load offset=16 align=1
    set_local $16
    get_local $6
    get_local $15
    i64.store offset=88
    get_local $6
    get_local $16
    i64.store offset=80
    get_local $6
    i64.load offset=104
    set_local $17
    get_local $6
    i64.load offset=96
    set_local $18
    get_local $6
    get_local $18
    get_local $17
    call $toRep
    get_local $6
    get_local $11
    i32.add
    set_local $19
    get_local $19
    i64.load align=1
    set_local $20
    get_local $6
    i64.load align=1
    set_local $21
    get_local $6
    get_local $20
    i64.store offset=72
    get_local $6
    get_local $21
    i64.store offset=64
    get_local $6
    i64.load offset=80
    set_local $22
    get_local $6
    i64.load offset=88
    set_local $23
    i64.const 9223372036854775807
    set_local $24
    get_local $23
    get_local $24
    i64.and
    set_local $25
    get_local $6
    get_local $22
    i64.store offset=48
    get_local $6
    get_local $25
    i64.store offset=56
    get_local $6
    i64.load offset=64
    set_local $26
    get_local $6
    i64.load offset=72
    set_local $27
    get_local $27
    get_local $24
    i64.and
    set_local $28
    get_local $6
    get_local $26
    i64.store offset=32
    get_local $6
    get_local $28
    i64.store offset=40
    get_local $6
    i64.load offset=48
    set_local $29
    get_local $6
    i64.load offset=56
    set_local $30
    i64.const 9223090561878065152
    set_local $31
    get_local $30
    get_local $31
    i64.eq
    set_local $32
    get_local $30
    get_local $31
    i64.gt_u
    set_local $33
    i64.const 0
    set_local $34
    get_local $29
    get_local $34
    i64.ne
    set_local $35
    get_local $35
    get_local $33
    get_local $32
    select
    set_local $36
    get_local $36
    set_local $37
    block $block
      block $block_0
        block $block_1
          get_local $37
          br_if $block_1
          get_local $6
          i64.load offset=32
          set_local $38
          get_local $6
          i64.load offset=40
          set_local $39
          i64.const 9223090561878065152
          set_local $40
          get_local $39
          get_local $40
          i64.eq
          set_local $41
          get_local $39
          get_local $40
          i64.gt_u
          set_local $42
          i64.const 0
          set_local $43
          get_local $38
          get_local $43
          i64.ne
          set_local $44
          get_local $44
          get_local $42
          get_local $41
          select
          set_local $45
          get_local $45
          set_local $46
          get_local $46
          i32.eqz
          br_if $block_0
        end ;; $block_1
        i32.const 1
        set_local $47
        get_local $6
        get_local $47
        i32.store offset=140
        br $block
      end ;; $block_0
      get_local $6
      i64.load offset=48
      set_local $48
      get_local $6
      i64.load offset=56
      set_local $49
      get_local $6
      i64.load offset=32
      set_local $50
      get_local $6
      i64.load offset=40
      set_local $51
      get_local $49
      get_local $51
      i64.or
      set_local $52
      get_local $48
      get_local $50
      i64.or
      set_local $53
      get_local $53
      get_local $52
      i64.or
      set_local $54
      get_local $54
      i64.eqz
      set_local $55
      get_local $55
      set_local $56
      block $block_2
        get_local $56
        i32.eqz
        br_if $block_2
        i32.const 0
        set_local $57
        get_local $6
        get_local $57
        i32.store offset=140
        br $block
      end ;; $block_2
      get_local $6
      i64.load offset=88
      set_local $58
      get_local $6
      i64.load offset=72
      set_local $59
      get_local $58
      get_local $59
      i64.and
      set_local $60
      i64.const -1
      set_local $61
      get_local $60
      get_local $61
      i64.gt_s
      set_local $62
      get_local $62
      set_local $63
      block $block_3
        get_local $63
        i32.eqz
        br_if $block_3
        get_local $6
        i64.load offset=80
        set_local $64
        get_local $6
        i64.load offset=88
        set_local $65
        get_local $6
        i64.load offset=64
        set_local $66
        get_local $6
        i64.load offset=72
        set_local $67
        get_local $65
        get_local $67
        i64.eq
        set_local $68
        get_local $65
        get_local $67
        i64.lt_s
        set_local $69
        get_local $64
        get_local $66
        i64.lt_u
        set_local $70
        get_local $70
        get_local $69
        get_local $68
        select
        set_local $71
        get_local $71
        set_local $72
        block $block_4
          get_local $72
          i32.eqz
          br_if $block_4
          i32.const -1
          set_local $73
          get_local $6
          get_local $73
          i32.store offset=140
          br $block
        end ;; $block_4
        get_local $6
        i64.load offset=80
        set_local $74
        get_local $6
        i64.load offset=88
        set_local $75
        get_local $6
        i64.load offset=64
        set_local $76
        get_local $6
        i64.load offset=72
        set_local $77
        get_local $75
        get_local $77
        i64.xor
        set_local $78
        get_local $74
        get_local $76
        i64.xor
        set_local $79
        get_local $79
        get_local $78
        i64.or
        set_local $80
        get_local $80
        i64.eqz
        set_local $81
        get_local $81
        set_local $82
        block $block_5
          get_local $82
          i32.eqz
          br_if $block_5
          i32.const 0
          set_local $83
          get_local $6
          get_local $83
          i32.store offset=140
          br $block
        end ;; $block_5
        i32.const 1
        set_local $84
        get_local $6
        get_local $84
        i32.store offset=140
        br $block
      end ;; $block_3
      get_local $6
      i64.load offset=80
      set_local $85
      get_local $6
      i64.load offset=88
      set_local $86
      get_local $6
      i64.load offset=64
      set_local $87
      get_local $6
      i64.load offset=72
      set_local $88
      get_local $86
      get_local $88
      i64.eq
      set_local $89
      get_local $86
      get_local $88
      i64.gt_s
      set_local $90
      get_local $85
      get_local $87
      i64.gt_u
      set_local $91
      get_local $91
      get_local $90
      get_local $89
      select
      set_local $92
      get_local $92
      set_local $93
      block $block_6
        get_local $93
        i32.eqz
        br_if $block_6
        i32.const -1
        set_local $94
        get_local $6
        get_local $94
        i32.store offset=140
        br $block
      end ;; $block_6
      get_local $6
      i64.load offset=80
      set_local $95
      get_local $6
      i64.load offset=88
      set_local $96
      get_local $6
      i64.load offset=64
      set_local $97
      get_local $6
      i64.load offset=72
      set_local $98
      get_local $96
      get_local $98
      i64.xor
      set_local $99
      get_local $95
      get_local $97
      i64.xor
      set_local $100
      get_local $100
      get_local $99
      i64.or
      set_local $101
      get_local $101
      i64.eqz
      set_local $102
      get_local $102
      set_local $103
      block $block_7
        get_local $103
        i32.eqz
        br_if $block_7
        i32.const 0
        set_local $104
        get_local $6
        get_local $104
        i32.store offset=140
        br $block
      end ;; $block_7
      i32.const 1
      set_local $105
      get_local $6
      get_local $105
      i32.store offset=140
    end ;; $block
    get_local $6
    i32.load offset=140
    set_local $106
    i32.const 144
    set_local $107
    get_local $6
    get_local $107
    i32.add
    set_local $108
    get_local $108
    set_global $25
    get_local $106
    return
    )
  
  (func $toRep (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $__unordtf2 (type $16)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i64)
    (local $22 i32)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i32)
    (local $30 i32)
    (local $31 i64)
    (local $32 i32)
    (local $33 i32)
    (local $34 i32)
    (local $35 i32)
    (local $36 i64)
    (local $37 i64)
    (local $38 i64)
    (local $39 i32)
    (local $40 i32)
    (local $41 i64)
    (local $42 i32)
    (local $43 i32)
    (local $44 i32)
    (local $45 i32)
    (local $46 i32)
    (local $47 i32)
    (local $48 i32)
    get_global $25
    set_local $4
    i32.const 96
    set_local $5
    get_local $4
    get_local $5
    i32.sub
    set_local $6
    get_local $6
    set_global $25
    i32.const 1
    set_local $7
    get_local $6
    get_local $0
    i64.store offset=80
    get_local $6
    get_local $1
    i64.store offset=88
    get_local $6
    get_local $3
    i64.store offset=72
    get_local $6
    get_local $2
    i64.store offset=64
    get_local $6
    i64.load offset=88
    set_local $8
    get_local $6
    i64.load offset=80
    set_local $9
    i32.const 16
    set_local $10
    get_local $6
    get_local $10
    i32.add
    set_local $11
    get_local $11
    get_local $9
    get_local $8
    call $toRep
    i32.const 8
    set_local $12
    i32.const 16
    set_local $13
    get_local $6
    get_local $13
    i32.add
    set_local $14
    get_local $14
    get_local $12
    i32.add
    set_local $15
    get_local $15
    i64.load align=1
    set_local $16
    get_local $6
    i64.load offset=16 align=1
    set_local $17
    i64.const 9223372036854775807
    set_local $18
    get_local $16
    get_local $18
    i64.and
    set_local $19
    get_local $6
    get_local $17
    i64.store offset=48
    get_local $6
    get_local $19
    i64.store offset=56
    get_local $6
    i64.load offset=72
    set_local $20
    get_local $6
    i64.load offset=64
    set_local $21
    get_local $6
    get_local $21
    get_local $20
    call $toRep
    get_local $6
    get_local $12
    i32.add
    set_local $22
    get_local $22
    i64.load align=1
    set_local $23
    get_local $6
    i64.load align=1
    set_local $24
    get_local $23
    get_local $18
    i64.and
    set_local $25
    get_local $6
    get_local $24
    i64.store offset=32
    get_local $6
    get_local $25
    i64.store offset=40
    get_local $6
    i64.load offset=48
    set_local $26
    get_local $6
    i64.load offset=56
    set_local $27
    i64.const 9223090561878065152
    set_local $28
    get_local $27
    get_local $28
    i64.eq
    set_local $29
    get_local $27
    get_local $28
    i64.gt_u
    set_local $30
    i64.const 0
    set_local $31
    get_local $26
    get_local $31
    i64.ne
    set_local $32
    get_local $32
    get_local $30
    get_local $29
    select
    set_local $33
    get_local $33
    set_local $34
    get_local $7
    set_local $35
    block $block
      get_local $34
      br_if $block
      get_local $6
      i64.load offset=32
      set_local $36
      get_local $6
      i64.load offset=40
      set_local $37
      i64.const 9223090561878065152
      set_local $38
      get_local $37
      get_local $38
      i64.eq
      set_local $39
      get_local $37
      get_local $38
      i64.gt_u
      set_local $40
      i64.const 0
      set_local $41
      get_local $36
      get_local $41
      i64.ne
      set_local $42
      get_local $42
      get_local $40
      get_local $39
      select
      set_local $43
      get_local $43
      set_local $35
    end ;; $block
    get_local $35
    set_local $44
    i32.const 1
    set_local $45
    get_local $44
    get_local $45
    i32.and
    set_local $46
    i32.const 96
    set_local $47
    get_local $6
    get_local $47
    i32.add
    set_local $48
    get_local $48
    set_global $25
    get_local $46
    return
    )
  
  (func $__eqtf2 (type $16)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    get_global $25
    set_local $4
    i32.const 32
    set_local $5
    get_local $4
    get_local $5
    i32.sub
    set_local $6
    get_local $6
    set_global $25
    get_local $6
    get_local $0
    i64.store offset=16
    get_local $6
    get_local $1
    i64.store offset=24
    get_local $6
    get_local $3
    i64.store offset=8
    get_local $6
    get_local $2
    i64.store
    get_local $6
    i64.load offset=24
    set_local $7
    get_local $6
    i64.load offset=16
    set_local $8
    get_local $6
    i64.load offset=8
    set_local $9
    get_local $6
    i64.load
    set_local $10
    get_local $8
    get_local $7
    get_local $10
    get_local $9
    call $__letf2
    set_local $11
    i32.const 32
    set_local $12
    get_local $6
    get_local $12
    i32.add
    set_local $13
    get_local $13
    set_global $25
    get_local $11
    return
    )
  
  (func $__netf2 (type $16)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (result i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    get_global $25
    set_local $4
    i32.const 32
    set_local $5
    get_local $4
    get_local $5
    i32.sub
    set_local $6
    get_local $6
    set_global $25
    get_local $6
    get_local $0
    i64.store offset=16
    get_local $6
    get_local $1
    i64.store offset=24
    get_local $6
    get_local $3
    i64.store offset=8
    get_local $6
    get_local $2
    i64.store
    get_local $6
    i64.load offset=24
    set_local $7
    get_local $6
    i64.load offset=16
    set_local $8
    get_local $6
    i64.load offset=8
    set_local $9
    get_local $6
    i64.load
    set_local $10
    get_local $8
    get_local $7
    get_local $10
    get_local $9
    call $__letf2
    set_local $11
    i32.const 32
    set_local $12
    get_local $6
    get_local $12
    i32.add
    set_local $13
    get_local $13
    set_global $25
    get_local $11
    return
    )
  
  (func $__extenddftf2 (type $18)
    (param $0 i32)
    (param $1 f64)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 f64)
    (local $6 i64)
    (local $7 i64)
    (local $8 i32)
    (local $9 i32)
    get_global $25
    set_local $2
    i32.const 32
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    get_local $4
    get_local $1
    f64.store offset=24
    get_local $4
    f64.load offset=24
    set_local $5
    get_local $4
    get_local $5
    call $__extendXfYf2__
    get_local $4
    i64.load align=1
    set_local $6
    get_local $4
    i64.load offset=8 align=1
    set_local $7
    get_local $0
    get_local $7
    i64.store offset=8
    get_local $0
    get_local $6
    i64.store
    i32.const 32
    set_local $8
    get_local $4
    get_local $8
    i32.add
    set_local $9
    get_local $9
    set_global $25
    return
    )
  
  (func $__extendXfYf2__ (type $18)
    (param $0 i32)
    (param $1 f64)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i64)
    (local $15 i64)
    (local $16 i32)
    (local $17 i32)
    (local $18 i32)
    (local $19 i32)
    (local $20 i64)
    (local $21 i64)
    (local $22 f64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i64)
    (local $35 i32)
    (local $36 i32)
    (local $37 i64)
    (local $38 i64)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    (local $45 i64)
    (local $46 i64)
    (local $47 i64)
    (local $48 i64)
    (local $49 i32)
    (local $50 i32)
    (local $51 i64)
    (local $52 i64)
    (local $53 i64)
    (local $54 i64)
    (local $55 i64)
    (local $56 i64)
    (local $57 i64)
    (local $58 i64)
    (local $59 i64)
    (local $60 i64)
    (local $61 i64)
    (local $62 i64)
    (local $63 i64)
    (local $64 i64)
    (local $65 i64)
    (local $66 i64)
    (local $67 i64)
    (local $68 i64)
    (local $69 i64)
    (local $70 i64)
    (local $71 i64)
    (local $72 i64)
    (local $73 i64)
    (local $74 i64)
    (local $75 i32)
    (local $76 i32)
    (local $77 i64)
    (local $78 i32)
    (local $79 i64)
    (local $80 i32)
    (local $81 i32)
    (local $82 i64)
    (local $83 i32)
    (local $84 i32)
    (local $85 i32)
    (local $86 i64)
    (local $87 i32)
    (local $88 i32)
    (local $89 i32)
    (local $90 i32)
    (local $91 i64)
    (local $92 i64)
    (local $93 i64)
    (local $94 i64)
    (local $95 i64)
    (local $96 i32)
    (local $97 i32)
    (local $98 i32)
    (local $99 i64)
    (local $100 i64)
    (local $101 i64)
    (local $102 i64)
    (local $103 i64)
    (local $104 i64)
    (local $105 i64)
    (local $106 i64)
    (local $107 i64)
    (local $108 i64)
    (local $109 i64)
    (local $110 i64)
    (local $111 i64)
    (local $112 i64)
    (local $113 i64)
    (local $114 i32)
    (local $115 i32)
    get_global $25
    set_local $2
    i32.const 224
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    i64.const 9214364837600034816
    set_local $5
    get_local $4
    get_local $1
    f64.store offset=216
    i32.const 64
    set_local $6
    get_local $4
    get_local $6
    i32.store offset=212
    i32.const 11
    set_local $7
    get_local $4
    get_local $7
    i32.store offset=208
    i32.const 2047
    set_local $8
    get_local $4
    get_local $8
    i32.store offset=204
    i32.const 1023
    set_local $9
    get_local $4
    get_local $9
    i32.store offset=200
    i64.const 4503599627370496
    set_local $10
    get_local $4
    get_local $10
    i64.store offset=192
    i64.const 9218868437227405312
    set_local $11
    get_local $4
    get_local $11
    i64.store offset=184
    i64.const -9223372036854775808
    set_local $12
    get_local $4
    get_local $12
    i64.store offset=176
    i64.const 9223372036854775807
    set_local $13
    get_local $4
    get_local $13
    i64.store offset=168
    i64.const 2251799813685248
    set_local $14
    get_local $4
    get_local $14
    i64.store offset=160
    i64.const 2251799813685247
    set_local $15
    get_local $4
    get_local $15
    i64.store offset=152
    i32.const 128
    set_local $16
    get_local $4
    get_local $16
    i32.store offset=148
    i32.const 15
    set_local $17
    get_local $4
    get_local $17
    i32.store offset=144
    i32.const 32767
    set_local $18
    get_local $4
    get_local $18
    i32.store offset=140
    i32.const 16383
    set_local $19
    get_local $4
    get_local $19
    i32.store offset=136
    i64.const 281474976710656
    set_local $20
    get_local $4
    get_local $20
    i64.store offset=120
    i64.const 0
    set_local $21
    get_local $4
    get_local $21
    i64.store offset=112
    get_local $4
    f64.load offset=216
    set_local $22
    get_local $22
    call $srcToRep
    set_local $23
    get_local $4
    get_local $23
    i64.store offset=104
    get_local $4
    i64.load offset=104
    set_local $24
    i64.const 9223372036854775807
    set_local $25
    get_local $24
    get_local $25
    i64.and
    set_local $26
    get_local $4
    get_local $26
    i64.store offset=96
    get_local $4
    i64.load offset=104
    set_local $27
    i64.const -9223372036854775808
    set_local $28
    get_local $27
    get_local $28
    i64.and
    set_local $29
    get_local $4
    get_local $29
    i64.store offset=88
    get_local $4
    i64.load offset=96
    set_local $30
    i64.const 4503599627370496
    set_local $31
    get_local $30
    get_local $31
    i64.sub
    set_local $32
    get_local $32
    set_local $33
    get_local $5
    set_local $34
    get_local $33
    get_local $34
    i64.lt_u
    set_local $35
    get_local $35
    set_local $36
    block $block
      block $block_0
        get_local $36
        i32.eqz
        br_if $block_0
        get_local $4
        i64.load offset=96
        set_local $37
        i64.const 60
        set_local $38
        get_local $37
        get_local $38
        i64.shl
        set_local $39
        i64.const 4
        set_local $40
        get_local $37
        get_local $40
        i64.shr_u
        set_local $41
        get_local $4
        get_local $41
        i64.store offset=72
        get_local $4
        get_local $39
        i64.store offset=64
        get_local $4
        i64.load offset=72
        set_local $42
        i64.const 4323455642275676160
        set_local $43
        get_local $42
        get_local $43
        i64.add
        set_local $44
        get_local $4
        get_local $44
        i64.store offset=72
        br $block
      end ;; $block_0
      i64.const 9218868437227405312
      set_local $45
      get_local $4
      i64.load offset=96
      set_local $46
      get_local $46
      set_local $47
      get_local $45
      set_local $48
      get_local $47
      get_local $48
      i64.ge_u
      set_local $49
      get_local $49
      set_local $50
      block $block_1
        block $block_2
          get_local $50
          i32.eqz
          br_if $block_2
          i64.const 9223090561878065152
          set_local $51
          get_local $4
          get_local $51
          i64.store offset=72
          i64.const 0
          set_local $52
          get_local $4
          get_local $52
          i64.store offset=64
          get_local $4
          i64.load offset=96
          set_local $53
          i64.const 2251799813685248
          set_local $54
          get_local $53
          get_local $54
          i64.and
          set_local $55
          i64.const 4
          set_local $56
          get_local $55
          get_local $56
          i64.shr_u
          set_local $57
          get_local $4
          i64.load offset=64
          set_local $58
          get_local $4
          i64.load offset=72
          set_local $59
          get_local $59
          get_local $57
          i64.or
          set_local $60
          get_local $4
          get_local $58
          i64.store offset=64
          get_local $4
          get_local $60
          i64.store offset=72
          get_local $4
          i64.load offset=96
          set_local $61
          i64.const 2251799813685247
          set_local $62
          get_local $61
          get_local $62
          i64.and
          set_local $63
          get_local $63
          get_local $56
          i64.shr_u
          set_local $64
          i64.const 60
          set_local $65
          get_local $63
          get_local $65
          i64.shl
          set_local $66
          get_local $4
          i64.load offset=72
          set_local $67
          get_local $4
          i64.load offset=64
          set_local $68
          get_local $68
          get_local $66
          i64.or
          set_local $69
          get_local $67
          get_local $64
          i64.or
          set_local $70
          get_local $4
          get_local $70
          i64.store offset=72
          get_local $4
          get_local $69
          i64.store offset=64
          br $block_1
        end ;; $block_2
        i64.const 0
        set_local $71
        get_local $4
        i64.load offset=96
        set_local $72
        get_local $72
        set_local $73
        get_local $71
        set_local $74
        get_local $73
        get_local $74
        i64.ne
        set_local $75
        get_local $75
        set_local $76
        block $block_3
          block $block_4
            get_local $76
            i32.eqz
            br_if $block_4
            get_local $4
            i64.load offset=96
            set_local $77
            get_local $77
            call $src_rep_t_clz
            set_local $78
            i64.const 4503599627370496
            set_local $79
            get_local $79
            call $src_rep_t_clz
            set_local $80
            get_local $78
            get_local $80
            i32.sub
            set_local $81
            get_local $4
            get_local $81
            i32.store offset=60
            get_local $4
            i64.load offset=96
            set_local $82
            get_local $4
            i32.load offset=60
            set_local $83
            i32.const 60
            set_local $84
            get_local $83
            get_local $84
            i32.add
            set_local $85
            i64.const 0
            set_local $86
            i32.const 16
            set_local $87
            get_local $4
            get_local $87
            i32.add
            set_local $88
            get_local $88
            get_local $82
            get_local $86
            get_local $85
            call $__ashlti3
            i32.const 24
            set_local $89
            get_local $4
            get_local $89
            i32.add
            set_local $90
            get_local $90
            i64.load align=1
            set_local $91
            get_local $4
            i64.load offset=16 align=1
            set_local $92
            get_local $4
            get_local $91
            i64.store offset=72
            get_local $4
            get_local $92
            i64.store offset=64
            get_local $4
            i64.load offset=72
            set_local $93
            i64.const 281474976710656
            set_local $94
            get_local $93
            get_local $94
            i64.xor
            set_local $95
            get_local $4
            get_local $95
            i64.store offset=72
            get_local $4
            i32.load offset=60
            set_local $96
            i32.const 15361
            set_local $97
            get_local $97
            get_local $96
            i32.sub
            set_local $98
            get_local $4
            get_local $98
            i32.store offset=56
            get_local $4
            i64.load32_u offset=56
            set_local $99
            i64.const 48
            set_local $100
            get_local $99
            get_local $100
            i64.shl
            set_local $101
            get_local $4
            i64.load offset=64
            set_local $102
            get_local $4
            i64.load offset=72
            set_local $103
            get_local $103
            get_local $101
            i64.or
            set_local $104
            get_local $4
            get_local $102
            i64.store offset=64
            get_local $4
            get_local $104
            i64.store offset=72
            br $block_3
          end ;; $block_4
          i64.const 0
          set_local $105
          get_local $4
          get_local $105
          i64.store offset=72
          get_local $4
          get_local $105
          i64.store offset=64
        end ;; $block_3
      end ;; $block_1
    end ;; $block
    get_local $4
    i64.load offset=64
    set_local $106
    get_local $4
    i64.load offset=72
    set_local $107
    get_local $4
    i64.load offset=88
    set_local $108
    get_local $107
    get_local $108
    i64.or
    set_local $109
    get_local $4
    get_local $106
    i64.store offset=32
    get_local $4
    get_local $109
    i64.store offset=40
    get_local $4
    i64.load offset=40
    set_local $110
    get_local $4
    i64.load offset=32
    set_local $111
    get_local $4
    get_local $111
    get_local $110
    call $dstFromRep
    get_local $4
    i64.load align=1
    set_local $112
    get_local $4
    i64.load offset=8 align=1
    set_local $113
    get_local $0
    get_local $113
    i64.store offset=8
    get_local $0
    get_local $112
    i64.store
    i32.const 224
    set_local $114
    get_local $4
    get_local $114
    i32.add
    set_local $115
    get_local $115
    set_global $25
    return
    )
  
  (func $srcToRep (type $19)
    (param $0 f64)
    (result i64)
    (local $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 f64)
    (local $5 i64)
    get_global $25
    set_local $1
    i32.const 16
    set_local $2
    get_local $1
    get_local $2
    i32.sub
    set_local $3
    get_local $3
    get_local $0
    f64.store offset=8
    get_local $3
    f64.load offset=8
    set_local $4
    get_local $3
    get_local $4
    f64.store
    get_local $3
    i64.load
    set_local $5
    get_local $5
    return
    )
  
  (func $src_rep_t_clz (type $20)
    (param $0 i64)
    (result i32)
    (local $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i64)
    (local $5 i64)
    (local $6 i64)
    (local $7 i64)
    (local $8 i32)
    (local $9 i32)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i32)
    (local $19 i32)
    get_global $25
    set_local $1
    i32.const 16
    set_local $2
    get_local $1
    get_local $2
    i32.sub
    set_local $3
    get_local $3
    get_local $0
    i64.store
    get_local $3
    i64.load32_u offset=4
    set_local $4
    i64.const 32
    set_local $5
    get_local $4
    get_local $5
    i64.rotl
    set_local $6
    i64.const 0
    set_local $7
    get_local $6
    get_local $7
    i64.ne
    set_local $8
    get_local $8
    set_local $9
    block $block
      block $block_0
        get_local $9
        i32.eqz
        br_if $block_0
        get_local $3
        i64.load
        set_local $10
        i64.const 32
        set_local $11
        get_local $10
        get_local $11
        i64.shr_u
        set_local $12
        get_local $12
        i32.wrap/i64
        set_local $13
        get_local $13
        i32.clz
        set_local $14
        get_local $3
        get_local $14
        i32.store offset=12
        br $block
      end ;; $block_0
      get_local $3
      i32.load
      set_local $15
      get_local $15
      i32.clz
      set_local $16
      i32.const 32
      set_local $17
      get_local $16
      get_local $17
      i32.add
      set_local $18
      get_local $3
      get_local $18
      i32.store offset=12
    end ;; $block
    get_local $3
    i32.load offset=12
    set_local $19
    get_local $19
    return
    )
  
  (func $dstFromRep (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $__fixtfsi (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i64)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    get_global $25
    set_local $2
    i32.const 16
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    get_local $4
    get_local $0
    i64.store
    get_local $4
    get_local $1
    i64.store offset=8
    get_local $4
    i64.load offset=8
    set_local $5
    get_local $4
    i64.load
    set_local $6
    get_local $6
    get_local $5
    call $__fixint
    set_local $7
    i32.const 16
    set_local $8
    get_local $4
    get_local $8
    i32.add
    set_local $9
    get_local $9
    set_global $25
    get_local $7
    return
    )
  
  (func $__fixint (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i64)
    (local $22 i64)
    (local $23 i32)
    (local $24 i32)
    (local $25 i32)
    (local $26 i32)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i64)
    (local $35 i64)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    (local $39 i32)
    (local $40 i32)
    (local $41 i32)
    (local $42 i32)
    (local $43 i32)
    (local $44 i32)
    (local $45 i32)
    (local $46 i32)
    (local $47 i32)
    (local $48 i32)
    (local $49 i32)
    (local $50 i32)
    (local $51 i32)
    (local $52 i32)
    (local $53 i32)
    (local $54 i32)
    (local $55 i32)
    (local $56 i32)
    (local $57 i32)
    (local $58 i32)
    (local $59 i32)
    (local $60 i32)
    (local $61 i32)
    (local $62 i32)
    (local $63 i64)
    (local $64 i64)
    (local $65 i64)
    (local $66 i32)
    (local $67 i32)
    (local $68 i32)
    (local $69 i32)
    (local $70 i32)
    (local $71 i32)
    (local $72 i32)
    (local $73 i64)
    (local $74 i64)
    (local $75 i64)
    (local $76 i32)
    (local $77 i32)
    (local $78 i32)
    (local $79 i32)
    (local $80 i32)
    (local $81 i32)
    (local $82 i32)
    (local $83 i32)
    (local $84 i32)
    (local $85 i32)
    (local $86 i32)
    get_global $25
    set_local $2
    i32.const 160
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    i32.const 0
    set_local $5
    get_local $4
    get_local $0
    i64.store offset=128
    get_local $4
    get_local $1
    i64.store offset=136
    i32.const 2147483647
    set_local $6
    get_local $4
    get_local $6
    i32.store offset=124
    i32.const -2147483648
    set_local $7
    get_local $4
    get_local $7
    i32.store offset=120
    get_local $4
    i64.load offset=136
    set_local $8
    get_local $4
    i64.load offset=128
    set_local $9
    i32.const 32
    set_local $10
    get_local $4
    get_local $10
    i32.add
    set_local $11
    get_local $11
    get_local $9
    get_local $8
    call $toRep_0
    i32.const 40
    set_local $12
    get_local $4
    get_local $12
    i32.add
    set_local $13
    get_local $13
    i64.load align=1
    set_local $14
    get_local $4
    i64.load offset=32 align=1
    set_local $15
    get_local $4
    get_local $14
    i64.store offset=104
    get_local $4
    get_local $15
    i64.store offset=96
    get_local $4
    i64.load offset=96
    set_local $16
    get_local $4
    i64.load offset=104
    set_local $17
    i64.const 9223372036854775807
    set_local $18
    get_local $17
    get_local $18
    i64.and
    set_local $19
    get_local $4
    get_local $16
    i64.store offset=80
    get_local $4
    get_local $19
    i64.store offset=88
    get_local $4
    i64.load offset=104
    set_local $20
    i64.const 63
    set_local $21
    get_local $20
    get_local $21
    i64.shr_u
    set_local $22
    get_local $22
    i32.wrap/i64
    set_local $23
    i32.const 1
    set_local $24
    i32.const -1
    set_local $25
    get_local $25
    get_local $24
    get_local $23
    select
    set_local $26
    get_local $4
    get_local $26
    i32.store offset=76
    get_local $4
    i64.load16_u offset=94
    set_local $27
    i64.const -16383
    set_local $28
    get_local $27
    get_local $28
    i64.add
    set_local $29
    get_local $4
    get_local $29
    i64.store32 offset=72
    get_local $4
    i64.load offset=80
    set_local $30
    get_local $4
    i64.load offset=88
    set_local $31
    i64.const 281474976710655
    set_local $32
    get_local $31
    get_local $32
    i64.and
    set_local $33
    i64.const 281474976710656
    set_local $34
    get_local $33
    get_local $34
    i64.or
    set_local $35
    get_local $4
    get_local $30
    i64.store offset=48
    get_local $4
    get_local $35
    i64.store offset=56
    get_local $4
    i32.load offset=72
    set_local $36
    get_local $36
    set_local $37
    get_local $5
    set_local $38
    get_local $37
    get_local $38
    i32.lt_s
    set_local $39
    get_local $39
    set_local $40
    block $block
      block $block_0
        get_local $40
        i32.eqz
        br_if $block_0
        i32.const 0
        set_local $41
        get_local $4
        get_local $41
        i32.store offset=156
        br $block
      end ;; $block_0
      i32.const 32
      set_local $42
      get_local $4
      i32.load offset=72
      set_local $43
      get_local $43
      set_local $44
      get_local $42
      set_local $45
      get_local $44
      get_local $45
      i32.ge_u
      set_local $46
      get_local $46
      set_local $47
      block $block_1
        get_local $47
        i32.eqz
        br_if $block_1
        i32.const 2147483647
        set_local $48
        i32.const -2147483648
        set_local $49
        i32.const 1
        set_local $50
        get_local $4
        i32.load offset=76
        set_local $51
        get_local $51
        set_local $52
        get_local $50
        set_local $53
        get_local $52
        get_local $53
        i32.eq
        set_local $54
        get_local $54
        set_local $55
        get_local $48
        get_local $49
        get_local $55
        select
        set_local $56
        get_local $4
        get_local $56
        i32.store offset=156
        br $block
      end ;; $block_1
      i32.const 112
      set_local $57
      get_local $4
      i32.load offset=72
      set_local $58
      get_local $58
      set_local $59
      get_local $57
      set_local $60
      get_local $59
      get_local $60
      i32.lt_s
      set_local $61
      get_local $61
      set_local $62
      block $block_2
        get_local $62
        i32.eqz
        br_if $block_2
        get_local $4
        i64.load32_u offset=76
        set_local $63
        get_local $4
        i64.load offset=56
        set_local $64
        get_local $4
        i64.load offset=48
        set_local $65
        get_local $4
        i32.load offset=72
        set_local $66
        i32.const 112
        set_local $67
        get_local $67
        get_local $66
        i32.sub
        set_local $68
        i32.const 16
        set_local $69
        get_local $4
        get_local $69
        i32.add
        set_local $70
        get_local $70
        get_local $65
        get_local $64
        get_local $68
        call $__lshrti3
        i32.const 24
        set_local $71
        get_local $4
        get_local $71
        i32.add
        set_local $72
        get_local $72
        i64.load align=1
        set_local $73
        get_local $4
        i64.load offset=16 align=1
        set_local $74
        get_local $4
        get_local $63
        get_local $75
        get_local $74
        get_local $73
        call $__multi3
        get_local $4
        i32.load align=1
        set_local $76
        get_local $4
        get_local $76
        i32.store offset=156
        br $block
      end ;; $block_2
      get_local $4
      i32.load offset=76
      set_local $77
      get_local $4
      i32.load offset=48
      set_local $78
      get_local $4
      i32.load offset=72
      set_local $79
      i32.const 112
      set_local $80
      get_local $79
      get_local $80
      i32.sub
      set_local $81
      get_local $78
      get_local $81
      i32.shl
      set_local $82
      get_local $77
      get_local $82
      i32.mul
      set_local $83
      get_local $4
      get_local $83
      i32.store offset=156
    end ;; $block
    get_local $4
    i32.load offset=156
    set_local $84
    i32.const 160
    set_local $85
    get_local $4
    get_local $85
    i32.add
    set_local $86
    get_local $86
    set_global $25
    get_local $84
    return
    )
  
  (func $toRep_0 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $__fixunstfsi (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i64)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    get_global $25
    set_local $2
    i32.const 16
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    get_local $4
    get_local $0
    i64.store
    get_local $4
    get_local $1
    i64.store offset=8
    get_local $4
    i64.load offset=8
    set_local $5
    get_local $4
    i64.load
    set_local $6
    get_local $6
    get_local $5
    call $__fixuint
    set_local $7
    i32.const 16
    set_local $8
    get_local $4
    get_local $8
    i32.add
    set_local $9
    get_local $9
    set_global $25
    get_local $7
    return
    )
  
  (func $__fixuint (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i32)
    (local $9 i32)
    (local $10 i32)
    (local $11 i32)
    (local $12 i64)
    (local $13 i64)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i32)
    (local $35 i32)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    (local $39 i32)
    (local $40 i32)
    (local $41 i32)
    (local $42 i32)
    (local $43 i32)
    (local $44 i32)
    (local $45 i32)
    (local $46 i32)
    (local $47 i32)
    (local $48 i32)
    (local $49 i32)
    (local $50 i32)
    (local $51 i32)
    (local $52 i32)
    (local $53 i32)
    (local $54 i32)
    (local $55 i32)
    (local $56 i32)
    (local $57 i32)
    (local $58 i32)
    (local $59 i64)
    (local $60 i64)
    (local $61 i32)
    (local $62 i32)
    (local $63 i32)
    (local $64 i32)
    (local $65 i32)
    (local $66 i32)
    (local $67 i32)
    (local $68 i32)
    (local $69 i32)
    (local $70 i32)
    (local $71 i32)
    (local $72 i32)
    get_global $25
    set_local $2
    i32.const 128
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    i32.const -1
    set_local $5
    get_local $4
    get_local $0
    i64.store offset=96
    get_local $4
    get_local $1
    i64.store offset=104
    get_local $4
    i64.load offset=104
    set_local $6
    get_local $4
    i64.load offset=96
    set_local $7
    i32.const 16
    set_local $8
    get_local $4
    get_local $8
    i32.add
    set_local $9
    get_local $9
    get_local $7
    get_local $6
    call $toRep_1
    i32.const 24
    set_local $10
    get_local $4
    get_local $10
    i32.add
    set_local $11
    get_local $11
    i64.load align=1
    set_local $12
    get_local $4
    i64.load offset=16 align=1
    set_local $13
    get_local $4
    get_local $12
    i64.store offset=88
    get_local $4
    get_local $13
    i64.store offset=80
    get_local $4
    i64.load offset=80
    set_local $14
    get_local $4
    i64.load offset=88
    set_local $15
    i64.const 9223372036854775807
    set_local $16
    get_local $15
    get_local $16
    i64.and
    set_local $17
    get_local $4
    get_local $14
    i64.store offset=64
    get_local $4
    get_local $17
    i64.store offset=72
    get_local $4
    i64.load offset=88
    set_local $18
    i64.const 63
    set_local $19
    get_local $18
    get_local $19
    i64.shr_u
    set_local $20
    get_local $20
    i32.wrap/i64
    set_local $21
    i32.const 1
    set_local $22
    i32.const -1
    set_local $23
    get_local $23
    get_local $22
    get_local $21
    select
    set_local $24
    get_local $4
    get_local $24
    i32.store offset=60
    get_local $4
    i64.load16_u offset=78
    set_local $25
    i64.const -16383
    set_local $26
    get_local $25
    get_local $26
    i64.add
    set_local $27
    get_local $4
    get_local $27
    i64.store32 offset=56
    get_local $4
    i64.load offset=64
    set_local $28
    get_local $4
    i64.load offset=72
    set_local $29
    i64.const 281474976710655
    set_local $30
    get_local $29
    get_local $30
    i64.and
    set_local $31
    i64.const 281474976710656
    set_local $32
    get_local $31
    get_local $32
    i64.or
    set_local $33
    get_local $4
    get_local $28
    i64.store offset=32
    get_local $4
    get_local $33
    i64.store offset=40
    get_local $4
    i32.load offset=60
    set_local $34
    get_local $34
    set_local $35
    get_local $5
    set_local $36
    get_local $35
    get_local $36
    i32.eq
    set_local $37
    get_local $37
    set_local $38
    block $block
      block $block_0
        block $block_1
          get_local $38
          br_if $block_1
          i32.const 0
          set_local $39
          get_local $4
          i32.load offset=56
          set_local $40
          get_local $40
          set_local $41
          get_local $39
          set_local $42
          get_local $41
          get_local $42
          i32.lt_s
          set_local $43
          get_local $43
          set_local $44
          get_local $44
          i32.eqz
          br_if $block_0
        end ;; $block_1
        i32.const 0
        set_local $45
        get_local $4
        get_local $45
        i32.store offset=124
        br $block
      end ;; $block_0
      i32.const 32
      set_local $46
      get_local $4
      i32.load offset=56
      set_local $47
      get_local $47
      set_local $48
      get_local $46
      set_local $49
      get_local $48
      get_local $49
      i32.ge_u
      set_local $50
      get_local $50
      set_local $51
      block $block_2
        get_local $51
        i32.eqz
        br_if $block_2
        i32.const -1
        set_local $52
        get_local $4
        get_local $52
        i32.store offset=124
        br $block
      end ;; $block_2
      i32.const 112
      set_local $53
      get_local $4
      i32.load offset=56
      set_local $54
      get_local $54
      set_local $55
      get_local $53
      set_local $56
      get_local $55
      get_local $56
      i32.lt_s
      set_local $57
      get_local $57
      set_local $58
      block $block_3
        get_local $58
        i32.eqz
        br_if $block_3
        get_local $4
        i64.load offset=40
        set_local $59
        get_local $4
        i64.load offset=32
        set_local $60
        get_local $4
        i32.load offset=56
        set_local $61
        i32.const 112
        set_local $62
        get_local $62
        get_local $61
        i32.sub
        set_local $63
        get_local $4
        get_local $60
        get_local $59
        get_local $63
        call $__lshrti3
        get_local $4
        i32.load align=1
        set_local $64
        get_local $4
        get_local $64
        i32.store offset=124
        br $block
      end ;; $block_3
      get_local $4
      i32.load offset=32
      set_local $65
      get_local $4
      i32.load offset=56
      set_local $66
      i32.const 112
      set_local $67
      get_local $66
      get_local $67
      i32.sub
      set_local $68
      get_local $65
      get_local $68
      i32.shl
      set_local $69
      get_local $4
      get_local $69
      i32.store offset=124
    end ;; $block
    get_local $4
    i32.load offset=124
    set_local $70
    i32.const 128
    set_local $71
    get_local $4
    get_local $71
    i32.add
    set_local $72
    get_local $72
    set_global $25
    get_local $70
    return
    )
  
  (func $toRep_1 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $__floatsitf (type $9)
    (param $0 i32)
    (param $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i64)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i64)
    (local $19 i64)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i32)
    (local $26 i32)
    (local $27 i32)
    (local $28 i32)
    (local $29 i32)
    (local $30 i32)
    (local $31 i32)
    (local $32 i64)
    (local $33 i32)
    (local $34 i64)
    (local $35 i32)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i32)
    (local $44 i32)
    (local $45 i32)
    (local $46 i64)
    (local $47 i64)
    (local $48 i64)
    (local $49 i64)
    (local $50 i64)
    (local $51 i64)
    (local $52 i64)
    (local $53 i64)
    (local $54 i64)
    (local $55 i64)
    (local $56 i64)
    (local $57 i64)
    (local $58 i32)
    (local $59 i32)
    (local $60 i64)
    (local $61 i64)
    (local $62 i64)
    (local $63 i64)
    (local $64 i32)
    (local $65 i32)
    get_global $25
    set_local $2
    i32.const 144
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    i32.const 32
    set_local $5
    get_local $4
    get_local $1
    i32.store offset=124
    get_local $4
    get_local $5
    i32.store offset=120
    get_local $4
    i32.load offset=124
    set_local $6
    block $block
      block $block_0
        get_local $6
        br_if $block_0
        i64.const 0
        set_local $7
        get_local $4
        get_local $7
        get_local $7
        call $fromRep
        get_local $4
        i64.load align=1
        set_local $8
        get_local $4
        i64.load offset=8 align=1
        set_local $9
        get_local $4
        get_local $9
        i64.store offset=136
        get_local $4
        get_local $8
        i64.store offset=128
        br $block
      end ;; $block_0
      i32.const 0
      set_local $10
      i64.const 0
      set_local $11
      get_local $4
      get_local $11
      i64.store offset=104
      get_local $4
      get_local $11
      i64.store offset=96
      get_local $4
      i32.load offset=124
      set_local $12
      get_local $4
      get_local $12
      i32.store offset=92
      get_local $4
      i32.load offset=124
      set_local $13
      get_local $13
      set_local $14
      get_local $10
      set_local $15
      get_local $14
      get_local $15
      i32.lt_s
      set_local $16
      get_local $16
      set_local $17
      block $block_1
        get_local $17
        i32.eqz
        br_if $block_1
        i64.const -9223372036854775808
        set_local $18
        get_local $4
        get_local $18
        i64.store offset=104
        i64.const 0
        set_local $19
        get_local $4
        get_local $19
        i64.store offset=96
        get_local $4
        i32.load offset=124
        set_local $20
        i32.const -1
        set_local $21
        get_local $20
        get_local $21
        i32.xor
        set_local $22
        i32.const 1
        set_local $23
        get_local $22
        get_local $23
        i32.add
        set_local $24
        get_local $4
        get_local $24
        i32.store offset=92
      end ;; $block_1
      get_local $4
      i32.load offset=92
      set_local $25
      get_local $25
      i32.clz
      set_local $26
      i32.const 31
      set_local $27
      get_local $27
      get_local $26
      i32.sub
      set_local $28
      get_local $4
      get_local $28
      i32.store offset=88
      get_local $4
      i32.load offset=88
      set_local $29
      i32.const 112
      set_local $30
      get_local $30
      get_local $29
      i32.sub
      set_local $31
      get_local $4
      get_local $31
      i32.store offset=60
      get_local $4
      i64.load32_u offset=92
      set_local $32
      get_local $4
      i32.load offset=60
      set_local $33
      i64.const 0
      set_local $34
      i32.const 16
      set_local $35
      get_local $4
      get_local $35
      i32.add
      set_local $36
      get_local $36
      get_local $32
      get_local $34
      get_local $33
      call $__ashlti3
      i32.const 24
      set_local $37
      get_local $4
      get_local $37
      i32.add
      set_local $38
      get_local $38
      i64.load align=1
      set_local $39
      get_local $4
      i64.load offset=16 align=1
      set_local $40
      i64.const 281474976710656
      set_local $41
      get_local $39
      get_local $41
      i64.xor
      set_local $42
      get_local $4
      get_local $40
      i64.store offset=64
      get_local $4
      get_local $42
      i64.store offset=72
      get_local $4
      i32.load offset=88
      set_local $43
      i32.const 16383
      set_local $44
      get_local $43
      get_local $44
      i32.add
      set_local $45
      get_local $45
      i64.extend_u/i32
      set_local $46
      i64.const 48
      set_local $47
      get_local $46
      get_local $47
      i64.shl
      set_local $48
      get_local $4
      i64.load offset=64
      set_local $49
      get_local $4
      i64.load offset=72
      set_local $50
      get_local $50
      get_local $48
      i64.add
      set_local $51
      get_local $4
      get_local $49
      i64.store offset=64
      get_local $4
      get_local $51
      i64.store offset=72
      get_local $4
      i64.load offset=64
      set_local $52
      get_local $4
      i64.load offset=72
      set_local $53
      get_local $4
      i64.load offset=96
      set_local $54
      get_local $4
      i64.load offset=104
      set_local $55
      get_local $53
      get_local $55
      i64.or
      set_local $56
      get_local $52
      get_local $54
      i64.or
      set_local $57
      i32.const 32
      set_local $58
      get_local $4
      get_local $58
      i32.add
      set_local $59
      get_local $59
      get_local $57
      get_local $56
      call $fromRep
      get_local $4
      i64.load offset=32 align=1
      set_local $60
      get_local $4
      i64.load offset=40 align=1
      set_local $61
      get_local $4
      get_local $61
      i64.store offset=136
      get_local $4
      get_local $60
      i64.store offset=128
    end ;; $block
    get_local $4
    i64.load offset=128
    set_local $62
    get_local $4
    i64.load offset=136
    set_local $63
    get_local $0
    get_local $63
    i64.store offset=8
    get_local $0
    get_local $62
    i64.store
    i32.const 144
    set_local $64
    get_local $4
    get_local $64
    i32.add
    set_local $65
    get_local $65
    set_global $25
    return
    )
  
  (func $fromRep (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $__floatunsitf (type $9)
    (param $0 i32)
    (param $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i64)
    (local $18 i32)
    (local $19 i64)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i32)
    (local $29 i32)
    (local $30 i32)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i64)
    (local $35 i64)
    (local $36 i64)
    (local $37 i64)
    (local $38 i64)
    (local $39 i32)
    (local $40 i32)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    (local $45 i32)
    (local $46 i32)
    get_global $25
    set_local $2
    i32.const 112
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    get_local $4
    set_global $25
    i32.const 32
    set_local $5
    get_local $4
    get_local $1
    i32.store offset=92
    get_local $4
    get_local $5
    i32.store offset=88
    get_local $4
    i32.load offset=92
    set_local $6
    block $block
      block $block_0
        get_local $6
        br_if $block_0
        i64.const 0
        set_local $7
        get_local $4
        get_local $7
        get_local $7
        call $fromRep_0
        get_local $4
        i64.load align=1
        set_local $8
        get_local $4
        i64.load offset=8 align=1
        set_local $9
        get_local $4
        get_local $9
        i64.store offset=104
        get_local $4
        get_local $8
        i64.store offset=96
        br $block
      end ;; $block_0
      get_local $4
      i32.load offset=92
      set_local $10
      get_local $10
      i32.clz
      set_local $11
      i32.const 31
      set_local $12
      get_local $12
      get_local $11
      i32.sub
      set_local $13
      get_local $4
      get_local $13
      i32.store offset=84
      get_local $4
      i32.load offset=84
      set_local $14
      i32.const 112
      set_local $15
      get_local $15
      get_local $14
      i32.sub
      set_local $16
      get_local $4
      get_local $16
      i32.store offset=60
      get_local $4
      i64.load32_u offset=92
      set_local $17
      get_local $4
      i32.load offset=60
      set_local $18
      i64.const 0
      set_local $19
      i32.const 16
      set_local $20
      get_local $4
      get_local $20
      i32.add
      set_local $21
      get_local $21
      get_local $17
      get_local $19
      get_local $18
      call $__ashlti3
      i32.const 24
      set_local $22
      get_local $4
      get_local $22
      i32.add
      set_local $23
      get_local $23
      i64.load align=1
      set_local $24
      get_local $4
      i64.load offset=16 align=1
      set_local $25
      i64.const 281474976710656
      set_local $26
      get_local $24
      get_local $26
      i64.xor
      set_local $27
      get_local $4
      get_local $25
      i64.store offset=64
      get_local $4
      get_local $27
      i64.store offset=72
      get_local $4
      i32.load offset=84
      set_local $28
      i32.const 16383
      set_local $29
      get_local $28
      get_local $29
      i32.add
      set_local $30
      get_local $30
      i64.extend_u/i32
      set_local $31
      i64.const 48
      set_local $32
      get_local $31
      get_local $32
      i64.shl
      set_local $33
      get_local $4
      i64.load offset=64
      set_local $34
      get_local $4
      i64.load offset=72
      set_local $35
      get_local $35
      get_local $33
      i64.add
      set_local $36
      get_local $4
      get_local $34
      i64.store offset=64
      get_local $4
      get_local $36
      i64.store offset=72
      get_local $4
      i64.load offset=72
      set_local $37
      get_local $4
      i64.load offset=64
      set_local $38
      i32.const 32
      set_local $39
      get_local $4
      get_local $39
      i32.add
      set_local $40
      get_local $40
      get_local $38
      get_local $37
      call $fromRep_0
      get_local $4
      i64.load offset=32 align=1
      set_local $41
      get_local $4
      i64.load offset=40 align=1
      set_local $42
      get_local $4
      get_local $42
      i64.store offset=104
      get_local $4
      get_local $41
      i64.store offset=96
    end ;; $block
    get_local $4
    i64.load offset=96
    set_local $43
    get_local $4
    i64.load offset=104
    set_local $44
    get_local $0
    get_local $44
    i64.store offset=8
    get_local $0
    get_local $43
    i64.store
    i32.const 112
    set_local $45
    get_local $4
    get_local $45
    i32.add
    set_local $46
    get_local $46
    set_global $25
    return
    )
  
  (func $fromRep_0 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $__addtf3 (type $21)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i32)
    (local $15 i32)
    get_global $25
    set_local $5
    i32.const 48
    set_local $6
    get_local $5
    get_local $6
    i32.sub
    set_local $7
    get_local $7
    set_global $25
    get_local $7
    get_local $1
    i64.store offset=32
    get_local $7
    get_local $2
    i64.store offset=40
    get_local $7
    get_local $4
    i64.store offset=24
    get_local $7
    get_local $3
    i64.store offset=16
    get_local $7
    i64.load offset=24
    set_local $8
    get_local $7
    i64.load offset=16
    set_local $9
    get_local $7
    i64.load offset=40
    set_local $10
    get_local $7
    i64.load offset=32
    set_local $11
    get_local $7
    get_local $11
    get_local $10
    get_local $9
    get_local $8
    call $__addXf3__
    get_local $7
    i64.load align=1
    set_local $12
    get_local $7
    i64.load offset=8 align=1
    set_local $13
    get_local $0
    get_local $13
    i64.store offset=8
    get_local $0
    get_local $12
    i64.store
    i32.const 48
    set_local $14
    get_local $7
    get_local $14
    i32.add
    set_local $15
    get_local $15
    set_global $25
    return
    )
  
  (func $__addXf3__ (type $21)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i64)
    (local $35 i64)
    (local $36 i64)
    (local $37 i64)
    (local $38 i32)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i32)
    (local $43 i64)
    (local $44 i32)
    (local $45 i32)
    (local $46 i32)
    (local $47 i32)
    (local $48 i64)
    (local $49 i64)
    (local $50 i64)
    (local $51 i64)
    (local $52 i32)
    (local $53 i64)
    (local $54 i64)
    (local $55 i64)
    (local $56 i32)
    (local $57 i64)
    (local $58 i32)
    (local $59 i32)
    (local $60 i32)
    (local $61 i32)
    (local $62 i64)
    (local $63 i64)
    (local $64 i64)
    (local $65 i32)
    (local $66 i32)
    (local $67 i64)
    (local $68 i32)
    (local $69 i32)
    (local $70 i32)
    (local $71 i64)
    (local $72 i64)
    (local $73 i32)
    (local $74 i32)
    (local $75 i32)
    (local $76 i32)
    (local $77 i64)
    (local $78 i64)
    (local $79 i64)
    (local $80 i64)
    (local $81 i64)
    (local $82 i64)
    (local $83 i64)
    (local $84 i64)
    (local $85 i64)
    (local $86 i32)
    (local $87 i32)
    (local $88 i64)
    (local $89 i32)
    (local $90 i32)
    (local $91 i32)
    (local $92 i64)
    (local $93 i64)
    (local $94 i32)
    (local $95 i32)
    (local $96 i32)
    (local $97 i32)
    (local $98 i64)
    (local $99 i64)
    (local $100 i64)
    (local $101 i64)
    (local $102 i32)
    (local $103 i32)
    (local $104 i64)
    (local $105 i64)
    (local $106 i64)
    (local $107 i64)
    (local $108 i64)
    (local $109 i64)
    (local $110 i64)
    (local $111 i32)
    (local $112 i32)
    (local $113 i64)
    (local $114 i64)
    (local $115 i32)
    (local $116 i32)
    (local $117 i32)
    (local $118 i32)
    (local $119 i32)
    (local $120 i32)
    (local $121 i64)
    (local $122 i64)
    (local $123 i64)
    (local $124 i64)
    (local $125 i32)
    (local $126 i32)
    (local $127 i32)
    (local $128 i32)
    (local $129 i32)
    (local $130 i64)
    (local $131 i64)
    (local $132 i64)
    (local $133 i64)
    (local $134 i64)
    (local $135 i64)
    (local $136 i64)
    (local $137 i32)
    (local $138 i32)
    (local $139 i64)
    (local $140 i64)
    (local $141 i32)
    (local $142 i32)
    (local $143 i64)
    (local $144 i64)
    (local $145 i64)
    (local $146 i64)
    (local $147 i64)
    (local $148 i64)
    (local $149 i64)
    (local $150 i64)
    (local $151 i64)
    (local $152 i32)
    (local $153 i32)
    (local $154 i64)
    (local $155 i64)
    (local $156 i64)
    (local $157 i64)
    (local $158 i64)
    (local $159 i64)
    (local $160 i32)
    (local $161 i32)
    (local $162 i64)
    (local $163 i64)
    (local $164 i64)
    (local $165 i64)
    (local $166 i32)
    (local $167 i32)
    (local $168 i64)
    (local $169 i64)
    (local $170 i32)
    (local $171 i32)
    (local $172 i32)
    (local $173 i32)
    (local $174 i32)
    (local $175 i32)
    (local $176 i64)
    (local $177 i64)
    (local $178 i64)
    (local $179 i64)
    (local $180 i32)
    (local $181 i32)
    (local $182 i32)
    (local $183 i32)
    (local $184 i32)
    (local $185 i64)
    (local $186 i64)
    (local $187 i64)
    (local $188 i64)
    (local $189 i32)
    (local $190 i32)
    (local $191 i64)
    (local $192 i64)
    (local $193 i64)
    (local $194 i64)
    (local $195 i64)
    (local $196 i64)
    (local $197 i64)
    (local $198 i64)
    (local $199 i32)
    (local $200 i32)
    (local $201 i64)
    (local $202 i64)
    (local $203 i64)
    (local $204 i64)
    (local $205 i64)
    (local $206 i64)
    (local $207 i32)
    (local $208 i32)
    (local $209 i32)
    (local $210 i32)
    (local $211 i32)
    (local $212 i64)
    (local $213 i64)
    (local $214 i64)
    (local $215 i64)
    (local $216 i64)
    (local $217 i64)
    (local $218 i64)
    (local $219 i64)
    (local $220 i64)
    (local $221 i64)
    (local $222 i64)
    (local $223 i64)
    (local $224 i64)
    (local $225 i64)
    (local $226 i64)
    (local $227 i64)
    (local $228 i64)
    (local $229 i64)
    (local $230 i32)
    (local $231 i32)
    (local $232 i32)
    (local $233 i32)
    (local $234 i32)
    (local $235 i32)
    (local $236 i32)
    (local $237 i32)
    (local $238 i32)
    (local $239 i32)
    (local $240 i64)
    (local $241 i64)
    (local $242 i64)
    (local $243 i64)
    (local $244 i64)
    (local $245 i64)
    (local $246 i64)
    (local $247 i64)
    (local $248 i64)
    (local $249 i64)
    (local $250 i64)
    (local $251 i64)
    (local $252 i64)
    (local $253 i64)
    (local $254 i64)
    (local $255 i64)
    (local $256 i64)
    (local $257 i64)
    (local $258 i64)
    (local $259 i64)
    (local $260 i64)
    (local $261 i64)
    (local $262 i64)
    (local $263 i64)
    (local $264 i64)
    (local $265 i64)
    (local $266 i32)
    (local $267 i32)
    (local $268 i32)
    (local $269 i32)
    (local $270 i32)
    (local $271 i32)
    (local $272 i32)
    (local $273 i32)
    (local $274 i32)
    (local $275 i32)
    (local $276 i64)
    (local $277 i64)
    (local $278 i32)
    (local $279 i32)
    (local $280 i32)
    (local $281 i32)
    (local $282 i32)
    (local $283 i32)
    (local $284 i32)
    (local $285 i32)
    (local $286 i32)
    (local $287 i64)
    (local $288 i64)
    (local $289 i64)
    (local $290 i64)
    (local $291 i32)
    (local $292 i64)
    (local $293 i64)
    (local $294 i32)
    (local $295 i32)
    (local $296 i32)
    (local $297 i32)
    (local $298 i32)
    (local $299 i32)
    (local $300 i64)
    (local $301 i64)
    (local $302 i64)
    (local $303 i64)
    (local $304 i64)
    (local $305 i64)
    (local $306 i64)
    (local $307 i64)
    (local $308 i32)
    (local $309 i32)
    (local $310 i32)
    (local $311 i64)
    (local $312 i64)
    (local $313 i64)
    (local $314 i64)
    (local $315 i64)
    (local $316 i32)
    (local $317 i64)
    (local $318 i64)
    (local $319 i64)
    (local $320 i64)
    (local $321 i64)
    (local $322 i64)
    (local $323 i32)
    (local $324 i32)
    (local $325 i64)
    (local $326 i32)
    (local $327 i32)
    (local $328 i64)
    (local $329 i64)
    (local $330 i64)
    (local $331 i64)
    (local $332 i32)
    (local $333 i32)
    (local $334 i64)
    (local $335 i64)
    (local $336 i32)
    (local $337 i64)
    (local $338 i64)
    (local $339 i32)
    (local $340 i32)
    (local $341 i32)
    (local $342 i64)
    (local $343 i64)
    (local $344 i32)
    (local $345 i32)
    (local $346 i32)
    (local $347 i32)
    (local $348 i64)
    (local $349 i64)
    (local $350 i32)
    (local $351 i32)
    (local $352 i32)
    (local $353 i64)
    (local $354 i64)
    (local $355 i64)
    (local $356 i64)
    (local $357 i64)
    (local $358 i64)
    (local $359 i32)
    (local $360 i64)
    (local $361 i64)
    (local $362 i32)
    (local $363 i32)
    (local $364 i32)
    (local $365 i32)
    (local $366 i32)
    (local $367 i32)
    (local $368 i64)
    (local $369 i64)
    (local $370 i64)
    (local $371 i64)
    (local $372 i64)
    (local $373 i64)
    (local $374 i64)
    (local $375 i64)
    (local $376 i64)
    (local $377 i64)
    (local $378 i64)
    (local $379 i64)
    (local $380 i64)
    (local $381 i32)
    (local $382 i32)
    (local $383 i32)
    (local $384 i32)
    (local $385 i32)
    (local $386 i32)
    (local $387 i32)
    (local $388 i32)
    (local $389 i32)
    (local $390 i64)
    (local $391 i64)
    (local $392 i64)
    (local $393 i64)
    (local $394 i32)
    (local $395 i32)
    (local $396 i64)
    (local $397 i64)
    (local $398 i32)
    (local $399 i32)
    (local $400 i32)
    (local $401 i32)
    (local $402 i32)
    (local $403 i32)
    (local $404 i32)
    (local $405 i32)
    (local $406 i32)
    (local $407 i32)
    (local $408 i64)
    (local $409 i64)
    (local $410 i32)
    (local $411 i32)
    (local $412 i32)
    (local $413 i32)
    (local $414 i32)
    (local $415 i32)
    (local $416 i32)
    (local $417 i32)
    (local $418 i32)
    (local $419 i64)
    (local $420 i64)
    (local $421 i64)
    (local $422 i64)
    (local $423 i32)
    (local $424 i64)
    (local $425 i64)
    (local $426 i32)
    (local $427 i32)
    (local $428 i32)
    (local $429 i32)
    (local $430 i32)
    (local $431 i32)
    (local $432 i64)
    (local $433 i64)
    (local $434 i64)
    (local $435 i64)
    (local $436 i64)
    (local $437 i64)
    (local $438 i32)
    (local $439 i64)
    (local $440 i64)
    (local $441 i64)
    (local $442 i64)
    (local $443 i64)
    (local $444 i64)
    (local $445 i64)
    (local $446 i64)
    (local $447 i64)
    (local $448 i64)
    (local $449 i64)
    (local $450 i64)
    (local $451 i64)
    (local $452 i64)
    (local $453 i64)
    (local $454 i64)
    (local $455 i64)
    (local $456 i64)
    (local $457 i64)
    (local $458 i64)
    (local $459 i64)
    (local $460 i64)
    (local $461 i64)
    (local $462 i64)
    (local $463 i64)
    (local $464 i32)
    (local $465 i32)
    (local $466 i32)
    (local $467 i32)
    (local $468 i32)
    (local $469 i64)
    (local $470 i64)
    (local $471 i64)
    (local $472 i64)
    (local $473 i32)
    (local $474 i64)
    (local $475 i64)
    (local $476 i32)
    (local $477 i32)
    (local $478 i32)
    (local $479 i32)
    (local $480 i32)
    (local $481 i32)
    (local $482 i64)
    (local $483 i64)
    (local $484 i64)
    (local $485 i64)
    (local $486 i64)
    (local $487 i32)
    (local $488 i64)
    (local $489 i64)
    (local $490 i64)
    (local $491 i64)
    (local $492 i32)
    (local $493 i32)
    (local $494 i64)
    (local $495 i64)
    (local $496 i64)
    (local $497 i64)
    (local $498 i32)
    (local $499 i32)
    get_global $25
    set_local $5
    i32.const 560
    set_local $6
    get_local $5
    get_local $6
    i32.sub
    set_local $7
    get_local $7
    set_global $25
    get_local $7
    get_local $1
    i64.store offset=528
    get_local $7
    get_local $2
    i64.store offset=536
    get_local $7
    get_local $4
    i64.store offset=520
    get_local $7
    get_local $3
    i64.store offset=512
    get_local $7
    i64.load offset=536
    set_local $8
    get_local $7
    i64.load offset=528
    set_local $9
    i32.const 304
    set_local $10
    get_local $7
    get_local $10
    i32.add
    set_local $11
    get_local $11
    get_local $9
    get_local $8
    call $toRep_2
    i32.const 8
    set_local $12
    i32.const 304
    set_local $13
    get_local $7
    get_local $13
    i32.add
    set_local $14
    get_local $14
    get_local $12
    i32.add
    set_local $15
    get_local $15
    i64.load align=1
    set_local $16
    get_local $7
    i64.load offset=304 align=1
    set_local $17
    get_local $7
    get_local $16
    i64.store offset=504
    get_local $7
    get_local $17
    i64.store offset=496
    get_local $7
    i64.load offset=520
    set_local $18
    get_local $7
    i64.load offset=512
    set_local $19
    i32.const 288
    set_local $20
    get_local $7
    get_local $20
    i32.add
    set_local $21
    get_local $21
    get_local $19
    get_local $18
    call $toRep_2
    i32.const 288
    set_local $22
    get_local $7
    get_local $22
    i32.add
    set_local $23
    get_local $23
    get_local $12
    i32.add
    set_local $24
    get_local $24
    i64.load align=1
    set_local $25
    get_local $7
    i64.load offset=288 align=1
    set_local $26
    get_local $7
    get_local $25
    i64.store offset=488
    get_local $7
    get_local $26
    i64.store offset=480
    get_local $7
    i64.load offset=496
    set_local $27
    get_local $7
    i64.load offset=504
    set_local $28
    i64.const 9223372036854775807
    set_local $29
    get_local $28
    get_local $29
    i64.and
    set_local $30
    get_local $7
    get_local $27
    i64.store offset=464
    get_local $7
    get_local $30
    i64.store offset=472
    get_local $7
    i64.load offset=480
    set_local $31
    get_local $7
    i64.load offset=488
    set_local $32
    get_local $32
    get_local $29
    i64.and
    set_local $33
    get_local $7
    get_local $31
    i64.store offset=448
    get_local $7
    get_local $33
    i64.store offset=456
    get_local $7
    i64.load offset=472
    set_local $34
    get_local $7
    i64.load offset=464
    set_local $35
    i64.const -1
    set_local $36
    get_local $35
    get_local $36
    i64.add
    set_local $37
    get_local $37
    get_local $35
    i64.lt_u
    set_local $38
    get_local $38
    i64.extend_u/i32
    set_local $39
    get_local $34
    get_local $39
    i64.add
    set_local $40
    get_local $40
    get_local $36
    i64.add
    set_local $41
    get_local $37
    get_local $36
    i64.eq
    set_local $42
    i64.const 9223090561878065151
    set_local $43
    get_local $41
    get_local $43
    i64.eq
    set_local $44
    get_local $41
    get_local $43
    i64.gt_u
    set_local $45
    get_local $42
    get_local $45
    get_local $44
    select
    set_local $46
    get_local $46
    set_local $47
    block $block
      block $block_0
        block $block_1
          get_local $47
          br_if $block_1
          get_local $7
          i64.load offset=456
          set_local $48
          get_local $7
          i64.load offset=448
          set_local $49
          i64.const -1
          set_local $50
          get_local $49
          get_local $50
          i64.add
          set_local $51
          get_local $51
          get_local $49
          i64.lt_u
          set_local $52
          get_local $52
          i64.extend_u/i32
          set_local $53
          get_local $48
          get_local $53
          i64.add
          set_local $54
          get_local $54
          get_local $50
          i64.add
          set_local $55
          get_local $51
          get_local $50
          i64.eq
          set_local $56
          i64.const 9223090561878065151
          set_local $57
          get_local $55
          get_local $57
          i64.eq
          set_local $58
          get_local $55
          get_local $57
          i64.gt_u
          set_local $59
          get_local $56
          get_local $59
          get_local $58
          select
          set_local $60
          get_local $60
          set_local $61
          get_local $61
          i32.eqz
          br_if $block_0
        end ;; $block_1
        get_local $7
        i64.load offset=464
        set_local $62
        get_local $7
        i64.load offset=472
        set_local $63
        i64.const 9223090561878065152
        set_local $64
        get_local $63
        get_local $64
        i64.eq
        set_local $65
        get_local $63
        get_local $64
        i64.gt_u
        set_local $66
        i64.const 0
        set_local $67
        get_local $62
        get_local $67
        i64.ne
        set_local $68
        get_local $68
        get_local $66
        get_local $65
        select
        set_local $69
        get_local $69
        set_local $70
        block $block_2
          get_local $70
          i32.eqz
          br_if $block_2
          get_local $7
          i64.load offset=536
          set_local $71
          get_local $7
          i64.load offset=528
          set_local $72
          i32.const 16
          set_local $73
          get_local $7
          get_local $73
          i32.add
          set_local $74
          get_local $74
          get_local $72
          get_local $71
          call $toRep_2
          i32.const 24
          set_local $75
          get_local $7
          get_local $75
          i32.add
          set_local $76
          get_local $76
          i64.load align=1
          set_local $77
          get_local $7
          i64.load offset=16 align=1
          set_local $78
          i64.const 140737488355328
          set_local $79
          get_local $77
          get_local $79
          i64.or
          set_local $80
          get_local $7
          get_local $78
          get_local $80
          call $fromRep_1
          get_local $7
          i64.load align=1
          set_local $81
          get_local $7
          i64.load offset=8 align=1
          set_local $82
          get_local $7
          get_local $82
          i64.store offset=552
          get_local $7
          get_local $81
          i64.store offset=544
          br $block
        end ;; $block_2
        get_local $7
        i64.load offset=448
        set_local $83
        get_local $7
        i64.load offset=456
        set_local $84
        i64.const 9223090561878065152
        set_local $85
        get_local $84
        get_local $85
        i64.eq
        set_local $86
        get_local $84
        get_local $85
        i64.gt_u
        set_local $87
        i64.const 0
        set_local $88
        get_local $83
        get_local $88
        i64.ne
        set_local $89
        get_local $89
        get_local $87
        get_local $86
        select
        set_local $90
        get_local $90
        set_local $91
        block $block_3
          get_local $91
          i32.eqz
          br_if $block_3
          get_local $7
          i64.load offset=520
          set_local $92
          get_local $7
          i64.load offset=512
          set_local $93
          i32.const 48
          set_local $94
          get_local $7
          get_local $94
          i32.add
          set_local $95
          get_local $95
          get_local $93
          get_local $92
          call $toRep_2
          i32.const 56
          set_local $96
          get_local $7
          get_local $96
          i32.add
          set_local $97
          get_local $97
          i64.load align=1
          set_local $98
          get_local $7
          i64.load offset=48 align=1
          set_local $99
          i64.const 140737488355328
          set_local $100
          get_local $98
          get_local $100
          i64.or
          set_local $101
          i32.const 32
          set_local $102
          get_local $7
          get_local $102
          i32.add
          set_local $103
          get_local $103
          get_local $99
          get_local $101
          call $fromRep_1
          get_local $7
          i64.load offset=32 align=1
          set_local $104
          get_local $7
          i64.load offset=40 align=1
          set_local $105
          get_local $7
          get_local $105
          i64.store offset=552
          get_local $7
          get_local $104
          i64.store offset=544
          br $block
        end ;; $block_3
        get_local $7
        i64.load offset=464
        set_local $106
        get_local $7
        i64.load offset=472
        set_local $107
        i64.const 9223090561878065152
        set_local $108
        get_local $107
        get_local $108
        i64.xor
        set_local $109
        get_local $106
        get_local $109
        i64.or
        set_local $110
        get_local $110
        i64.eqz
        set_local $111
        get_local $111
        set_local $112
        block $block_4
          get_local $112
          i32.eqz
          br_if $block_4
          get_local $7
          i64.load offset=536
          set_local $113
          get_local $7
          i64.load offset=528
          set_local $114
          i32.const 96
          set_local $115
          get_local $7
          get_local $115
          i32.add
          set_local $116
          get_local $116
          get_local $114
          get_local $113
          call $toRep_2
          i32.const 8
          set_local $117
          i32.const 96
          set_local $118
          get_local $7
          get_local $118
          i32.add
          set_local $119
          get_local $119
          get_local $117
          i32.add
          set_local $120
          get_local $120
          i64.load align=1
          set_local $121
          get_local $7
          i64.load offset=96 align=1
          set_local $122
          get_local $7
          i64.load offset=520
          set_local $123
          get_local $7
          i64.load offset=512
          set_local $124
          i32.const 80
          set_local $125
          get_local $7
          get_local $125
          i32.add
          set_local $126
          get_local $126
          get_local $124
          get_local $123
          call $toRep_2
          i32.const 80
          set_local $127
          get_local $7
          get_local $127
          i32.add
          set_local $128
          get_local $128
          get_local $117
          i32.add
          set_local $129
          get_local $129
          i64.load align=1
          set_local $130
          get_local $7
          i64.load offset=80 align=1
          set_local $131
          get_local $122
          get_local $131
          i64.xor
          set_local $132
          get_local $121
          get_local $130
          i64.xor
          set_local $133
          i64.const -9223372036854775808
          set_local $134
          get_local $133
          get_local $134
          i64.xor
          set_local $135
          get_local $132
          get_local $135
          i64.or
          set_local $136
          get_local $136
          i64.eqz
          set_local $137
          get_local $137
          set_local $138
          block $block_5
            get_local $138
            i32.eqz
            br_if $block_5
            i64.const 9223231299366420480
            set_local $139
            i64.const 0
            set_local $140
            i32.const 64
            set_local $141
            get_local $7
            get_local $141
            i32.add
            set_local $142
            get_local $142
            get_local $140
            get_local $139
            call $fromRep_1
            get_local $7
            i64.load offset=64 align=1
            set_local $143
            get_local $7
            i64.load offset=72 align=1
            set_local $144
            get_local $7
            get_local $144
            i64.store offset=552
            get_local $7
            get_local $143
            i64.store offset=544
            br $block
          end ;; $block_5
          get_local $7
          i64.load offset=528
          set_local $145
          get_local $7
          i64.load offset=536
          set_local $146
          get_local $7
          get_local $146
          i64.store offset=552
          get_local $7
          get_local $145
          i64.store offset=544
          br $block
        end ;; $block_4
        get_local $7
        i64.load offset=448
        set_local $147
        get_local $7
        i64.load offset=456
        set_local $148
        i64.const 9223090561878065152
        set_local $149
        get_local $148
        get_local $149
        i64.xor
        set_local $150
        get_local $147
        get_local $150
        i64.or
        set_local $151
        get_local $151
        i64.eqz
        set_local $152
        get_local $152
        set_local $153
        block $block_6
          get_local $153
          i32.eqz
          br_if $block_6
          get_local $7
          i64.load offset=512
          set_local $154
          get_local $7
          i64.load offset=520
          set_local $155
          get_local $7
          get_local $155
          i64.store offset=552
          get_local $7
          get_local $154
          i64.store offset=544
          br $block
        end ;; $block_6
        get_local $7
        i64.load offset=472
        set_local $156
        get_local $7
        i64.load offset=464
        set_local $157
        get_local $157
        get_local $156
        i64.or
        set_local $158
        i64.const 0
        set_local $159
        get_local $158
        get_local $159
        i64.ne
        set_local $160
        get_local $160
        set_local $161
        block $block_7
          get_local $161
          br_if $block_7
          get_local $7
          i64.load offset=456
          set_local $162
          get_local $7
          i64.load offset=448
          set_local $163
          get_local $163
          get_local $162
          i64.or
          set_local $164
          i64.const 0
          set_local $165
          get_local $164
          get_local $165
          i64.ne
          set_local $166
          get_local $166
          set_local $167
          block $block_8
            get_local $167
            br_if $block_8
            get_local $7
            i64.load offset=536
            set_local $168
            get_local $7
            i64.load offset=528
            set_local $169
            i32.const 272
            set_local $170
            get_local $7
            get_local $170
            i32.add
            set_local $171
            get_local $171
            get_local $169
            get_local $168
            call $toRep_2
            i32.const 8
            set_local $172
            i32.const 272
            set_local $173
            get_local $7
            get_local $173
            i32.add
            set_local $174
            get_local $174
            get_local $172
            i32.add
            set_local $175
            get_local $175
            i64.load align=1
            set_local $176
            get_local $7
            i64.load offset=272 align=1
            set_local $177
            get_local $7
            i64.load offset=520
            set_local $178
            get_local $7
            i64.load offset=512
            set_local $179
            i32.const 256
            set_local $180
            get_local $7
            get_local $180
            i32.add
            set_local $181
            get_local $181
            get_local $179
            get_local $178
            call $toRep_2
            i32.const 256
            set_local $182
            get_local $7
            get_local $182
            i32.add
            set_local $183
            get_local $183
            get_local $172
            i32.add
            set_local $184
            get_local $184
            i64.load align=1
            set_local $185
            get_local $7
            i64.load offset=256 align=1
            set_local $186
            get_local $176
            get_local $185
            i64.and
            set_local $187
            get_local $177
            get_local $186
            i64.and
            set_local $188
            i32.const 240
            set_local $189
            get_local $7
            get_local $189
            i32.add
            set_local $190
            get_local $190
            get_local $188
            get_local $187
            call $fromRep_1
            get_local $7
            i64.load offset=240 align=1
            set_local $191
            get_local $7
            i64.load offset=248 align=1
            set_local $192
            get_local $7
            get_local $192
            i64.store offset=552
            get_local $7
            get_local $191
            i64.store offset=544
            br $block
          end ;; $block_8
          get_local $7
          i64.load offset=512
          set_local $193
          get_local $7
          i64.load offset=520
          set_local $194
          get_local $7
          get_local $194
          i64.store offset=552
          get_local $7
          get_local $193
          i64.store offset=544
          br $block
        end ;; $block_7
        get_local $7
        i64.load offset=456
        set_local $195
        get_local $7
        i64.load offset=448
        set_local $196
        get_local $196
        get_local $195
        i64.or
        set_local $197
        i64.const 0
        set_local $198
        get_local $197
        get_local $198
        i64.ne
        set_local $199
        get_local $199
        set_local $200
        block $block_9
          get_local $200
          br_if $block_9
          get_local $7
          i64.load offset=528
          set_local $201
          get_local $7
          i64.load offset=536
          set_local $202
          get_local $7
          get_local $202
          i64.store offset=552
          get_local $7
          get_local $201
          i64.store offset=544
          br $block
        end ;; $block_9
      end ;; $block_0
      get_local $7
      i64.load offset=448
      set_local $203
      get_local $7
      i64.load offset=456
      set_local $204
      get_local $7
      i64.load offset=464
      set_local $205
      get_local $7
      i64.load offset=472
      set_local $206
      get_local $204
      get_local $206
      i64.eq
      set_local $207
      get_local $204
      get_local $206
      i64.gt_u
      set_local $208
      get_local $203
      get_local $205
      i64.gt_u
      set_local $209
      get_local $209
      get_local $208
      get_local $207
      select
      set_local $210
      get_local $210
      set_local $211
      block $block_10
        get_local $211
        i32.eqz
        br_if $block_10
        get_local $7
        i64.load offset=496
        set_local $212
        get_local $7
        i64.load offset=504
        set_local $213
        get_local $7
        get_local $213
        i64.store offset=440
        get_local $7
        get_local $212
        i64.store offset=432
        get_local $7
        i64.load offset=480
        set_local $214
        get_local $7
        i64.load offset=488
        set_local $215
        get_local $7
        get_local $215
        i64.store offset=504
        get_local $7
        get_local $214
        i64.store offset=496
        get_local $7
        i64.load offset=432
        set_local $216
        get_local $7
        i64.load offset=440
        set_local $217
        get_local $7
        get_local $217
        i64.store offset=488
        get_local $7
        get_local $216
        i64.store offset=480
      end ;; $block_10
      get_local $7
      i64.load16_u offset=510
      set_local $218
      i64.const 32767
      set_local $219
      get_local $218
      get_local $219
      i64.and
      set_local $220
      get_local $7
      get_local $220
      i64.store32 offset=428
      get_local $7
      i64.load16_u offset=494
      set_local $221
      get_local $221
      get_local $219
      i64.and
      set_local $222
      get_local $7
      get_local $222
      i64.store32 offset=424
      get_local $7
      i64.load offset=496
      set_local $223
      get_local $7
      i64.load offset=504
      set_local $224
      i64.const 281474976710655
      set_local $225
      get_local $224
      get_local $225
      i64.and
      set_local $226
      get_local $7
      get_local $223
      i64.store offset=400
      get_local $7
      get_local $226
      i64.store offset=408
      get_local $7
      i64.load offset=480
      set_local $227
      get_local $7
      i64.load offset=488
      set_local $228
      get_local $228
      get_local $225
      i64.and
      set_local $229
      get_local $7
      get_local $227
      i64.store offset=384
      get_local $7
      get_local $229
      i64.store offset=392
      get_local $7
      i32.load offset=428
      set_local $230
      block $block_11
        get_local $230
        br_if $block_11
        i32.const 400
        set_local $231
        get_local $7
        get_local $231
        i32.add
        set_local $232
        get_local $232
        set_local $233
        get_local $233
        call $normalize
        set_local $234
        get_local $7
        get_local $234
        i32.store offset=428
      end ;; $block_11
      get_local $7
      i32.load offset=424
      set_local $235
      block $block_12
        get_local $235
        br_if $block_12
        i32.const 384
        set_local $236
        get_local $7
        get_local $236
        i32.add
        set_local $237
        get_local $237
        set_local $238
        get_local $238
        call $normalize
        set_local $239
        get_local $7
        get_local $239
        i32.store offset=424
      end ;; $block_12
      get_local $7
      i64.load offset=504
      set_local $240
      i64.const -9223372036854775808
      set_local $241
      get_local $240
      get_local $241
      i64.and
      set_local $242
      i64.const 0
      set_local $243
      get_local $7
      get_local $243
      i64.store offset=368
      get_local $7
      get_local $242
      i64.store offset=376
      get_local $7
      i64.load offset=504
      set_local $244
      get_local $7
      i64.load offset=488
      set_local $245
      get_local $244
      get_local $245
      i64.xor
      set_local $246
      i64.const 63
      set_local $247
      get_local $246
      get_local $247
      i64.shr_u
      set_local $248
      get_local $7
      get_local $248
      i64.store8 offset=367
      get_local $7
      i64.load offset=408
      set_local $249
      get_local $7
      i64.load offset=400
      set_local $250
      i64.const 61
      set_local $251
      get_local $250
      get_local $251
      i64.shr_u
      set_local $252
      i64.const 3
      set_local $253
      get_local $249
      get_local $253
      i64.shl
      set_local $254
      get_local $254
      get_local $252
      i64.or
      set_local $255
      get_local $250
      get_local $253
      i64.shl
      set_local $256
      i64.const 2251799813685248
      set_local $257
      get_local $255
      get_local $257
      i64.or
      set_local $258
      get_local $7
      get_local $256
      i64.store offset=400
      get_local $7
      get_local $258
      i64.store offset=408
      get_local $7
      i64.load offset=392
      set_local $259
      get_local $7
      i64.load offset=384
      set_local $260
      get_local $260
      get_local $251
      i64.shr_u
      set_local $261
      get_local $259
      get_local $253
      i64.shl
      set_local $262
      get_local $262
      get_local $261
      i64.or
      set_local $263
      get_local $260
      get_local $253
      i64.shl
      set_local $264
      get_local $263
      get_local $257
      i64.or
      set_local $265
      get_local $7
      get_local $264
      i64.store offset=384
      get_local $7
      get_local $265
      i64.store offset=392
      get_local $7
      i32.load offset=428
      set_local $266
      get_local $7
      i32.load offset=424
      set_local $267
      get_local $266
      get_local $267
      i32.sub
      set_local $268
      get_local $7
      get_local $268
      i32.store offset=360
      get_local $7
      i32.load offset=360
      set_local $269
      block $block_13
        get_local $269
        i32.eqz
        br_if $block_13
        i32.const 128
        set_local $270
        get_local $7
        i32.load offset=360
        set_local $271
        get_local $271
        set_local $272
        get_local $270
        set_local $273
        get_local $272
        get_local $273
        i32.lt_u
        set_local $274
        get_local $274
        set_local $275
        block $block_14
          block $block_15
            get_local $275
            i32.eqz
            br_if $block_15
            get_local $7
            i64.load offset=392
            set_local $276
            get_local $7
            i64.load offset=384
            set_local $277
            get_local $7
            i32.load offset=360
            set_local $278
            i32.const 128
            set_local $279
            get_local $279
            get_local $278
            i32.sub
            set_local $280
            i32.const 224
            set_local $281
            get_local $7
            get_local $281
            i32.add
            set_local $282
            get_local $282
            get_local $277
            get_local $276
            get_local $280
            call $__ashlti3
            i32.const 8
            set_local $283
            i32.const 224
            set_local $284
            get_local $7
            get_local $284
            i32.add
            set_local $285
            get_local $285
            get_local $283
            i32.add
            set_local $286
            get_local $286
            i64.load align=1
            set_local $287
            get_local $7
            i64.load offset=224 align=1
            set_local $288
            get_local $288
            get_local $287
            i64.or
            set_local $289
            i64.const 0
            set_local $290
            get_local $289
            get_local $290
            i64.ne
            set_local $291
            get_local $7
            get_local $291
            i32.store8 offset=359
            get_local $7
            i64.load offset=392
            set_local $292
            get_local $7
            i64.load offset=384
            set_local $293
            get_local $7
            i32.load offset=360
            set_local $294
            i32.const 208
            set_local $295
            get_local $7
            get_local $295
            i32.add
            set_local $296
            get_local $296
            get_local $293
            get_local $292
            get_local $294
            call $__lshrti3
            i32.const 208
            set_local $297
            get_local $7
            get_local $297
            i32.add
            set_local $298
            get_local $298
            get_local $283
            i32.add
            set_local $299
            get_local $299
            i64.load align=1
            set_local $300
            get_local $7
            i64.load offset=208 align=1
            set_local $301
            get_local $7
            i64.load8_u offset=359
            set_local $302
            i64.const 1
            set_local $303
            get_local $302
            get_local $303
            i64.and
            set_local $304
            get_local $301
            get_local $304
            i64.or
            set_local $305
            get_local $7
            get_local $300
            i64.store offset=392
            get_local $7
            get_local $305
            i64.store offset=384
            br $block_14
          end ;; $block_15
          i64.const 0
          set_local $306
          get_local $7
          get_local $306
          i64.store offset=392
          i64.const 1
          set_local $307
          get_local $7
          get_local $307
          i64.store offset=384
        end ;; $block_14
      end ;; $block_13
      get_local $7
      i32.load8_u offset=367
      set_local $308
      i32.const 1
      set_local $309
      get_local $308
      get_local $309
      i32.and
      set_local $310
      block $block_16
        block $block_17
          get_local $310
          i32.eqz
          br_if $block_17
          get_local $7
          i64.load offset=384
          set_local $311
          get_local $7
          i64.load offset=392
          set_local $312
          get_local $7
          i64.load offset=400
          set_local $313
          get_local $7
          i64.load offset=408
          set_local $314
          get_local $314
          get_local $312
          i64.sub
          set_local $315
          get_local $313
          get_local $311
          i64.lt_u
          set_local $316
          get_local $316
          i64.extend_u/i32
          set_local $317
          get_local $315
          get_local $317
          i64.sub
          set_local $318
          get_local $313
          get_local $311
          i64.sub
          set_local $319
          get_local $7
          get_local $319
          i64.store offset=400
          get_local $7
          get_local $318
          i64.store offset=408
          get_local $7
          i64.load offset=408
          set_local $320
          get_local $7
          i64.load offset=400
          set_local $321
          get_local $321
          get_local $320
          i64.or
          set_local $322
          get_local $322
          i64.eqz
          set_local $323
          get_local $323
          set_local $324
          block $block_18
            get_local $324
            i32.eqz
            br_if $block_18
            i64.const 0
            set_local $325
            i32.const 112
            set_local $326
            get_local $7
            get_local $326
            i32.add
            set_local $327
            get_local $327
            get_local $325
            get_local $325
            call $fromRep_1
            get_local $7
            i64.load offset=112 align=1
            set_local $328
            get_local $7
            i64.load offset=120 align=1
            set_local $329
            get_local $7
            get_local $329
            i64.store offset=552
            get_local $7
            get_local $328
            i64.store offset=544
            br $block
          end ;; $block_18
          get_local $7
          i64.load offset=408
          set_local $330
          i64.const 2251799813685248
          set_local $331
          get_local $330
          get_local $331
          i64.lt_u
          set_local $332
          get_local $332
          set_local $333
          block $block_19
            get_local $333
            i32.eqz
            br_if $block_19
            get_local $7
            i64.load offset=408
            set_local $334
            get_local $7
            i64.load offset=400
            set_local $335
            get_local $335
            get_local $334
            call $rep_clz
            set_local $336
            i64.const 2251799813685248
            set_local $337
            i64.const 0
            set_local $338
            get_local $338
            get_local $337
            call $rep_clz
            set_local $339
            get_local $336
            get_local $339
            i32.sub
            set_local $340
            get_local $7
            get_local $340
            i32.store offset=352
            get_local $7
            i32.load offset=352
            set_local $341
            get_local $7
            i64.load offset=408
            set_local $342
            get_local $7
            i64.load offset=400
            set_local $343
            i32.const 192
            set_local $344
            get_local $7
            get_local $344
            i32.add
            set_local $345
            get_local $345
            get_local $343
            get_local $342
            get_local $341
            call $__ashlti3
            i32.const 200
            set_local $346
            get_local $7
            get_local $346
            i32.add
            set_local $347
            get_local $347
            i64.load align=1
            set_local $348
            get_local $7
            i64.load offset=192 align=1
            set_local $349
            get_local $7
            get_local $348
            i64.store offset=408
            get_local $7
            get_local $349
            i64.store offset=400
            get_local $7
            i32.load offset=352
            set_local $350
            get_local $7
            i32.load offset=428
            set_local $351
            get_local $351
            get_local $350
            i32.sub
            set_local $352
            get_local $7
            get_local $352
            i32.store offset=428
          end ;; $block_19
          br $block_16
        end ;; $block_17
        get_local $7
        i64.load offset=384
        set_local $353
        get_local $7
        i64.load offset=392
        set_local $354
        get_local $7
        i64.load offset=400
        set_local $355
        get_local $7
        i64.load offset=408
        set_local $356
        get_local $356
        get_local $354
        i64.add
        set_local $357
        get_local $355
        get_local $353
        i64.add
        set_local $358
        get_local $358
        get_local $355
        i64.lt_u
        set_local $359
        get_local $359
        i64.extend_u/i32
        set_local $360
        get_local $357
        get_local $360
        i64.add
        set_local $361
        get_local $7
        get_local $358
        i64.store offset=400
        get_local $7
        get_local $361
        i64.store offset=408
        get_local $7
        i32.load8_u offset=414
        set_local $362
        i32.const 16
        set_local $363
        get_local $362
        get_local $363
        i32.and
        set_local $364
        i32.const 4
        set_local $365
        get_local $364
        get_local $365
        i32.shr_u
        set_local $366
        get_local $366
        set_local $367
        block $block_20
          get_local $367
          i32.eqz
          br_if $block_20
          get_local $7
          i64.load offset=400
          set_local $368
          i64.const 1
          set_local $369
          get_local $368
          get_local $369
          i64.and
          set_local $370
          get_local $7
          get_local $370
          i64.store8 offset=351
          get_local $7
          i64.load offset=400
          set_local $371
          get_local $7
          i64.load offset=408
          set_local $372
          i64.const 63
          set_local $373
          get_local $372
          get_local $373
          i64.shl
          set_local $374
          get_local $371
          get_local $369
          i64.shr_u
          set_local $375
          get_local $375
          get_local $374
          i64.or
          set_local $376
          get_local $372
          get_local $369
          i64.shr_u
          set_local $377
          get_local $7
          i64.load8_u offset=351
          set_local $378
          get_local $378
          get_local $369
          i64.and
          set_local $379
          get_local $376
          get_local $379
          i64.or
          set_local $380
          get_local $7
          get_local $377
          i64.store offset=408
          get_local $7
          get_local $380
          i64.store offset=400
          get_local $7
          i32.load offset=428
          set_local $381
          i32.const 1
          set_local $382
          get_local $381
          get_local $382
          i32.add
          set_local $383
          get_local $7
          get_local $383
          i32.store offset=428
        end ;; $block_20
      end ;; $block_16
      i32.const 32767
      set_local $384
      get_local $7
      i32.load offset=428
      set_local $385
      get_local $385
      set_local $386
      get_local $384
      set_local $387
      get_local $386
      get_local $387
      i32.ge_s
      set_local $388
      get_local $388
      set_local $389
      block $block_21
        get_local $389
        i32.eqz
        br_if $block_21
        get_local $7
        i64.load offset=368
        set_local $390
        get_local $7
        i64.load offset=376
        set_local $391
        i64.const 9223090561878065152
        set_local $392
        get_local $391
        get_local $392
        i64.or
        set_local $393
        i32.const 128
        set_local $394
        get_local $7
        get_local $394
        i32.add
        set_local $395
        get_local $395
        get_local $390
        get_local $393
        call $fromRep_1
        get_local $7
        i64.load offset=128 align=1
        set_local $396
        get_local $7
        i64.load offset=136 align=1
        set_local $397
        get_local $7
        get_local $397
        i64.store offset=552
        get_local $7
        get_local $396
        i64.store offset=544
        br $block
      end ;; $block_21
      i32.const 0
      set_local $398
      get_local $7
      i32.load offset=428
      set_local $399
      get_local $399
      set_local $400
      get_local $398
      set_local $401
      get_local $400
      get_local $401
      i32.le_s
      set_local $402
      get_local $402
      set_local $403
      block $block_22
        get_local $403
        i32.eqz
        br_if $block_22
        i32.const 0
        set_local $404
        get_local $7
        i32.load offset=428
        set_local $405
        i32.const 1
        set_local $406
        get_local $406
        get_local $405
        i32.sub
        set_local $407
        get_local $7
        get_local $407
        i32.store offset=344
        get_local $7
        i64.load offset=408
        set_local $408
        get_local $7
        i64.load offset=400
        set_local $409
        get_local $7
        i32.load offset=344
        set_local $410
        i32.const 128
        set_local $411
        get_local $411
        get_local $410
        i32.sub
        set_local $412
        i32.const 176
        set_local $413
        get_local $7
        get_local $413
        i32.add
        set_local $414
        get_local $414
        get_local $409
        get_local $408
        get_local $412
        call $__ashlti3
        i32.const 8
        set_local $415
        i32.const 176
        set_local $416
        get_local $7
        get_local $416
        i32.add
        set_local $417
        get_local $417
        get_local $415
        i32.add
        set_local $418
        get_local $418
        i64.load align=1
        set_local $419
        get_local $7
        i64.load offset=176 align=1
        set_local $420
        get_local $420
        get_local $419
        i64.or
        set_local $421
        i64.const 0
        set_local $422
        get_local $421
        get_local $422
        i64.ne
        set_local $423
        get_local $7
        get_local $423
        i32.store8 offset=343
        get_local $7
        i64.load offset=408
        set_local $424
        get_local $7
        i64.load offset=400
        set_local $425
        get_local $7
        i32.load offset=344
        set_local $426
        i32.const 160
        set_local $427
        get_local $7
        get_local $427
        i32.add
        set_local $428
        get_local $428
        get_local $425
        get_local $424
        get_local $426
        call $__lshrti3
        i32.const 160
        set_local $429
        get_local $7
        get_local $429
        i32.add
        set_local $430
        get_local $430
        get_local $415
        i32.add
        set_local $431
        get_local $431
        i64.load align=1
        set_local $432
        get_local $7
        i64.load offset=160 align=1
        set_local $433
        get_local $7
        i64.load8_u offset=343
        set_local $434
        i64.const 1
        set_local $435
        get_local $434
        get_local $435
        i64.and
        set_local $436
        get_local $433
        get_local $436
        i64.or
        set_local $437
        get_local $7
        get_local $432
        i64.store offset=408
        get_local $7
        get_local $437
        i64.store offset=400
        get_local $7
        get_local $404
        i32.store offset=428
      end ;; $block_22
      i32.const 4
      set_local $438
      get_local $7
      i64.load offset=400
      set_local $439
      i64.const 7
      set_local $440
      get_local $439
      get_local $440
      i64.and
      set_local $441
      get_local $7
      get_local $441
      i64.store32 offset=336
      get_local $7
      i64.load offset=400
      set_local $442
      get_local $7
      i64.load offset=408
      set_local $443
      i64.const 61
      set_local $444
      get_local $443
      get_local $444
      i64.shl
      set_local $445
      i64.const 3
      set_local $446
      get_local $442
      get_local $446
      i64.shr_u
      set_local $447
      get_local $447
      get_local $445
      i64.or
      set_local $448
      get_local $443
      get_local $446
      i64.shr_u
      set_local $449
      i64.const 281474976710655
      set_local $450
      get_local $449
      get_local $450
      i64.and
      set_local $451
      get_local $7
      get_local $451
      i64.store offset=328
      get_local $7
      get_local $448
      i64.store offset=320
      get_local $7
      i64.load32_u offset=428
      set_local $452
      i64.const 48
      set_local $453
      get_local $452
      get_local $453
      i64.shl
      set_local $454
      get_local $7
      i64.load offset=320
      set_local $455
      get_local $7
      i64.load offset=328
      set_local $456
      get_local $456
      get_local $454
      i64.or
      set_local $457
      get_local $7
      get_local $455
      i64.store offset=320
      get_local $7
      get_local $457
      i64.store offset=328
      get_local $7
      i64.load offset=376
      set_local $458
      get_local $7
      i64.load offset=368
      set_local $459
      get_local $7
      i64.load offset=328
      set_local $460
      get_local $7
      i64.load offset=320
      set_local $461
      get_local $461
      get_local $459
      i64.or
      set_local $462
      get_local $460
      get_local $458
      i64.or
      set_local $463
      get_local $7
      get_local $463
      i64.store offset=328
      get_local $7
      get_local $462
      i64.store offset=320
      get_local $7
      i32.load offset=336
      set_local $464
      get_local $464
      set_local $465
      get_local $438
      set_local $466
      get_local $465
      get_local $466
      i32.gt_s
      set_local $467
      get_local $467
      set_local $468
      block $block_23
        get_local $468
        i32.eqz
        br_if $block_23
        get_local $7
        i64.load offset=328
        set_local $469
        get_local $7
        i64.load offset=320
        set_local $470
        i64.const 1
        set_local $471
        get_local $470
        get_local $471
        i64.add
        set_local $472
        get_local $472
        get_local $470
        i64.lt_u
        set_local $473
        get_local $473
        i64.extend_u/i32
        set_local $474
        get_local $469
        get_local $474
        i64.add
        set_local $475
        get_local $7
        get_local $472
        i64.store offset=320
        get_local $7
        get_local $475
        i64.store offset=328
      end ;; $block_23
      i32.const 4
      set_local $476
      get_local $7
      i32.load offset=336
      set_local $477
      get_local $477
      set_local $478
      get_local $476
      set_local $479
      get_local $478
      get_local $479
      i32.eq
      set_local $480
      get_local $480
      set_local $481
      block $block_24
        get_local $481
        i32.eqz
        br_if $block_24
        get_local $7
        i64.load offset=328
        set_local $482
        get_local $7
        i64.load offset=320
        set_local $483
        i64.const 1
        set_local $484
        get_local $483
        get_local $484
        i64.and
        set_local $485
        get_local $483
        get_local $485
        i64.add
        set_local $486
        get_local $486
        get_local $483
        i64.lt_u
        set_local $487
        get_local $487
        i64.extend_u/i32
        set_local $488
        get_local $482
        get_local $488
        i64.add
        set_local $489
        get_local $7
        get_local $486
        i64.store offset=320
        get_local $7
        get_local $489
        i64.store offset=328
      end ;; $block_24
      get_local $7
      i64.load offset=328
      set_local $490
      get_local $7
      i64.load offset=320
      set_local $491
      i32.const 144
      set_local $492
      get_local $7
      get_local $492
      i32.add
      set_local $493
      get_local $493
      get_local $491
      get_local $490
      call $fromRep_1
      get_local $7
      i64.load offset=144 align=1
      set_local $494
      get_local $7
      i64.load offset=152 align=1
      set_local $495
      get_local $7
      get_local $495
      i64.store offset=552
      get_local $7
      get_local $494
      i64.store offset=544
    end ;; $block
    get_local $7
    i64.load offset=544
    set_local $496
    get_local $7
    i64.load offset=552
    set_local $497
    get_local $0
    get_local $497
    i64.store offset=8
    get_local $0
    get_local $496
    i64.store
    i32.const 560
    set_local $498
    get_local $7
    get_local $498
    i32.add
    set_local $499
    get_local $499
    set_global $25
    return
    )
  
  (func $toRep_2 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $fromRep_1 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $normalize (type $4)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i64)
    (local $12 i64)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i64)
    (local $19 i64)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i64)
    (local $26 i64)
    (local $27 i32)
    (local $28 i32)
    (local $29 i32)
    (local $30 i32)
    get_global $25
    set_local $1
    i32.const 32
    set_local $2
    get_local $1
    get_local $2
    i32.sub
    set_local $3
    get_local $3
    set_global $25
    i32.const 1
    set_local $4
    get_local $3
    get_local $0
    i32.store offset=28
    get_local $3
    i32.load offset=28
    set_local $5
    i32.const 8
    set_local $6
    get_local $5
    get_local $6
    i32.add
    set_local $7
    get_local $7
    i64.load
    set_local $8
    get_local $5
    i64.load
    set_local $9
    get_local $9
    get_local $8
    call $rep_clz
    set_local $10
    i64.const 281474976710656
    set_local $11
    i64.const 0
    set_local $12
    get_local $12
    get_local $11
    call $rep_clz
    set_local $13
    get_local $10
    get_local $13
    i32.sub
    set_local $14
    get_local $3
    get_local $14
    i32.store offset=24
    get_local $3
    i32.load offset=24
    set_local $15
    get_local $3
    i32.load offset=28
    set_local $16
    get_local $16
    get_local $6
    i32.add
    set_local $17
    get_local $17
    i64.load
    set_local $18
    get_local $16
    i64.load
    set_local $19
    i32.const 8
    set_local $20
    get_local $3
    get_local $20
    i32.add
    set_local $21
    get_local $21
    get_local $19
    get_local $18
    get_local $15
    call $__ashlti3
    i32.const 8
    set_local $22
    get_local $3
    get_local $22
    i32.add
    set_local $23
    get_local $23
    get_local $6
    i32.add
    set_local $24
    get_local $24
    i64.load align=1
    set_local $25
    get_local $3
    i64.load offset=8 align=1
    set_local $26
    get_local $17
    get_local $25
    i64.store
    get_local $16
    get_local $26
    i64.store
    get_local $3
    i32.load offset=24
    set_local $27
    get_local $4
    get_local $27
    i32.sub
    set_local $28
    i32.const 32
    set_local $29
    get_local $3
    get_local $29
    i32.add
    set_local $30
    get_local $30
    set_global $25
    get_local $28
    return
    )
  
  (func $rep_clz (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i32)
    (local $12 i32)
    (local $13 i64)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i32)
    (local $20 i32)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i32)
    get_global $25
    set_local $2
    i32.const 48
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    i64.const 0
    set_local $5
    get_local $4
    get_local $0
    i64.store offset=32
    get_local $4
    get_local $1
    i64.store offset=40
    get_local $4
    i64.load offset=32
    set_local $6
    get_local $4
    i64.load offset=40
    set_local $7
    get_local $4
    get_local $7
    i64.store offset=24
    get_local $4
    get_local $6
    i64.store offset=16
    get_local $4
    i64.load offset=24
    set_local $8
    get_local $8
    set_local $9
    get_local $5
    set_local $10
    get_local $9
    get_local $10
    i64.ne
    set_local $11
    get_local $11
    set_local $12
    block $block
      block $block_0
        get_local $12
        i32.eqz
        br_if $block_0
        i64.const 0
        set_local $13
        get_local $4
        i64.load offset=24
        set_local $14
        get_local $4
        get_local $14
        i64.store offset=8
        get_local $4
        get_local $13
        i64.store
        br $block
      end ;; $block_0
      i64.const 64
      set_local $15
      get_local $4
      i64.load offset=16
      set_local $16
      get_local $4
      get_local $16
      i64.store offset=8
      get_local $4
      get_local $15
      i64.store
    end ;; $block
    get_local $4
    i64.load offset=8
    set_local $17
    get_local $17
    i64.clz
    set_local $18
    get_local $18
    i32.wrap/i64
    set_local $19
    get_local $19
    set_local $20
    get_local $20
    i64.extend_s/i32
    set_local $21
    get_local $4
    i64.load
    set_local $22
    get_local $21
    get_local $22
    i64.add
    set_local $23
    get_local $23
    i32.wrap/i64
    set_local $24
    get_local $24
    return
    )
  
  (func $__ashlti3 (type $12)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i64)
    (local $14 i64)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i32)
    (local $19 i64)
    (local $20 i64)
    (local $21 i32)
    (local $22 i64)
    (local $23 i64)
    (local $24 i32)
    (local $25 i64)
    (local $26 i32)
    (local $27 i32)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i32)
    (local $32 i32)
    (local $33 i64)
    (local $34 i64)
    (local $35 i64)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    (local $45 i64)
    get_global $25
    set_local $4
    i32.const 80
    set_local $5
    get_local $4
    get_local $5
    i32.sub
    set_local $6
    get_local $6
    get_local $1
    i64.store offset=48
    get_local $6
    get_local $2
    i64.store offset=56
    get_local $6
    get_local $3
    i32.store offset=44
    i32.const 64
    set_local $7
    get_local $6
    get_local $7
    i32.store offset=40
    get_local $6
    i64.load offset=48
    set_local $8
    get_local $6
    i64.load offset=56
    set_local $9
    get_local $6
    get_local $9
    i64.store offset=24
    get_local $6
    get_local $8
    i64.store offset=16
    get_local $6
    i32.load offset=44
    set_local $10
    i32.const 64
    set_local $11
    get_local $10
    get_local $11
    i32.and
    set_local $12
    block $block
      block $block_0
        block $block_1
          get_local $12
          i32.eqz
          br_if $block_1
          i64.const 0
          set_local $13
          get_local $6
          get_local $13
          i64.store
          get_local $6
          i64.load offset=16
          set_local $14
          get_local $6
          i32.load offset=44
          set_local $15
          i32.const 64
          set_local $16
          get_local $15
          get_local $16
          i32.sub
          set_local $17
          get_local $17
          set_local $18
          get_local $18
          i64.extend_u/i32
          set_local $19
          get_local $14
          get_local $19
          i64.shl
          set_local $20
          get_local $6
          get_local $20
          i64.store offset=8
          br $block_0
        end ;; $block_1
        get_local $6
        i32.load offset=44
        set_local $21
        block $block_2
          get_local $21
          br_if $block_2
          get_local $6
          i64.load offset=48
          set_local $22
          get_local $6
          i64.load offset=56
          set_local $23
          get_local $6
          get_local $23
          i64.store offset=72
          get_local $6
          get_local $22
          i64.store offset=64
          br $block
        end ;; $block_2
        i32.const 64
        set_local $24
        get_local $6
        i64.load offset=16
        set_local $25
        get_local $6
        i32.load offset=44
        set_local $26
        get_local $26
        set_local $27
        get_local $27
        i64.extend_u/i32
        set_local $28
        get_local $25
        get_local $28
        i64.shl
        set_local $29
        get_local $6
        get_local $29
        i64.store
        get_local $6
        i64.load offset=24
        set_local $30
        get_local $6
        i32.load offset=44
        set_local $31
        get_local $31
        set_local $32
        get_local $32
        i64.extend_u/i32
        set_local $33
        get_local $30
        get_local $33
        i64.shl
        set_local $34
        get_local $6
        i64.load offset=16
        set_local $35
        get_local $6
        i32.load offset=44
        set_local $36
        get_local $24
        get_local $36
        i32.sub
        set_local $37
        get_local $37
        set_local $38
        get_local $38
        i64.extend_u/i32
        set_local $39
        get_local $35
        get_local $39
        i64.shr_u
        set_local $40
        get_local $34
        get_local $40
        i64.or
        set_local $41
        get_local $6
        get_local $41
        i64.store offset=8
      end ;; $block_0
      get_local $6
      i64.load
      set_local $42
      get_local $6
      i64.load offset=8
      set_local $43
      get_local $6
      get_local $43
      i64.store offset=72
      get_local $6
      get_local $42
      i64.store offset=64
    end ;; $block
    get_local $6
    i64.load offset=64
    set_local $44
    get_local $6
    i64.load offset=72
    set_local $45
    get_local $0
    get_local $45
    i64.store offset=8
    get_local $0
    get_local $44
    i64.store
    return
    )
  
  (func $__lshrti3 (type $12)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i32)
    (local $12 i32)
    (local $13 i64)
    (local $14 i64)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i32)
    (local $19 i64)
    (local $20 i64)
    (local $21 i32)
    (local $22 i64)
    (local $23 i64)
    (local $24 i32)
    (local $25 i64)
    (local $26 i32)
    (local $27 i32)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i32)
    (local $32 i32)
    (local $33 i32)
    (local $34 i64)
    (local $35 i64)
    (local $36 i64)
    (local $37 i32)
    (local $38 i32)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    (local $45 i64)
    get_global $25
    set_local $4
    i32.const 80
    set_local $5
    get_local $4
    get_local $5
    i32.sub
    set_local $6
    get_local $6
    get_local $1
    i64.store offset=48
    get_local $6
    get_local $2
    i64.store offset=56
    get_local $6
    get_local $3
    i32.store offset=44
    i32.const 64
    set_local $7
    get_local $6
    get_local $7
    i32.store offset=40
    get_local $6
    i64.load offset=48
    set_local $8
    get_local $6
    i64.load offset=56
    set_local $9
    get_local $6
    get_local $9
    i64.store offset=24
    get_local $6
    get_local $8
    i64.store offset=16
    get_local $6
    i32.load offset=44
    set_local $10
    i32.const 64
    set_local $11
    get_local $10
    get_local $11
    i32.and
    set_local $12
    block $block
      block $block_0
        block $block_1
          get_local $12
          i32.eqz
          br_if $block_1
          i64.const 0
          set_local $13
          get_local $6
          get_local $13
          i64.store offset=8
          get_local $6
          i64.load offset=24
          set_local $14
          get_local $6
          i32.load offset=44
          set_local $15
          i32.const 64
          set_local $16
          get_local $15
          get_local $16
          i32.sub
          set_local $17
          get_local $17
          set_local $18
          get_local $18
          i64.extend_u/i32
          set_local $19
          get_local $14
          get_local $19
          i64.shr_u
          set_local $20
          get_local $6
          get_local $20
          i64.store
          br $block_0
        end ;; $block_1
        get_local $6
        i32.load offset=44
        set_local $21
        block $block_2
          get_local $21
          br_if $block_2
          get_local $6
          i64.load offset=48
          set_local $22
          get_local $6
          i64.load offset=56
          set_local $23
          get_local $6
          get_local $23
          i64.store offset=72
          get_local $6
          get_local $22
          i64.store offset=64
          br $block
        end ;; $block_2
        i32.const 64
        set_local $24
        get_local $6
        i64.load offset=24
        set_local $25
        get_local $6
        i32.load offset=44
        set_local $26
        get_local $26
        set_local $27
        get_local $27
        i64.extend_u/i32
        set_local $28
        get_local $25
        get_local $28
        i64.shr_u
        set_local $29
        get_local $6
        get_local $29
        i64.store offset=8
        get_local $6
        i64.load offset=24
        set_local $30
        get_local $6
        i32.load offset=44
        set_local $31
        get_local $24
        get_local $31
        i32.sub
        set_local $32
        get_local $32
        set_local $33
        get_local $33
        i64.extend_u/i32
        set_local $34
        get_local $30
        get_local $34
        i64.shl
        set_local $35
        get_local $6
        i64.load offset=16
        set_local $36
        get_local $6
        i32.load offset=44
        set_local $37
        get_local $37
        set_local $38
        get_local $38
        i64.extend_u/i32
        set_local $39
        get_local $36
        get_local $39
        i64.shr_u
        set_local $40
        get_local $35
        get_local $40
        i64.or
        set_local $41
        get_local $6
        get_local $41
        i64.store
      end ;; $block_0
      get_local $6
      i64.load
      set_local $42
      get_local $6
      i64.load offset=8
      set_local $43
      get_local $6
      get_local $43
      i64.store offset=72
      get_local $6
      get_local $42
      i64.store offset=64
    end ;; $block
    get_local $6
    i64.load offset=64
    set_local $44
    get_local $6
    i64.load offset=72
    set_local $45
    get_local $0
    get_local $45
    i64.store offset=8
    get_local $0
    get_local $44
    i64.store
    return
    )
  
  (func $__multi3 (type $21)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i32)
    (local $15 i32)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i32)
    (local $30 i32)
    get_global $25
    set_local $5
    i32.const 96
    set_local $6
    get_local $5
    get_local $6
    i32.sub
    set_local $7
    get_local $7
    set_global $25
    get_local $7
    get_local $1
    i64.store offset=80
    get_local $7
    get_local $2
    i64.store offset=88
    get_local $7
    get_local $4
    i64.store offset=72
    get_local $7
    get_local $3
    i64.store offset=64
    get_local $7
    i64.load offset=80
    set_local $8
    get_local $7
    i64.load offset=88
    set_local $9
    get_local $7
    get_local $9
    i64.store offset=56
    get_local $7
    get_local $8
    i64.store offset=48
    get_local $7
    i64.load offset=64
    set_local $10
    get_local $7
    i64.load offset=72
    set_local $11
    get_local $7
    get_local $11
    i64.store offset=40
    get_local $7
    get_local $10
    i64.store offset=32
    get_local $7
    i64.load offset=48
    set_local $12
    get_local $7
    i64.load offset=32
    set_local $13
    get_local $7
    get_local $12
    get_local $13
    call $__mulddi3
    i32.const 8
    set_local $14
    get_local $7
    get_local $14
    i32.add
    set_local $15
    get_local $15
    i64.load align=1
    set_local $16
    get_local $7
    i64.load align=1
    set_local $17
    get_local $7
    get_local $16
    i64.store offset=24
    get_local $7
    get_local $17
    i64.store offset=16
    get_local $7
    i64.load offset=56
    set_local $18
    get_local $7
    i64.load offset=32
    set_local $19
    get_local $18
    get_local $19
    i64.mul
    set_local $20
    get_local $7
    i64.load offset=48
    set_local $21
    get_local $7
    i64.load offset=40
    set_local $22
    get_local $21
    get_local $22
    i64.mul
    set_local $23
    get_local $20
    get_local $23
    i64.add
    set_local $24
    get_local $7
    i64.load offset=24
    set_local $25
    get_local $25
    get_local $24
    i64.add
    set_local $26
    get_local $7
    get_local $26
    i64.store offset=24
    get_local $7
    i64.load offset=16
    set_local $27
    get_local $7
    i64.load offset=24
    set_local $28
    get_local $0
    get_local $28
    i64.store offset=8
    get_local $0
    get_local $27
    i64.store
    i32.const 96
    set_local $29
    get_local $7
    get_local $29
    i32.add
    set_local $30
    get_local $30
    set_global $25
    return
    )
  
  (func $__mulddi3 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i64)
    (local $35 i64)
    (local $36 i64)
    (local $37 i64)
    (local $38 i64)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    get_global $25
    set_local $3
    i32.const 64
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=56
    get_local $5
    get_local $2
    i64.store offset=48
    i32.const 32
    set_local $6
    get_local $5
    get_local $6
    i32.store offset=28
    i64.const 4294967295
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=16
    get_local $5
    i64.load32_u offset=56
    set_local $8
    get_local $5
    i64.load32_u offset=48
    set_local $9
    get_local $8
    get_local $9
    i64.mul
    set_local $10
    get_local $5
    get_local $10
    i64.store offset=32
    get_local $5
    i64.load32_u offset=36
    set_local $11
    get_local $5
    get_local $11
    i64.store offset=8
    get_local $5
    i64.load32_u offset=32
    set_local $12
    get_local $5
    get_local $12
    i64.store offset=32
    get_local $5
    i64.load32_u offset=60
    set_local $13
    get_local $5
    i64.load32_u offset=48
    set_local $14
    get_local $13
    get_local $14
    i64.mul
    set_local $15
    get_local $5
    i64.load offset=8
    set_local $16
    get_local $16
    get_local $15
    i64.add
    set_local $17
    get_local $5
    get_local $17
    i64.store offset=8
    get_local $5
    i64.load offset=8
    set_local $18
    i64.const 32
    set_local $19
    get_local $18
    get_local $19
    i64.shl
    set_local $20
    get_local $5
    i64.load offset=32
    set_local $21
    get_local $21
    get_local $20
    i64.add
    set_local $22
    get_local $5
    get_local $22
    i64.store offset=32
    get_local $5
    i64.load32_u offset=12
    set_local $23
    get_local $5
    get_local $23
    i64.store offset=40
    get_local $5
    i64.load32_u offset=36
    set_local $24
    get_local $5
    get_local $24
    i64.store offset=8
    get_local $5
    i64.load32_u offset=32
    set_local $25
    get_local $5
    get_local $25
    i64.store offset=32
    get_local $5
    i64.load32_u offset=52
    set_local $26
    get_local $5
    i64.load32_u offset=56
    set_local $27
    get_local $26
    get_local $27
    i64.mul
    set_local $28
    get_local $5
    i64.load offset=8
    set_local $29
    get_local $29
    get_local $28
    i64.add
    set_local $30
    get_local $5
    get_local $30
    i64.store offset=8
    get_local $5
    i64.load offset=8
    set_local $31
    get_local $31
    get_local $19
    i64.shl
    set_local $32
    get_local $5
    i64.load offset=32
    set_local $33
    get_local $33
    get_local $32
    i64.add
    set_local $34
    get_local $5
    get_local $34
    i64.store offset=32
    get_local $5
    i64.load32_u offset=12
    set_local $35
    get_local $5
    i64.load offset=40
    set_local $36
    get_local $36
    get_local $35
    i64.add
    set_local $37
    get_local $5
    get_local $37
    i64.store offset=40
    get_local $5
    i64.load32_u offset=60
    set_local $38
    get_local $5
    i64.load32_u offset=52
    set_local $39
    get_local $38
    get_local $39
    i64.mul
    set_local $40
    get_local $5
    i64.load offset=40
    set_local $41
    get_local $41
    get_local $40
    i64.add
    set_local $42
    get_local $5
    get_local $42
    i64.store offset=40
    get_local $5
    i64.load offset=32
    set_local $43
    get_local $5
    i64.load offset=40
    set_local $44
    get_local $0
    get_local $44
    i64.store offset=8
    get_local $0
    get_local $43
    i64.store
    return
    )
  
  (func $__multf3 (type $21)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i32)
    (local $15 i32)
    get_global $25
    set_local $5
    i32.const 48
    set_local $6
    get_local $5
    get_local $6
    i32.sub
    set_local $7
    get_local $7
    set_global $25
    get_local $7
    get_local $1
    i64.store offset=32
    get_local $7
    get_local $2
    i64.store offset=40
    get_local $7
    get_local $4
    i64.store offset=24
    get_local $7
    get_local $3
    i64.store offset=16
    get_local $7
    i64.load offset=24
    set_local $8
    get_local $7
    i64.load offset=16
    set_local $9
    get_local $7
    i64.load offset=40
    set_local $10
    get_local $7
    i64.load offset=32
    set_local $11
    get_local $7
    get_local $11
    get_local $10
    get_local $9
    get_local $8
    call $__mulXf3__
    get_local $7
    i64.load align=1
    set_local $12
    get_local $7
    i64.load offset=8 align=1
    set_local $13
    get_local $0
    get_local $13
    i64.store offset=8
    get_local $0
    get_local $12
    i64.store
    i32.const 48
    set_local $14
    get_local $7
    get_local $14
    i32.add
    set_local $15
    get_local $15
    set_global $25
    return
    )
  
  (func $__mulXf3__ (type $21)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i32)
    (local $10 i64)
    (local $11 i64)
    (local $12 i32)
    (local $13 i32)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i32)
    (local $20 i32)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i32)
    (local $26 i32)
    (local $27 i32)
    (local $28 i32)
    (local $29 i32)
    (local $30 i32)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i32)
    (local $35 i32)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    (local $45 i64)
    (local $46 i32)
    (local $47 i32)
    (local $48 i32)
    (local $49 i32)
    (local $50 i32)
    (local $51 i64)
    (local $52 i64)
    (local $53 i64)
    (local $54 i64)
    (local $55 i64)
    (local $56 i64)
    (local $57 i32)
    (local $58 i32)
    (local $59 i32)
    (local $60 i32)
    (local $61 i32)
    (local $62 i64)
    (local $63 i64)
    (local $64 i64)
    (local $65 i32)
    (local $66 i32)
    (local $67 i32)
    (local $68 i32)
    (local $69 i32)
    (local $70 i32)
    (local $71 i32)
    (local $72 i32)
    (local $73 i32)
    (local $74 i32)
    (local $75 i32)
    (local $76 i32)
    (local $77 i32)
    (local $78 i32)
    (local $79 i32)
    (local $80 i64)
    (local $81 i64)
    (local $82 i32)
    (local $83 i32)
    (local $84 i32)
    (local $85 i32)
    (local $86 i32)
    (local $87 i32)
    (local $88 i64)
    (local $89 i64)
    (local $90 i64)
    (local $91 i64)
    (local $92 i64)
    (local $93 i64)
    (local $94 i32)
    (local $95 i32)
    (local $96 i32)
    (local $97 i32)
    (local $98 i32)
    (local $99 i64)
    (local $100 i64)
    (local $101 i64)
    (local $102 i64)
    (local $103 i64)
    (local $104 i64)
    (local $105 i32)
    (local $106 i32)
    (local $107 i64)
    (local $108 i32)
    (local $109 i32)
    (local $110 i32)
    (local $111 i64)
    (local $112 i64)
    (local $113 i32)
    (local $114 i32)
    (local $115 i32)
    (local $116 i32)
    (local $117 i64)
    (local $118 i64)
    (local $119 i64)
    (local $120 i64)
    (local $121 i64)
    (local $122 i64)
    (local $123 i64)
    (local $124 i64)
    (local $125 i64)
    (local $126 i32)
    (local $127 i32)
    (local $128 i64)
    (local $129 i32)
    (local $130 i32)
    (local $131 i32)
    (local $132 i64)
    (local $133 i64)
    (local $134 i32)
    (local $135 i32)
    (local $136 i32)
    (local $137 i32)
    (local $138 i64)
    (local $139 i64)
    (local $140 i64)
    (local $141 i64)
    (local $142 i32)
    (local $143 i32)
    (local $144 i64)
    (local $145 i64)
    (local $146 i64)
    (local $147 i64)
    (local $148 i64)
    (local $149 i64)
    (local $150 i64)
    (local $151 i32)
    (local $152 i32)
    (local $153 i64)
    (local $154 i64)
    (local $155 i64)
    (local $156 i64)
    (local $157 i32)
    (local $158 i32)
    (local $159 i64)
    (local $160 i64)
    (local $161 i64)
    (local $162 i64)
    (local $163 i64)
    (local $164 i64)
    (local $165 i32)
    (local $166 i32)
    (local $167 i64)
    (local $168 i64)
    (local $169 i64)
    (local $170 i64)
    (local $171 i32)
    (local $172 i32)
    (local $173 i64)
    (local $174 i64)
    (local $175 i64)
    (local $176 i64)
    (local $177 i64)
    (local $178 i64)
    (local $179 i64)
    (local $180 i32)
    (local $181 i32)
    (local $182 i64)
    (local $183 i64)
    (local $184 i64)
    (local $185 i64)
    (local $186 i32)
    (local $187 i32)
    (local $188 i64)
    (local $189 i64)
    (local $190 i64)
    (local $191 i64)
    (local $192 i64)
    (local $193 i64)
    (local $194 i32)
    (local $195 i32)
    (local $196 i64)
    (local $197 i64)
    (local $198 i64)
    (local $199 i64)
    (local $200 i32)
    (local $201 i32)
    (local $202 i64)
    (local $203 i64)
    (local $204 i64)
    (local $205 i64)
    (local $206 i64)
    (local $207 i64)
    (local $208 i32)
    (local $209 i32)
    (local $210 i64)
    (local $211 i64)
    (local $212 i32)
    (local $213 i32)
    (local $214 i64)
    (local $215 i64)
    (local $216 i64)
    (local $217 i64)
    (local $218 i64)
    (local $219 i64)
    (local $220 i32)
    (local $221 i32)
    (local $222 i64)
    (local $223 i64)
    (local $224 i32)
    (local $225 i32)
    (local $226 i64)
    (local $227 i64)
    (local $228 i64)
    (local $229 i64)
    (local $230 i32)
    (local $231 i32)
    (local $232 i32)
    (local $233 i32)
    (local $234 i32)
    (local $235 i32)
    (local $236 i32)
    (local $237 i32)
    (local $238 i64)
    (local $239 i64)
    (local $240 i32)
    (local $241 i32)
    (local $242 i32)
    (local $243 i32)
    (local $244 i32)
    (local $245 i32)
    (local $246 i32)
    (local $247 i32)
    (local $248 i64)
    (local $249 i64)
    (local $250 i64)
    (local $251 i64)
    (local $252 i64)
    (local $253 i64)
    (local $254 i64)
    (local $255 i64)
    (local $256 i64)
    (local $257 i64)
    (local $258 i64)
    (local $259 i64)
    (local $260 i64)
    (local $261 i64)
    (local $262 i64)
    (local $263 i32)
    (local $264 i32)
    (local $265 i32)
    (local $266 i32)
    (local $267 i32)
    (local $268 i32)
    (local $269 i32)
    (local $270 i32)
    (local $271 i32)
    (local $272 i32)
    (local $273 i32)
    (local $274 i32)
    (local $275 i32)
    (local $276 i32)
    (local $277 i32)
    (local $278 i32)
    (local $279 i32)
    (local $280 i32)
    (local $281 i32)
    (local $282 i32)
    (local $283 i32)
    (local $284 i32)
    (local $285 i32)
    (local $286 i32)
    (local $287 i32)
    (local $288 i32)
    (local $289 i32)
    (local $290 i32)
    (local $291 i32)
    (local $292 i32)
    (local $293 i32)
    (local $294 i64)
    (local $295 i64)
    (local $296 i64)
    (local $297 i64)
    (local $298 i32)
    (local $299 i32)
    (local $300 i64)
    (local $301 i64)
    (local $302 i32)
    (local $303 i32)
    (local $304 i32)
    (local $305 i32)
    (local $306 i32)
    (local $307 i32)
    (local $308 i32)
    (local $309 i64)
    (local $310 i64)
    (local $311 i64)
    (local $312 i32)
    (local $313 i32)
    (local $314 i32)
    (local $315 i32)
    (local $316 i32)
    (local $317 i32)
    (local $318 i64)
    (local $319 i64)
    (local $320 i32)
    (local $321 i32)
    (local $322 i64)
    (local $323 i64)
    (local $324 i32)
    (local $325 i32)
    (local $326 i32)
    (local $327 i32)
    (local $328 i32)
    (local $329 i32)
    (local $330 i32)
    (local $331 i64)
    (local $332 i64)
    (local $333 i64)
    (local $334 i64)
    (local $335 i64)
    (local $336 i64)
    (local $337 i64)
    (local $338 i64)
    (local $339 i64)
    (local $340 i64)
    (local $341 i64)
    (local $342 i64)
    (local $343 i64)
    (local $344 i64)
    (local $345 i64)
    (local $346 i64)
    (local $347 i64)
    (local $348 i64)
    (local $349 i32)
    (local $350 i32)
    (local $351 i64)
    (local $352 i32)
    (local $353 i32)
    (local $354 i32)
    (local $355 i64)
    (local $356 i64)
    (local $357 i64)
    (local $358 i64)
    (local $359 i32)
    (local $360 i64)
    (local $361 i64)
    (local $362 i64)
    (local $363 i64)
    (local $364 i64)
    (local $365 i64)
    (local $366 i64)
    (local $367 i32)
    (local $368 i32)
    (local $369 i64)
    (local $370 i64)
    (local $371 i64)
    (local $372 i64)
    (local $373 i64)
    (local $374 i32)
    (local $375 i64)
    (local $376 i64)
    (local $377 i64)
    (local $378 i64)
    (local $379 i32)
    (local $380 i32)
    (local $381 i64)
    (local $382 i64)
    (local $383 i64)
    (local $384 i64)
    (local $385 i32)
    (local $386 i32)
    get_global $25
    set_local $5
    i32.const 544
    set_local $6
    get_local $5
    get_local $6
    i32.sub
    set_local $7
    get_local $7
    set_global $25
    i32.const 32766
    set_local $8
    i32.const 0
    set_local $9
    get_local $7
    get_local $1
    i64.store offset=512
    get_local $7
    get_local $2
    i64.store offset=520
    get_local $7
    get_local $4
    i64.store offset=504
    get_local $7
    get_local $3
    i64.store offset=496
    get_local $7
    i64.load offset=520
    set_local $10
    get_local $7
    i64.load offset=512
    set_local $11
    i32.const 328
    set_local $12
    get_local $7
    get_local $12
    i32.add
    set_local $13
    get_local $13
    get_local $11
    get_local $10
    call $toRep_3
    get_local $7
    i64.load16_u offset=342 align=1
    set_local $14
    i64.const 32767
    set_local $15
    get_local $14
    get_local $15
    i64.and
    set_local $16
    get_local $7
    get_local $16
    i64.store32 offset=492
    get_local $7
    i64.load offset=504
    set_local $17
    get_local $7
    i64.load offset=496
    set_local $18
    i32.const 312
    set_local $19
    get_local $7
    get_local $19
    i32.add
    set_local $20
    get_local $20
    get_local $18
    get_local $17
    call $toRep_3
    get_local $7
    i64.load16_u offset=326 align=1
    set_local $21
    get_local $21
    get_local $15
    i64.and
    set_local $22
    get_local $7
    get_local $22
    i64.store32 offset=488
    get_local $7
    i64.load offset=520
    set_local $23
    get_local $7
    i64.load offset=512
    set_local $24
    i32.const 296
    set_local $25
    get_local $7
    get_local $25
    i32.add
    set_local $26
    get_local $26
    get_local $24
    get_local $23
    call $toRep_3
    i32.const 8
    set_local $27
    i32.const 296
    set_local $28
    get_local $7
    get_local $28
    i32.add
    set_local $29
    get_local $29
    get_local $27
    i32.add
    set_local $30
    get_local $30
    i64.load align=1
    set_local $31
    get_local $7
    i64.load offset=504
    set_local $32
    get_local $7
    i64.load offset=496
    set_local $33
    i32.const 280
    set_local $34
    get_local $7
    get_local $34
    i32.add
    set_local $35
    get_local $35
    get_local $33
    get_local $32
    call $toRep_3
    i32.const 280
    set_local $36
    get_local $7
    get_local $36
    i32.add
    set_local $37
    get_local $37
    get_local $27
    i32.add
    set_local $38
    get_local $38
    i64.load align=1
    set_local $39
    get_local $31
    get_local $39
    i64.xor
    set_local $40
    i64.const -9223372036854775808
    set_local $41
    get_local $40
    get_local $41
    i64.and
    set_local $42
    i64.const 0
    set_local $43
    get_local $7
    get_local $43
    i64.store offset=464
    get_local $7
    get_local $42
    i64.store offset=472
    get_local $7
    i64.load offset=520
    set_local $44
    get_local $7
    i64.load offset=512
    set_local $45
    i32.const 264
    set_local $46
    get_local $7
    get_local $46
    i32.add
    set_local $47
    get_local $47
    get_local $45
    get_local $44
    call $toRep_3
    i32.const 264
    set_local $48
    get_local $7
    get_local $48
    i32.add
    set_local $49
    get_local $49
    get_local $27
    i32.add
    set_local $50
    get_local $50
    i64.load align=1
    set_local $51
    get_local $7
    i64.load offset=264 align=1
    set_local $52
    i64.const 281474976710655
    set_local $53
    get_local $51
    get_local $53
    i64.and
    set_local $54
    get_local $7
    get_local $52
    i64.store offset=448
    get_local $7
    get_local $54
    i64.store offset=456
    get_local $7
    i64.load offset=504
    set_local $55
    get_local $7
    i64.load offset=496
    set_local $56
    i32.const 248
    set_local $57
    get_local $7
    get_local $57
    i32.add
    set_local $58
    get_local $58
    get_local $56
    get_local $55
    call $toRep_3
    i32.const 248
    set_local $59
    get_local $7
    get_local $59
    i32.add
    set_local $60
    get_local $60
    get_local $27
    i32.add
    set_local $61
    get_local $61
    i64.load align=1
    set_local $62
    get_local $7
    i64.load offset=248 align=1
    set_local $63
    get_local $62
    get_local $53
    i64.and
    set_local $64
    get_local $7
    get_local $63
    i64.store offset=432
    get_local $7
    get_local $64
    i64.store offset=440
    get_local $7
    get_local $9
    i32.store offset=428
    get_local $7
    i32.load offset=492
    set_local $65
    i32.const 1
    set_local $66
    get_local $65
    get_local $66
    i32.sub
    set_local $67
    get_local $67
    set_local $68
    get_local $8
    set_local $69
    get_local $68
    get_local $69
    i32.ge_u
    set_local $70
    get_local $70
    set_local $71
    block $block
      block $block_0
        block $block_1
          get_local $71
          br_if $block_1
          i32.const 32766
          set_local $72
          get_local $7
          i32.load offset=488
          set_local $73
          i32.const 1
          set_local $74
          get_local $73
          get_local $74
          i32.sub
          set_local $75
          get_local $75
          set_local $76
          get_local $72
          set_local $77
          get_local $76
          get_local $77
          i32.ge_u
          set_local $78
          get_local $78
          set_local $79
          get_local $79
          i32.eqz
          br_if $block_0
        end ;; $block_1
        get_local $7
        i64.load offset=520
        set_local $80
        get_local $7
        i64.load offset=512
        set_local $81
        i32.const 232
        set_local $82
        get_local $7
        get_local $82
        i32.add
        set_local $83
        get_local $83
        get_local $81
        get_local $80
        call $toRep_3
        i32.const 8
        set_local $84
        i32.const 232
        set_local $85
        get_local $7
        get_local $85
        i32.add
        set_local $86
        get_local $86
        get_local $84
        i32.add
        set_local $87
        get_local $87
        i64.load align=1
        set_local $88
        get_local $7
        i64.load offset=232 align=1
        set_local $89
        i64.const 9223372036854775807
        set_local $90
        get_local $88
        get_local $90
        i64.and
        set_local $91
        get_local $7
        get_local $89
        i64.store offset=400
        get_local $7
        get_local $91
        i64.store offset=408
        get_local $7
        i64.load offset=504
        set_local $92
        get_local $7
        i64.load offset=496
        set_local $93
        i32.const 216
        set_local $94
        get_local $7
        get_local $94
        i32.add
        set_local $95
        get_local $95
        get_local $93
        get_local $92
        call $toRep_3
        i32.const 216
        set_local $96
        get_local $7
        get_local $96
        i32.add
        set_local $97
        get_local $97
        get_local $84
        i32.add
        set_local $98
        get_local $98
        i64.load align=1
        set_local $99
        get_local $7
        i64.load offset=216 align=1
        set_local $100
        get_local $99
        get_local $90
        i64.and
        set_local $101
        get_local $7
        get_local $100
        i64.store offset=384
        get_local $7
        get_local $101
        i64.store offset=392
        get_local $7
        i64.load offset=400
        set_local $102
        get_local $7
        i64.load offset=408
        set_local $103
        i64.const 9223090561878065152
        set_local $104
        get_local $103
        get_local $104
        i64.eq
        set_local $105
        get_local $103
        get_local $104
        i64.gt_u
        set_local $106
        i64.const 0
        set_local $107
        get_local $102
        get_local $107
        i64.ne
        set_local $108
        get_local $108
        get_local $106
        get_local $105
        select
        set_local $109
        get_local $109
        set_local $110
        block $block_2
          get_local $110
          i32.eqz
          br_if $block_2
          get_local $7
          i64.load offset=520
          set_local $111
          get_local $7
          i64.load offset=512
          set_local $112
          i32.const 16
          set_local $113
          get_local $7
          get_local $113
          i32.add
          set_local $114
          get_local $114
          get_local $112
          get_local $111
          call $toRep_3
          i32.const 24
          set_local $115
          get_local $7
          get_local $115
          i32.add
          set_local $116
          get_local $116
          i64.load align=1
          set_local $117
          get_local $7
          i64.load offset=16 align=1
          set_local $118
          i64.const 140737488355328
          set_local $119
          get_local $117
          get_local $119
          i64.or
          set_local $120
          get_local $7
          get_local $118
          get_local $120
          call $fromRep_2
          get_local $7
          i64.load align=1
          set_local $121
          get_local $7
          i64.load offset=8 align=1
          set_local $122
          get_local $7
          get_local $122
          i64.store offset=536
          get_local $7
          get_local $121
          i64.store offset=528
          br $block
        end ;; $block_2
        get_local $7
        i64.load offset=384
        set_local $123
        get_local $7
        i64.load offset=392
        set_local $124
        i64.const 9223090561878065152
        set_local $125
        get_local $124
        get_local $125
        i64.eq
        set_local $126
        get_local $124
        get_local $125
        i64.gt_u
        set_local $127
        i64.const 0
        set_local $128
        get_local $123
        get_local $128
        i64.ne
        set_local $129
        get_local $129
        get_local $127
        get_local $126
        select
        set_local $130
        get_local $130
        set_local $131
        block $block_3
          get_local $131
          i32.eqz
          br_if $block_3
          get_local $7
          i64.load offset=504
          set_local $132
          get_local $7
          i64.load offset=496
          set_local $133
          i32.const 48
          set_local $134
          get_local $7
          get_local $134
          i32.add
          set_local $135
          get_local $135
          get_local $133
          get_local $132
          call $toRep_3
          i32.const 56
          set_local $136
          get_local $7
          get_local $136
          i32.add
          set_local $137
          get_local $137
          i64.load align=1
          set_local $138
          get_local $7
          i64.load offset=48 align=1
          set_local $139
          i64.const 140737488355328
          set_local $140
          get_local $138
          get_local $140
          i64.or
          set_local $141
          i32.const 32
          set_local $142
          get_local $7
          get_local $142
          i32.add
          set_local $143
          get_local $143
          get_local $139
          get_local $141
          call $fromRep_2
          get_local $7
          i64.load offset=32 align=1
          set_local $144
          get_local $7
          i64.load offset=40 align=1
          set_local $145
          get_local $7
          get_local $145
          i64.store offset=536
          get_local $7
          get_local $144
          i64.store offset=528
          br $block
        end ;; $block_3
        get_local $7
        i64.load offset=400
        set_local $146
        get_local $7
        i64.load offset=408
        set_local $147
        i64.const 9223090561878065152
        set_local $148
        get_local $147
        get_local $148
        i64.xor
        set_local $149
        get_local $146
        get_local $149
        i64.or
        set_local $150
        get_local $150
        i64.eqz
        set_local $151
        get_local $151
        set_local $152
        block $block_4
          get_local $152
          i32.eqz
          br_if $block_4
          get_local $7
          i64.load offset=392
          set_local $153
          get_local $7
          i64.load offset=384
          set_local $154
          get_local $154
          get_local $153
          i64.or
          set_local $155
          i64.const 0
          set_local $156
          get_local $155
          get_local $156
          i64.ne
          set_local $157
          get_local $157
          set_local $158
          block $block_5
            get_local $158
            i32.eqz
            br_if $block_5
            get_local $7
            i64.load offset=400
            set_local $159
            get_local $7
            i64.load offset=408
            set_local $160
            get_local $7
            i64.load offset=464
            set_local $161
            get_local $7
            i64.load offset=472
            set_local $162
            get_local $160
            get_local $162
            i64.or
            set_local $163
            get_local $159
            get_local $161
            i64.or
            set_local $164
            i32.const 64
            set_local $165
            get_local $7
            get_local $165
            i32.add
            set_local $166
            get_local $166
            get_local $164
            get_local $163
            call $fromRep_2
            get_local $7
            i64.load offset=64 align=1
            set_local $167
            get_local $7
            i64.load offset=72 align=1
            set_local $168
            get_local $7
            get_local $168
            i64.store offset=536
            get_local $7
            get_local $167
            i64.store offset=528
            br $block
          end ;; $block_5
          i64.const 9223231299366420480
          set_local $169
          i64.const 0
          set_local $170
          i32.const 80
          set_local $171
          get_local $7
          get_local $171
          i32.add
          set_local $172
          get_local $172
          get_local $170
          get_local $169
          call $fromRep_2
          get_local $7
          i64.load offset=80 align=1
          set_local $173
          get_local $7
          i64.load offset=88 align=1
          set_local $174
          get_local $7
          get_local $174
          i64.store offset=536
          get_local $7
          get_local $173
          i64.store offset=528
          br $block
        end ;; $block_4
        get_local $7
        i64.load offset=384
        set_local $175
        get_local $7
        i64.load offset=392
        set_local $176
        i64.const 9223090561878065152
        set_local $177
        get_local $176
        get_local $177
        i64.xor
        set_local $178
        get_local $175
        get_local $178
        i64.or
        set_local $179
        get_local $179
        i64.eqz
        set_local $180
        get_local $180
        set_local $181
        block $block_6
          get_local $181
          i32.eqz
          br_if $block_6
          get_local $7
          i64.load offset=408
          set_local $182
          get_local $7
          i64.load offset=400
          set_local $183
          get_local $183
          get_local $182
          i64.or
          set_local $184
          i64.const 0
          set_local $185
          get_local $184
          get_local $185
          i64.ne
          set_local $186
          get_local $186
          set_local $187
          block $block_7
            get_local $187
            i32.eqz
            br_if $block_7
            get_local $7
            i64.load offset=384
            set_local $188
            get_local $7
            i64.load offset=392
            set_local $189
            get_local $7
            i64.load offset=464
            set_local $190
            get_local $7
            i64.load offset=472
            set_local $191
            get_local $189
            get_local $191
            i64.or
            set_local $192
            get_local $188
            get_local $190
            i64.or
            set_local $193
            i32.const 96
            set_local $194
            get_local $7
            get_local $194
            i32.add
            set_local $195
            get_local $195
            get_local $193
            get_local $192
            call $fromRep_2
            get_local $7
            i64.load offset=96 align=1
            set_local $196
            get_local $7
            i64.load offset=104 align=1
            set_local $197
            get_local $7
            get_local $197
            i64.store offset=536
            get_local $7
            get_local $196
            i64.store offset=528
            br $block
          end ;; $block_7
          i64.const 9223231299366420480
          set_local $198
          i64.const 0
          set_local $199
          i32.const 112
          set_local $200
          get_local $7
          get_local $200
          i32.add
          set_local $201
          get_local $201
          get_local $199
          get_local $198
          call $fromRep_2
          get_local $7
          i64.load offset=112 align=1
          set_local $202
          get_local $7
          i64.load offset=120 align=1
          set_local $203
          get_local $7
          get_local $203
          i64.store offset=536
          get_local $7
          get_local $202
          i64.store offset=528
          br $block
        end ;; $block_6
        get_local $7
        i64.load offset=408
        set_local $204
        get_local $7
        i64.load offset=400
        set_local $205
        get_local $205
        get_local $204
        i64.or
        set_local $206
        i64.const 0
        set_local $207
        get_local $206
        get_local $207
        i64.ne
        set_local $208
        get_local $208
        set_local $209
        block $block_8
          get_local $209
          br_if $block_8
          get_local $7
          i64.load offset=472
          set_local $210
          get_local $7
          i64.load offset=464
          set_local $211
          i32.const 192
          set_local $212
          get_local $7
          get_local $212
          i32.add
          set_local $213
          get_local $213
          get_local $211
          get_local $210
          call $fromRep_2
          get_local $7
          i64.load offset=192 align=1
          set_local $214
          get_local $7
          i64.load offset=200 align=1
          set_local $215
          get_local $7
          get_local $215
          i64.store offset=536
          get_local $7
          get_local $214
          i64.store offset=528
          br $block
        end ;; $block_8
        get_local $7
        i64.load offset=392
        set_local $216
        get_local $7
        i64.load offset=384
        set_local $217
        get_local $217
        get_local $216
        i64.or
        set_local $218
        i64.const 0
        set_local $219
        get_local $218
        get_local $219
        i64.ne
        set_local $220
        get_local $220
        set_local $221
        block $block_9
          get_local $221
          br_if $block_9
          get_local $7
          i64.load offset=472
          set_local $222
          get_local $7
          i64.load offset=464
          set_local $223
          i32.const 176
          set_local $224
          get_local $7
          get_local $224
          i32.add
          set_local $225
          get_local $225
          get_local $223
          get_local $222
          call $fromRep_2
          get_local $7
          i64.load offset=176 align=1
          set_local $226
          get_local $7
          i64.load offset=184 align=1
          set_local $227
          get_local $7
          get_local $227
          i64.store offset=536
          get_local $7
          get_local $226
          i64.store offset=528
          br $block
        end ;; $block_9
        get_local $7
        i64.load offset=408
        set_local $228
        i64.const 281474976710656
        set_local $229
        get_local $228
        get_local $229
        i64.lt_u
        set_local $230
        get_local $230
        set_local $231
        block $block_10
          get_local $231
          i32.eqz
          br_if $block_10
          i32.const 448
          set_local $232
          get_local $7
          get_local $232
          i32.add
          set_local $233
          get_local $233
          set_local $234
          get_local $234
          call $normalize_0
          set_local $235
          get_local $7
          i32.load offset=428
          set_local $236
          get_local $236
          get_local $235
          i32.add
          set_local $237
          get_local $7
          get_local $237
          i32.store offset=428
        end ;; $block_10
        get_local $7
        i64.load offset=392
        set_local $238
        i64.const 281474976710656
        set_local $239
        get_local $238
        get_local $239
        i64.lt_u
        set_local $240
        get_local $240
        set_local $241
        block $block_11
          get_local $241
          i32.eqz
          br_if $block_11
          i32.const 432
          set_local $242
          get_local $7
          get_local $242
          i32.add
          set_local $243
          get_local $243
          set_local $244
          get_local $244
          call $normalize_0
          set_local $245
          get_local $7
          i32.load offset=428
          set_local $246
          get_local $246
          get_local $245
          i32.add
          set_local $247
          get_local $7
          get_local $247
          i32.store offset=428
        end ;; $block_11
      end ;; $block_0
      get_local $7
      i64.load offset=456
      set_local $248
      i64.const 281474976710656
      set_local $249
      get_local $248
      get_local $249
      i64.or
      set_local $250
      get_local $7
      get_local $250
      i64.store offset=456
      get_local $7
      i64.load offset=440
      set_local $251
      get_local $251
      get_local $249
      i64.or
      set_local $252
      get_local $7
      get_local $252
      i64.store offset=440
      get_local $7
      i64.load offset=456
      set_local $253
      get_local $7
      i64.load offset=448
      set_local $254
      get_local $7
      i64.load offset=440
      set_local $255
      get_local $7
      i64.load offset=432
      set_local $256
      i64.const 49
      set_local $257
      get_local $256
      get_local $257
      i64.shr_u
      set_local $258
      i64.const 15
      set_local $259
      get_local $255
      get_local $259
      i64.shl
      set_local $260
      get_local $260
      get_local $258
      i64.or
      set_local $261
      get_local $256
      get_local $259
      i64.shl
      set_local $262
      i32.const 368
      set_local $263
      get_local $7
      get_local $263
      i32.add
      set_local $264
      i32.const 352
      set_local $265
      get_local $7
      get_local $265
      i32.add
      set_local $266
      get_local $254
      get_local $253
      get_local $262
      get_local $261
      get_local $264
      get_local $266
      call $wideMultiply
      get_local $7
      i32.load offset=492
      set_local $267
      get_local $7
      i32.load offset=488
      set_local $268
      get_local $267
      get_local $268
      i32.add
      set_local $269
      get_local $7
      i32.load offset=428
      set_local $270
      get_local $269
      get_local $270
      i32.add
      set_local $271
      i32.const -16383
      set_local $272
      get_local $271
      get_local $272
      i32.add
      set_local $273
      get_local $7
      get_local $273
      i32.store offset=348
      get_local $7
      i32.load8_u offset=382
      set_local $274
      i32.const 1
      set_local $275
      get_local $274
      get_local $275
      i32.and
      set_local $276
      get_local $276
      set_local $277
      block $block_12
        block $block_13
          get_local $277
          i32.eqz
          br_if $block_13
          get_local $7
          i32.load offset=348
          set_local $278
          i32.const 1
          set_local $279
          get_local $278
          get_local $279
          i32.add
          set_local $280
          get_local $7
          get_local $280
          i32.store offset=348
          br $block_12
        end ;; $block_13
        i32.const 368
        set_local $281
        get_local $7
        get_local $281
        i32.add
        set_local $282
        get_local $282
        set_local $283
        i32.const 352
        set_local $284
        get_local $7
        get_local $284
        i32.add
        set_local $285
        get_local $285
        set_local $286
        i32.const 1
        set_local $287
        get_local $283
        get_local $286
        get_local $287
        call $wideLeftShift
      end ;; $block_12
      i32.const 32767
      set_local $288
      get_local $7
      i32.load offset=348
      set_local $289
      get_local $289
      set_local $290
      get_local $288
      set_local $291
      get_local $290
      get_local $291
      i32.ge_s
      set_local $292
      get_local $292
      set_local $293
      block $block_14
        get_local $293
        i32.eqz
        br_if $block_14
        get_local $7
        i64.load offset=464
        set_local $294
        get_local $7
        i64.load offset=472
        set_local $295
        i64.const 9223090561878065152
        set_local $296
        get_local $295
        get_local $296
        i64.or
        set_local $297
        i32.const 128
        set_local $298
        get_local $7
        get_local $298
        i32.add
        set_local $299
        get_local $299
        get_local $294
        get_local $297
        call $fromRep_2
        get_local $7
        i64.load offset=128 align=1
        set_local $300
        get_local $7
        i64.load offset=136 align=1
        set_local $301
        get_local $7
        get_local $301
        i64.store offset=536
        get_local $7
        get_local $300
        i64.store offset=528
        br $block
      end ;; $block_14
      i32.const 0
      set_local $302
      get_local $7
      i32.load offset=348
      set_local $303
      get_local $303
      set_local $304
      get_local $302
      set_local $305
      get_local $304
      get_local $305
      i32.le_s
      set_local $306
      get_local $306
      set_local $307
      block $block_15
        block $block_16
          get_local $307
          i32.eqz
          br_if $block_16
          i32.const 128
          set_local $308
          get_local $7
          i64.load32_u offset=348
          set_local $309
          i64.const 1
          set_local $310
          get_local $310
          get_local $309
          i64.sub
          set_local $311
          get_local $311
          i32.wrap/i64
          set_local $312
          get_local $7
          get_local $312
          i32.store offset=344
          get_local $7
          i32.load offset=344
          set_local $313
          get_local $313
          set_local $314
          get_local $308
          set_local $315
          get_local $314
          get_local $315
          i32.ge_u
          set_local $316
          get_local $316
          set_local $317
          block $block_17
            get_local $317
            i32.eqz
            br_if $block_17
            get_local $7
            i64.load offset=472
            set_local $318
            get_local $7
            i64.load offset=464
            set_local $319
            i32.const 144
            set_local $320
            get_local $7
            get_local $320
            i32.add
            set_local $321
            get_local $321
            get_local $319
            get_local $318
            call $fromRep_2
            get_local $7
            i64.load offset=144 align=1
            set_local $322
            get_local $7
            i64.load offset=152 align=1
            set_local $323
            get_local $7
            get_local $323
            i64.store offset=536
            get_local $7
            get_local $322
            i64.store offset=528
            br $block
          end ;; $block_17
          i32.const 368
          set_local $324
          get_local $7
          get_local $324
          i32.add
          set_local $325
          get_local $325
          set_local $326
          i32.const 352
          set_local $327
          get_local $7
          get_local $327
          i32.add
          set_local $328
          get_local $328
          set_local $329
          get_local $7
          i32.load offset=344
          set_local $330
          get_local $326
          get_local $329
          get_local $330
          call $wideRightShiftWithSticky
          br $block_15
        end ;; $block_16
        get_local $7
        i64.load offset=376
        set_local $331
        i64.const 281474976710655
        set_local $332
        get_local $331
        get_local $332
        i64.and
        set_local $333
        get_local $7
        get_local $333
        i64.store offset=376
        get_local $7
        i64.load32_u offset=348
        set_local $334
        i64.const 48
        set_local $335
        get_local $334
        get_local $335
        i64.shl
        set_local $336
        get_local $7
        i64.load offset=368
        set_local $337
        get_local $7
        i64.load offset=376
        set_local $338
        get_local $338
        get_local $336
        i64.or
        set_local $339
        get_local $7
        get_local $337
        i64.store offset=368
        get_local $7
        get_local $339
        i64.store offset=376
      end ;; $block_15
      get_local $7
      i64.load offset=472
      set_local $340
      get_local $7
      i64.load offset=464
      set_local $341
      get_local $7
      i64.load offset=376
      set_local $342
      get_local $7
      i64.load offset=368
      set_local $343
      get_local $343
      get_local $341
      i64.or
      set_local $344
      get_local $342
      get_local $340
      i64.or
      set_local $345
      get_local $7
      get_local $345
      i64.store offset=376
      get_local $7
      get_local $344
      i64.store offset=368
      get_local $7
      i64.load offset=352
      set_local $346
      get_local $7
      i64.load offset=360
      set_local $347
      i64.const -9223372036854775808
      set_local $348
      get_local $347
      get_local $348
      i64.eq
      set_local $349
      get_local $347
      get_local $348
      i64.gt_u
      set_local $350
      i64.const 0
      set_local $351
      get_local $346
      get_local $351
      i64.ne
      set_local $352
      get_local $352
      get_local $350
      get_local $349
      select
      set_local $353
      get_local $353
      set_local $354
      block $block_18
        get_local $354
        i32.eqz
        br_if $block_18
        get_local $7
        i64.load offset=376
        set_local $355
        get_local $7
        i64.load offset=368
        set_local $356
        i64.const 1
        set_local $357
        get_local $356
        get_local $357
        i64.add
        set_local $358
        get_local $358
        get_local $356
        i64.lt_u
        set_local $359
        get_local $359
        i64.extend_u/i32
        set_local $360
        get_local $355
        get_local $360
        i64.add
        set_local $361
        get_local $7
        get_local $358
        i64.store offset=368
        get_local $7
        get_local $361
        i64.store offset=376
      end ;; $block_18
      get_local $7
      i64.load offset=352
      set_local $362
      get_local $7
      i64.load offset=360
      set_local $363
      i64.const -9223372036854775808
      set_local $364
      get_local $363
      get_local $364
      i64.xor
      set_local $365
      get_local $362
      get_local $365
      i64.or
      set_local $366
      get_local $366
      i64.eqz
      set_local $367
      get_local $367
      set_local $368
      block $block_19
        get_local $368
        i32.eqz
        br_if $block_19
        get_local $7
        i64.load offset=376
        set_local $369
        get_local $7
        i64.load offset=368
        set_local $370
        i64.const 1
        set_local $371
        get_local $370
        get_local $371
        i64.and
        set_local $372
        get_local $370
        get_local $372
        i64.add
        set_local $373
        get_local $373
        get_local $370
        i64.lt_u
        set_local $374
        get_local $374
        i64.extend_u/i32
        set_local $375
        get_local $369
        get_local $375
        i64.add
        set_local $376
        get_local $7
        get_local $373
        i64.store offset=368
        get_local $7
        get_local $376
        i64.store offset=376
      end ;; $block_19
      get_local $7
      i64.load offset=376
      set_local $377
      get_local $7
      i64.load offset=368
      set_local $378
      i32.const 160
      set_local $379
      get_local $7
      get_local $379
      i32.add
      set_local $380
      get_local $380
      get_local $378
      get_local $377
      call $fromRep_2
      get_local $7
      i64.load offset=160 align=1
      set_local $381
      get_local $7
      i64.load offset=168 align=1
      set_local $382
      get_local $7
      get_local $382
      i64.store offset=536
      get_local $7
      get_local $381
      i64.store offset=528
    end ;; $block
    get_local $7
    i64.load offset=528
    set_local $383
    get_local $7
    i64.load offset=536
    set_local $384
    get_local $0
    get_local $384
    i64.store offset=8
    get_local $0
    get_local $383
    i64.store
    i32.const 544
    set_local $385
    get_local $7
    get_local $385
    i32.add
    set_local $386
    get_local $386
    set_global $25
    return
    )
  
  (func $toRep_3 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $fromRep_2 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $normalize_0 (type $4)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i32)
    (local $11 i64)
    (local $12 i64)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i32)
    (local $18 i64)
    (local $19 i64)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i64)
    (local $26 i64)
    (local $27 i32)
    (local $28 i32)
    (local $29 i32)
    (local $30 i32)
    get_global $25
    set_local $1
    i32.const 32
    set_local $2
    get_local $1
    get_local $2
    i32.sub
    set_local $3
    get_local $3
    set_global $25
    i32.const 1
    set_local $4
    get_local $3
    get_local $0
    i32.store offset=28
    get_local $3
    i32.load offset=28
    set_local $5
    i32.const 8
    set_local $6
    get_local $5
    get_local $6
    i32.add
    set_local $7
    get_local $7
    i64.load
    set_local $8
    get_local $5
    i64.load
    set_local $9
    get_local $9
    get_local $8
    call $rep_clz_0
    set_local $10
    i64.const 281474976710656
    set_local $11
    i64.const 0
    set_local $12
    get_local $12
    get_local $11
    call $rep_clz_0
    set_local $13
    get_local $10
    get_local $13
    i32.sub
    set_local $14
    get_local $3
    get_local $14
    i32.store offset=24
    get_local $3
    i32.load offset=24
    set_local $15
    get_local $3
    i32.load offset=28
    set_local $16
    get_local $16
    get_local $6
    i32.add
    set_local $17
    get_local $17
    i64.load
    set_local $18
    get_local $16
    i64.load
    set_local $19
    i32.const 8
    set_local $20
    get_local $3
    get_local $20
    i32.add
    set_local $21
    get_local $21
    get_local $19
    get_local $18
    get_local $15
    call $__ashlti3
    i32.const 8
    set_local $22
    get_local $3
    get_local $22
    i32.add
    set_local $23
    get_local $23
    get_local $6
    i32.add
    set_local $24
    get_local $24
    i64.load align=1
    set_local $25
    get_local $3
    i64.load offset=8 align=1
    set_local $26
    get_local $17
    get_local $25
    i64.store
    get_local $16
    get_local $26
    i64.store
    get_local $3
    i32.load offset=24
    set_local $27
    get_local $4
    get_local $27
    i32.sub
    set_local $28
    i32.const 32
    set_local $29
    get_local $3
    get_local $29
    i32.add
    set_local $30
    get_local $30
    set_global $25
    get_local $28
    return
    )
  
  (func $wideMultiply (type $22)
    (param $0 i64)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i32)
    (param $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i64)
    (local $13 i64)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i64)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i64)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i64)
    (local $35 i64)
    (local $36 i64)
    (local $37 i64)
    (local $38 i64)
    (local $39 i64)
    (local $40 i64)
    (local $41 i64)
    (local $42 i64)
    (local $43 i64)
    (local $44 i64)
    (local $45 i64)
    (local $46 i64)
    (local $47 i64)
    (local $48 i64)
    (local $49 i64)
    (local $50 i64)
    (local $51 i64)
    (local $52 i64)
    (local $53 i64)
    (local $54 i64)
    (local $55 i64)
    (local $56 i64)
    (local $57 i64)
    (local $58 i64)
    (local $59 i64)
    (local $60 i64)
    (local $61 i64)
    (local $62 i32)
    (local $63 i64)
    (local $64 i64)
    (local $65 i64)
    (local $66 i64)
    (local $67 i32)
    (local $68 i64)
    (local $69 i64)
    (local $70 i64)
    (local $71 i32)
    (local $72 i64)
    (local $73 i64)
    (local $74 i64)
    (local $75 i64)
    (local $76 i64)
    (local $77 i32)
    (local $78 i64)
    (local $79 i64)
    (local $80 i64)
    (local $81 i32)
    (local $82 i64)
    (local $83 i64)
    (local $84 i64)
    (local $85 i64)
    (local $86 i32)
    (local $87 i64)
    (local $88 i64)
    (local $89 i64)
    (local $90 i64)
    (local $91 i64)
    (local $92 i32)
    (local $93 i64)
    (local $94 i64)
    (local $95 i64)
    (local $96 i32)
    (local $97 i64)
    (local $98 i64)
    (local $99 i64)
    (local $100 i64)
    (local $101 i64)
    (local $102 i32)
    (local $103 i64)
    (local $104 i64)
    (local $105 i64)
    (local $106 i64)
    (local $107 i64)
    (local $108 i64)
    (local $109 i64)
    (local $110 i32)
    (local $111 i64)
    (local $112 i64)
    (local $113 i64)
    (local $114 i64)
    (local $115 i32)
    (local $116 i64)
    (local $117 i64)
    (local $118 i64)
    (local $119 i32)
    (local $120 i64)
    (local $121 i64)
    (local $122 i64)
    (local $123 i64)
    (local $124 i64)
    (local $125 i32)
    (local $126 i64)
    (local $127 i64)
    (local $128 i64)
    (local $129 i64)
    (local $130 i64)
    (local $131 i64)
    (local $132 i32)
    (local $133 i64)
    (local $134 i64)
    (local $135 i64)
    (local $136 i32)
    (local $137 i64)
    (local $138 i64)
    (local $139 i64)
    (local $140 i32)
    (local $141 i64)
    (local $142 i64)
    (local $143 i64)
    (local $144 i64)
    (local $145 i64)
    (local $146 i64)
    (local $147 i64)
    (local $148 i64)
    (local $149 i64)
    (local $150 i32)
    (local $151 i64)
    (local $152 i64)
    (local $153 i64)
    (local $154 i64)
    (local $155 i64)
    (local $156 i64)
    (local $157 i32)
    (local $158 i64)
    (local $159 i64)
    (local $160 i64)
    (local $161 i64)
    (local $162 i64)
    (local $163 i64)
    (local $164 i64)
    (local $165 i64)
    (local $166 i64)
    (local $167 i64)
    (local $168 i32)
    (local $169 i64)
    (local $170 i64)
    (local $171 i64)
    (local $172 i64)
    (local $173 i32)
    (local $174 i32)
    (local $175 i32)
    get_global $25
    set_local $6
    i32.const 320
    set_local $7
    get_local $6
    get_local $7
    i32.sub
    set_local $8
    get_local $8
    set_global $25
    get_local $8
    get_local $0
    i64.store offset=304
    get_local $8
    get_local $1
    i64.store offset=312
    get_local $8
    get_local $3
    i64.store offset=296
    get_local $8
    get_local $2
    i64.store offset=288
    get_local $8
    get_local $4
    i32.store offset=284
    get_local $8
    get_local $5
    i32.store offset=280
    get_local $8
    i64.load32_u offset=316
    set_local $9
    get_local $8
    i64.load32_u offset=300
    set_local $10
    get_local $9
    get_local $10
    i64.mul
    set_local $11
    get_local $8
    get_local $11
    i64.store offset=272
    get_local $8
    i64.load32_u offset=316
    set_local $12
    get_local $8
    i64.load32_u offset=296
    set_local $13
    get_local $12
    get_local $13
    i64.mul
    set_local $14
    get_local $8
    get_local $14
    i64.store offset=264
    get_local $8
    i64.load32_u offset=316
    set_local $15
    get_local $8
    i64.load32_u offset=292
    set_local $16
    get_local $15
    get_local $16
    i64.mul
    set_local $17
    get_local $8
    get_local $17
    i64.store offset=256
    get_local $8
    i64.load32_u offset=316
    set_local $18
    get_local $8
    i64.load32_u offset=288
    set_local $19
    get_local $18
    get_local $19
    i64.mul
    set_local $20
    get_local $8
    get_local $20
    i64.store offset=248
    get_local $8
    i64.load32_u offset=312
    set_local $21
    get_local $8
    i64.load32_u offset=300
    set_local $22
    get_local $21
    get_local $22
    i64.mul
    set_local $23
    get_local $8
    get_local $23
    i64.store offset=240
    get_local $8
    i64.load32_u offset=312
    set_local $24
    get_local $8
    i64.load32_u offset=296
    set_local $25
    get_local $24
    get_local $25
    i64.mul
    set_local $26
    get_local $8
    get_local $26
    i64.store offset=232
    get_local $8
    i64.load32_u offset=312
    set_local $27
    get_local $8
    i64.load32_u offset=292
    set_local $28
    get_local $27
    get_local $28
    i64.mul
    set_local $29
    get_local $8
    get_local $29
    i64.store offset=224
    get_local $8
    i64.load32_u offset=312
    set_local $30
    get_local $8
    i64.load32_u offset=288
    set_local $31
    get_local $30
    get_local $31
    i64.mul
    set_local $32
    get_local $8
    get_local $32
    i64.store offset=216
    get_local $8
    i64.load32_u offset=308
    set_local $33
    get_local $8
    i64.load32_u offset=300
    set_local $34
    get_local $33
    get_local $34
    i64.mul
    set_local $35
    get_local $8
    get_local $35
    i64.store offset=208
    get_local $8
    i64.load32_u offset=308
    set_local $36
    get_local $8
    i64.load32_u offset=296
    set_local $37
    get_local $36
    get_local $37
    i64.mul
    set_local $38
    get_local $8
    get_local $38
    i64.store offset=200
    get_local $8
    i64.load32_u offset=308
    set_local $39
    get_local $8
    i64.load32_u offset=292
    set_local $40
    get_local $39
    get_local $40
    i64.mul
    set_local $41
    get_local $8
    get_local $41
    i64.store offset=192
    get_local $8
    i64.load32_u offset=308
    set_local $42
    get_local $8
    i64.load32_u offset=288
    set_local $43
    get_local $42
    get_local $43
    i64.mul
    set_local $44
    get_local $8
    get_local $44
    i64.store offset=184
    get_local $8
    i64.load32_u offset=304
    set_local $45
    get_local $8
    i64.load32_u offset=300
    set_local $46
    get_local $45
    get_local $46
    i64.mul
    set_local $47
    get_local $8
    get_local $47
    i64.store offset=176
    get_local $8
    i64.load32_u offset=304
    set_local $48
    get_local $8
    i64.load32_u offset=296
    set_local $49
    get_local $48
    get_local $49
    i64.mul
    set_local $50
    get_local $8
    get_local $50
    i64.store offset=168
    get_local $8
    i64.load32_u offset=304
    set_local $51
    get_local $8
    i64.load32_u offset=292
    set_local $52
    get_local $51
    get_local $52
    i64.mul
    set_local $53
    get_local $8
    get_local $53
    i64.store offset=160
    get_local $8
    i64.load32_u offset=304
    set_local $54
    get_local $8
    i64.load32_u offset=288
    set_local $55
    get_local $54
    get_local $55
    i64.mul
    set_local $56
    get_local $8
    get_local $56
    i64.store offset=152
    get_local $8
    i64.load offset=152
    set_local $57
    i64.const 0
    set_local $58
    get_local $8
    get_local $58
    i64.store offset=136
    get_local $8
    get_local $57
    i64.store offset=128
    get_local $8
    i64.load offset=184
    set_local $59
    get_local $8
    i64.load offset=160
    set_local $60
    get_local $59
    get_local $60
    i64.add
    set_local $61
    get_local $61
    get_local $59
    i64.lt_u
    set_local $62
    get_local $62
    i64.extend_u/i32
    set_local $63
    get_local $8
    get_local $61
    i64.store offset=112
    get_local $8
    get_local $63
    i64.store offset=120
    get_local $8
    i64.load offset=216
    set_local $64
    get_local $8
    i64.load offset=192
    set_local $65
    get_local $64
    get_local $65
    i64.add
    set_local $66
    get_local $66
    get_local $64
    i64.lt_u
    set_local $67
    get_local $67
    i64.extend_u/i32
    set_local $68
    get_local $8
    i64.load offset=168
    set_local $69
    get_local $66
    get_local $69
    i64.add
    set_local $70
    get_local $70
    get_local $66
    i64.lt_u
    set_local $71
    get_local $71
    i64.extend_u/i32
    set_local $72
    get_local $68
    get_local $72
    i64.add
    set_local $73
    get_local $8
    get_local $70
    i64.store offset=96
    get_local $8
    get_local $73
    i64.store offset=104
    get_local $8
    i64.load offset=248
    set_local $74
    get_local $8
    i64.load offset=224
    set_local $75
    get_local $74
    get_local $75
    i64.add
    set_local $76
    get_local $76
    get_local $74
    i64.lt_u
    set_local $77
    get_local $77
    i64.extend_u/i32
    set_local $78
    get_local $8
    i64.load offset=200
    set_local $79
    get_local $76
    get_local $79
    i64.add
    set_local $80
    get_local $80
    get_local $76
    i64.lt_u
    set_local $81
    get_local $81
    i64.extend_u/i32
    set_local $82
    get_local $78
    get_local $82
    i64.add
    set_local $83
    get_local $8
    i64.load offset=176
    set_local $84
    get_local $80
    get_local $84
    i64.add
    set_local $85
    get_local $85
    get_local $80
    i64.lt_u
    set_local $86
    get_local $86
    i64.extend_u/i32
    set_local $87
    get_local $83
    get_local $87
    i64.add
    set_local $88
    get_local $8
    get_local $85
    i64.store offset=80
    get_local $8
    get_local $88
    i64.store offset=88
    get_local $8
    i64.load offset=256
    set_local $89
    get_local $8
    i64.load offset=232
    set_local $90
    get_local $89
    get_local $90
    i64.add
    set_local $91
    get_local $91
    get_local $89
    i64.lt_u
    set_local $92
    get_local $92
    i64.extend_u/i32
    set_local $93
    get_local $8
    i64.load offset=208
    set_local $94
    get_local $91
    get_local $94
    i64.add
    set_local $95
    get_local $95
    get_local $91
    i64.lt_u
    set_local $96
    get_local $96
    i64.extend_u/i32
    set_local $97
    get_local $93
    get_local $97
    i64.add
    set_local $98
    get_local $8
    get_local $95
    i64.store offset=64
    get_local $8
    get_local $98
    i64.store offset=72
    get_local $8
    i64.load offset=264
    set_local $99
    get_local $8
    i64.load offset=240
    set_local $100
    get_local $99
    get_local $100
    i64.add
    set_local $101
    get_local $101
    get_local $99
    i64.lt_u
    set_local $102
    get_local $102
    i64.extend_u/i32
    set_local $103
    get_local $8
    get_local $101
    i64.store offset=48
    get_local $8
    get_local $103
    i64.store offset=56
    get_local $8
    i64.load offset=272
    set_local $104
    get_local $8
    get_local $58
    i64.store offset=40
    get_local $8
    get_local $104
    i64.store offset=32
    get_local $8
    i64.load offset=128
    set_local $105
    get_local $8
    i64.load32_u offset=112
    set_local $106
    i64.const 32
    set_local $107
    get_local $106
    get_local $107
    i64.shl
    set_local $108
    get_local $105
    get_local $108
    i64.add
    set_local $109
    get_local $109
    get_local $105
    i64.lt_u
    set_local $110
    get_local $110
    i64.extend_u/i32
    set_local $111
    get_local $8
    get_local $109
    i64.store offset=16
    get_local $8
    get_local $111
    i64.store offset=24
    get_local $8
    i64.load offset=136
    set_local $112
    get_local $8
    i64.load offset=116 align=4
    set_local $113
    get_local $112
    get_local $113
    i64.add
    set_local $114
    get_local $114
    get_local $112
    i64.lt_u
    set_local $115
    get_local $115
    i64.extend_u/i32
    set_local $116
    get_local $8
    i64.load offset=96
    set_local $117
    get_local $114
    get_local $117
    i64.add
    set_local $118
    get_local $118
    get_local $114
    i64.lt_u
    set_local $119
    get_local $119
    i64.extend_u/i32
    set_local $120
    get_local $116
    get_local $120
    i64.add
    set_local $121
    get_local $8
    i64.load offset=80
    set_local $122
    get_local $122
    get_local $107
    i64.shl
    set_local $123
    get_local $118
    get_local $123
    i64.add
    set_local $124
    get_local $124
    get_local $118
    i64.lt_u
    set_local $125
    get_local $125
    i64.extend_u/i32
    set_local $126
    get_local $121
    get_local $126
    i64.add
    set_local $127
    get_local $8
    get_local $124
    i64.store
    get_local $8
    get_local $127
    i64.store offset=8
    get_local $8
    i64.load offset=16
    set_local $128
    get_local $8
    i64.load offset=24
    set_local $129
    get_local $8
    i64.load
    set_local $130
    get_local $129
    get_local $130
    i64.add
    set_local $131
    get_local $8
    i32.load offset=280
    set_local $132
    get_local $132
    get_local $128
    i64.store
    get_local $132
    get_local $131
    i64.store offset=8
    get_local $8
    i64.load offset=8
    set_local $133
    get_local $8
    i64.load32_u offset=124
    set_local $134
    get_local $133
    get_local $134
    i64.add
    set_local $135
    get_local $135
    get_local $133
    i64.lt_u
    set_local $136
    get_local $136
    i64.extend_u/i32
    set_local $137
    get_local $8
    i64.load offset=104
    set_local $138
    get_local $135
    get_local $138
    i64.add
    set_local $139
    get_local $139
    get_local $135
    i64.lt_u
    set_local $140
    get_local $140
    i64.extend_u/i32
    set_local $141
    get_local $137
    get_local $141
    i64.add
    set_local $142
    get_local $8
    i64.load32_u offset=84
    set_local $143
    get_local $8
    i64.load offset=88
    set_local $144
    get_local $144
    get_local $107
    i64.shl
    set_local $145
    get_local $143
    get_local $145
    i64.or
    set_local $146
    get_local $144
    get_local $107
    i64.shr_u
    set_local $147
    get_local $142
    get_local $147
    i64.add
    set_local $148
    get_local $139
    get_local $146
    i64.add
    set_local $149
    get_local $149
    get_local $139
    i64.lt_u
    set_local $150
    get_local $150
    i64.extend_u/i32
    set_local $151
    get_local $148
    get_local $151
    i64.add
    set_local $152
    get_local $8
    i64.load offset=64
    set_local $153
    get_local $8
    i64.load offset=72
    set_local $154
    get_local $152
    get_local $154
    i64.add
    set_local $155
    get_local $149
    get_local $153
    i64.add
    set_local $156
    get_local $156
    get_local $149
    i64.lt_u
    set_local $157
    get_local $157
    i64.extend_u/i32
    set_local $158
    get_local $155
    get_local $158
    i64.add
    set_local $159
    get_local $8
    i64.load offset=56
    set_local $160
    get_local $8
    i64.load offset=48
    set_local $161
    get_local $161
    get_local $107
    i64.shr_u
    set_local $162
    get_local $160
    get_local $107
    i64.shl
    set_local $163
    get_local $163
    get_local $162
    i64.or
    set_local $164
    get_local $161
    get_local $107
    i64.shl
    set_local $165
    get_local $159
    get_local $164
    i64.add
    set_local $166
    get_local $156
    get_local $165
    i64.add
    set_local $167
    get_local $167
    get_local $156
    i64.lt_u
    set_local $168
    get_local $168
    i64.extend_u/i32
    set_local $169
    get_local $166
    get_local $169
    i64.add
    set_local $170
    get_local $8
    i64.load offset=32
    set_local $171
    get_local $170
    get_local $171
    i64.add
    set_local $172
    get_local $8
    i32.load offset=284
    set_local $173
    get_local $173
    get_local $167
    i64.store
    get_local $173
    get_local $172
    i64.store offset=8
    i32.const 320
    set_local $174
    get_local $8
    get_local $174
    i32.add
    set_local $175
    get_local $175
    set_global $25
    return
    )
  
  (func $wideLeftShift (type $13)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i32)
    (local $9 i64)
    (local $10 i64)
    (local $11 i32)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i32)
    (local $17 i64)
    (local $18 i64)
    (local $19 i32)
    (local $20 i32)
    (local $21 i64)
    (local $22 i64)
    (local $23 i32)
    (local $24 i32)
    (local $25 i32)
    (local $26 i32)
    (local $27 i32)
    (local $28 i32)
    (local $29 i32)
    (local $30 i64)
    (local $31 i64)
    (local $32 i64)
    (local $33 i64)
    (local $34 i32)
    (local $35 i32)
    (local $36 i64)
    (local $37 i64)
    (local $38 i32)
    (local $39 i32)
    (local $40 i64)
    (local $41 i64)
    (local $42 i32)
    (local $43 i32)
    get_global $25
    set_local $3
    i32.const 64
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    set_global $25
    get_local $5
    get_local $0
    i32.store offset=60
    get_local $5
    get_local $1
    i32.store offset=56
    get_local $5
    get_local $2
    i32.store offset=52
    get_local $5
    i32.load offset=60
    set_local $6
    i32.const 8
    set_local $7
    get_local $6
    get_local $7
    i32.add
    set_local $8
    get_local $8
    i64.load
    set_local $9
    get_local $6
    i64.load
    set_local $10
    get_local $5
    i32.load offset=52
    set_local $11
    i32.const 32
    set_local $12
    get_local $5
    get_local $12
    i32.add
    set_local $13
    get_local $13
    get_local $10
    get_local $9
    get_local $11
    call $__ashlti3
    i32.const 32
    set_local $14
    get_local $5
    get_local $14
    i32.add
    set_local $15
    get_local $15
    get_local $7
    i32.add
    set_local $16
    get_local $16
    i64.load align=1
    set_local $17
    get_local $5
    i64.load offset=32 align=1
    set_local $18
    get_local $5
    i32.load offset=56
    set_local $19
    get_local $19
    get_local $7
    i32.add
    set_local $20
    get_local $20
    i64.load
    set_local $21
    get_local $19
    i64.load
    set_local $22
    i32.const 128
    set_local $23
    get_local $23
    get_local $11
    i32.sub
    set_local $24
    i32.const 16
    set_local $25
    get_local $5
    get_local $25
    i32.add
    set_local $26
    get_local $26
    get_local $22
    get_local $21
    get_local $24
    call $__lshrti3
    i32.const 16
    set_local $27
    get_local $5
    get_local $27
    i32.add
    set_local $28
    get_local $28
    get_local $7
    i32.add
    set_local $29
    get_local $29
    i64.load align=1
    set_local $30
    get_local $5
    i64.load offset=16 align=1
    set_local $31
    get_local $18
    get_local $31
    i64.or
    set_local $32
    get_local $17
    get_local $30
    i64.or
    set_local $33
    get_local $8
    get_local $33
    i64.store
    get_local $6
    get_local $32
    i64.store
    get_local $5
    i32.load offset=56
    set_local $34
    get_local $34
    get_local $7
    i32.add
    set_local $35
    get_local $35
    i64.load
    set_local $36
    get_local $34
    i64.load
    set_local $37
    get_local $5
    i32.load offset=52
    set_local $38
    get_local $5
    get_local $37
    get_local $36
    get_local $38
    call $__ashlti3
    get_local $5
    get_local $7
    i32.add
    set_local $39
    get_local $39
    i64.load align=1
    set_local $40
    get_local $5
    i64.load align=1
    set_local $41
    get_local $35
    get_local $40
    i64.store
    get_local $34
    get_local $41
    i64.store
    i32.const 64
    set_local $42
    get_local $5
    get_local $42
    i32.add
    set_local $43
    get_local $43
    set_global $25
    return
    )
  
  (func $wideRightShiftWithSticky (type $13)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (local $3 i32)
    (local $4 i32)
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
    (local $15 i64)
    (local $16 i64)
    (local $17 i32)
    (local $18 i32)
    (local $19 i32)
    (local $20 i32)
    (local $21 i32)
    (local $22 i32)
    (local $23 i32)
    (local $24 i32)
    (local $25 i64)
    (local $26 i64)
    (local $27 i64)
    (local $28 i64)
    (local $29 i32)
    (local $30 i32)
    (local $31 i32)
    (local $32 i64)
    (local $33 i64)
    (local $34 i32)
    (local $35 i32)
    (local $36 i32)
    (local $37 i32)
    (local $38 i32)
    (local $39 i32)
    (local $40 i32)
    (local $41 i64)
    (local $42 i64)
    (local $43 i32)
    (local $44 i32)
    (local $45 i64)
    (local $46 i64)
    (local $47 i32)
    (local $48 i32)
    (local $49 i32)
    (local $50 i32)
    (local $51 i32)
    (local $52 i64)
    (local $53 i64)
    (local $54 i64)
    (local $55 i64)
    (local $56 i64)
    (local $57 i64)
    (local $58 i64)
    (local $59 i64)
    (local $60 i32)
    (local $61 i32)
    (local $62 i64)
    (local $63 i64)
    (local $64 i32)
    (local $65 i32)
    (local $66 i64)
    (local $67 i64)
    (local $68 i32)
    (local $69 i32)
    (local $70 i32)
    (local $71 i32)
    (local $72 i32)
    (local $73 i32)
    (local $74 i32)
    (local $75 i32)
    (local $76 i32)
    (local $77 i64)
    (local $78 i64)
    (local $79 i32)
    (local $80 i32)
    (local $81 i32)
    (local $82 i32)
    (local $83 i32)
    (local $84 i32)
    (local $85 i32)
    (local $86 i32)
    (local $87 i64)
    (local $88 i64)
    (local $89 i32)
    (local $90 i32)
    (local $91 i64)
    (local $92 i64)
    (local $93 i64)
    (local $94 i64)
    (local $95 i64)
    (local $96 i64)
    (local $97 i32)
    (local $98 i32)
    (local $99 i32)
    (local $100 i64)
    (local $101 i64)
    (local $102 i32)
    (local $103 i32)
    (local $104 i32)
    (local $105 i32)
    (local $106 i32)
    (local $107 i32)
    (local $108 i32)
    (local $109 i32)
    (local $110 i64)
    (local $111 i64)
    (local $112 i64)
    (local $113 i64)
    (local $114 i64)
    (local $115 i64)
    (local $116 i32)
    (local $117 i32)
    (local $118 i32)
    (local $119 i32)
    (local $120 i32)
    (local $121 i64)
    (local $122 i64)
    (local $123 i32)
    (local $124 i32)
    (local $125 i64)
    (local $126 i64)
    (local $127 i64)
    (local $128 i64)
    (local $129 i64)
    (local $130 i64)
    (local $131 i32)
    (local $132 i64)
    (local $133 i64)
    (local $134 i64)
    (local $135 i32)
    (local $136 i32)
    (local $137 i32)
    (local $138 i32)
    get_global $25
    set_local $3
    i32.const 112
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    set_global $25
    i32.const 128
    set_local $6
    get_local $5
    get_local $0
    i32.store offset=108
    get_local $5
    get_local $1
    i32.store offset=104
    get_local $5
    get_local $2
    i32.store offset=100
    get_local $5
    i32.load offset=100
    set_local $7
    get_local $7
    set_local $8
    get_local $6
    set_local $9
    get_local $8
    get_local $9
    i32.lt_u
    set_local $10
    get_local $10
    set_local $11
    block $block
      block $block_0
        get_local $11
        i32.eqz
        br_if $block_0
        get_local $5
        i32.load offset=104
        set_local $12
        i32.const 8
        set_local $13
        get_local $12
        get_local $13
        i32.add
        set_local $14
        get_local $14
        i64.load
        set_local $15
        get_local $12
        i64.load
        set_local $16
        get_local $5
        i32.load offset=100
        set_local $17
        i32.const 128
        set_local $18
        get_local $18
        get_local $17
        i32.sub
        set_local $19
        i32.const 48
        set_local $20
        get_local $5
        get_local $20
        i32.add
        set_local $21
        get_local $21
        get_local $16
        get_local $15
        get_local $19
        call $__ashlti3
        i32.const 48
        set_local $22
        get_local $5
        get_local $22
        i32.add
        set_local $23
        get_local $23
        get_local $13
        i32.add
        set_local $24
        get_local $24
        i64.load align=1
        set_local $25
        get_local $5
        i64.load offset=48 align=1
        set_local $26
        get_local $26
        get_local $25
        i64.or
        set_local $27
        i64.const 0
        set_local $28
        get_local $27
        get_local $28
        i64.ne
        set_local $29
        get_local $5
        get_local $29
        i32.store8 offset=99
        get_local $5
        i32.load offset=108
        set_local $30
        get_local $30
        get_local $13
        i32.add
        set_local $31
        get_local $31
        i64.load
        set_local $32
        get_local $30
        i64.load
        set_local $33
        get_local $5
        i32.load offset=100
        set_local $34
        get_local $18
        get_local $34
        i32.sub
        set_local $35
        i32.const 16
        set_local $36
        get_local $5
        get_local $36
        i32.add
        set_local $37
        get_local $37
        get_local $33
        get_local $32
        get_local $35
        call $__ashlti3
        i32.const 16
        set_local $38
        get_local $5
        get_local $38
        i32.add
        set_local $39
        get_local $39
        get_local $13
        i32.add
        set_local $40
        get_local $40
        i64.load align=1
        set_local $41
        get_local $5
        i64.load offset=16 align=1
        set_local $42
        get_local $5
        i32.load offset=104
        set_local $43
        get_local $43
        get_local $13
        i32.add
        set_local $44
        get_local $44
        i64.load
        set_local $45
        get_local $43
        i64.load
        set_local $46
        i32.const 32
        set_local $47
        get_local $5
        get_local $47
        i32.add
        set_local $48
        get_local $48
        get_local $46
        get_local $45
        get_local $34
        call $__lshrti3
        i32.const 32
        set_local $49
        get_local $5
        get_local $49
        i32.add
        set_local $50
        get_local $50
        get_local $13
        i32.add
        set_local $51
        get_local $51
        i64.load align=1
        set_local $52
        get_local $5
        i64.load offset=32 align=1
        set_local $53
        get_local $41
        get_local $52
        i64.or
        set_local $54
        get_local $42
        get_local $53
        i64.or
        set_local $55
        get_local $5
        i64.load8_u offset=99
        set_local $56
        i64.const 1
        set_local $57
        get_local $56
        get_local $57
        i64.and
        set_local $58
        get_local $55
        get_local $58
        i64.or
        set_local $59
        get_local $44
        get_local $54
        i64.store
        get_local $43
        get_local $59
        i64.store
        get_local $5
        i32.load offset=108
        set_local $60
        get_local $60
        get_local $13
        i32.add
        set_local $61
        get_local $61
        i64.load
        set_local $62
        get_local $60
        i64.load
        set_local $63
        get_local $5
        i32.load offset=100
        set_local $64
        get_local $5
        get_local $63
        get_local $62
        get_local $64
        call $__lshrti3
        get_local $5
        get_local $13
        i32.add
        set_local $65
        get_local $65
        i64.load align=1
        set_local $66
        get_local $5
        i64.load align=1
        set_local $67
        get_local $61
        get_local $66
        i64.store
        get_local $60
        get_local $67
        i64.store
        br $block
      end ;; $block_0
      i32.const 256
      set_local $68
      get_local $5
      i32.load offset=100
      set_local $69
      get_local $69
      set_local $70
      get_local $68
      set_local $71
      get_local $70
      get_local $71
      i32.lt_u
      set_local $72
      get_local $72
      set_local $73
      block $block_1
        block $block_2
          get_local $73
          i32.eqz
          br_if $block_2
          get_local $5
          i32.load offset=108
          set_local $74
          i32.const 8
          set_local $75
          get_local $74
          get_local $75
          i32.add
          set_local $76
          get_local $76
          i64.load
          set_local $77
          get_local $74
          i64.load
          set_local $78
          get_local $5
          i32.load offset=100
          set_local $79
          i32.const 256
          set_local $80
          get_local $80
          get_local $79
          i32.sub
          set_local $81
          i32.const 80
          set_local $82
          get_local $5
          get_local $82
          i32.add
          set_local $83
          get_local $83
          get_local $78
          get_local $77
          get_local $81
          call $__ashlti3
          i32.const 80
          set_local $84
          get_local $5
          get_local $84
          i32.add
          set_local $85
          get_local $85
          get_local $75
          i32.add
          set_local $86
          get_local $86
          i64.load align=1
          set_local $87
          get_local $5
          i64.load offset=80 align=1
          set_local $88
          get_local $5
          i32.load offset=104
          set_local $89
          get_local $89
          get_local $75
          i32.add
          set_local $90
          get_local $90
          i64.load
          set_local $91
          get_local $89
          i64.load
          set_local $92
          get_local $87
          get_local $91
          i64.or
          set_local $93
          get_local $88
          get_local $92
          i64.or
          set_local $94
          get_local $94
          get_local $93
          i64.or
          set_local $95
          i64.const 0
          set_local $96
          get_local $95
          get_local $96
          i64.ne
          set_local $97
          get_local $5
          get_local $97
          i32.store8 offset=98
          get_local $5
          i32.load offset=108
          set_local $98
          get_local $98
          get_local $75
          i32.add
          set_local $99
          get_local $99
          i64.load
          set_local $100
          get_local $98
          i64.load
          set_local $101
          get_local $5
          i32.load offset=100
          set_local $102
          i32.const -128
          set_local $103
          get_local $102
          get_local $103
          i32.add
          set_local $104
          i32.const 64
          set_local $105
          get_local $5
          get_local $105
          i32.add
          set_local $106
          get_local $106
          get_local $101
          get_local $100
          get_local $104
          call $__lshrti3
          i32.const 64
          set_local $107
          get_local $5
          get_local $107
          i32.add
          set_local $108
          get_local $108
          get_local $75
          i32.add
          set_local $109
          get_local $109
          i64.load align=1
          set_local $110
          get_local $5
          i64.load offset=64 align=1
          set_local $111
          get_local $5
          i64.load8_u offset=98
          set_local $112
          i64.const 1
          set_local $113
          get_local $112
          get_local $113
          i64.and
          set_local $114
          get_local $111
          get_local $114
          i64.or
          set_local $115
          get_local $5
          i32.load offset=104
          set_local $116
          get_local $116
          get_local $110
          i64.store offset=8
          get_local $116
          get_local $115
          i64.store
          get_local $5
          i32.load offset=108
          set_local $117
          get_local $117
          get_local $96
          i64.store offset=8
          get_local $117
          get_local $96
          i64.store
          br $block_1
        end ;; $block_2
        get_local $5
        i32.load offset=108
        set_local $118
        i32.const 8
        set_local $119
        get_local $118
        get_local $119
        i32.add
        set_local $120
        get_local $120
        i64.load
        set_local $121
        get_local $118
        i64.load
        set_local $122
        get_local $5
        i32.load offset=104
        set_local $123
        get_local $123
        get_local $119
        i32.add
        set_local $124
        get_local $124
        i64.load
        set_local $125
        get_local $123
        i64.load
        set_local $126
        get_local $121
        get_local $125
        i64.or
        set_local $127
        get_local $122
        get_local $126
        i64.or
        set_local $128
        get_local $128
        get_local $127
        i64.or
        set_local $129
        i64.const 0
        set_local $130
        get_local $129
        get_local $130
        i64.ne
        set_local $131
        get_local $5
        get_local $131
        i32.store8 offset=97
        get_local $5
        i64.load8_u offset=97
        set_local $132
        i64.const 1
        set_local $133
        get_local $132
        get_local $133
        i64.and
        set_local $134
        get_local $5
        i32.load offset=104
        set_local $135
        get_local $135
        get_local $130
        i64.store offset=8
        get_local $135
        get_local $134
        i64.store
        get_local $5
        i32.load offset=108
        set_local $136
        get_local $136
        get_local $130
        i64.store offset=8
        get_local $136
        get_local $130
        i64.store
      end ;; $block_1
    end ;; $block
    i32.const 112
    set_local $137
    get_local $5
    get_local $137
    i32.add
    set_local $138
    get_local $138
    set_global $25
    return
    )
  
  (func $rep_clz_0 (type $11)
    (param $0 i64)
    (param $1 i64)
    (result i32)
    (local $2 i32)
    (local $3 i32)
    (local $4 i32)
    (local $5 i64)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i32)
    (local $12 i32)
    (local $13 i64)
    (local $14 i64)
    (local $15 i64)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i32)
    (local $20 i32)
    (local $21 i64)
    (local $22 i64)
    (local $23 i64)
    (local $24 i32)
    get_global $25
    set_local $2
    i32.const 48
    set_local $3
    get_local $2
    get_local $3
    i32.sub
    set_local $4
    i64.const 0
    set_local $5
    get_local $4
    get_local $0
    i64.store offset=32
    get_local $4
    get_local $1
    i64.store offset=40
    get_local $4
    i64.load offset=32
    set_local $6
    get_local $4
    i64.load offset=40
    set_local $7
    get_local $4
    get_local $7
    i64.store offset=24
    get_local $4
    get_local $6
    i64.store offset=16
    get_local $4
    i64.load offset=24
    set_local $8
    get_local $8
    set_local $9
    get_local $5
    set_local $10
    get_local $9
    get_local $10
    i64.ne
    set_local $11
    get_local $11
    set_local $12
    block $block
      block $block_0
        get_local $12
        i32.eqz
        br_if $block_0
        i64.const 0
        set_local $13
        get_local $4
        i64.load offset=24
        set_local $14
        get_local $4
        get_local $14
        i64.store offset=8
        get_local $4
        get_local $13
        i64.store
        br $block
      end ;; $block_0
      i64.const 64
      set_local $15
      get_local $4
      i64.load offset=16
      set_local $16
      get_local $4
      get_local $16
      i64.store offset=8
      get_local $4
      get_local $15
      i64.store
    end ;; $block
    get_local $4
    i64.load offset=8
    set_local $17
    get_local $17
    i64.clz
    set_local $18
    get_local $18
    i32.wrap/i64
    set_local $19
    get_local $19
    set_local $20
    get_local $20
    i64.extend_s/i32
    set_local $21
    get_local $4
    i64.load
    set_local $22
    get_local $21
    get_local $22
    i64.add
    set_local $23
    get_local $23
    i32.wrap/i64
    set_local $24
    get_local $24
    return
    )
  
  (func $__subtf3 (type $21)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (param $3 i64)
    (param $4 i64)
    (local $5 i32)
    (local $6 i32)
    (local $7 i32)
    (local $8 i64)
    (local $9 i64)
    (local $10 i64)
    (local $11 i64)
    (local $12 i32)
    (local $13 i32)
    (local $14 i32)
    (local $15 i32)
    (local $16 i64)
    (local $17 i64)
    (local $18 i64)
    (local $19 i64)
    (local $20 i32)
    (local $21 i32)
    (local $22 i64)
    (local $23 i64)
    (local $24 i64)
    (local $25 i64)
    (local $26 i32)
    (local $27 i32)
    get_global $25
    set_local $5
    i32.const 80
    set_local $6
    get_local $5
    get_local $6
    i32.sub
    set_local $7
    get_local $7
    set_global $25
    get_local $7
    get_local $1
    i64.store offset=64
    get_local $7
    get_local $2
    i64.store offset=72
    get_local $7
    get_local $4
    i64.store offset=56
    get_local $7
    get_local $3
    i64.store offset=48
    get_local $7
    i64.load offset=56
    set_local $8
    get_local $7
    i64.load offset=48
    set_local $9
    get_local $7
    i64.load offset=72
    set_local $10
    get_local $7
    i64.load offset=64
    set_local $11
    i32.const 32
    set_local $12
    get_local $7
    get_local $12
    i32.add
    set_local $13
    get_local $13
    get_local $9
    get_local $8
    call $toRep_4
    i32.const 40
    set_local $14
    get_local $7
    get_local $14
    i32.add
    set_local $15
    get_local $15
    i64.load align=1
    set_local $16
    get_local $7
    i64.load offset=32 align=1
    set_local $17
    i64.const -9223372036854775808
    set_local $18
    get_local $16
    get_local $18
    i64.xor
    set_local $19
    i32.const 16
    set_local $20
    get_local $7
    get_local $20
    i32.add
    set_local $21
    get_local $21
    get_local $17
    get_local $19
    call $fromRep_3
    get_local $7
    i64.load offset=24 align=1
    set_local $22
    get_local $7
    i64.load offset=16 align=1
    set_local $23
    get_local $7
    get_local $11
    get_local $10
    get_local $23
    get_local $22
    call $__addtf3
    get_local $7
    i64.load align=1
    set_local $24
    get_local $7
    i64.load offset=8 align=1
    set_local $25
    get_local $0
    get_local $25
    i64.store offset=8
    get_local $0
    get_local $24
    i64.store
    i32.const 80
    set_local $26
    get_local $7
    get_local $26
    i32.add
    set_local $27
    get_local $27
    set_global $25
    return
    )
  
  (func $toRep_4 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    )
  
  (func $fromRep_3 (type $17)
    (param $0 i32)
    (param $1 i64)
    (param $2 i64)
    (local $3 i32)
    (local $4 i32)
    (local $5 i32)
    (local $6 i64)
    (local $7 i64)
    (local $8 i64)
    (local $9 i64)
    get_global $25
    set_local $3
    i32.const 32
    set_local $4
    get_local $3
    get_local $4
    i32.sub
    set_local $5
    get_local $5
    get_local $1
    i64.store offset=16
    get_local $5
    get_local $2
    i64.store offset=24
    get_local $5
    i64.load offset=16
    set_local $6
    get_local $5
    i64.load offset=24
    set_local $7
    get_local $5
    get_local $7
    i64.store offset=8
    get_local $5
    get_local $6
    i64.store
    get_local $5
    i64.load
    set_local $8
    get_local $5
    i64.load offset=8
    set_local $9
    get_local $0
    get_local $9
    i64.store offset=8
    get_local $0
    get_local $8
    i64.store
    return
    ))