(module
 (type $FUNCSIG$iiii (func (param i32 i32 i32) (result i32)))
 (type $FUNCSIG$ii (func (param i32) (result i32)))
 (type $FUNCSIG$vi (func (param i32)))
 (type $FUNCSIG$i (func (result i32)))
 (type $FUNCSIG$iii (func (param i32 i32) (result i32)))
 (type $FUNCSIG$v (func))
 (import "env" "memory" (memory $0 256 256))
 (import "env" "table" (table 6 6 anyfunc))
 (import "env" "tableBase" (global $tableBase i32))
 (import "env" "DYNAMICTOP_PTR" (global $DYNAMICTOP_PTR$asm2wasm$import i32))
 (import "env" "STACKTOP" (global $STACKTOP$asm2wasm$import i32))
 (import "env" "STACK_MAX" (global $STACK_MAX$asm2wasm$import i32))
 (import "env" "abort" (func $abort (param i32)))
 (import "env" "enlargeMemory" (func $enlargeMemory (result i32)))
 (import "env" "getTotalMemory" (func $getTotalMemory (result i32)))
 (import "env" "abortOnCannotGrowMemory" (func $abortOnCannotGrowMemory (result i32)))
 (import "env" "___setErrNo" (func $___setErrNo (param i32)))
 (import "env" "___syscall140" (func $___syscall140 (param i32 i32) (result i32)))
 (import "env" "___syscall146" (func $___syscall146 (param i32 i32) (result i32)))
 (import "env" "___syscall6" (func $___syscall6 (param i32 i32) (result i32)))
 (import "env" "_abort" (func $_abort))
 (import "env" "_emscripten_memcpy_big" (func $_emscripten_memcpy_big (param i32 i32 i32) (result i32)))
 (global $DYNAMICTOP_PTR (mut i32) (get_global $DYNAMICTOP_PTR$asm2wasm$import))
 (global $STACKTOP (mut i32) (get_global $STACKTOP$asm2wasm$import))
 (global $STACK_MAX (mut i32) (get_global $STACK_MAX$asm2wasm$import))
 (global $__THREW__ (mut i32) (i32.const 0))
 (global $threwValue (mut i32) (i32.const 0))
 (global $tempRet0 (mut i32) (i32.const 0))
 (elem (get_global $tableBase) $b0 $___stdio_close $b1 $___stdio_write $___stdio_seek $_sn_write)
 (data (i32.const 1024) "T!\"\19\0d\01\02\03\11K\1c\0c\10\04\0b\1d\12\1e\'hnopqb \05\06\0f\13\14\15\1a\08\16\07($\17\18\t\n\0e\1b\1f%#\83\82}&*+<=>?CGJMXYZ[\\]^_`acdefgijklrstyz{|")
 (data (i32.const 1120) "Illegal byte sequence\00Domain error\00Result not representable\00Not a tty\00Permission denied\00Operation not permitted\00No such file or directory\00No such process\00File exists\00Value too large for data type\00No space left on device\00Out of memory\00Resource busy\00Interrupted system call\00Resource temporarily unavailable\00Invalid seek\00Cross-device link\00Read-only file system\00Directory not empty\00Connection reset by peer\00Operation timed out\00Connection refused\00Host is down\00Host is unreachable\00Address in use\00Broken pipe\00I/O error\00No such device or address\00Block device required\00No such device\00Not a directory\00Is a directory\00Text file busy\00Exec format error\00Invalid argument\00Argument list too long\00Symbolic link loop\00Filename too long\00Too many open files in system\00No file descriptors available\00Bad file descriptor\00No child process\00Bad address\00File too large\00Too many links\00No locks available\00Resource deadlock would occur\00State not recoverable\00Previous owner died\00Operation canceled\00Function not implemented\00No message of desired type\00Identifier removed\00Device not a stream\00No data available\00Device timeout\00Out of streams resources\00Link has been severed\00Protocol error\00Bad message\00File descriptor in bad state\00Not a socket\00Destination address required\00Message too large\00Protocol wrong type for socket\00Protocol not available\00Protocol not supported\00Socket type not supported\00Not supported\00Protocol family not supported\00Address family not supported by protocol\00Address not available\00Network is down\00Network unreachable\00Connection reset by network\00Connection aborted\00No buffer space available\00Socket is connected\00Socket not connected\00Cannot send after socket shutdown\00Operation already in progress\00Operation in progress\00Stale file handle\00Remote I/O error\00Quota exceeded\00No medium found\00Wrong medium type\00No error information\00\00\00\00\00\00\11\00\n\00\11\11\11\00\00\00\00\05\00\00\00\00\00\00\t\00\00\00\00\0b")
 (data (i32.const 2960) "\11\00\0f\n\11\11\11\03\n\07\00\01\13\t\0b\0b\00\00\t\06\0b\00\00\0b\00\06\11\00\00\00\11\11\11")
 (data (i32.const 3009) "\0b")
 (data (i32.const 3018) "\11\00\n\n\11\11\11\00\n\00\00\02\00\t\0b\00\00\00\t\00\0b\00\00\0b")
 (data (i32.const 3067) "\0c")
 (data (i32.const 3079) "\0c\00\00\00\00\0c\00\00\00\00\t\0c\00\00\00\00\00\0c\00\00\0c")
 (data (i32.const 3125) "\0e")
 (data (i32.const 3137) "\0d\00\00\00\04\0d\00\00\00\00\t\0e\00\00\00\00\00\0e\00\00\0e")
 (data (i32.const 3183) "\10")
 (data (i32.const 3195) "\0f\00\00\00\00\0f\00\00\00\00\t\10\00\00\00\00\00\10\00\00\10\00\00\12\00\00\00\12\12\12")
 (data (i32.const 3250) "\12\00\00\00\12\12\12\00\00\00\00\00\00\t")
 (data (i32.const 3299) "\0b")
 (data (i32.const 3311) "\n\00\00\00\00\n\00\00\00\00\t\0b\00\00\00\00\00\0b\00\00\0b")
 (data (i32.const 3357) "\0c")
 (data (i32.const 3369) "\0c\00\00\00\00\0c\00\00\00\00\t\0c\00\00\00\00\00\0c\00\00\0c\00\000123456789ABCDEFT\0d\00\00\05")
 (data (i32.const 3424) "\01")
 (data (i32.const 3448) "\01\00\00\00\02\00\00\00\dc\11")
 (data (i32.const 3472) "\02")
 (data (i32.const 3487) "\ff\ff\ff\ff\ff")
 (data (i32.const 3724) "\bc\11")
 (data (i32.const 3816) "\03")
 (data (i32.const 3855) "\ff\ff\ff\ff\ff")
 (data (i32.const 3904) "Reached max chains\00Function %i\00-+   0X0x\00(null)\00-0X+0X 0X-0x+0x 0x\00inf\00INF\00nan\00NAN\00.")
 (export "___errno_location" (func $___errno_location))
 (export "_exec" (func $_exec))
 (export "_free" (func $_free))
 (export "_llvm_bswap_i32" (func $_llvm_bswap_i32))
 (export "_malloc" (func $_malloc))
 (export "_memcpy" (func $_memcpy))
 (export "_memset" (func $_memset))
 (export "_run" (func $_run))
 (export "_sbrk" (func $_sbrk))
 (export "dynCall_ii" (func $dynCall_ii))
 (export "dynCall_iiii" (func $dynCall_iiii))
 (export "establishStackSpace" (func $establishStackSpace))
 (export "getTempRet0" (func $getTempRet0))
 (export "runPostSets" (func $runPostSets))
 (export "setTempRet0" (func $setTempRet0))
 (export "setThrew" (func $setThrew))
 (export "stackAlloc" (func $stackAlloc))
 (export "stackRestore" (func $stackRestore))
 (export "stackSave" (func $stackSave))
 (func $stackAlloc (; 10 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (set_local $1
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (get_local $0)
   )
  )
  (set_global $STACKTOP
   (i32.and
    (i32.add
     (get_global $STACKTOP)
     (i32.const 15)
    )
    (i32.const -16)
   )
  )
  (get_local $1)
 )
 (func $stackSave (; 11 ;) (; has Stack IR ;) (result i32)
  (get_global $STACKTOP)
 )
 (func $stackRestore (; 12 ;) (; has Stack IR ;) (param $0 i32)
  (set_global $STACKTOP
   (get_local $0)
  )
 )
 (func $establishStackSpace (; 13 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32)
  (set_global $STACKTOP
   (get_local $0)
  )
  (set_global $STACK_MAX
   (get_local $1)
  )
 )
 (func $setThrew (; 14 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32)
  (if
   (i32.eqz
    (get_global $__THREW__)
   )
   (block
    (set_global $__THREW__
     (get_local $0)
    )
    (set_global $threwValue
     (get_local $1)
    )
   )
  )
 )
 (func $setTempRet0 (; 15 ;) (; has Stack IR ;) (param $0 i32)
  (set_global $tempRet0
   (get_local $0)
  )
 )
 (func $getTempRet0 (; 16 ;) (; has Stack IR ;) (result i32)
  (get_global $tempRet0)
 )
 (func $_run (; 17 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  (set_local $4
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 32)
   )
  )
  ;;@ ../include/faasm/faasm.h:44:0
  (i32.store
   (get_local $4)
   (get_local $0)
  )
  ;;@ ../include/faasm/faasm.h:45:0
  (set_local $0
   (i32.add
    (get_local $4)
    (i32.const 4)
   )
  )
  (i32.store
   (get_local $0)
   (get_local $1)
  )
  ;;@ ../include/faasm/faasm.h:47:0
  (set_local $0
   (i32.add
    (get_local $4)
    (i32.const 8)
   )
  )
  (i32.store
   (get_local $0)
   (get_local $2)
  )
  ;;@ ../include/faasm/faasm.h:48:0
  (set_local $0
   (i32.add
    (get_local $4)
    (i32.const 12)
   )
  )
  (i32.store
   (get_local $0)
   (get_local $3)
  )
  ;;@ ../include/faasm/faasm.h:49:0
  (set_local $0
   (i32.add
    (get_local $4)
    (i32.const 16)
   )
  )
  (i32.store
   (get_local $0)
   (i32.const 0)
  )
  ;;@ ../include/faasm/faasm.h:51:0
  (drop
   (call $_exec
    (get_local $4)
   )
  )
  (set_global $STACKTOP
   (get_local $4)
  )
  (i32.const 0)
 )
 (func $_exec (; 18 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (set_local $2
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 16)
   )
  )
  (set_local $4
   (i32.add
    (get_local $2)
    (i32.const 4)
   )
  )
  (set_local $5
   (i32.add
    (tee_local $3
     (get_local $2)
    )
    (i32.const 1)
   )
  )
  (set_local $6
   (i32.add
    (get_local $3)
    (i32.const 2)
   )
  )
  (loop $while-in
   (block $while-out
    ;;@ ../func/function_chain.c:8:0
    (br_if $while-out
     (i32.eq
      (get_local $1)
      (i32.const 3)
     )
    )
    ;;@ ../func/function_chain.c:10:0
    (i32.store
     (get_local $3)
     (get_local $1)
    )
    (drop
     (call $_sprintf
      (get_local $4)
      (i32.const 3923)
      (get_local $3)
     )
    )
    ;;@ ../func/function_chain.c:12:0
    (set_local $2
     (i32.and
      (get_local $1)
      (i32.const 255)
     )
    )
    (i32.store8
     (get_local $3)
     (get_local $2)
    )
    (set_local $2
     (i32.add
      (get_local $1)
      (i32.const 1)
     )
    )
    (set_local $7
     (i32.and
      (get_local $2)
      (i32.const 255)
     )
    )
    (i32.store8
     (get_local $5)
     (get_local $7)
    )
    (set_local $1
     (i32.add
      (get_local $1)
      (i32.const 2)
     )
    )
    (set_local $1
     (i32.and
      (get_local $1)
      (i32.const 255)
     )
    )
    (i32.store8
     (get_local $6)
     (get_local $1)
    )
    ;;@ ../func/function_chain.c:15:0
    (call $_chainFunction
     (get_local $0)
     (get_local $4)
     (get_local $3)
     (i32.const 3)
    )
    (set_local $1
     (get_local $2)
    )
    (br $while-in)
   )
  )
  (set_global $STACKTOP
   (get_local $3)
  )
  ;;@ ../func/function_chain.c:18:0
  (i32.const 0)
 )
 (func $_chainFunction (; 19 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  ;;@ ../include/faasm/faasm.h:64:0
  (set_local $7
   (i32.add
    (get_local $0)
    (i32.const 16)
   )
  )
  (set_local $5
   (i32.load
    (get_local $7)
   )
  )
  (if
   ;;@ ../include/faasm/faasm.h:66:0
   (i32.gt_s
    (get_local $5)
    (i32.const 50)
   )
   (block
    ;;@ ../include/faasm/faasm.h:67:0
    (set_local $0
     (i32.load
      (i32.const 3408)
     )
    )
    (drop
     (call $_fputs
      (i32.const 3904)
      (get_local $0)
     )
    )
   )
   (block
    ;;@ ../include/faasm/faasm.h:72:0
    (set_local $4
     (i32.add
      (get_local $0)
      (i32.const 8)
     )
    )
    (set_local $4
     (i32.load
      (get_local $4)
     )
    )
    (set_local $6
     (i32.shl
      (get_local $5)
      (i32.const 5)
     )
    )
    (set_local $4
     (i32.add
      (get_local $4)
      (get_local $6)
     )
    )
    ;;@ ../include/faasm/faasm.h:73:0
    (set_local $0
     (i32.add
      (get_local $0)
      (i32.const 12)
     )
    )
    (set_local $0
     (i32.load
      (get_local $0)
     )
    )
    (set_local $6
     (i32.shl
      (get_local $5)
      (i32.const 10)
     )
    )
    (set_local $0
     (i32.add
      (get_local $0)
      (get_local $6)
     )
    )
    ;;@ ../include/faasm/faasm.h:76:0
    (drop
     (call $_strcpy
      (get_local $4)
      (get_local $1)
     )
    )
    ;;@ ../include/faasm/faasm.h:77:0
    (drop
     (call $_memcpy
      (get_local $0)
      (get_local $2)
      (get_local $3)
     )
    )
    ;;@ ../include/faasm/faasm.h:80:0
    (set_local $0
     (i32.add
      (get_local $5)
      (i32.const 1)
     )
    )
    (i32.store
     (get_local $7)
     (get_local $0)
    )
   )
  )
 )
 (func $_malloc (; 20 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
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
  (local $15 i32)
  (local $16 i32)
  (local $17 i32)
  (local $18 i32)
  (local $19 i32)
  (local $20 i32)
  (local $21 i32)
  (local $22 i32)
  (block $folding-inner1
   (block $folding-inner0
    (set_local $2
     (get_global $STACKTOP)
    )
    (set_global $STACKTOP
     (i32.add
      (get_global $STACKTOP)
      (i32.const 16)
     )
    )
    (set_local $14
     (get_local $2)
    )
    (set_local $3
     (block $do-once (result i32)
      (if (result i32)
       (i32.lt_u
        (get_local $0)
        (i32.const 245)
       )
       (block (result i32)
        (if
         (i32.and
          (tee_local $3
           (i32.shr_u
            (tee_local $7
             (i32.load
              (i32.const 4000)
             )
            )
            (tee_local $0
             (i32.shr_u
              (tee_local $2
               (select
                (i32.const 16)
                (i32.and
                 (i32.add
                  (get_local $0)
                  (i32.const 11)
                 )
                 (i32.const -8)
                )
                (i32.lt_u
                 (get_local $0)
                 (i32.const 11)
                )
               )
              )
              (i32.const 3)
             )
            )
           )
          )
          (i32.const 3)
         )
         (block
          (if
           (i32.eq
            (tee_local $3
             (i32.load
              (tee_local $6
               (i32.add
                (tee_local $0
                 (i32.load
                  (tee_local $4
                   (i32.add
                    (tee_local $2
                     (i32.add
                      (i32.shl
                       (tee_local $1
                        (i32.add
                         (i32.xor
                          (i32.and
                           (get_local $3)
                           (i32.const 1)
                          )
                          (i32.const 1)
                         )
                         (get_local $0)
                        )
                       )
                       (i32.const 3)
                      )
                      (i32.const 4040)
                     )
                    )
                    (i32.const 8)
                   )
                  )
                 )
                )
                (i32.const 8)
               )
              )
             )
            )
            (get_local $2)
           )
           (i32.store
            (i32.const 4000)
            (i32.and
             (get_local $7)
             (i32.xor
              (i32.shl
               (i32.const 1)
               (get_local $1)
              )
              (i32.const -1)
             )
            )
           )
           (block
            (if
             (i32.gt_u
              (i32.load
               (i32.const 4016)
              )
              (get_local $3)
             )
             (call $_abort)
            )
            (if
             (i32.eq
              (i32.load
               (tee_local $5
                (i32.add
                 (get_local $3)
                 (i32.const 12)
                )
               )
              )
              (get_local $0)
             )
             (block
              (i32.store
               (get_local $5)
               (get_local $2)
              )
              (i32.store
               (get_local $4)
               (get_local $3)
              )
             )
             (call $_abort)
            )
           )
          )
          (i32.store offset=4
           (get_local $0)
           (i32.or
            (tee_local $3
             (i32.shl
              (get_local $1)
              (i32.const 3)
             )
            )
            (i32.const 3)
           )
          )
          (i32.store
           (tee_local $0
            (i32.add
             (i32.add
              (get_local $0)
              (get_local $3)
             )
             (i32.const 4)
            )
           )
           (i32.or
            (i32.load
             (get_local $0)
            )
            (i32.const 1)
           )
          )
          (set_global $STACKTOP
           (get_local $14)
          )
          (return
           (get_local $6)
          )
         )
        )
        (if (result i32)
         (i32.gt_u
          (get_local $2)
          (tee_local $13
           (i32.load
            (i32.const 4008)
           )
          )
         )
         (block (result i32)
          (if
           (get_local $3)
           (block
            (set_local $0
             (i32.and
              (i32.shr_u
               (tee_local $3
                (i32.add
                 (i32.and
                  (tee_local $0
                   (i32.and
                    (i32.shl
                     (get_local $3)
                     (get_local $0)
                    )
                    (i32.or
                     (tee_local $0
                      (i32.shl
                       (i32.const 2)
                       (get_local $0)
                      )
                     )
                     (i32.sub
                      (i32.const 0)
                      (get_local $0)
                     )
                    )
                   )
                  )
                  (i32.sub
                   (i32.const 0)
                   (get_local $0)
                  )
                 )
                 (i32.const -1)
                )
               )
               (i32.const 12)
              )
              (i32.const 16)
             )
            )
            (if
             (i32.eq
              (tee_local $3
               (i32.load
                (tee_local $10
                 (i32.add
                  (tee_local $0
                   (i32.load
                    (tee_local $9
                     (i32.add
                      (tee_local $5
                       (i32.add
                        (i32.shl
                         (tee_local $1
                          (i32.add
                           (i32.or
                            (i32.or
                             (i32.or
                              (i32.or
                               (tee_local $1
                                (i32.and
                                 (i32.shr_u
                                  (tee_local $3
                                   (i32.shr_u
                                    (get_local $3)
                                    (get_local $0)
                                   )
                                  )
                                  (i32.const 5)
                                 )
                                 (i32.const 8)
                                )
                               )
                               (get_local $0)
                              )
                              (tee_local $3
                               (i32.and
                                (i32.shr_u
                                 (tee_local $0
                                  (i32.shr_u
                                   (get_local $3)
                                   (get_local $1)
                                  )
                                 )
                                 (i32.const 2)
                                )
                                (i32.const 4)
                               )
                              )
                             )
                             (tee_local $3
                              (i32.and
                               (i32.shr_u
                                (tee_local $0
                                 (i32.shr_u
                                  (get_local $0)
                                  (get_local $3)
                                 )
                                )
                                (i32.const 1)
                               )
                               (i32.const 2)
                              )
                             )
                            )
                            (tee_local $3
                             (i32.and
                              (i32.shr_u
                               (tee_local $0
                                (i32.shr_u
                                 (get_local $0)
                                 (get_local $3)
                                )
                               )
                               (i32.const 1)
                              )
                              (i32.const 1)
                             )
                            )
                           )
                           (i32.shr_u
                            (get_local $0)
                            (get_local $3)
                           )
                          )
                         )
                         (i32.const 3)
                        )
                        (i32.const 4040)
                       )
                      )
                      (i32.const 8)
                     )
                    )
                   )
                  )
                  (i32.const 8)
                 )
                )
               )
              )
              (get_local $5)
             )
             (i32.store
              (i32.const 4000)
              (tee_local $4
               (i32.and
                (get_local $7)
                (i32.xor
                 (i32.shl
                  (i32.const 1)
                  (get_local $1)
                 )
                 (i32.const -1)
                )
               )
              )
             )
             (block
              (if
               (i32.gt_u
                (i32.load
                 (i32.const 4016)
                )
                (get_local $3)
               )
               (call $_abort)
              )
              (if
               (i32.eq
                (i32.load
                 (tee_local $11
                  (i32.add
                   (get_local $3)
                   (i32.const 12)
                  )
                 )
                )
                (get_local $0)
               )
               (block
                (i32.store
                 (get_local $11)
                 (get_local $5)
                )
                (i32.store
                 (get_local $9)
                 (get_local $3)
                )
                (set_local $4
                 (get_local $7)
                )
               )
               (call $_abort)
              )
             )
            )
            (i32.store offset=4
             (get_local $0)
             (i32.or
              (get_local $2)
              (i32.const 3)
             )
            )
            (i32.store offset=4
             (tee_local $7
              (i32.add
               (get_local $0)
               (get_local $2)
              )
             )
             (i32.or
              (tee_local $5
               (i32.sub
                (tee_local $3
                 (i32.shl
                  (get_local $1)
                  (i32.const 3)
                 )
                )
                (get_local $2)
               )
              )
              (i32.const 1)
             )
            )
            (i32.store
             (i32.add
              (get_local $0)
              (get_local $3)
             )
             (get_local $5)
            )
            (if
             (get_local $13)
             (block
              (set_local $2
               (i32.load
                (i32.const 4020)
               )
              )
              (set_local $0
               (i32.add
                (i32.shl
                 (tee_local $3
                  (i32.shr_u
                   (get_local $13)
                   (i32.const 3)
                  )
                 )
                 (i32.const 3)
                )
                (i32.const 4040)
               )
              )
              (if
               (i32.and
                (get_local $4)
                (tee_local $3
                 (i32.shl
                  (i32.const 1)
                  (get_local $3)
                 )
                )
               )
               (if
                (i32.gt_u
                 (i32.load
                  (i32.const 4016)
                 )
                 (tee_local $1
                  (i32.load
                   (tee_local $3
                    (i32.add
                     (get_local $0)
                     (i32.const 8)
                    )
                   )
                  )
                 )
                )
                (call $_abort)
                (block
                 (set_local $6
                  (get_local $1)
                 )
                 (set_local $12
                  (get_local $3)
                 )
                )
               )
               (block
                (i32.store
                 (i32.const 4000)
                 (i32.or
                  (get_local $4)
                  (get_local $3)
                 )
                )
                (set_local $6
                 (get_local $0)
                )
                (set_local $12
                 (i32.add
                  (get_local $0)
                  (i32.const 8)
                 )
                )
               )
              )
              (i32.store
               (get_local $12)
               (get_local $2)
              )
              (i32.store offset=12
               (get_local $6)
               (get_local $2)
              )
              (i32.store offset=8
               (get_local $2)
               (get_local $6)
              )
              (i32.store offset=12
               (get_local $2)
               (get_local $0)
              )
             )
            )
            (i32.store
             (i32.const 4008)
             (get_local $5)
            )
            (i32.store
             (i32.const 4020)
             (get_local $7)
            )
            (set_global $STACKTOP
             (get_local $14)
            )
            (return
             (get_local $10)
            )
           )
          )
          (if (result i32)
           (tee_local $12
            (i32.load
             (i32.const 4004)
            )
           )
           (block
            (set_local $0
             (i32.and
              (i32.shr_u
               (tee_local $3
                (i32.add
                 (i32.and
                  (get_local $12)
                  (i32.sub
                   (i32.const 0)
                   (get_local $12)
                  )
                 )
                 (i32.const -1)
                )
               )
               (i32.const 12)
              )
              (i32.const 16)
             )
            )
            (set_local $0
             (tee_local $4
              (i32.load
               (i32.add
                (i32.shl
                 (i32.add
                  (i32.or
                   (i32.or
                    (i32.or
                     (i32.or
                      (tee_local $4
                       (i32.and
                        (i32.shr_u
                         (tee_local $3
                          (i32.shr_u
                           (get_local $3)
                           (get_local $0)
                          )
                         )
                         (i32.const 5)
                        )
                        (i32.const 8)
                       )
                      )
                      (get_local $0)
                     )
                     (tee_local $3
                      (i32.and
                       (i32.shr_u
                        (tee_local $0
                         (i32.shr_u
                          (get_local $3)
                          (get_local $4)
                         )
                        )
                        (i32.const 2)
                       )
                       (i32.const 4)
                      )
                     )
                    )
                    (tee_local $3
                     (i32.and
                      (i32.shr_u
                       (tee_local $0
                        (i32.shr_u
                         (get_local $0)
                         (get_local $3)
                        )
                       )
                       (i32.const 1)
                      )
                      (i32.const 2)
                     )
                    )
                   )
                   (tee_local $3
                    (i32.and
                     (i32.shr_u
                      (tee_local $0
                       (i32.shr_u
                        (get_local $0)
                        (get_local $3)
                       )
                      )
                      (i32.const 1)
                     )
                     (i32.const 1)
                    )
                   )
                  )
                  (i32.shr_u
                   (get_local $0)
                   (get_local $3)
                  )
                 )
                 (i32.const 2)
                )
                (i32.const 4304)
               )
              )
             )
            )
            (set_local $10
             (i32.sub
              (i32.and
               (i32.load offset=4
                (get_local $4)
               )
               (i32.const -8)
              )
              (get_local $2)
             )
            )
            (loop $while-in
             (block $while-out
              (if
               (tee_local $3
                (i32.load offset=16
                 (get_local $0)
                )
               )
               (set_local $0
                (get_local $3)
               )
               (br_if $while-out
                (i32.eqz
                 (tee_local $0
                  (i32.load offset=20
                   (get_local $0)
                  )
                 )
                )
               )
              )
              (set_local $4
               (select
                (get_local $0)
                (get_local $4)
                (tee_local $6
                 (i32.lt_u
                  (tee_local $3
                   (i32.sub
                    (i32.and
                     (i32.load offset=4
                      (get_local $0)
                     )
                     (i32.const -8)
                    )
                    (get_local $2)
                   )
                  )
                  (get_local $10)
                 )
                )
               )
              )
              (set_local $10
               (select
                (get_local $3)
                (get_local $10)
                (get_local $6)
               )
              )
              (br $while-in)
             )
            )
            (if
             (i32.gt_u
              (tee_local $15
               (i32.load
                (i32.const 4016)
               )
              )
              (get_local $4)
             )
             (call $_abort)
            )
            (if
             (i32.le_u
              (tee_local $8
               (i32.add
                (get_local $4)
                (get_local $2)
               )
              )
              (get_local $4)
             )
             (call $_abort)
            )
            (set_local $11
             (i32.load offset=24
              (get_local $4)
             )
            )
            (block $do-once4
             (if
              (i32.eq
               (tee_local $0
                (i32.load offset=12
                 (get_local $4)
                )
               )
               (get_local $4)
              )
              (block
               (if
                (i32.eqz
                 (tee_local $0
                  (i32.load
                   (tee_local $3
                    (i32.add
                     (get_local $4)
                     (i32.const 20)
                    )
                   )
                  )
                 )
                )
                (br_if $do-once4
                 (i32.eqz
                  (tee_local $0
                   (i32.load
                    (tee_local $3
                     (i32.add
                      (get_local $4)
                      (i32.const 16)
                     )
                    )
                   )
                  )
                 )
                )
               )
               (loop $while-in7
                (block $while-out6
                 (if
                  (i32.eqz
                   (tee_local $9
                    (i32.load
                     (tee_local $6
                      (i32.add
                       (get_local $0)
                       (i32.const 20)
                      )
                     )
                    )
                   )
                  )
                  (br_if $while-out6
                   (i32.eqz
                    (tee_local $9
                     (i32.load
                      (tee_local $6
                       (i32.add
                        (get_local $0)
                        (i32.const 16)
                       )
                      )
                     )
                    )
                   )
                  )
                 )
                 (set_local $3
                  (get_local $6)
                 )
                 (set_local $0
                  (get_local $9)
                 )
                 (br $while-in7)
                )
               )
               (if
                (i32.gt_u
                 (get_local $15)
                 (get_local $3)
                )
                (call $_abort)
                (block
                 (i32.store
                  (get_local $3)
                  (i32.const 0)
                 )
                 (set_local $1
                  (get_local $0)
                 )
                )
               )
              )
              (block
               (if
                (i32.gt_u
                 (get_local $15)
                 (tee_local $3
                  (i32.load offset=8
                   (get_local $4)
                  )
                 )
                )
                (call $_abort)
               )
               (if
                (i32.ne
                 (i32.load
                  (tee_local $6
                   (i32.add
                    (get_local $3)
                    (i32.const 12)
                   )
                  )
                 )
                 (get_local $4)
                )
                (call $_abort)
               )
               (if
                (i32.eq
                 (i32.load
                  (tee_local $9
                   (i32.add
                    (get_local $0)
                    (i32.const 8)
                   )
                  )
                 )
                 (get_local $4)
                )
                (block
                 (i32.store
                  (get_local $6)
                  (get_local $0)
                 )
                 (i32.store
                  (get_local $9)
                  (get_local $3)
                 )
                 (set_local $1
                  (get_local $0)
                 )
                )
                (call $_abort)
               )
              )
             )
            )
            (block $label$break$L78
             (if
              (get_local $11)
              (block
               (if
                (i32.eq
                 (get_local $4)
                 (i32.load
                  (tee_local $3
                   (i32.add
                    (i32.shl
                     (tee_local $0
                      (i32.load offset=28
                       (get_local $4)
                      )
                     )
                     (i32.const 2)
                    )
                    (i32.const 4304)
                   )
                  )
                 )
                )
                (block
                 (i32.store
                  (get_local $3)
                  (get_local $1)
                 )
                 (if
                  (i32.eqz
                   (get_local $1)
                  )
                  (block
                   (i32.store
                    (i32.const 4004)
                    (i32.and
                     (get_local $12)
                     (i32.xor
                      (i32.shl
                       (i32.const 1)
                       (get_local $0)
                      )
                      (i32.const -1)
                     )
                    )
                   )
                   (br $label$break$L78)
                  )
                 )
                )
                (if
                 (i32.gt_u
                  (i32.load
                   (i32.const 4016)
                  )
                  (get_local $11)
                 )
                 (call $_abort)
                 (block
                  (i32.store
                   (select
                    (tee_local $0
                     (i32.add
                      (get_local $11)
                      (i32.const 16)
                     )
                    )
                    (i32.add
                     (get_local $11)
                     (i32.const 20)
                    )
                    (i32.eq
                     (i32.load
                      (get_local $0)
                     )
                     (get_local $4)
                    )
                   )
                   (get_local $1)
                  )
                  (br_if $label$break$L78
                   (i32.eqz
                    (get_local $1)
                   )
                  )
                 )
                )
               )
               (if
                (i32.gt_u
                 (tee_local $3
                  (i32.load
                   (i32.const 4016)
                  )
                 )
                 (get_local $1)
                )
                (call $_abort)
               )
               (i32.store offset=24
                (get_local $1)
                (get_local $11)
               )
               (if
                (tee_local $0
                 (i32.load offset=16
                  (get_local $4)
                 )
                )
                (if
                 (i32.gt_u
                  (get_local $3)
                  (get_local $0)
                 )
                 (call $_abort)
                 (block
                  (i32.store offset=16
                   (get_local $1)
                   (get_local $0)
                  )
                  (i32.store offset=24
                   (get_local $0)
                   (get_local $1)
                  )
                 )
                )
               )
               (if
                (tee_local $0
                 (i32.load offset=20
                  (get_local $4)
                 )
                )
                (if
                 (i32.gt_u
                  (i32.load
                   (i32.const 4016)
                  )
                  (get_local $0)
                 )
                 (call $_abort)
                 (block
                  (i32.store offset=20
                   (get_local $1)
                   (get_local $0)
                  )
                  (i32.store offset=24
                   (get_local $0)
                   (get_local $1)
                  )
                 )
                )
               )
              )
             )
            )
            (if
             (i32.lt_u
              (get_local $10)
              (i32.const 16)
             )
             (block
              (i32.store offset=4
               (get_local $4)
               (i32.or
                (tee_local $0
                 (i32.add
                  (get_local $10)
                  (get_local $2)
                 )
                )
                (i32.const 3)
               )
              )
              (i32.store
               (tee_local $0
                (i32.add
                 (i32.add
                  (get_local $4)
                  (get_local $0)
                 )
                 (i32.const 4)
                )
               )
               (i32.or
                (i32.load
                 (get_local $0)
                )
                (i32.const 1)
               )
              )
             )
             (block
              (i32.store offset=4
               (get_local $4)
               (i32.or
                (get_local $2)
                (i32.const 3)
               )
              )
              (i32.store offset=4
               (get_local $8)
               (i32.or
                (get_local $10)
                (i32.const 1)
               )
              )
              (i32.store
               (i32.add
                (get_local $8)
                (get_local $10)
               )
               (get_local $10)
              )
              (if
               (get_local $13)
               (block
                (set_local $2
                 (i32.load
                  (i32.const 4020)
                 )
                )
                (set_local $0
                 (i32.add
                  (i32.shl
                   (tee_local $3
                    (i32.shr_u
                     (get_local $13)
                     (i32.const 3)
                    )
                   )
                   (i32.const 3)
                  )
                  (i32.const 4040)
                 )
                )
                (if
                 (i32.and
                  (tee_local $3
                   (i32.shl
                    (i32.const 1)
                    (get_local $3)
                   )
                  )
                  (get_local $7)
                 )
                 (if
                  (i32.gt_u
                   (i32.load
                    (i32.const 4016)
                   )
                   (tee_local $1
                    (i32.load
                     (tee_local $3
                      (i32.add
                       (get_local $0)
                       (i32.const 8)
                      )
                     )
                    )
                   )
                  )
                  (call $_abort)
                  (block
                   (set_local $5
                    (get_local $1)
                   )
                   (set_local $16
                    (get_local $3)
                   )
                  )
                 )
                 (block
                  (i32.store
                   (i32.const 4000)
                   (i32.or
                    (get_local $3)
                    (get_local $7)
                   )
                  )
                  (set_local $5
                   (get_local $0)
                  )
                  (set_local $16
                   (i32.add
                    (get_local $0)
                    (i32.const 8)
                   )
                  )
                 )
                )
                (i32.store
                 (get_local $16)
                 (get_local $2)
                )
                (i32.store offset=12
                 (get_local $5)
                 (get_local $2)
                )
                (i32.store offset=8
                 (get_local $2)
                 (get_local $5)
                )
                (i32.store offset=12
                 (get_local $2)
                 (get_local $0)
                )
               )
              )
              (i32.store
               (i32.const 4008)
               (get_local $10)
              )
              (i32.store
               (i32.const 4020)
               (get_local $8)
              )
             )
            )
            (set_global $STACKTOP
             (get_local $14)
            )
            (return
             (i32.add
              (get_local $4)
              (i32.const 8)
             )
            )
           )
           (get_local $2)
          )
         )
         (get_local $2)
        )
       )
       (if (result i32)
        (i32.gt_u
         (get_local $0)
         (i32.const -65)
        )
        (i32.const -1)
        (block (result i32)
         (set_local $4
          (i32.and
           (tee_local $0
            (i32.add
             (get_local $0)
             (i32.const 11)
            )
           )
           (i32.const -8)
          )
         )
         (if (result i32)
          (tee_local $6
           (i32.load
            (i32.const 4004)
           )
          )
          (block (result i32)
           (set_local $18
            (if (result i32)
             (tee_local $0
              (i32.shr_u
               (get_local $0)
               (i32.const 8)
              )
             )
             (if (result i32)
              (i32.gt_u
               (get_local $4)
               (i32.const 16777215)
              )
              (i32.const 31)
              (i32.or
               (i32.and
                (i32.shr_u
                 (get_local $4)
                 (i32.add
                  (tee_local $0
                   (i32.add
                    (i32.sub
                     (i32.const 14)
                     (i32.or
                      (i32.or
                       (tee_local $2
                        (i32.and
                         (i32.shr_u
                          (i32.add
                           (tee_local $1
                            (i32.shl
                             (get_local $0)
                             (tee_local $0
                              (i32.and
                               (i32.shr_u
                                (i32.add
                                 (get_local $0)
                                 (i32.const 1048320)
                                )
                                (i32.const 16)
                               )
                               (i32.const 8)
                              )
                             )
                            )
                           )
                           (i32.const 520192)
                          )
                          (i32.const 16)
                         )
                         (i32.const 4)
                        )
                       )
                       (get_local $0)
                      )
                      (tee_local $1
                       (i32.and
                        (i32.shr_u
                         (i32.add
                          (tee_local $0
                           (i32.shl
                            (get_local $1)
                            (get_local $2)
                           )
                          )
                          (i32.const 245760)
                         )
                         (i32.const 16)
                        )
                        (i32.const 2)
                       )
                      )
                     )
                    )
                    (i32.shr_u
                     (i32.shl
                      (get_local $0)
                      (get_local $1)
                     )
                     (i32.const 15)
                    )
                   )
                  )
                  (i32.const 7)
                 )
                )
                (i32.const 1)
               )
               (i32.shl
                (get_local $0)
                (i32.const 1)
               )
              )
             )
             (i32.const 0)
            )
           )
           (set_local $2
            (i32.sub
             (i32.const 0)
             (get_local $4)
            )
           )
           (block $__rjto$1
            (block $__rjti$1
             (if
              (tee_local $0
               (i32.load
                (i32.add
                 (i32.shl
                  (get_local $18)
                  (i32.const 2)
                 )
                 (i32.const 4304)
                )
               )
              )
              (block
               (set_local $1
                (i32.const 0)
               )
               (set_local $12
                (i32.shl
                 (get_local $4)
                 (select
                  (i32.const 0)
                  (i32.sub
                   (i32.const 25)
                   (i32.shr_u
                    (get_local $18)
                    (i32.const 1)
                   )
                  )
                  (i32.eq
                   (get_local $18)
                   (i32.const 31)
                  )
                 )
                )
               )
               (loop $while-in15
                (if
                 (i32.lt_u
                  (tee_local $16
                   (i32.sub
                    (i32.and
                     (i32.load offset=4
                      (get_local $0)
                     )
                     (i32.const -8)
                    )
                    (get_local $4)
                   )
                  )
                  (get_local $2)
                 )
                 (set_local $1
                  (if (result i32)
                   (get_local $16)
                   (block (result i32)
                    (set_local $2
                     (get_local $16)
                    )
                    (get_local $0)
                   )
                   (block
                    (set_local $2
                     (i32.const 0)
                    )
                    (set_local $1
                     (get_local $0)
                    )
                    (br $__rjti$1)
                   )
                  )
                 )
                )
                (set_local $5
                 (select
                  (get_local $5)
                  (tee_local $5
                   (i32.load offset=20
                    (get_local $0)
                   )
                  )
                  (i32.or
                   (i32.eqz
                    (get_local $5)
                   )
                   (i32.eq
                    (get_local $5)
                    (tee_local $0
                     (i32.load
                      (i32.add
                       (i32.add
                        (get_local $0)
                        (i32.const 16)
                       )
                       (i32.shl
                        (i32.shr_u
                         (get_local $12)
                         (i32.const 31)
                        )
                        (i32.const 2)
                       )
                      )
                     )
                    )
                   )
                  )
                 )
                )
                (set_local $12
                 (i32.shl
                  (get_local $12)
                  (i32.const 1)
                 )
                )
                (br_if $while-in15
                 (get_local $0)
                )
               )
               (set_local $0
                (get_local $1)
               )
              )
              (set_local $0
               (i32.const 0)
              )
             )
             (br_if $__rjti$1
              (tee_local $1
               (if (result i32)
                (i32.or
                 (get_local $5)
                 (get_local $0)
                )
                (get_local $5)
                (block (result i32)
                 (drop
                  (br_if $do-once
                   (get_local $4)
                   (i32.eqz
                    (tee_local $0
                     (i32.and
                      (i32.or
                       (tee_local $0
                        (i32.shl
                         (i32.const 2)
                         (get_local $18)
                        )
                       )
                       (i32.sub
                        (i32.const 0)
                        (get_local $0)
                       )
                      )
                      (get_local $6)
                     )
                    )
                   )
                  )
                 )
                 (set_local $1
                  (i32.and
                   (i32.shr_u
                    (tee_local $5
                     (i32.add
                      (i32.and
                       (get_local $0)
                       (i32.sub
                        (i32.const 0)
                        (get_local $0)
                       )
                      )
                      (i32.const -1)
                     )
                    )
                    (i32.const 12)
                   )
                   (i32.const 16)
                  )
                 )
                 (set_local $0
                  (i32.const 0)
                 )
                 (i32.load
                  (i32.add
                   (i32.shl
                    (i32.add
                     (i32.or
                      (i32.or
                       (i32.or
                        (i32.or
                         (tee_local $12
                          (i32.and
                           (i32.shr_u
                            (tee_local $5
                             (i32.shr_u
                              (get_local $5)
                              (get_local $1)
                             )
                            )
                            (i32.const 5)
                           )
                           (i32.const 8)
                          )
                         )
                         (get_local $1)
                        )
                        (tee_local $5
                         (i32.and
                          (i32.shr_u
                           (tee_local $1
                            (i32.shr_u
                             (get_local $5)
                             (get_local $12)
                            )
                           )
                           (i32.const 2)
                          )
                          (i32.const 4)
                         )
                        )
                       )
                       (tee_local $5
                        (i32.and
                         (i32.shr_u
                          (tee_local $1
                           (i32.shr_u
                            (get_local $1)
                            (get_local $5)
                           )
                          )
                          (i32.const 1)
                         )
                         (i32.const 2)
                        )
                       )
                      )
                      (tee_local $5
                       (i32.and
                        (i32.shr_u
                         (tee_local $1
                          (i32.shr_u
                           (get_local $1)
                           (get_local $5)
                          )
                         )
                         (i32.const 1)
                        )
                        (i32.const 1)
                       )
                      )
                     )
                     (i32.shr_u
                      (get_local $1)
                      (get_local $5)
                     )
                    )
                    (i32.const 2)
                   )
                   (i32.const 4304)
                  )
                 )
                )
               )
              )
             )
             (set_local $5
              (get_local $0)
             )
             (br $__rjto$1)
            )
            (set_local $5
             (get_local $0)
            )
            (set_local $0
             (get_local $2)
            )
            (loop $while-in17
             (set_local $12
              (i32.load offset=4
               (get_local $1)
              )
             )
             (if
              (i32.eqz
               (tee_local $2
                (i32.load offset=16
                 (get_local $1)
                )
               )
              )
              (set_local $2
               (i32.load offset=20
                (get_local $1)
               )
              )
             )
             (set_local $12
              (i32.lt_u
               (tee_local $16
                (i32.sub
                 (i32.and
                  (get_local $12)
                  (i32.const -8)
                 )
                 (get_local $4)
                )
               )
               (get_local $0)
              )
             )
             (set_local $0
              (select
               (get_local $16)
               (get_local $0)
               (get_local $12)
              )
             )
             (set_local $5
              (select
               (get_local $1)
               (get_local $5)
               (get_local $12)
              )
             )
             (set_local $2
              (if (result i32)
               (get_local $2)
               (block
                (set_local $1
                 (get_local $2)
                )
                (br $while-in17)
               )
               (get_local $0)
              )
             )
            )
           )
           (if (result i32)
            (get_local $5)
            (if (result i32)
             (i32.lt_u
              (get_local $2)
              (i32.sub
               (i32.load
                (i32.const 4008)
               )
               (get_local $4)
              )
             )
             (block
              (if
               (i32.gt_u
                (tee_local $17
                 (i32.load
                  (i32.const 4016)
                 )
                )
                (get_local $5)
               )
               (call $_abort)
              )
              (if
               (i32.le_u
                (tee_local $8
                 (i32.add
                  (get_local $5)
                  (get_local $4)
                 )
                )
                (get_local $5)
               )
               (call $_abort)
              )
              (set_local $15
               (i32.load offset=24
                (get_local $5)
               )
              )
              (block $do-once18
               (if
                (i32.eq
                 (tee_local $0
                  (i32.load offset=12
                   (get_local $5)
                  )
                 )
                 (get_local $5)
                )
                (block
                 (if
                  (i32.eqz
                   (tee_local $0
                    (i32.load
                     (tee_local $1
                      (i32.add
                       (get_local $5)
                       (i32.const 20)
                      )
                     )
                    )
                   )
                  )
                  (br_if $do-once18
                   (i32.eqz
                    (tee_local $0
                     (i32.load
                      (tee_local $1
                       (i32.add
                        (get_local $5)
                        (i32.const 16)
                       )
                      )
                     )
                    )
                   )
                  )
                 )
                 (loop $while-in21
                  (block $while-out20
                   (if
                    (i32.eqz
                     (tee_local $11
                      (i32.load
                       (tee_local $9
                        (i32.add
                         (get_local $0)
                         (i32.const 20)
                        )
                       )
                      )
                     )
                    )
                    (br_if $while-out20
                     (i32.eqz
                      (tee_local $11
                       (i32.load
                        (tee_local $9
                         (i32.add
                          (get_local $0)
                          (i32.const 16)
                         )
                        )
                       )
                      )
                     )
                    )
                   )
                   (set_local $1
                    (get_local $9)
                   )
                   (set_local $0
                    (get_local $11)
                   )
                   (br $while-in21)
                  )
                 )
                 (if
                  (i32.gt_u
                   (get_local $17)
                   (get_local $1)
                  )
                  (call $_abort)
                  (block
                   (i32.store
                    (get_local $1)
                    (i32.const 0)
                   )
                   (set_local $7
                    (get_local $0)
                   )
                  )
                 )
                )
                (block
                 (if
                  (i32.gt_u
                   (get_local $17)
                   (tee_local $1
                    (i32.load offset=8
                     (get_local $5)
                    )
                   )
                  )
                  (call $_abort)
                 )
                 (if
                  (i32.ne
                   (i32.load
                    (tee_local $9
                     (i32.add
                      (get_local $1)
                      (i32.const 12)
                     )
                    )
                   )
                   (get_local $5)
                  )
                  (call $_abort)
                 )
                 (if
                  (i32.eq
                   (i32.load
                    (tee_local $11
                     (i32.add
                      (get_local $0)
                      (i32.const 8)
                     )
                    )
                   )
                   (get_local $5)
                  )
                  (block
                   (i32.store
                    (get_local $9)
                    (get_local $0)
                   )
                   (i32.store
                    (get_local $11)
                    (get_local $1)
                   )
                   (set_local $7
                    (get_local $0)
                   )
                  )
                  (call $_abort)
                 )
                )
               )
              )
              (block $label$break$L176
               (if
                (get_local $15)
                (block
                 (if
                  (i32.eq
                   (get_local $5)
                   (i32.load
                    (tee_local $1
                     (i32.add
                      (i32.shl
                       (tee_local $0
                        (i32.load offset=28
                         (get_local $5)
                        )
                       )
                       (i32.const 2)
                      )
                      (i32.const 4304)
                     )
                    )
                   )
                  )
                  (block
                   (i32.store
                    (get_local $1)
                    (get_local $7)
                   )
                   (if
                    (i32.eqz
                     (get_local $7)
                    )
                    (block
                     (i32.store
                      (i32.const 4004)
                      (tee_local $3
                       (i32.and
                        (get_local $6)
                        (i32.xor
                         (i32.shl
                          (i32.const 1)
                          (get_local $0)
                         )
                         (i32.const -1)
                        )
                       )
                      )
                     )
                     (br $label$break$L176)
                    )
                   )
                  )
                  (if
                   (i32.gt_u
                    (i32.load
                     (i32.const 4016)
                    )
                    (get_local $15)
                   )
                   (call $_abort)
                   (block
                    (i32.store
                     (select
                      (tee_local $0
                       (i32.add
                        (get_local $15)
                        (i32.const 16)
                       )
                      )
                      (i32.add
                       (get_local $15)
                       (i32.const 20)
                      )
                      (i32.eq
                       (i32.load
                        (get_local $0)
                       )
                       (get_local $5)
                      )
                     )
                     (get_local $7)
                    )
                    (if
                     (i32.eqz
                      (get_local $7)
                     )
                     (block
                      (set_local $3
                       (get_local $6)
                      )
                      (br $label$break$L176)
                     )
                    )
                   )
                  )
                 )
                 (if
                  (i32.gt_u
                   (tee_local $1
                    (i32.load
                     (i32.const 4016)
                    )
                   )
                   (get_local $7)
                  )
                  (call $_abort)
                 )
                 (i32.store offset=24
                  (get_local $7)
                  (get_local $15)
                 )
                 (if
                  (tee_local $0
                   (i32.load offset=16
                    (get_local $5)
                   )
                  )
                  (if
                   (i32.gt_u
                    (get_local $1)
                    (get_local $0)
                   )
                   (call $_abort)
                   (block
                    (i32.store offset=16
                     (get_local $7)
                     (get_local $0)
                    )
                    (i32.store offset=24
                     (get_local $0)
                     (get_local $7)
                    )
                   )
                  )
                 )
                 (if
                  (tee_local $0
                   (i32.load offset=20
                    (get_local $5)
                   )
                  )
                  (if
                   (i32.gt_u
                    (i32.load
                     (i32.const 4016)
                    )
                    (get_local $0)
                   )
                   (call $_abort)
                   (block
                    (i32.store offset=20
                     (get_local $7)
                     (get_local $0)
                    )
                    (i32.store offset=24
                     (get_local $0)
                     (get_local $7)
                    )
                    (set_local $3
                     (get_local $6)
                    )
                   )
                  )
                  (set_local $3
                   (get_local $6)
                  )
                 )
                )
                (set_local $3
                 (get_local $6)
                )
               )
              )
              (block $label$break$L200
               (if
                (i32.lt_u
                 (get_local $2)
                 (i32.const 16)
                )
                (block
                 (i32.store offset=4
                  (get_local $5)
                  (i32.or
                   (tee_local $0
                    (i32.add
                     (get_local $2)
                     (get_local $4)
                    )
                   )
                   (i32.const 3)
                  )
                 )
                 (i32.store
                  (tee_local $0
                   (i32.add
                    (i32.add
                     (get_local $5)
                     (get_local $0)
                    )
                    (i32.const 4)
                   )
                  )
                  (i32.or
                   (i32.load
                    (get_local $0)
                   )
                   (i32.const 1)
                  )
                 )
                )
                (block
                 (i32.store offset=4
                  (get_local $5)
                  (i32.or
                   (get_local $4)
                   (i32.const 3)
                  )
                 )
                 (i32.store offset=4
                  (get_local $8)
                  (i32.or
                   (get_local $2)
                   (i32.const 1)
                  )
                 )
                 (i32.store
                  (i32.add
                   (get_local $8)
                   (get_local $2)
                  )
                  (get_local $2)
                 )
                 (set_local $1
                  (i32.shr_u
                   (get_local $2)
                   (i32.const 3)
                  )
                 )
                 (if
                  (i32.lt_u
                   (get_local $2)
                   (i32.const 256)
                  )
                  (block
                   (set_local $0
                    (i32.add
                     (i32.shl
                      (get_local $1)
                      (i32.const 3)
                     )
                     (i32.const 4040)
                    )
                   )
                   (if
                    (i32.and
                     (tee_local $3
                      (i32.load
                       (i32.const 4000)
                      )
                     )
                     (tee_local $1
                      (i32.shl
                       (i32.const 1)
                       (get_local $1)
                      )
                     )
                    )
                    (if
                     (i32.gt_u
                      (i32.load
                       (i32.const 4016)
                      )
                      (tee_local $1
                       (i32.load
                        (tee_local $3
                         (i32.add
                          (get_local $0)
                          (i32.const 8)
                         )
                        )
                       )
                      )
                     )
                     (call $_abort)
                     (block
                      (set_local $13
                       (get_local $1)
                      )
                      (set_local $19
                       (get_local $3)
                      )
                     )
                    )
                    (block
                     (i32.store
                      (i32.const 4000)
                      (i32.or
                       (get_local $3)
                       (get_local $1)
                      )
                     )
                     (set_local $13
                      (get_local $0)
                     )
                     (set_local $19
                      (i32.add
                       (get_local $0)
                       (i32.const 8)
                      )
                     )
                    )
                   )
                   (i32.store
                    (get_local $19)
                    (get_local $8)
                   )
                   (i32.store offset=12
                    (get_local $13)
                    (get_local $8)
                   )
                   (i32.store offset=8
                    (get_local $8)
                    (get_local $13)
                   )
                   (i32.store offset=12
                    (get_local $8)
                    (get_local $0)
                   )
                   (br $label$break$L200)
                  )
                 )
                 (set_local $0
                  (i32.add
                   (i32.shl
                    (tee_local $1
                     (if (result i32)
                      (tee_local $0
                       (i32.shr_u
                        (get_local $2)
                        (i32.const 8)
                       )
                      )
                      (if (result i32)
                       (i32.gt_u
                        (get_local $2)
                        (i32.const 16777215)
                       )
                       (i32.const 31)
                       (i32.or
                        (i32.and
                         (i32.shr_u
                          (get_local $2)
                          (i32.add
                           (tee_local $0
                            (i32.add
                             (i32.sub
                              (i32.const 14)
                              (i32.or
                               (i32.or
                                (tee_local $4
                                 (i32.and
                                  (i32.shr_u
                                   (i32.add
                                    (tee_local $1
                                     (i32.shl
                                      (get_local $0)
                                      (tee_local $0
                                       (i32.and
                                        (i32.shr_u
                                         (i32.add
                                          (get_local $0)
                                          (i32.const 1048320)
                                         )
                                         (i32.const 16)
                                        )
                                        (i32.const 8)
                                       )
                                      )
                                     )
                                    )
                                    (i32.const 520192)
                                   )
                                   (i32.const 16)
                                  )
                                  (i32.const 4)
                                 )
                                )
                                (get_local $0)
                               )
                               (tee_local $1
                                (i32.and
                                 (i32.shr_u
                                  (i32.add
                                   (tee_local $0
                                    (i32.shl
                                     (get_local $1)
                                     (get_local $4)
                                    )
                                   )
                                   (i32.const 245760)
                                  )
                                  (i32.const 16)
                                 )
                                 (i32.const 2)
                                )
                               )
                              )
                             )
                             (i32.shr_u
                              (i32.shl
                               (get_local $0)
                               (get_local $1)
                              )
                              (i32.const 15)
                             )
                            )
                           )
                           (i32.const 7)
                          )
                         )
                         (i32.const 1)
                        )
                        (i32.shl
                         (get_local $0)
                         (i32.const 1)
                        )
                       )
                      )
                      (i32.const 0)
                     )
                    )
                    (i32.const 2)
                   )
                   (i32.const 4304)
                  )
                 )
                 (i32.store offset=28
                  (get_local $8)
                  (get_local $1)
                 )
                 (i32.store offset=4
                  (tee_local $4
                   (i32.add
                    (get_local $8)
                    (i32.const 16)
                   )
                  )
                  (i32.const 0)
                 )
                 (i32.store
                  (get_local $4)
                  (i32.const 0)
                 )
                 (if
                  (i32.eqz
                   (i32.and
                    (get_local $3)
                    (tee_local $4
                     (i32.shl
                      (i32.const 1)
                      (get_local $1)
                     )
                    )
                   )
                  )
                  (block
                   (i32.store
                    (i32.const 4004)
                    (i32.or
                     (get_local $3)
                     (get_local $4)
                    )
                   )
                   (i32.store
                    (get_local $0)
                    (get_local $8)
                   )
                   (i32.store offset=24
                    (get_local $8)
                    (get_local $0)
                   )
                   (i32.store offset=12
                    (get_local $8)
                    (get_local $8)
                   )
                   (i32.store offset=8
                    (get_local $8)
                    (get_local $8)
                   )
                   (br $label$break$L200)
                  )
                 )
                 (block $label$break$L218
                  (if
                   (i32.eq
                    (i32.and
                     (i32.load offset=4
                      (tee_local $0
                       (i32.load
                        (get_local $0)
                       )
                      )
                     )
                     (i32.const -8)
                    )
                    (get_local $2)
                   )
                   (set_local $10
                    (get_local $0)
                   )
                   (block
                    (set_local $1
                     (i32.shl
                      (get_local $2)
                      (select
                       (i32.const 0)
                       (i32.sub
                        (i32.const 25)
                        (i32.shr_u
                         (get_local $1)
                         (i32.const 1)
                        )
                       )
                       (i32.eq
                        (get_local $1)
                        (i32.const 31)
                       )
                      )
                     )
                    )
                    (loop $while-in30
                     (if
                      (tee_local $3
                       (i32.load
                        (tee_local $4
                         (i32.add
                          (i32.add
                           (get_local $0)
                           (i32.const 16)
                          )
                          (i32.shl
                           (i32.shr_u
                            (get_local $1)
                            (i32.const 31)
                           )
                           (i32.const 2)
                          )
                         )
                        )
                       )
                      )
                      (block
                       (set_local $1
                        (i32.shl
                         (get_local $1)
                         (i32.const 1)
                        )
                       )
                       (if
                        (i32.eq
                         (i32.and
                          (i32.load offset=4
                           (get_local $3)
                          )
                          (i32.const -8)
                         )
                         (get_local $2)
                        )
                        (block
                         (set_local $10
                          (get_local $3)
                         )
                         (br $label$break$L218)
                        )
                        (block
                         (set_local $0
                          (get_local $3)
                         )
                         (br $while-in30)
                        )
                       )
                      )
                     )
                    )
                    (if
                     (i32.gt_u
                      (i32.load
                       (i32.const 4016)
                      )
                      (get_local $4)
                     )
                     (call $_abort)
                     (block
                      (i32.store
                       (get_local $4)
                       (get_local $8)
                      )
                      (i32.store offset=24
                       (get_local $8)
                       (get_local $0)
                      )
                      (i32.store offset=12
                       (get_local $8)
                       (get_local $8)
                      )
                      (i32.store offset=8
                       (get_local $8)
                       (get_local $8)
                      )
                      (br $label$break$L200)
                     )
                    )
                   )
                  )
                 )
                 (if
                  (i32.and
                   (i32.le_u
                    (tee_local $3
                     (i32.load
                      (i32.const 4016)
                     )
                    )
                    (tee_local $0
                     (i32.load
                      (tee_local $1
                       (i32.add
                        (get_local $10)
                        (i32.const 8)
                       )
                      )
                     )
                    )
                   )
                   (i32.le_u
                    (get_local $3)
                    (get_local $10)
                   )
                  )
                  (block
                   (i32.store offset=12
                    (get_local $0)
                    (get_local $8)
                   )
                   (i32.store
                    (get_local $1)
                    (get_local $8)
                   )
                   (i32.store offset=8
                    (get_local $8)
                    (get_local $0)
                   )
                   (i32.store offset=12
                    (get_local $8)
                    (get_local $10)
                   )
                   (i32.store offset=24
                    (get_local $8)
                    (i32.const 0)
                   )
                  )
                  (call $_abort)
                 )
                )
               )
              )
              (set_global $STACKTOP
               (get_local $14)
              )
              (return
               (i32.add
                (get_local $5)
                (i32.const 8)
               )
              )
             )
             (get_local $4)
            )
            (get_local $4)
           )
          )
          (get_local $4)
         )
        )
       )
      )
     )
    )
    (if
     (i32.ge_u
      (tee_local $1
       (i32.load
        (i32.const 4008)
       )
      )
      (get_local $3)
     )
     (block
      (set_local $0
       (i32.load
        (i32.const 4020)
       )
      )
      (if
       (i32.gt_u
        (tee_local $2
         (i32.sub
          (get_local $1)
          (get_local $3)
         )
        )
        (i32.const 15)
       )
       (block
        (i32.store
         (i32.const 4020)
         (tee_local $4
          (i32.add
           (get_local $0)
           (get_local $3)
          )
         )
        )
        (i32.store
         (i32.const 4008)
         (get_local $2)
        )
        (i32.store offset=4
         (get_local $4)
         (i32.or
          (get_local $2)
          (i32.const 1)
         )
        )
        (i32.store
         (i32.add
          (get_local $0)
          (get_local $1)
         )
         (get_local $2)
        )
        (i32.store offset=4
         (get_local $0)
         (i32.or
          (get_local $3)
          (i32.const 3)
         )
        )
       )
       (block
        (i32.store
         (i32.const 4008)
         (i32.const 0)
        )
        (i32.store
         (i32.const 4020)
         (i32.const 0)
        )
        (i32.store offset=4
         (get_local $0)
         (i32.or
          (get_local $1)
          (i32.const 3)
         )
        )
        (i32.store
         (tee_local $3
          (i32.add
           (i32.add
            (get_local $0)
            (get_local $1)
           )
           (i32.const 4)
          )
         )
         (i32.or
          (i32.load
           (get_local $3)
          )
          (i32.const 1)
         )
        )
       )
      )
      (br $folding-inner1)
     )
    )
    (if
     (i32.gt_u
      (tee_local $1
       (i32.load
        (i32.const 4012)
       )
      )
      (get_local $3)
     )
     (block
      (i32.store
       (i32.const 4012)
       (tee_local $1
        (i32.sub
         (get_local $1)
         (get_local $3)
        )
       )
      )
      (br $folding-inner0)
     )
    )
    (if
     (i32.le_u
      (tee_local $4
       (i32.and
        (tee_local $5
         (i32.add
          (tee_local $0
           (if (result i32)
            (i32.load
             (i32.const 4472)
            )
            (i32.load
             (i32.const 4480)
            )
            (block (result i32)
             (i32.store
              (i32.const 4480)
              (i32.const 4096)
             )
             (i32.store
              (i32.const 4476)
              (i32.const 4096)
             )
             (i32.store
              (i32.const 4484)
              (i32.const -1)
             )
             (i32.store
              (i32.const 4488)
              (i32.const -1)
             )
             (i32.store
              (i32.const 4492)
              (i32.const 0)
             )
             (i32.store
              (i32.const 4444)
              (i32.const 0)
             )
             (i32.store
              (i32.const 4472)
              (i32.xor
               (i32.and
                (get_local $14)
                (i32.const -16)
               )
               (i32.const 1431655768)
              )
             )
             (i32.const 4096)
            )
           )
          )
          (tee_local $6
           (i32.add
            (get_local $3)
            (i32.const 47)
           )
          )
         )
        )
        (tee_local $7
         (i32.sub
          (i32.const 0)
          (get_local $0)
         )
        )
       )
      )
      (get_local $3)
     )
     (block
      (set_global $STACKTOP
       (get_local $14)
      )
      (return
       (i32.const 0)
      )
     )
    )
    (if
     (tee_local $0
      (i32.load
       (i32.const 4440)
      )
     )
     (if
      (i32.or
       (i32.le_u
        (tee_local $10
         (i32.add
          (tee_local $2
           (i32.load
            (i32.const 4432)
           )
          )
          (get_local $4)
         )
        )
        (get_local $2)
       )
       (i32.gt_u
        (get_local $10)
        (get_local $0)
       )
      )
      (block
       (set_global $STACKTOP
        (get_local $14)
       )
       (return
        (i32.const 0)
       )
      )
     )
    )
    (set_local $10
     (i32.add
      (get_local $3)
      (i32.const 48)
     )
    )
    (block $__rjto$7
     (block $__rjti$7
      (if
       (i32.and
        (i32.load
         (i32.const 4444)
        )
        (i32.const 4)
       )
       (set_local $1
        (i32.const 0)
       )
       (block
        (block $do-once37
         (block $__rjti$3
          (block $__rjti$2
           (br_if $__rjti$2
            (i32.eqz
             (tee_local $0
              (i32.load
               (i32.const 4024)
              )
             )
            )
           )
           (set_local $2
            (i32.const 4448)
           )
           (loop $while-in34
            (block $while-out33
             (if
              (i32.le_u
               (tee_local $13
                (i32.load
                 (get_local $2)
                )
               )
               (get_local $0)
              )
              (br_if $while-out33
               (i32.gt_u
                (i32.add
                 (get_local $13)
                 (i32.load offset=4
                  (get_local $2)
                 )
                )
                (get_local $0)
               )
              )
             )
             (br_if $while-in34
              (tee_local $2
               (i32.load offset=8
                (get_local $2)
               )
              )
             )
             (br $__rjti$2)
            )
           )
           (if
            (i32.lt_u
             (tee_local $1
              (i32.and
               (i32.sub
                (get_local $5)
                (get_local $1)
               )
               (get_local $7)
              )
             )
             (i32.const 2147483647)
            )
            (if
             (i32.eq
              (tee_local $0
               (call $_sbrk
                (get_local $1)
               )
              )
              (i32.add
               (i32.load
                (get_local $2)
               )
               (i32.load offset=4
                (get_local $2)
               )
              )
             )
             (br_if $__rjti$7
              (i32.ne
               (get_local $0)
               (i32.const -1)
              )
             )
             (br $__rjti$3)
            )
            (set_local $1
             (i32.const 0)
            )
           )
           (br $do-once37)
          )
          (set_local $1
           (if (result i32)
            (i32.eq
             (tee_local $0
              (call $_sbrk
               (i32.const 0)
              )
             )
             (i32.const -1)
            )
            (i32.const 0)
            (block (result i32)
             (set_local $2
              (i32.add
               (tee_local $1
                (i32.add
                 (select
                  (i32.sub
                   (i32.and
                    (i32.add
                     (tee_local $2
                      (i32.add
                       (tee_local $1
                        (i32.load
                         (i32.const 4476)
                        )
                       )
                       (i32.const -1)
                      )
                     )
                     (get_local $0)
                    )
                    (i32.sub
                     (i32.const 0)
                     (get_local $1)
                    )
                   )
                   (get_local $0)
                  )
                  (i32.const 0)
                  (i32.and
                   (get_local $2)
                   (get_local $0)
                  )
                 )
                 (get_local $4)
                )
               )
               (tee_local $5
                (i32.load
                 (i32.const 4432)
                )
               )
              )
             )
             (if (result i32)
              (i32.and
               (i32.gt_u
                (get_local $1)
                (get_local $3)
               )
               (i32.lt_u
                (get_local $1)
                (i32.const 2147483647)
               )
              )
              (block
               (if
                (tee_local $7
                 (i32.load
                  (i32.const 4440)
                 )
                )
                (if
                 (i32.or
                  (i32.le_u
                   (get_local $2)
                   (get_local $5)
                  )
                  (i32.gt_u
                   (get_local $2)
                   (get_local $7)
                  )
                 )
                 (block
                  (set_local $1
                   (i32.const 0)
                  )
                  (br $do-once37)
                 )
                )
               )
               (br_if $__rjti$7
                (i32.eq
                 (tee_local $2
                  (call $_sbrk
                   (get_local $1)
                  )
                 )
                 (get_local $0)
                )
               )
               (set_local $0
                (get_local $2)
               )
               (br $__rjti$3)
              )
              (i32.const 0)
             )
            )
           )
          )
          (br $do-once37)
         )
         (if
          (i32.eqz
           (i32.and
            (i32.gt_u
             (get_local $10)
             (get_local $1)
            )
            (i32.and
             (i32.lt_u
              (get_local $1)
              (i32.const 2147483647)
             )
             (i32.ne
              (get_local $0)
              (i32.const -1)
             )
            )
           )
          )
          (if
           (i32.eq
            (get_local $0)
            (i32.const -1)
           )
           (block
            (set_local $1
             (i32.const 0)
            )
            (br $do-once37)
           )
           (br $__rjti$7)
          )
         )
         (br_if $__rjti$7
          (i32.ge_u
           (tee_local $2
            (i32.and
             (i32.add
              (i32.sub
               (get_local $6)
               (get_local $1)
              )
              (tee_local $2
               (i32.load
                (i32.const 4480)
               )
              )
             )
             (i32.sub
              (i32.const 0)
              (get_local $2)
             )
            )
           )
           (i32.const 2147483647)
          )
         )
         (set_local $6
          (i32.sub
           (i32.const 0)
           (get_local $1)
          )
         )
         (set_local $1
          (if (result i32)
           (i32.eq
            (call $_sbrk
             (get_local $2)
            )
            (i32.const -1)
           )
           (block (result i32)
            (drop
             (call $_sbrk
              (get_local $6)
             )
            )
            (i32.const 0)
           )
           (block
            (set_local $1
             (i32.add
              (get_local $2)
              (get_local $1)
             )
            )
            (br $__rjti$7)
           )
          )
         )
        )
        (i32.store
         (i32.const 4444)
         (i32.or
          (i32.load
           (i32.const 4444)
          )
          (i32.const 4)
         )
        )
       )
      )
      (if
       (i32.lt_u
        (get_local $4)
        (i32.const 2147483647)
       )
       (block
        (set_local $0
         (call $_sbrk
          (get_local $4)
         )
        )
        (set_local $4
         (i32.gt_u
          (tee_local $6
           (i32.sub
            (tee_local $2
             (call $_sbrk
              (i32.const 0)
             )
            )
            (get_local $0)
           )
          )
          (i32.add
           (get_local $3)
           (i32.const 40)
          )
         )
        )
        (set_local $1
         (select
          (get_local $6)
          (get_local $1)
          (get_local $4)
         )
        )
        (br_if $__rjti$7
         (i32.eqz
          (i32.or
           (i32.or
            (i32.eq
             (get_local $0)
             (i32.const -1)
            )
            (i32.xor
             (get_local $4)
             (i32.const 1)
            )
           )
           (i32.xor
            (i32.and
             (i32.lt_u
              (get_local $0)
              (get_local $2)
             )
             (i32.and
              (i32.ne
               (get_local $0)
               (i32.const -1)
              )
              (i32.ne
               (get_local $2)
               (i32.const -1)
              )
             )
            )
            (i32.const 1)
           )
          )
         )
        )
       )
      )
      (br $__rjto$7)
     )
     (i32.store
      (i32.const 4432)
      (tee_local $2
       (i32.add
        (i32.load
         (i32.const 4432)
        )
        (get_local $1)
       )
      )
     )
     (if
      (i32.gt_u
       (get_local $2)
       (i32.load
        (i32.const 4436)
       )
      )
      (i32.store
       (i32.const 4436)
       (get_local $2)
      )
     )
     (block $label$break$L294
      (if
       (tee_local $6
        (i32.load
         (i32.const 4024)
        )
       )
       (block
        (set_local $2
         (i32.const 4448)
        )
        (block $__rjto$4
         (block $__rjti$4
          (loop $while-in41
           (block $while-out40
            (br_if $__rjti$4
             (i32.eq
              (get_local $0)
              (i32.add
               (tee_local $4
                (i32.load
                 (get_local $2)
                )
               )
               (tee_local $5
                (i32.load offset=4
                 (get_local $2)
                )
               )
              )
             )
            )
            (br_if $while-in41
             (tee_local $2
              (i32.load offset=8
               (get_local $2)
              )
             )
            )
           )
          )
          (br $__rjto$4)
         )
         (set_local $7
          (i32.add
           (get_local $2)
           (i32.const 4)
          )
         )
         (if
          (i32.eqz
           (i32.and
            (i32.load offset=12
             (get_local $2)
            )
            (i32.const 8)
           )
          )
          (if
           (i32.and
            (i32.gt_u
             (get_local $0)
             (get_local $6)
            )
            (i32.le_u
             (get_local $4)
             (get_local $6)
            )
           )
           (block
            (i32.store
             (get_local $7)
             (i32.add
              (get_local $5)
              (get_local $1)
             )
            )
            (set_local $0
             (i32.add
              (get_local $6)
              (tee_local $2
               (select
                (i32.and
                 (i32.sub
                  (i32.const 0)
                  (tee_local $0
                   (i32.add
                    (get_local $6)
                    (i32.const 8)
                   )
                  )
                 )
                 (i32.const 7)
                )
                (i32.const 0)
                (i32.and
                 (get_local $0)
                 (i32.const 7)
                )
               )
              )
             )
            )
            (set_local $1
             (i32.sub
              (tee_local $4
               (i32.add
                (i32.load
                 (i32.const 4012)
                )
                (get_local $1)
               )
              )
              (get_local $2)
             )
            )
            (i32.store
             (i32.const 4024)
             (get_local $0)
            )
            (i32.store
             (i32.const 4012)
             (get_local $1)
            )
            (i32.store offset=4
             (get_local $0)
             (i32.or
              (get_local $1)
              (i32.const 1)
             )
            )
            (i32.store offset=4
             (i32.add
              (get_local $6)
              (get_local $4)
             )
             (i32.const 40)
            )
            (i32.store
             (i32.const 4028)
             (i32.load
              (i32.const 4488)
             )
            )
            (br $label$break$L294)
           )
          )
         )
        )
        (if
         (i32.lt_u
          (get_local $0)
          (tee_local $2
           (i32.load
            (i32.const 4016)
           )
          )
         )
         (block
          (i32.store
           (i32.const 4016)
           (get_local $0)
          )
          (set_local $2
           (get_local $0)
          )
         )
        )
        (set_local $5
         (i32.add
          (get_local $0)
          (get_local $1)
         )
        )
        (set_local $4
         (i32.const 4448)
        )
        (block $__rjto$5
         (block $__rjti$5
          (loop $while-in43
           (block $while-out42
            (br_if $__rjti$5
             (i32.eq
              (i32.load
               (get_local $4)
              )
              (get_local $5)
             )
            )
            (br_if $while-in43
             (tee_local $4
              (i32.load offset=8
               (get_local $4)
              )
             )
            )
           )
          )
          (br $__rjto$5)
         )
         (if
          (i32.eqz
           (i32.and
            (i32.load offset=12
             (get_local $4)
            )
            (i32.const 8)
           )
          )
          (block
           (i32.store
            (get_local $4)
            (get_local $0)
           )
           (i32.store
            (tee_local $4
             (i32.add
              (get_local $4)
              (i32.const 4)
             )
            )
            (i32.add
             (i32.load
              (get_local $4)
             )
             (get_local $1)
            )
           )
           (set_local $7
            (i32.add
             (tee_local $8
              (i32.add
               (get_local $0)
               (select
                (i32.and
                 (i32.sub
                  (i32.const 0)
                  (tee_local $0
                   (i32.add
                    (get_local $0)
                    (i32.const 8)
                   )
                  )
                 )
                 (i32.const 7)
                )
                (i32.const 0)
                (i32.and
                 (get_local $0)
                 (i32.const 7)
                )
               )
              )
             )
             (get_local $3)
            )
           )
           (set_local $4
            (i32.sub
             (i32.sub
              (tee_local $1
               (i32.add
                (get_local $5)
                (select
                 (i32.and
                  (i32.sub
                   (i32.const 0)
                   (tee_local $0
                    (i32.add
                     (get_local $5)
                     (i32.const 8)
                    )
                   )
                  )
                  (i32.const 7)
                 )
                 (i32.const 0)
                 (i32.and
                  (get_local $0)
                  (i32.const 7)
                 )
                )
               )
              )
              (get_local $8)
             )
             (get_local $3)
            )
           )
           (i32.store offset=4
            (get_local $8)
            (i32.or
             (get_local $3)
             (i32.const 3)
            )
           )
           (block $label$break$L317
            (if
             (i32.eq
              (get_local $6)
              (get_local $1)
             )
             (block
              (i32.store
               (i32.const 4012)
               (tee_local $0
                (i32.add
                 (i32.load
                  (i32.const 4012)
                 )
                 (get_local $4)
                )
               )
              )
              (i32.store
               (i32.const 4024)
               (get_local $7)
              )
              (i32.store offset=4
               (get_local $7)
               (i32.or
                (get_local $0)
                (i32.const 1)
               )
              )
             )
             (block
              (if
               (i32.eq
                (i32.load
                 (i32.const 4020)
                )
                (get_local $1)
               )
               (block
                (i32.store
                 (i32.const 4008)
                 (tee_local $0
                  (i32.add
                   (i32.load
                    (i32.const 4008)
                   )
                   (get_local $4)
                  )
                 )
                )
                (i32.store
                 (i32.const 4020)
                 (get_local $7)
                )
                (i32.store offset=4
                 (get_local $7)
                 (i32.or
                  (get_local $0)
                  (i32.const 1)
                 )
                )
                (i32.store
                 (i32.add
                  (get_local $7)
                  (get_local $0)
                 )
                 (get_local $0)
                )
                (br $label$break$L317)
               )
              )
              (set_local $2
               (if (result i32)
                (i32.eq
                 (i32.and
                  (tee_local $0
                   (i32.load offset=4
                    (get_local $1)
                   )
                  )
                  (i32.const 3)
                 )
                 (i32.const 1)
                )
                (block (result i32)
                 (set_local $13
                  (i32.and
                   (get_local $0)
                   (i32.const -8)
                  )
                 )
                 (set_local $5
                  (i32.shr_u
                   (get_local $0)
                   (i32.const 3)
                  )
                 )
                 (block $label$break$L325
                  (if
                   (i32.lt_u
                    (get_local $0)
                    (i32.const 256)
                   )
                   (block
                    (set_local $3
                     (i32.load offset=12
                      (get_local $1)
                     )
                    )
                    (block $do-once46
                     (if
                      (i32.ne
                       (tee_local $6
                        (i32.load offset=8
                         (get_local $1)
                        )
                       )
                       (tee_local $0
                        (i32.add
                         (i32.shl
                          (get_local $5)
                          (i32.const 3)
                         )
                         (i32.const 4040)
                        )
                       )
                      )
                      (block
                       (if
                        (i32.gt_u
                         (get_local $2)
                         (get_local $6)
                        )
                        (call $_abort)
                       )
                       (br_if $do-once46
                        (i32.eq
                         (i32.load offset=12
                          (get_local $6)
                         )
                         (get_local $1)
                        )
                       )
                       (call $_abort)
                      )
                     )
                    )
                    (if
                     (i32.eq
                      (get_local $3)
                      (get_local $6)
                     )
                     (block
                      (i32.store
                       (i32.const 4000)
                       (i32.and
                        (i32.load
                         (i32.const 4000)
                        )
                        (i32.xor
                         (i32.shl
                          (i32.const 1)
                          (get_local $5)
                         )
                         (i32.const -1)
                        )
                       )
                      )
                      (br $label$break$L325)
                     )
                    )
                    (block $do-once48
                     (if
                      (i32.eq
                       (get_local $3)
                       (get_local $0)
                      )
                      (set_local $20
                       (i32.add
                        (get_local $3)
                        (i32.const 8)
                       )
                      )
                      (block
                       (if
                        (i32.gt_u
                         (get_local $2)
                         (get_local $3)
                        )
                        (call $_abort)
                       )
                       (if
                        (i32.eq
                         (i32.load
                          (tee_local $0
                           (i32.add
                            (get_local $3)
                            (i32.const 8)
                           )
                          )
                         )
                         (get_local $1)
                        )
                        (block
                         (set_local $20
                          (get_local $0)
                         )
                         (br $do-once48)
                        )
                       )
                       (call $_abort)
                      )
                     )
                    )
                    (i32.store offset=12
                     (get_local $6)
                     (get_local $3)
                    )
                    (i32.store
                     (get_local $20)
                     (get_local $6)
                    )
                   )
                   (block
                    (set_local $10
                     (i32.load offset=24
                      (get_local $1)
                     )
                    )
                    (block $do-once50
                     (if
                      (i32.eq
                       (tee_local $0
                        (i32.load offset=12
                         (get_local $1)
                        )
                       )
                       (get_local $1)
                      )
                      (block
                       (if
                        (tee_local $0
                         (i32.load
                          (tee_local $6
                           (i32.add
                            (tee_local $3
                             (i32.add
                              (get_local $1)
                              (i32.const 16)
                             )
                            )
                            (i32.const 4)
                           )
                          )
                         )
                        )
                        (set_local $3
                         (get_local $6)
                        )
                        (br_if $do-once50
                         (i32.eqz
                          (tee_local $0
                           (i32.load
                            (get_local $3)
                           )
                          )
                         )
                        )
                       )
                       (loop $while-in53
                        (block $while-out52
                         (if
                          (i32.eqz
                           (tee_local $5
                            (i32.load
                             (tee_local $6
                              (i32.add
                               (get_local $0)
                               (i32.const 20)
                              )
                             )
                            )
                           )
                          )
                          (br_if $while-out52
                           (i32.eqz
                            (tee_local $5
                             (i32.load
                              (tee_local $6
                               (i32.add
                                (get_local $0)
                                (i32.const 16)
                               )
                              )
                             )
                            )
                           )
                          )
                         )
                         (set_local $3
                          (get_local $6)
                         )
                         (set_local $0
                          (get_local $5)
                         )
                         (br $while-in53)
                        )
                       )
                       (if
                        (i32.gt_u
                         (get_local $2)
                         (get_local $3)
                        )
                        (call $_abort)
                        (block
                         (i32.store
                          (get_local $3)
                          (i32.const 0)
                         )
                         (set_local $9
                          (get_local $0)
                         )
                        )
                       )
                      )
                      (block
                       (if
                        (i32.gt_u
                         (get_local $2)
                         (tee_local $3
                          (i32.load offset=8
                           (get_local $1)
                          )
                         )
                        )
                        (call $_abort)
                       )
                       (if
                        (i32.ne
                         (i32.load
                          (tee_local $2
                           (i32.add
                            (get_local $3)
                            (i32.const 12)
                           )
                          )
                         )
                         (get_local $1)
                        )
                        (call $_abort)
                       )
                       (if
                        (i32.eq
                         (i32.load
                          (tee_local $6
                           (i32.add
                            (get_local $0)
                            (i32.const 8)
                           )
                          )
                         )
                         (get_local $1)
                        )
                        (block
                         (i32.store
                          (get_local $2)
                          (get_local $0)
                         )
                         (i32.store
                          (get_local $6)
                          (get_local $3)
                         )
                         (set_local $9
                          (get_local $0)
                         )
                        )
                        (call $_abort)
                       )
                      )
                     )
                    )
                    (br_if $label$break$L325
                     (i32.eqz
                      (get_local $10)
                     )
                    )
                    (block $do-once54
                     (if
                      (i32.eq
                       (i32.load
                        (tee_local $3
                         (i32.add
                          (i32.shl
                           (tee_local $0
                            (i32.load offset=28
                             (get_local $1)
                            )
                           )
                           (i32.const 2)
                          )
                          (i32.const 4304)
                         )
                        )
                       )
                       (get_local $1)
                      )
                      (block
                       (i32.store
                        (get_local $3)
                        (get_local $9)
                       )
                       (br_if $do-once54
                        (get_local $9)
                       )
                       (i32.store
                        (i32.const 4004)
                        (i32.and
                         (i32.load
                          (i32.const 4004)
                         )
                         (i32.xor
                          (i32.shl
                           (i32.const 1)
                           (get_local $0)
                          )
                          (i32.const -1)
                         )
                        )
                       )
                       (br $label$break$L325)
                      )
                      (if
                       (i32.gt_u
                        (i32.load
                         (i32.const 4016)
                        )
                        (get_local $10)
                       )
                       (call $_abort)
                       (block
                        (i32.store
                         (select
                          (tee_local $0
                           (i32.add
                            (get_local $10)
                            (i32.const 16)
                           )
                          )
                          (i32.add
                           (get_local $10)
                           (i32.const 20)
                          )
                          (i32.eq
                           (i32.load
                            (get_local $0)
                           )
                           (get_local $1)
                          )
                         )
                         (get_local $9)
                        )
                        (br_if $label$break$L325
                         (i32.eqz
                          (get_local $9)
                         )
                        )
                       )
                      )
                     )
                    )
                    (if
                     (i32.gt_u
                      (tee_local $3
                       (i32.load
                        (i32.const 4016)
                       )
                      )
                      (get_local $9)
                     )
                     (call $_abort)
                    )
                    (i32.store offset=24
                     (get_local $9)
                     (get_local $10)
                    )
                    (if
                     (tee_local $0
                      (i32.load
                       (tee_local $2
                        (i32.add
                         (get_local $1)
                         (i32.const 16)
                        )
                       )
                      )
                     )
                     (if
                      (i32.gt_u
                       (get_local $3)
                       (get_local $0)
                      )
                      (call $_abort)
                      (block
                       (i32.store offset=16
                        (get_local $9)
                        (get_local $0)
                       )
                       (i32.store offset=24
                        (get_local $0)
                        (get_local $9)
                       )
                      )
                     )
                    )
                    (br_if $label$break$L325
                     (i32.eqz
                      (tee_local $0
                       (i32.load offset=4
                        (get_local $2)
                       )
                      )
                     )
                    )
                    (if
                     (i32.gt_u
                      (i32.load
                       (i32.const 4016)
                      )
                      (get_local $0)
                     )
                     (call $_abort)
                     (block
                      (i32.store offset=20
                       (get_local $9)
                       (get_local $0)
                      )
                      (i32.store offset=24
                       (get_local $0)
                       (get_local $9)
                      )
                     )
                    )
                   )
                  )
                 )
                 (set_local $1
                  (i32.add
                   (get_local $1)
                   (get_local $13)
                  )
                 )
                 (i32.add
                  (get_local $13)
                  (get_local $4)
                 )
                )
                (get_local $4)
               )
              )
              (i32.store
               (tee_local $0
                (i32.add
                 (get_local $1)
                 (i32.const 4)
                )
               )
               (i32.and
                (i32.load
                 (get_local $0)
                )
                (i32.const -2)
               )
              )
              (i32.store offset=4
               (get_local $7)
               (i32.or
                (get_local $2)
                (i32.const 1)
               )
              )
              (i32.store
               (i32.add
                (get_local $7)
                (get_local $2)
               )
               (get_local $2)
              )
              (set_local $3
               (i32.shr_u
                (get_local $2)
                (i32.const 3)
               )
              )
              (if
               (i32.lt_u
                (get_local $2)
                (i32.const 256)
               )
               (block
                (set_local $0
                 (i32.add
                  (i32.shl
                   (get_local $3)
                   (i32.const 3)
                  )
                  (i32.const 4040)
                 )
                )
                (block $do-once58
                 (if
                  (i32.and
                   (tee_local $1
                    (i32.load
                     (i32.const 4000)
                    )
                   )
                   (tee_local $3
                    (i32.shl
                     (i32.const 1)
                     (get_local $3)
                    )
                   )
                  )
                  (block
                   (if
                    (i32.le_u
                     (i32.load
                      (i32.const 4016)
                     )
                     (tee_local $1
                      (i32.load
                       (tee_local $3
                        (i32.add
                         (get_local $0)
                         (i32.const 8)
                        )
                       )
                      )
                     )
                    )
                    (block
                     (set_local $15
                      (get_local $1)
                     )
                     (set_local $21
                      (get_local $3)
                     )
                     (br $do-once58)
                    )
                   )
                   (call $_abort)
                  )
                  (block
                   (i32.store
                    (i32.const 4000)
                    (i32.or
                     (get_local $1)
                     (get_local $3)
                    )
                   )
                   (set_local $15
                    (get_local $0)
                   )
                   (set_local $21
                    (i32.add
                     (get_local $0)
                     (i32.const 8)
                    )
                   )
                  )
                 )
                )
                (i32.store
                 (get_local $21)
                 (get_local $7)
                )
                (i32.store offset=12
                 (get_local $15)
                 (get_local $7)
                )
                (i32.store offset=8
                 (get_local $7)
                 (get_local $15)
                )
                (i32.store offset=12
                 (get_local $7)
                 (get_local $0)
                )
                (br $label$break$L317)
               )
              )
              (set_local $0
               (i32.add
                (i32.shl
                 (tee_local $3
                  (block $do-once60 (result i32)
                   (if (result i32)
                    (tee_local $0
                     (i32.shr_u
                      (get_local $2)
                      (i32.const 8)
                     )
                    )
                    (block (result i32)
                     (drop
                      (br_if $do-once60
                       (i32.const 31)
                       (i32.gt_u
                        (get_local $2)
                        (i32.const 16777215)
                       )
                      )
                     )
                     (i32.or
                      (i32.and
                       (i32.shr_u
                        (get_local $2)
                        (i32.add
                         (tee_local $0
                          (i32.add
                           (i32.sub
                            (i32.const 14)
                            (i32.or
                             (i32.or
                              (tee_local $1
                               (i32.and
                                (i32.shr_u
                                 (i32.add
                                  (tee_local $3
                                   (i32.shl
                                    (get_local $0)
                                    (tee_local $0
                                     (i32.and
                                      (i32.shr_u
                                       (i32.add
                                        (get_local $0)
                                        (i32.const 1048320)
                                       )
                                       (i32.const 16)
                                      )
                                      (i32.const 8)
                                     )
                                    )
                                   )
                                  )
                                  (i32.const 520192)
                                 )
                                 (i32.const 16)
                                )
                                (i32.const 4)
                               )
                              )
                              (get_local $0)
                             )
                             (tee_local $3
                              (i32.and
                               (i32.shr_u
                                (i32.add
                                 (tee_local $0
                                  (i32.shl
                                   (get_local $3)
                                   (get_local $1)
                                  )
                                 )
                                 (i32.const 245760)
                                )
                                (i32.const 16)
                               )
                               (i32.const 2)
                              )
                             )
                            )
                           )
                           (i32.shr_u
                            (i32.shl
                             (get_local $0)
                             (get_local $3)
                            )
                            (i32.const 15)
                           )
                          )
                         )
                         (i32.const 7)
                        )
                       )
                       (i32.const 1)
                      )
                      (i32.shl
                       (get_local $0)
                       (i32.const 1)
                      )
                     )
                    )
                    (i32.const 0)
                   )
                  )
                 )
                 (i32.const 2)
                )
                (i32.const 4304)
               )
              )
              (i32.store offset=28
               (get_local $7)
               (get_local $3)
              )
              (i32.store offset=4
               (tee_local $1
                (i32.add
                 (get_local $7)
                 (i32.const 16)
                )
               )
               (i32.const 0)
              )
              (i32.store
               (get_local $1)
               (i32.const 0)
              )
              (if
               (i32.eqz
                (i32.and
                 (tee_local $1
                  (i32.load
                   (i32.const 4004)
                  )
                 )
                 (tee_local $4
                  (i32.shl
                   (i32.const 1)
                   (get_local $3)
                  )
                 )
                )
               )
               (block
                (i32.store
                 (i32.const 4004)
                 (i32.or
                  (get_local $1)
                  (get_local $4)
                 )
                )
                (i32.store
                 (get_local $0)
                 (get_local $7)
                )
                (i32.store offset=24
                 (get_local $7)
                 (get_local $0)
                )
                (i32.store offset=12
                 (get_local $7)
                 (get_local $7)
                )
                (i32.store offset=8
                 (get_local $7)
                 (get_local $7)
                )
                (br $label$break$L317)
               )
              )
              (block $label$break$L410
               (if
                (i32.eq
                 (i32.and
                  (i32.load offset=4
                   (tee_local $0
                    (i32.load
                     (get_local $0)
                    )
                   )
                  )
                  (i32.const -8)
                 )
                 (get_local $2)
                )
                (set_local $11
                 (get_local $0)
                )
                (block
                 (set_local $1
                  (i32.shl
                   (get_local $2)
                   (select
                    (i32.const 0)
                    (i32.sub
                     (i32.const 25)
                     (i32.shr_u
                      (get_local $3)
                      (i32.const 1)
                     )
                    )
                    (i32.eq
                     (get_local $3)
                     (i32.const 31)
                    )
                   )
                  )
                 )
                 (loop $while-in64
                  (if
                   (tee_local $3
                    (i32.load
                     (tee_local $4
                      (i32.add
                       (i32.add
                        (get_local $0)
                        (i32.const 16)
                       )
                       (i32.shl
                        (i32.shr_u
                         (get_local $1)
                         (i32.const 31)
                        )
                        (i32.const 2)
                       )
                      )
                     )
                    )
                   )
                   (block
                    (set_local $1
                     (i32.shl
                      (get_local $1)
                      (i32.const 1)
                     )
                    )
                    (if
                     (i32.eq
                      (i32.and
                       (i32.load offset=4
                        (get_local $3)
                       )
                       (i32.const -8)
                      )
                      (get_local $2)
                     )
                     (block
                      (set_local $11
                       (get_local $3)
                      )
                      (br $label$break$L410)
                     )
                     (block
                      (set_local $0
                       (get_local $3)
                      )
                      (br $while-in64)
                     )
                    )
                   )
                  )
                 )
                 (if
                  (i32.gt_u
                   (i32.load
                    (i32.const 4016)
                   )
                   (get_local $4)
                  )
                  (call $_abort)
                  (block
                   (i32.store
                    (get_local $4)
                    (get_local $7)
                   )
                   (i32.store offset=24
                    (get_local $7)
                    (get_local $0)
                   )
                   (i32.store offset=12
                    (get_local $7)
                    (get_local $7)
                   )
                   (i32.store offset=8
                    (get_local $7)
                    (get_local $7)
                   )
                   (br $label$break$L317)
                  )
                 )
                )
               )
              )
              (if
               (i32.and
                (i32.le_u
                 (tee_local $3
                  (i32.load
                   (i32.const 4016)
                  )
                 )
                 (tee_local $0
                  (i32.load
                   (tee_local $1
                    (i32.add
                     (get_local $11)
                     (i32.const 8)
                    )
                   )
                  )
                 )
                )
                (i32.le_u
                 (get_local $3)
                 (get_local $11)
                )
               )
               (block
                (i32.store offset=12
                 (get_local $0)
                 (get_local $7)
                )
                (i32.store
                 (get_local $1)
                 (get_local $7)
                )
                (i32.store offset=8
                 (get_local $7)
                 (get_local $0)
                )
                (i32.store offset=12
                 (get_local $7)
                 (get_local $11)
                )
                (i32.store offset=24
                 (get_local $7)
                 (i32.const 0)
                )
               )
               (call $_abort)
              )
             )
            )
           )
           (set_global $STACKTOP
            (get_local $14)
           )
           (return
            (i32.add
             (get_local $8)
             (i32.const 8)
            )
           )
          )
         )
        )
        (set_local $2
         (i32.const 4448)
        )
        (loop $while-in66
         (block $while-out65
          (if
           (i32.le_u
            (tee_local $4
             (i32.load
              (get_local $2)
             )
            )
            (get_local $6)
           )
           (br_if $while-out65
            (i32.gt_u
             (tee_local $5
              (i32.add
               (get_local $4)
               (i32.load offset=4
                (get_local $2)
               )
              )
             )
             (get_local $6)
            )
           )
          )
          (set_local $2
           (i32.load offset=8
            (get_local $2)
           )
          )
          (br $while-in66)
         )
        )
        (set_local $2
         (i32.add
          (tee_local $4
           (i32.add
            (get_local $5)
            (i32.const -47)
           )
          )
          (i32.const 8)
         )
        )
        (set_local $4
         (i32.add
          (tee_local $2
           (select
            (get_local $6)
            (tee_local $2
             (i32.add
              (get_local $4)
              (select
               (i32.and
                (i32.sub
                 (i32.const 0)
                 (get_local $2)
                )
                (i32.const 7)
               )
               (i32.const 0)
               (i32.and
                (get_local $2)
                (i32.const 7)
               )
              )
             )
            )
            (i32.lt_u
             (get_local $2)
             (tee_local $9
              (i32.add
               (get_local $6)
               (i32.const 16)
              )
             )
            )
           )
          )
          (i32.const 8)
         )
        )
        (i32.store
         (i32.const 4024)
         (tee_local $10
          (i32.add
           (get_local $0)
           (tee_local $7
            (select
             (i32.and
              (i32.sub
               (i32.const 0)
               (tee_local $7
                (i32.add
                 (get_local $0)
                 (i32.const 8)
                )
               )
              )
              (i32.const 7)
             )
             (i32.const 0)
             (i32.and
              (get_local $7)
              (i32.const 7)
             )
            )
           )
          )
         )
        )
        (i32.store
         (i32.const 4012)
         (tee_local $7
          (i32.sub
           (tee_local $11
            (i32.add
             (get_local $1)
             (i32.const -40)
            )
           )
           (get_local $7)
          )
         )
        )
        (i32.store offset=4
         (get_local $10)
         (i32.or
          (get_local $7)
          (i32.const 1)
         )
        )
        (i32.store offset=4
         (i32.add
          (get_local $0)
          (get_local $11)
         )
         (i32.const 40)
        )
        (i32.store
         (i32.const 4028)
         (i32.load
          (i32.const 4488)
         )
        )
        (i32.store
         (tee_local $7
          (i32.add
           (get_local $2)
           (i32.const 4)
          )
         )
         (i32.const 27)
        )
        (i64.store align=4
         (get_local $4)
         (i64.load align=4
          (i32.const 4448)
         )
        )
        (i64.store offset=8 align=4
         (get_local $4)
         (i64.load align=4
          (i32.const 4456)
         )
        )
        (i32.store
         (i32.const 4448)
         (get_local $0)
        )
        (i32.store
         (i32.const 4452)
         (get_local $1)
        )
        (i32.store
         (i32.const 4460)
         (i32.const 0)
        )
        (i32.store
         (i32.const 4456)
         (get_local $4)
        )
        (set_local $0
         (i32.add
          (get_local $2)
          (i32.const 24)
         )
        )
        (loop $while-in68
         (i32.store
          (tee_local $1
           (i32.add
            (get_local $0)
            (i32.const 4)
           )
          )
          (i32.const 7)
         )
         (if
          (i32.lt_u
           (i32.add
            (get_local $0)
            (i32.const 8)
           )
           (get_local $5)
          )
          (block
           (set_local $0
            (get_local $1)
           )
           (br $while-in68)
          )
         )
        )
        (if
         (i32.ne
          (get_local $2)
          (get_local $6)
         )
         (block
          (i32.store
           (get_local $7)
           (i32.and
            (i32.load
             (get_local $7)
            )
            (i32.const -2)
           )
          )
          (i32.store offset=4
           (get_local $6)
           (i32.or
            (tee_local $4
             (i32.sub
              (get_local $2)
              (get_local $6)
             )
            )
            (i32.const 1)
           )
          )
          (i32.store
           (get_local $2)
           (get_local $4)
          )
          (set_local $1
           (i32.shr_u
            (get_local $4)
            (i32.const 3)
           )
          )
          (if
           (i32.lt_u
            (get_local $4)
            (i32.const 256)
           )
           (block
            (set_local $0
             (i32.add
              (i32.shl
               (get_local $1)
               (i32.const 3)
              )
              (i32.const 4040)
             )
            )
            (if
             (i32.and
              (tee_local $2
               (i32.load
                (i32.const 4000)
               )
              )
              (tee_local $1
               (i32.shl
                (i32.const 1)
                (get_local $1)
               )
              )
             )
             (if
              (i32.gt_u
               (i32.load
                (i32.const 4016)
               )
               (tee_local $2
                (i32.load
                 (tee_local $1
                  (i32.add
                   (get_local $0)
                   (i32.const 8)
                  )
                 )
                )
               )
              )
              (call $_abort)
              (block
               (set_local $17
                (get_local $2)
               )
               (set_local $22
                (get_local $1)
               )
              )
             )
             (block
              (i32.store
               (i32.const 4000)
               (i32.or
                (get_local $2)
                (get_local $1)
               )
              )
              (set_local $17
               (get_local $0)
              )
              (set_local $22
               (i32.add
                (get_local $0)
                (i32.const 8)
               )
              )
             )
            )
            (i32.store
             (get_local $22)
             (get_local $6)
            )
            (i32.store offset=12
             (get_local $17)
             (get_local $6)
            )
            (i32.store offset=8
             (get_local $6)
             (get_local $17)
            )
            (i32.store offset=12
             (get_local $6)
             (get_local $0)
            )
            (br $label$break$L294)
           )
          )
          (set_local $0
           (i32.add
            (i32.shl
             (tee_local $1
              (if (result i32)
               (tee_local $0
                (i32.shr_u
                 (get_local $4)
                 (i32.const 8)
                )
               )
               (if (result i32)
                (i32.gt_u
                 (get_local $4)
                 (i32.const 16777215)
                )
                (i32.const 31)
                (i32.or
                 (i32.and
                  (i32.shr_u
                   (get_local $4)
                   (i32.add
                    (tee_local $0
                     (i32.add
                      (i32.sub
                       (i32.const 14)
                       (i32.or
                        (i32.or
                         (tee_local $2
                          (i32.and
                           (i32.shr_u
                            (i32.add
                             (tee_local $1
                              (i32.shl
                               (get_local $0)
                               (tee_local $0
                                (i32.and
                                 (i32.shr_u
                                  (i32.add
                                   (get_local $0)
                                   (i32.const 1048320)
                                  )
                                  (i32.const 16)
                                 )
                                 (i32.const 8)
                                )
                               )
                              )
                             )
                             (i32.const 520192)
                            )
                            (i32.const 16)
                           )
                           (i32.const 4)
                          )
                         )
                         (get_local $0)
                        )
                        (tee_local $1
                         (i32.and
                          (i32.shr_u
                           (i32.add
                            (tee_local $0
                             (i32.shl
                              (get_local $1)
                              (get_local $2)
                             )
                            )
                            (i32.const 245760)
                           )
                           (i32.const 16)
                          )
                          (i32.const 2)
                         )
                        )
                       )
                      )
                      (i32.shr_u
                       (i32.shl
                        (get_local $0)
                        (get_local $1)
                       )
                       (i32.const 15)
                      )
                     )
                    )
                    (i32.const 7)
                   )
                  )
                  (i32.const 1)
                 )
                 (i32.shl
                  (get_local $0)
                  (i32.const 1)
                 )
                )
               )
               (i32.const 0)
              )
             )
             (i32.const 2)
            )
            (i32.const 4304)
           )
          )
          (i32.store offset=28
           (get_local $6)
           (get_local $1)
          )
          (i32.store offset=20
           (get_local $6)
           (i32.const 0)
          )
          (i32.store
           (get_local $9)
           (i32.const 0)
          )
          (if
           (i32.eqz
            (i32.and
             (tee_local $2
              (i32.load
               (i32.const 4004)
              )
             )
             (tee_local $5
              (i32.shl
               (i32.const 1)
               (get_local $1)
              )
             )
            )
           )
           (block
            (i32.store
             (i32.const 4004)
             (i32.or
              (get_local $2)
              (get_local $5)
             )
            )
            (i32.store
             (get_local $0)
             (get_local $6)
            )
            (i32.store offset=24
             (get_local $6)
             (get_local $0)
            )
            (i32.store offset=12
             (get_local $6)
             (get_local $6)
            )
            (i32.store offset=8
             (get_local $6)
             (get_local $6)
            )
            (br $label$break$L294)
           )
          )
          (block $label$break$L451
           (if
            (i32.eq
             (i32.and
              (i32.load offset=4
               (tee_local $0
                (i32.load
                 (get_local $0)
                )
               )
              )
              (i32.const -8)
             )
             (get_local $4)
            )
            (set_local $8
             (get_local $0)
            )
            (block
             (set_local $2
              (i32.shl
               (get_local $4)
               (select
                (i32.const 0)
                (i32.sub
                 (i32.const 25)
                 (i32.shr_u
                  (get_local $1)
                  (i32.const 1)
                 )
                )
                (i32.eq
                 (get_local $1)
                 (i32.const 31)
                )
               )
              )
             )
             (loop $while-in71
              (if
               (tee_local $1
                (i32.load
                 (tee_local $5
                  (i32.add
                   (i32.add
                    (get_local $0)
                    (i32.const 16)
                   )
                   (i32.shl
                    (i32.shr_u
                     (get_local $2)
                     (i32.const 31)
                    )
                    (i32.const 2)
                   )
                  )
                 )
                )
               )
               (block
                (set_local $2
                 (i32.shl
                  (get_local $2)
                  (i32.const 1)
                 )
                )
                (if
                 (i32.eq
                  (i32.and
                   (i32.load offset=4
                    (get_local $1)
                   )
                   (i32.const -8)
                  )
                  (get_local $4)
                 )
                 (block
                  (set_local $8
                   (get_local $1)
                  )
                  (br $label$break$L451)
                 )
                 (block
                  (set_local $0
                   (get_local $1)
                  )
                  (br $while-in71)
                 )
                )
               )
              )
             )
             (if
              (i32.gt_u
               (i32.load
                (i32.const 4016)
               )
               (get_local $5)
              )
              (call $_abort)
              (block
               (i32.store
                (get_local $5)
                (get_local $6)
               )
               (i32.store offset=24
                (get_local $6)
                (get_local $0)
               )
               (i32.store offset=12
                (get_local $6)
                (get_local $6)
               )
               (i32.store offset=8
                (get_local $6)
                (get_local $6)
               )
               (br $label$break$L294)
              )
             )
            )
           )
          )
          (if
           (i32.and
            (i32.le_u
             (tee_local $1
              (i32.load
               (i32.const 4016)
              )
             )
             (tee_local $0
              (i32.load
               (tee_local $2
                (i32.add
                 (get_local $8)
                 (i32.const 8)
                )
               )
              )
             )
            )
            (i32.le_u
             (get_local $1)
             (get_local $8)
            )
           )
           (block
            (i32.store offset=12
             (get_local $0)
             (get_local $6)
            )
            (i32.store
             (get_local $2)
             (get_local $6)
            )
            (i32.store offset=8
             (get_local $6)
             (get_local $0)
            )
            (i32.store offset=12
             (get_local $6)
             (get_local $8)
            )
            (i32.store offset=24
             (get_local $6)
             (i32.const 0)
            )
           )
           (call $_abort)
          )
         )
        )
       )
       (block
        (if
         (i32.or
          (i32.eqz
           (tee_local $2
            (i32.load
             (i32.const 4016)
            )
           )
          )
          (i32.lt_u
           (get_local $0)
           (get_local $2)
          )
         )
         (i32.store
          (i32.const 4016)
          (get_local $0)
         )
        )
        (i32.store
         (i32.const 4448)
         (get_local $0)
        )
        (i32.store
         (i32.const 4452)
         (get_local $1)
        )
        (i32.store
         (i32.const 4460)
         (i32.const 0)
        )
        (i32.store
         (i32.const 4036)
         (i32.load
          (i32.const 4472)
         )
        )
        (i32.store
         (i32.const 4032)
         (i32.const -1)
        )
        (i32.store
         (i32.const 4052)
         (i32.const 4040)
        )
        (i32.store
         (i32.const 4048)
         (i32.const 4040)
        )
        (i32.store
         (i32.const 4060)
         (i32.const 4048)
        )
        (i32.store
         (i32.const 4056)
         (i32.const 4048)
        )
        (i32.store
         (i32.const 4068)
         (i32.const 4056)
        )
        (i32.store
         (i32.const 4064)
         (i32.const 4056)
        )
        (i32.store
         (i32.const 4076)
         (i32.const 4064)
        )
        (i32.store
         (i32.const 4072)
         (i32.const 4064)
        )
        (i32.store
         (i32.const 4084)
         (i32.const 4072)
        )
        (i32.store
         (i32.const 4080)
         (i32.const 4072)
        )
        (i32.store
         (i32.const 4092)
         (i32.const 4080)
        )
        (i32.store
         (i32.const 4088)
         (i32.const 4080)
        )
        (i32.store
         (i32.const 4100)
         (i32.const 4088)
        )
        (i32.store
         (i32.const 4096)
         (i32.const 4088)
        )
        (i32.store
         (i32.const 4108)
         (i32.const 4096)
        )
        (i32.store
         (i32.const 4104)
         (i32.const 4096)
        )
        (i32.store
         (i32.const 4116)
         (i32.const 4104)
        )
        (i32.store
         (i32.const 4112)
         (i32.const 4104)
        )
        (i32.store
         (i32.const 4124)
         (i32.const 4112)
        )
        (i32.store
         (i32.const 4120)
         (i32.const 4112)
        )
        (i32.store
         (i32.const 4132)
         (i32.const 4120)
        )
        (i32.store
         (i32.const 4128)
         (i32.const 4120)
        )
        (i32.store
         (i32.const 4140)
         (i32.const 4128)
        )
        (i32.store
         (i32.const 4136)
         (i32.const 4128)
        )
        (i32.store
         (i32.const 4148)
         (i32.const 4136)
        )
        (i32.store
         (i32.const 4144)
         (i32.const 4136)
        )
        (i32.store
         (i32.const 4156)
         (i32.const 4144)
        )
        (i32.store
         (i32.const 4152)
         (i32.const 4144)
        )
        (i32.store
         (i32.const 4164)
         (i32.const 4152)
        )
        (i32.store
         (i32.const 4160)
         (i32.const 4152)
        )
        (i32.store
         (i32.const 4172)
         (i32.const 4160)
        )
        (i32.store
         (i32.const 4168)
         (i32.const 4160)
        )
        (i32.store
         (i32.const 4180)
         (i32.const 4168)
        )
        (i32.store
         (i32.const 4176)
         (i32.const 4168)
        )
        (i32.store
         (i32.const 4188)
         (i32.const 4176)
        )
        (i32.store
         (i32.const 4184)
         (i32.const 4176)
        )
        (i32.store
         (i32.const 4196)
         (i32.const 4184)
        )
        (i32.store
         (i32.const 4192)
         (i32.const 4184)
        )
        (i32.store
         (i32.const 4204)
         (i32.const 4192)
        )
        (i32.store
         (i32.const 4200)
         (i32.const 4192)
        )
        (i32.store
         (i32.const 4212)
         (i32.const 4200)
        )
        (i32.store
         (i32.const 4208)
         (i32.const 4200)
        )
        (i32.store
         (i32.const 4220)
         (i32.const 4208)
        )
        (i32.store
         (i32.const 4216)
         (i32.const 4208)
        )
        (i32.store
         (i32.const 4228)
         (i32.const 4216)
        )
        (i32.store
         (i32.const 4224)
         (i32.const 4216)
        )
        (i32.store
         (i32.const 4236)
         (i32.const 4224)
        )
        (i32.store
         (i32.const 4232)
         (i32.const 4224)
        )
        (i32.store
         (i32.const 4244)
         (i32.const 4232)
        )
        (i32.store
         (i32.const 4240)
         (i32.const 4232)
        )
        (i32.store
         (i32.const 4252)
         (i32.const 4240)
        )
        (i32.store
         (i32.const 4248)
         (i32.const 4240)
        )
        (i32.store
         (i32.const 4260)
         (i32.const 4248)
        )
        (i32.store
         (i32.const 4256)
         (i32.const 4248)
        )
        (i32.store
         (i32.const 4268)
         (i32.const 4256)
        )
        (i32.store
         (i32.const 4264)
         (i32.const 4256)
        )
        (i32.store
         (i32.const 4276)
         (i32.const 4264)
        )
        (i32.store
         (i32.const 4272)
         (i32.const 4264)
        )
        (i32.store
         (i32.const 4284)
         (i32.const 4272)
        )
        (i32.store
         (i32.const 4280)
         (i32.const 4272)
        )
        (i32.store
         (i32.const 4292)
         (i32.const 4280)
        )
        (i32.store
         (i32.const 4288)
         (i32.const 4280)
        )
        (i32.store
         (i32.const 4300)
         (i32.const 4288)
        )
        (i32.store
         (i32.const 4296)
         (i32.const 4288)
        )
        (i32.store
         (i32.const 4024)
         (tee_local $4
          (i32.add
           (get_local $0)
           (tee_local $2
            (select
             (i32.and
              (i32.sub
               (i32.const 0)
               (tee_local $2
                (i32.add
                 (get_local $0)
                 (i32.const 8)
                )
               )
              )
              (i32.const 7)
             )
             (i32.const 0)
             (i32.and
              (get_local $2)
              (i32.const 7)
             )
            )
           )
          )
         )
        )
        (i32.store
         (i32.const 4012)
         (tee_local $2
          (i32.sub
           (tee_local $1
            (i32.add
             (get_local $1)
             (i32.const -40)
            )
           )
           (get_local $2)
          )
         )
        )
        (i32.store offset=4
         (get_local $4)
         (i32.or
          (get_local $2)
          (i32.const 1)
         )
        )
        (i32.store offset=4
         (i32.add
          (get_local $0)
          (get_local $1)
         )
         (i32.const 40)
        )
        (i32.store
         (i32.const 4028)
         (i32.load
          (i32.const 4488)
         )
        )
       )
      )
     )
     (if
      (i32.gt_u
       (tee_local $0
        (i32.load
         (i32.const 4012)
        )
       )
       (get_local $3)
      )
      (block
       (i32.store
        (i32.const 4012)
        (tee_local $1
         (i32.sub
          (get_local $0)
          (get_local $3)
         )
        )
       )
       (br $folding-inner0)
      )
     )
    )
    (i32.store
     (call $___errno_location)
     (i32.const 12)
    )
    (set_global $STACKTOP
     (get_local $14)
    )
    (return
     (i32.const 0)
    )
   )
   (i32.store
    (i32.const 4024)
    (tee_local $2
     (i32.add
      (tee_local $0
       (i32.load
        (i32.const 4024)
       )
      )
      (get_local $3)
     )
    )
   )
   (i32.store offset=4
    (get_local $2)
    (i32.or
     (get_local $1)
     (i32.const 1)
    )
   )
   (i32.store offset=4
    (get_local $0)
    (i32.or
     (get_local $3)
     (i32.const 3)
    )
   )
  )
  (set_global $STACKTOP
   (get_local $14)
  )
  (i32.add
   (get_local $0)
   (i32.const 8)
  )
 )
 (func $_free (; 21 ;) (; has Stack IR ;) (param $0 i32)
  (local $1 i32)
  (local $2 i32)
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
  (local $15 i32)
  (local $16 i32)
  (local $17 i32)
  (if
   (i32.eqz
    (get_local $0)
   )
   (return)
  )
  (if
   (i32.lt_u
    (tee_local $4
     (i32.add
      (get_local $0)
      (i32.const -8)
     )
    )
    (tee_local $12
     (i32.load
      (i32.const 4016)
     )
    )
   )
   (call $_abort)
  )
  (if
   (i32.eq
    (tee_local $11
     (i32.and
      (tee_local $0
       (i32.load
        (i32.add
         (get_local $0)
         (i32.const -4)
        )
       )
      )
      (i32.const 3)
     )
    )
    (i32.const 1)
   )
   (call $_abort)
  )
  (set_local $7
   (i32.add
    (get_local $4)
    (tee_local $2
     (i32.and
      (get_local $0)
      (i32.const -8)
     )
    )
   )
  )
  (block $label$break$L10
   (if
    (i32.and
     (get_local $0)
     (i32.const 1)
    )
    (block
     (set_local $1
      (get_local $2)
     )
     (set_local $5
      (tee_local $3
       (get_local $4)
      )
     )
    )
    (block
     (set_local $9
      (i32.load
       (get_local $4)
      )
     )
     (if
      (i32.eqz
       (get_local $11)
      )
      (return)
     )
     (if
      (i32.lt_u
       (tee_local $0
        (i32.sub
         (get_local $4)
         (get_local $9)
        )
       )
       (get_local $12)
      )
      (call $_abort)
     )
     (set_local $4
      (i32.add
       (get_local $9)
       (get_local $2)
      )
     )
     (if
      (i32.eq
       (i32.load
        (i32.const 4020)
       )
       (get_local $0)
      )
      (block
       (if
        (i32.ne
         (i32.and
          (tee_local $1
           (i32.load
            (tee_local $3
             (i32.add
              (get_local $7)
              (i32.const 4)
             )
            )
           )
          )
          (i32.const 3)
         )
         (i32.const 3)
        )
        (block
         (set_local $1
          (get_local $4)
         )
         (set_local $5
          (tee_local $3
           (get_local $0)
          )
         )
         (br $label$break$L10)
        )
       )
       (i32.store
        (i32.const 4008)
        (get_local $4)
       )
       (i32.store
        (get_local $3)
        (i32.and
         (get_local $1)
         (i32.const -2)
        )
       )
       (i32.store offset=4
        (get_local $0)
        (i32.or
         (get_local $4)
         (i32.const 1)
        )
       )
       (i32.store
        (i32.add
         (get_local $0)
         (get_local $4)
        )
        (get_local $4)
       )
       (return)
      )
     )
     (set_local $2
      (i32.shr_u
       (get_local $9)
       (i32.const 3)
      )
     )
     (if
      (i32.lt_u
       (get_local $9)
       (i32.const 256)
      )
      (block
       (set_local $1
        (i32.load offset=12
         (get_local $0)
        )
       )
       (if
        (i32.ne
         (tee_local $5
          (i32.load offset=8
           (get_local $0)
          )
         )
         (tee_local $3
          (i32.add
           (i32.shl
            (get_local $2)
            (i32.const 3)
           )
           (i32.const 4040)
          )
         )
        )
        (block
         (if
          (i32.gt_u
           (get_local $12)
           (get_local $5)
          )
          (call $_abort)
         )
         (if
          (i32.ne
           (i32.load offset=12
            (get_local $5)
           )
           (get_local $0)
          )
          (call $_abort)
         )
        )
       )
       (if
        (i32.eq
         (get_local $1)
         (get_local $5)
        )
        (block
         (i32.store
          (i32.const 4000)
          (i32.and
           (i32.load
            (i32.const 4000)
           )
           (i32.xor
            (i32.shl
             (i32.const 1)
             (get_local $2)
            )
            (i32.const -1)
           )
          )
         )
         (set_local $1
          (get_local $4)
         )
         (set_local $5
          (tee_local $3
           (get_local $0)
          )
         )
         (br $label$break$L10)
        )
       )
       (if
        (i32.eq
         (get_local $1)
         (get_local $3)
        )
        (set_local $6
         (i32.add
          (get_local $1)
          (i32.const 8)
         )
        )
        (block
         (if
          (i32.gt_u
           (get_local $12)
           (get_local $1)
          )
          (call $_abort)
         )
         (if
          (i32.eq
           (i32.load
            (tee_local $3
             (i32.add
              (get_local $1)
              (i32.const 8)
             )
            )
           )
           (get_local $0)
          )
          (set_local $6
           (get_local $3)
          )
          (call $_abort)
         )
        )
       )
       (i32.store offset=12
        (get_local $5)
        (get_local $1)
       )
       (i32.store
        (get_local $6)
        (get_local $5)
       )
       (set_local $1
        (get_local $4)
       )
       (set_local $5
        (tee_local $3
         (get_local $0)
        )
       )
       (br $label$break$L10)
      )
     )
     (set_local $13
      (i32.load offset=24
       (get_local $0)
      )
     )
     (block $do-once
      (if
       (i32.eq
        (tee_local $2
         (i32.load offset=12
          (get_local $0)
         )
        )
        (get_local $0)
       )
       (block
        (if
         (tee_local $2
          (i32.load
           (tee_local $9
            (i32.add
             (tee_local $6
              (i32.add
               (get_local $0)
               (i32.const 16)
              )
             )
             (i32.const 4)
            )
           )
          )
         )
         (set_local $6
          (get_local $9)
         )
         (br_if $do-once
          (i32.eqz
           (tee_local $2
            (i32.load
             (get_local $6)
            )
           )
          )
         )
        )
        (loop $while-in
         (block $while-out
          (if
           (i32.eqz
            (tee_local $11
             (i32.load
              (tee_local $9
               (i32.add
                (get_local $2)
                (i32.const 20)
               )
              )
             )
            )
           )
           (br_if $while-out
            (i32.eqz
             (tee_local $11
              (i32.load
               (tee_local $9
                (i32.add
                 (get_local $2)
                 (i32.const 16)
                )
               )
              )
             )
            )
           )
          )
          (set_local $6
           (get_local $9)
          )
          (set_local $2
           (get_local $11)
          )
          (br $while-in)
         )
        )
        (if
         (i32.gt_u
          (get_local $12)
          (get_local $6)
         )
         (call $_abort)
         (block
          (i32.store
           (get_local $6)
           (i32.const 0)
          )
          (set_local $8
           (get_local $2)
          )
         )
        )
       )
       (block
        (if
         (i32.gt_u
          (get_local $12)
          (tee_local $6
           (i32.load offset=8
            (get_local $0)
           )
          )
         )
         (call $_abort)
        )
        (if
         (i32.ne
          (i32.load
           (tee_local $9
            (i32.add
             (get_local $6)
             (i32.const 12)
            )
           )
          )
          (get_local $0)
         )
         (call $_abort)
        )
        (if
         (i32.eq
          (i32.load
           (tee_local $11
            (i32.add
             (get_local $2)
             (i32.const 8)
            )
           )
          )
          (get_local $0)
         )
         (block
          (i32.store
           (get_local $9)
           (get_local $2)
          )
          (i32.store
           (get_local $11)
           (get_local $6)
          )
          (set_local $8
           (get_local $2)
          )
         )
         (call $_abort)
        )
       )
      )
     )
     (if
      (get_local $13)
      (block
       (if
        (i32.eq
         (i32.load
          (tee_local $6
           (i32.add
            (i32.shl
             (tee_local $2
              (i32.load offset=28
               (get_local $0)
              )
             )
             (i32.const 2)
            )
            (i32.const 4304)
           )
          )
         )
         (get_local $0)
        )
        (block
         (i32.store
          (get_local $6)
          (get_local $8)
         )
         (if
          (i32.eqz
           (get_local $8)
          )
          (block
           (i32.store
            (i32.const 4004)
            (i32.and
             (i32.load
              (i32.const 4004)
             )
             (i32.xor
              (i32.shl
               (i32.const 1)
               (get_local $2)
              )
              (i32.const -1)
             )
            )
           )
           (set_local $1
            (get_local $4)
           )
           (set_local $5
            (tee_local $3
             (get_local $0)
            )
           )
           (br $label$break$L10)
          )
         )
        )
        (if
         (i32.gt_u
          (i32.load
           (i32.const 4016)
          )
          (get_local $13)
         )
         (call $_abort)
         (block
          (i32.store
           (select
            (tee_local $2
             (i32.add
              (get_local $13)
              (i32.const 16)
             )
            )
            (i32.add
             (get_local $13)
             (i32.const 20)
            )
            (i32.eq
             (i32.load
              (get_local $2)
             )
             (get_local $0)
            )
           )
           (get_local $8)
          )
          (if
           (i32.eqz
            (get_local $8)
           )
           (block
            (set_local $1
             (get_local $4)
            )
            (set_local $5
             (tee_local $3
              (get_local $0)
             )
            )
            (br $label$break$L10)
           )
          )
         )
        )
       )
       (if
        (i32.gt_u
         (tee_local $6
          (i32.load
           (i32.const 4016)
          )
         )
         (get_local $8)
        )
        (call $_abort)
       )
       (i32.store offset=24
        (get_local $8)
        (get_local $13)
       )
       (if
        (tee_local $2
         (i32.load
          (tee_local $9
           (i32.add
            (get_local $0)
            (i32.const 16)
           )
          )
         )
        )
        (if
         (i32.gt_u
          (get_local $6)
          (get_local $2)
         )
         (call $_abort)
         (block
          (i32.store offset=16
           (get_local $8)
           (get_local $2)
          )
          (i32.store offset=24
           (get_local $2)
           (get_local $8)
          )
         )
        )
       )
       (if
        (tee_local $2
         (i32.load offset=4
          (get_local $9)
         )
        )
        (if
         (i32.gt_u
          (i32.load
           (i32.const 4016)
          )
          (get_local $2)
         )
         (call $_abort)
         (block
          (i32.store offset=20
           (get_local $8)
           (get_local $2)
          )
          (i32.store offset=24
           (get_local $2)
           (get_local $8)
          )
          (set_local $1
           (get_local $4)
          )
          (set_local $5
           (tee_local $3
            (get_local $0)
           )
          )
         )
        )
        (block
         (set_local $1
          (get_local $4)
         )
         (set_local $5
          (tee_local $3
           (get_local $0)
          )
         )
        )
       )
      )
      (block
       (set_local $1
        (get_local $4)
       )
       (set_local $5
        (tee_local $3
         (get_local $0)
        )
       )
      )
     )
    )
   )
  )
  (if
   (i32.ge_u
    (get_local $5)
    (get_local $7)
   )
   (call $_abort)
  )
  (if
   (i32.eqz
    (i32.and
     (tee_local $0
      (i32.load
       (tee_local $4
        (i32.add
         (get_local $7)
         (i32.const 4)
        )
       )
      )
     )
     (i32.const 1)
    )
   )
   (call $_abort)
  )
  (set_local $1
   (i32.shr_u
    (tee_local $5
     (if (result i32)
      (i32.and
       (get_local $0)
       (i32.const 2)
      )
      (block (result i32)
       (i32.store
        (get_local $4)
        (i32.and
         (get_local $0)
         (i32.const -2)
        )
       )
       (i32.store offset=4
        (get_local $3)
        (i32.or
         (get_local $1)
         (i32.const 1)
        )
       )
       (i32.store
        (i32.add
         (get_local $5)
         (get_local $1)
        )
        (get_local $1)
       )
       (get_local $1)
      )
      (block (result i32)
       (if
        (i32.eq
         (i32.load
          (i32.const 4024)
         )
         (get_local $7)
        )
        (block
         (i32.store
          (i32.const 4012)
          (tee_local $0
           (i32.add
            (i32.load
             (i32.const 4012)
            )
            (get_local $1)
           )
          )
         )
         (i32.store
          (i32.const 4024)
          (get_local $3)
         )
         (i32.store offset=4
          (get_local $3)
          (i32.or
           (get_local $0)
           (i32.const 1)
          )
         )
         (if
          (i32.ne
           (get_local $3)
           (i32.load
            (i32.const 4020)
           )
          )
          (return)
         )
         (i32.store
          (i32.const 4020)
          (i32.const 0)
         )
         (i32.store
          (i32.const 4008)
          (i32.const 0)
         )
         (return)
        )
       )
       (if
        (i32.eq
         (i32.load
          (i32.const 4020)
         )
         (get_local $7)
        )
        (block
         (i32.store
          (i32.const 4008)
          (tee_local $0
           (i32.add
            (i32.load
             (i32.const 4008)
            )
            (get_local $1)
           )
          )
         )
         (i32.store
          (i32.const 4020)
          (get_local $5)
         )
         (i32.store offset=4
          (get_local $3)
          (i32.or
           (get_local $0)
           (i32.const 1)
          )
         )
         (i32.store
          (i32.add
           (get_local $5)
           (get_local $0)
          )
          (get_local $0)
         )
         (return)
        )
       )
       (set_local $4
        (i32.add
         (i32.and
          (get_local $0)
          (i32.const -8)
         )
         (get_local $1)
        )
       )
       (set_local $6
        (i32.shr_u
         (get_local $0)
         (i32.const 3)
        )
       )
       (block $label$break$L111
        (if
         (i32.lt_u
          (get_local $0)
          (i32.const 256)
         )
         (block
          (set_local $1
           (i32.load offset=12
            (get_local $7)
           )
          )
          (if
           (i32.ne
            (tee_local $2
             (i32.load offset=8
              (get_local $7)
             )
            )
            (tee_local $0
             (i32.add
              (i32.shl
               (get_local $6)
               (i32.const 3)
              )
              (i32.const 4040)
             )
            )
           )
           (block
            (if
             (i32.gt_u
              (i32.load
               (i32.const 4016)
              )
              (get_local $2)
             )
             (call $_abort)
            )
            (if
             (i32.ne
              (i32.load offset=12
               (get_local $2)
              )
              (get_local $7)
             )
             (call $_abort)
            )
           )
          )
          (if
           (i32.eq
            (get_local $1)
            (get_local $2)
           )
           (block
            (i32.store
             (i32.const 4000)
             (i32.and
              (i32.load
               (i32.const 4000)
              )
              (i32.xor
               (i32.shl
                (i32.const 1)
                (get_local $6)
               )
               (i32.const -1)
              )
             )
            )
            (br $label$break$L111)
           )
          )
          (if
           (i32.eq
            (get_local $1)
            (get_local $0)
           )
           (set_local $16
            (i32.add
             (get_local $1)
             (i32.const 8)
            )
           )
           (block
            (if
             (i32.gt_u
              (i32.load
               (i32.const 4016)
              )
              (get_local $1)
             )
             (call $_abort)
            )
            (if
             (i32.eq
              (i32.load
               (tee_local $0
                (i32.add
                 (get_local $1)
                 (i32.const 8)
                )
               )
              )
              (get_local $7)
             )
             (set_local $16
              (get_local $0)
             )
             (call $_abort)
            )
           )
          )
          (i32.store offset=12
           (get_local $2)
           (get_local $1)
          )
          (i32.store
           (get_local $16)
           (get_local $2)
          )
         )
         (block
          (set_local $8
           (i32.load offset=24
            (get_local $7)
           )
          )
          (block $do-once6
           (if
            (i32.eq
             (tee_local $0
              (i32.load offset=12
               (get_local $7)
              )
             )
             (get_local $7)
            )
            (block
             (if
              (tee_local $0
               (i32.load
                (tee_local $2
                 (i32.add
                  (tee_local $1
                   (i32.add
                    (get_local $7)
                    (i32.const 16)
                   )
                  )
                  (i32.const 4)
                 )
                )
               )
              )
              (set_local $1
               (get_local $2)
              )
              (br_if $do-once6
               (i32.eqz
                (tee_local $0
                 (i32.load
                  (get_local $1)
                 )
                )
               )
              )
             )
             (loop $while-in9
              (block $while-out8
               (if
                (i32.eqz
                 (tee_local $6
                  (i32.load
                   (tee_local $2
                    (i32.add
                     (get_local $0)
                     (i32.const 20)
                    )
                   )
                  )
                 )
                )
                (br_if $while-out8
                 (i32.eqz
                  (tee_local $6
                   (i32.load
                    (tee_local $2
                     (i32.add
                      (get_local $0)
                      (i32.const 16)
                     )
                    )
                   )
                  )
                 )
                )
               )
               (set_local $1
                (get_local $2)
               )
               (set_local $0
                (get_local $6)
               )
               (br $while-in9)
              )
             )
             (if
              (i32.gt_u
               (i32.load
                (i32.const 4016)
               )
               (get_local $1)
              )
              (call $_abort)
              (block
               (i32.store
                (get_local $1)
                (i32.const 0)
               )
               (set_local $10
                (get_local $0)
               )
              )
             )
            )
            (block
             (if
              (i32.gt_u
               (i32.load
                (i32.const 4016)
               )
               (tee_local $1
                (i32.load offset=8
                 (get_local $7)
                )
               )
              )
              (call $_abort)
             )
             (if
              (i32.ne
               (i32.load
                (tee_local $2
                 (i32.add
                  (get_local $1)
                  (i32.const 12)
                 )
                )
               )
               (get_local $7)
              )
              (call $_abort)
             )
             (if
              (i32.eq
               (i32.load
                (tee_local $6
                 (i32.add
                  (get_local $0)
                  (i32.const 8)
                 )
                )
               )
               (get_local $7)
              )
              (block
               (i32.store
                (get_local $2)
                (get_local $0)
               )
               (i32.store
                (get_local $6)
                (get_local $1)
               )
               (set_local $10
                (get_local $0)
               )
              )
              (call $_abort)
             )
            )
           )
          )
          (if
           (get_local $8)
           (block
            (if
             (i32.eq
              (i32.load
               (tee_local $1
                (i32.add
                 (i32.shl
                  (tee_local $0
                   (i32.load offset=28
                    (get_local $7)
                   )
                  )
                  (i32.const 2)
                 )
                 (i32.const 4304)
                )
               )
              )
              (get_local $7)
             )
             (block
              (i32.store
               (get_local $1)
               (get_local $10)
              )
              (if
               (i32.eqz
                (get_local $10)
               )
               (block
                (i32.store
                 (i32.const 4004)
                 (i32.and
                  (i32.load
                   (i32.const 4004)
                  )
                  (i32.xor
                   (i32.shl
                    (i32.const 1)
                    (get_local $0)
                   )
                   (i32.const -1)
                  )
                 )
                )
                (br $label$break$L111)
               )
              )
             )
             (if
              (i32.gt_u
               (i32.load
                (i32.const 4016)
               )
               (get_local $8)
              )
              (call $_abort)
              (block
               (i32.store
                (select
                 (tee_local $0
                  (i32.add
                   (get_local $8)
                   (i32.const 16)
                  )
                 )
                 (i32.add
                  (get_local $8)
                  (i32.const 20)
                 )
                 (i32.eq
                  (i32.load
                   (get_local $0)
                  )
                  (get_local $7)
                 )
                )
                (get_local $10)
               )
               (br_if $label$break$L111
                (i32.eqz
                 (get_local $10)
                )
               )
              )
             )
            )
            (if
             (i32.gt_u
              (tee_local $1
               (i32.load
                (i32.const 4016)
               )
              )
              (get_local $10)
             )
             (call $_abort)
            )
            (i32.store offset=24
             (get_local $10)
             (get_local $8)
            )
            (if
             (tee_local $0
              (i32.load
               (tee_local $2
                (i32.add
                 (get_local $7)
                 (i32.const 16)
                )
               )
              )
             )
             (if
              (i32.gt_u
               (get_local $1)
               (get_local $0)
              )
              (call $_abort)
              (block
               (i32.store offset=16
                (get_local $10)
                (get_local $0)
               )
               (i32.store offset=24
                (get_local $0)
                (get_local $10)
               )
              )
             )
            )
            (if
             (tee_local $0
              (i32.load offset=4
               (get_local $2)
              )
             )
             (if
              (i32.gt_u
               (i32.load
                (i32.const 4016)
               )
               (get_local $0)
              )
              (call $_abort)
              (block
               (i32.store offset=20
                (get_local $10)
                (get_local $0)
               )
               (i32.store offset=24
                (get_local $0)
                (get_local $10)
               )
              )
             )
            )
           )
          )
         )
        )
       )
       (i32.store offset=4
        (get_local $3)
        (i32.or
         (get_local $4)
         (i32.const 1)
        )
       )
       (i32.store
        (i32.add
         (get_local $5)
         (get_local $4)
        )
        (get_local $4)
       )
       (if (result i32)
        (i32.eq
         (get_local $3)
         (i32.load
          (i32.const 4020)
         )
        )
        (block
         (i32.store
          (i32.const 4008)
          (get_local $4)
         )
         (return)
        )
        (get_local $4)
       )
      )
     )
    )
    (i32.const 3)
   )
  )
  (if
   (i32.lt_u
    (get_local $5)
    (i32.const 256)
   )
   (block
    (set_local $0
     (i32.add
      (i32.shl
       (get_local $1)
       (i32.const 3)
      )
      (i32.const 4040)
     )
    )
    (if
     (i32.and
      (tee_local $5
       (i32.load
        (i32.const 4000)
       )
      )
      (tee_local $1
       (i32.shl
        (i32.const 1)
        (get_local $1)
       )
      )
     )
     (if
      (i32.gt_u
       (i32.load
        (i32.const 4016)
       )
       (tee_local $5
        (i32.load
         (tee_local $1
          (i32.add
           (get_local $0)
           (i32.const 8)
          )
         )
        )
       )
      )
      (call $_abort)
      (block
       (set_local $15
        (get_local $5)
       )
       (set_local $17
        (get_local $1)
       )
      )
     )
     (block
      (i32.store
       (i32.const 4000)
       (i32.or
        (get_local $5)
        (get_local $1)
       )
      )
      (set_local $15
       (get_local $0)
      )
      (set_local $17
       (i32.add
        (get_local $0)
        (i32.const 8)
       )
      )
     )
    )
    (i32.store
     (get_local $17)
     (get_local $3)
    )
    (i32.store offset=12
     (get_local $15)
     (get_local $3)
    )
    (i32.store offset=8
     (get_local $3)
     (get_local $15)
    )
    (i32.store offset=12
     (get_local $3)
     (get_local $0)
    )
    (return)
   )
  )
  (set_local $0
   (i32.add
    (i32.shl
     (tee_local $1
      (if (result i32)
       (tee_local $0
        (i32.shr_u
         (get_local $5)
         (i32.const 8)
        )
       )
       (if (result i32)
        (i32.gt_u
         (get_local $5)
         (i32.const 16777215)
        )
        (i32.const 31)
        (i32.or
         (i32.and
          (i32.shr_u
           (get_local $5)
           (i32.add
            (tee_local $0
             (i32.add
              (i32.sub
               (i32.const 14)
               (i32.or
                (i32.or
                 (tee_local $4
                  (i32.and
                   (i32.shr_u
                    (i32.add
                     (tee_local $1
                      (i32.shl
                       (get_local $0)
                       (tee_local $0
                        (i32.and
                         (i32.shr_u
                          (i32.add
                           (get_local $0)
                           (i32.const 1048320)
                          )
                          (i32.const 16)
                         )
                         (i32.const 8)
                        )
                       )
                      )
                     )
                     (i32.const 520192)
                    )
                    (i32.const 16)
                   )
                   (i32.const 4)
                  )
                 )
                 (get_local $0)
                )
                (tee_local $1
                 (i32.and
                  (i32.shr_u
                   (i32.add
                    (tee_local $0
                     (i32.shl
                      (get_local $1)
                      (get_local $4)
                     )
                    )
                    (i32.const 245760)
                   )
                   (i32.const 16)
                  )
                  (i32.const 2)
                 )
                )
               )
              )
              (i32.shr_u
               (i32.shl
                (get_local $0)
                (get_local $1)
               )
               (i32.const 15)
              )
             )
            )
            (i32.const 7)
           )
          )
          (i32.const 1)
         )
         (i32.shl
          (get_local $0)
          (i32.const 1)
         )
        )
       )
       (i32.const 0)
      )
     )
     (i32.const 2)
    )
    (i32.const 4304)
   )
  )
  (i32.store offset=28
   (get_local $3)
   (get_local $1)
  )
  (i32.store offset=20
   (get_local $3)
   (i32.const 0)
  )
  (i32.store offset=16
   (get_local $3)
   (i32.const 0)
  )
  (block $label$break$L197
   (if
    (i32.and
     (tee_local $4
      (i32.load
       (i32.const 4004)
      )
     )
     (tee_local $2
      (i32.shl
       (i32.const 1)
       (get_local $1)
      )
     )
    )
    (block
     (block $label$break$L200
      (if
       (i32.eq
        (i32.and
         (i32.load offset=4
          (tee_local $0
           (i32.load
            (get_local $0)
           )
          )
         )
         (i32.const -8)
        )
        (get_local $5)
       )
       (set_local $14
        (get_local $0)
       )
       (block
        (set_local $4
         (i32.shl
          (get_local $5)
          (select
           (i32.const 0)
           (i32.sub
            (i32.const 25)
            (i32.shr_u
             (get_local $1)
             (i32.const 1)
            )
           )
           (i32.eq
            (get_local $1)
            (i32.const 31)
           )
          )
         )
        )
        (loop $while-in17
         (if
          (tee_local $1
           (i32.load
            (tee_local $2
             (i32.add
              (i32.add
               (get_local $0)
               (i32.const 16)
              )
              (i32.shl
               (i32.shr_u
                (get_local $4)
                (i32.const 31)
               )
               (i32.const 2)
              )
             )
            )
           )
          )
          (block
           (set_local $4
            (i32.shl
             (get_local $4)
             (i32.const 1)
            )
           )
           (if
            (i32.eq
             (i32.and
              (i32.load offset=4
               (get_local $1)
              )
              (i32.const -8)
             )
             (get_local $5)
            )
            (block
             (set_local $14
              (get_local $1)
             )
             (br $label$break$L200)
            )
            (block
             (set_local $0
              (get_local $1)
             )
             (br $while-in17)
            )
           )
          )
         )
        )
        (if
         (i32.gt_u
          (i32.load
           (i32.const 4016)
          )
          (get_local $2)
         )
         (call $_abort)
         (block
          (i32.store
           (get_local $2)
           (get_local $3)
          )
          (i32.store offset=24
           (get_local $3)
           (get_local $0)
          )
          (i32.store offset=12
           (get_local $3)
           (get_local $3)
          )
          (i32.store offset=8
           (get_local $3)
           (get_local $3)
          )
          (br $label$break$L197)
         )
        )
       )
      )
     )
     (if
      (i32.and
       (i32.le_u
        (tee_local $1
         (i32.load
          (i32.const 4016)
         )
        )
        (tee_local $0
         (i32.load
          (tee_local $5
           (i32.add
            (get_local $14)
            (i32.const 8)
           )
          )
         )
        )
       )
       (i32.le_u
        (get_local $1)
        (get_local $14)
       )
      )
      (block
       (i32.store offset=12
        (get_local $0)
        (get_local $3)
       )
       (i32.store
        (get_local $5)
        (get_local $3)
       )
       (i32.store offset=8
        (get_local $3)
        (get_local $0)
       )
       (i32.store offset=12
        (get_local $3)
        (get_local $14)
       )
       (i32.store offset=24
        (get_local $3)
        (i32.const 0)
       )
      )
      (call $_abort)
     )
    )
    (block
     (i32.store
      (i32.const 4004)
      (i32.or
       (get_local $4)
       (get_local $2)
      )
     )
     (i32.store
      (get_local $0)
      (get_local $3)
     )
     (i32.store offset=24
      (get_local $3)
      (get_local $0)
     )
     (i32.store offset=12
      (get_local $3)
      (get_local $3)
     )
     (i32.store offset=8
      (get_local $3)
      (get_local $3)
     )
    )
   )
  )
  (i32.store
   (i32.const 4032)
   (tee_local $0
    (i32.add
     (i32.load
      (i32.const 4032)
     )
     (i32.const -1)
    )
   )
  )
  (if
   (get_local $0)
   (return)
  )
  (set_local $0
   (i32.const 4456)
  )
  (loop $while-in19
   (set_local $0
    (i32.add
     (tee_local $3
      (i32.load
       (get_local $0)
      )
     )
     (i32.const 8)
    )
   )
   (br_if $while-in19
    (get_local $3)
   )
  )
  (i32.store
   (i32.const 4032)
   (i32.const -1)
  )
 )
 (func $___stdio_close (; 22 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (set_local $1
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 16)
   )
  )
  (i32.store
   (get_local $1)
   (call $_dummy_133
    (i32.load offset=60
     (get_local $0)
    )
   )
  )
  (set_local $0
   (call $___syscall_ret
    (call $___syscall6
     (i32.const 6)
     (get_local $1)
    )
   )
  )
  (set_global $STACKTOP
   (get_local $1)
  )
  (get_local $0)
 )
 (func $___stdio_write (; 23 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
  (set_local $7
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 48)
   )
  )
  (set_local $6
   (i32.add
    (get_local $7)
    (i32.const 32)
   )
  )
  (i32.store
   (tee_local $3
    (get_local $7)
   )
   (tee_local $4
    (i32.load
     (tee_local $10
      (i32.add
       (get_local $0)
       (i32.const 28)
      )
     )
    )
   )
  )
  (i32.store offset=4
   (get_local $3)
   (tee_local $4
    (i32.sub
     (i32.load
      (tee_local $11
       (i32.add
        (get_local $0)
        (i32.const 20)
       )
      )
     )
     (get_local $4)
    )
   )
  )
  (i32.store offset=8
   (get_local $3)
   (get_local $1)
  )
  (i32.store offset=12
   (get_local $3)
   (get_local $2)
  )
  (set_local $4
   (i32.add
    (get_local $4)
    (get_local $2)
   )
  )
  (i32.store
   (tee_local $1
    (i32.add
     (get_local $3)
     (i32.const 16)
    )
   )
   (i32.load
    (tee_local $12
     (i32.add
      (get_local $0)
      (i32.const 60)
     )
    )
   )
  )
  (i32.store offset=4
   (get_local $1)
   (get_local $3)
  )
  (i32.store offset=8
   (get_local $1)
   (i32.const 2)
  )
  (set_local $5
   (call $___syscall_ret
    (call $___syscall146
     (i32.const 146)
     (get_local $1)
    )
   )
  )
  (block $__rjto$0
   (block $__rjti$0
    (br_if $__rjti$0
     (i32.eq
      (get_local $4)
      (get_local $5)
     )
    )
    (set_local $8
     (i32.const 2)
    )
    (set_local $1
     (get_local $3)
    )
    (set_local $3
     (get_local $5)
    )
    (loop $while-in
     (if
      (i32.ge_s
       (get_local $3)
       (i32.const 0)
      )
      (block
       (set_local $4
        (i32.sub
         (get_local $4)
         (get_local $3)
        )
       )
       (i32.store
        (tee_local $1
         (select
          (i32.add
           (get_local $1)
           (i32.const 8)
          )
          (get_local $1)
          (tee_local $5
           (i32.gt_u
            (get_local $3)
            (tee_local $9
             (i32.load offset=4
              (get_local $1)
             )
            )
           )
          )
         )
        )
        (i32.add
         (i32.load
          (get_local $1)
         )
         (tee_local $3
          (i32.sub
           (get_local $3)
           (select
            (get_local $9)
            (i32.const 0)
            (get_local $5)
           )
          )
         )
        )
       )
       (i32.store
        (tee_local $9
         (i32.add
          (get_local $1)
          (i32.const 4)
         )
        )
        (i32.sub
         (i32.load
          (get_local $9)
         )
         (get_local $3)
        )
       )
       (i32.store
        (get_local $6)
        (i32.load
         (get_local $12)
        )
       )
       (i32.store offset=4
        (get_local $6)
        (get_local $1)
       )
       (i32.store offset=8
        (get_local $6)
        (tee_local $8
         (i32.add
          (get_local $8)
          (i32.shr_s
           (i32.shl
            (get_local $5)
            (i32.const 31)
           )
           (i32.const 31)
          )
         )
        )
       )
       (set_local $3
        (call $___syscall_ret
         (call $___syscall146
          (i32.const 146)
          (get_local $6)
         )
        )
       )
       (br_if $__rjti$0
        (i32.eq
         (get_local $4)
         (get_local $3)
        )
       )
       (br $while-in)
      )
     )
    )
    (i32.store offset=16
     (get_local $0)
     (i32.const 0)
    )
    (i32.store
     (get_local $10)
     (i32.const 0)
    )
    (i32.store
     (get_local $11)
     (i32.const 0)
    )
    (i32.store
     (get_local $0)
     (i32.or
      (i32.load
       (get_local $0)
      )
      (i32.const 32)
     )
    )
    (set_local $2
     (if (result i32)
      (i32.eq
       (get_local $8)
       (i32.const 2)
      )
      (i32.const 0)
      (i32.sub
       (get_local $2)
       (i32.load offset=4
        (get_local $1)
       )
      )
     )
    )
    (br $__rjto$0)
   )
   (i32.store offset=16
    (get_local $0)
    (i32.add
     (tee_local $1
      (i32.load offset=44
       (get_local $0)
      )
     )
     (i32.load offset=48
      (get_local $0)
     )
    )
   )
   (i32.store
    (get_local $10)
    (get_local $1)
   )
   (i32.store
    (get_local $11)
    (get_local $1)
   )
  )
  (set_global $STACKTOP
   (get_local $7)
  )
  (get_local $2)
 )
 (func $___stdio_seek (; 24 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (set_local $3
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 32)
   )
  )
  (i32.store
   (get_local $3)
   (i32.load offset=60
    (get_local $0)
   )
  )
  (i32.store offset=4
   (get_local $3)
   (i32.const 0)
  )
  (i32.store offset=8
   (get_local $3)
   (get_local $1)
  )
  (i32.store offset=12
   (get_local $3)
   (tee_local $0
    (i32.add
     (get_local $3)
     (i32.const 20)
    )
   )
  )
  (i32.store offset=16
   (get_local $3)
   (get_local $2)
  )
  (set_local $0
   (if (result i32)
    (i32.lt_s
     (call $___syscall_ret
      (call $___syscall140
       (i32.const 140)
       (get_local $3)
      )
     )
     (i32.const 0)
    )
    (block (result i32)
     (i32.store
      (get_local $0)
      (i32.const -1)
     )
     (i32.const -1)
    )
    (i32.load
     (get_local $0)
    )
   )
  )
  (set_global $STACKTOP
   (get_local $3)
  )
  (get_local $0)
 )
 (func $___syscall_ret (; 25 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (if (result i32)
   (i32.gt_u
    (get_local $0)
    (i32.const -4096)
   )
   (block (result i32)
    (i32.store
     (call $___errno_location)
     (i32.sub
      (i32.const 0)
      (get_local $0)
     )
    )
    (i32.const -1)
   )
   (get_local $0)
  )
 )
 (func $___errno_location (; 26 ;) (; has Stack IR ;) (result i32)
  (i32.const 4496)
 )
 (func $_dummy_133 (; 27 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (get_local $0)
 )
 (func $___lctrans (; 28 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (call $___lctrans_impl
   (get_local $0)
   (get_local $1)
  )
 )
 (func $___lctrans_impl (; 29 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (select
   (tee_local $2
    (if (result i32)
     (get_local $1)
     (call $___mo_lookup
      (i32.load
       (get_local $1)
      )
      (i32.load offset=4
       (get_local $1)
      )
      (get_local $0)
     )
     (i32.const 0)
    )
   )
   (get_local $0)
   (get_local $2)
  )
 )
 (func $___mo_lookup (; 30 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
  (set_local $4
   (call $_swapc
    (i32.load offset=8
     (get_local $0)
    )
    (tee_local $6
     (i32.add
      (i32.load
       (get_local $0)
      )
      (i32.const 1794895138)
     )
    )
   )
  )
  (set_local $5
   (call $_swapc
    (i32.load offset=12
     (get_local $0)
    )
    (get_local $6)
   )
  )
  (set_local $3
   (call $_swapc
    (i32.load offset=16
     (get_local $0)
    )
    (get_local $6)
   )
  )
  (tee_local $0
   (block $label$break$L1 (result i32)
    (if (result i32)
     (i32.lt_u
      (get_local $4)
      (i32.shr_u
       (get_local $1)
       (i32.const 2)
      )
     )
     (if (result i32)
      (i32.and
       (i32.lt_u
        (get_local $5)
        (tee_local $7
         (i32.sub
          (get_local $1)
          (i32.shl
           (get_local $4)
           (i32.const 2)
          )
         )
        )
       )
       (i32.lt_u
        (get_local $3)
        (get_local $7)
       )
      )
      (if (result i32)
       (i32.and
        (i32.or
         (get_local $3)
         (get_local $5)
        )
        (i32.const 3)
       )
       (i32.const 0)
       (block (result i32)
        (set_local $9
         (i32.shr_u
          (get_local $5)
          (i32.const 2)
         )
        )
        (set_local $10
         (i32.shr_u
          (get_local $3)
          (i32.const 2)
         )
        )
        (set_local $5
         (i32.const 0)
        )
        (loop $while-in
         (block $while-out
          (set_local $8
           (call $_swapc
            (i32.load
             (i32.add
              (get_local $0)
              (i32.shl
               (tee_local $3
                (i32.add
                 (tee_local $12
                  (i32.shl
                   (tee_local $11
                    (i32.add
                     (get_local $5)
                     (tee_local $7
                      (i32.shr_u
                       (get_local $4)
                       (i32.const 1)
                      )
                     )
                    )
                   )
                   (i32.const 1)
                  )
                 )
                 (get_local $9)
                )
               )
               (i32.const 2)
              )
             )
            )
            (get_local $6)
           )
          )
          (drop
           (br_if $label$break$L1
            (i32.const 0)
            (i32.eqz
             (i32.and
              (i32.lt_u
               (tee_local $3
                (call $_swapc
                 (i32.load
                  (i32.add
                   (get_local $0)
                   (i32.shl
                    (i32.add
                     (get_local $3)
                     (i32.const 1)
                    )
                    (i32.const 2)
                   )
                  )
                 )
                 (get_local $6)
                )
               )
               (get_local $1)
              )
              (i32.lt_u
               (get_local $8)
               (i32.sub
                (get_local $1)
                (get_local $3)
               )
              )
             )
            )
           )
          )
          (drop
           (br_if $label$break$L1
            (i32.const 0)
            (i32.load8_s
             (i32.add
              (get_local $0)
              (i32.add
               (get_local $3)
               (get_local $8)
              )
             )
            )
           )
          )
          (br_if $while-out
           (i32.eqz
            (tee_local $3
             (call $_strcmp
              (get_local $2)
              (i32.add
               (get_local $0)
               (get_local $3)
              )
             )
            )
           )
          )
          (set_local $3
           (i32.lt_s
            (get_local $3)
            (i32.const 0)
           )
          )
          (drop
           (br_if $label$break$L1
            (i32.const 0)
            (i32.eq
             (get_local $4)
             (i32.const 1)
            )
           )
          )
          (set_local $5
           (select
            (get_local $5)
            (get_local $11)
            (get_local $3)
           )
          )
          (set_local $4
           (select
            (get_local $7)
            (i32.sub
             (get_local $4)
             (get_local $7)
            )
            (get_local $3)
           )
          )
          (br $while-in)
         )
        )
        (set_local $4
         (call $_swapc
          (i32.load
           (i32.add
            (get_local $0)
            (i32.shl
             (tee_local $2
              (i32.add
               (get_local $12)
               (get_local $10)
              )
             )
             (i32.const 2)
            )
           )
          )
          (get_local $6)
         )
        )
        (if (result i32)
         (i32.and
          (i32.lt_u
           (tee_local $2
            (call $_swapc
             (i32.load
              (i32.add
               (get_local $0)
               (i32.shl
                (i32.add
                 (get_local $2)
                 (i32.const 1)
                )
                (i32.const 2)
               )
              )
             )
             (get_local $6)
            )
           )
           (get_local $1)
          )
          (i32.lt_u
           (get_local $4)
           (i32.sub
            (get_local $1)
            (get_local $2)
           )
          )
         )
         (select
          (i32.const 0)
          (i32.add
           (get_local $0)
           (get_local $2)
          )
          (i32.load8_s
           (i32.add
            (get_local $0)
            (i32.add
             (get_local $2)
             (get_local $4)
            )
           )
          )
         )
         (i32.const 0)
        )
       )
      )
      (i32.const 0)
     )
     (i32.const 0)
    )
   )
  )
 )
 (func $_swapc (; 31 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (select
   (call $_llvm_bswap_i32
    (get_local $0)
   )
   (get_local $0)
   (get_local $1)
  )
 )
 (func $_strcmp (; 32 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (set_local $0
   (if (result i32)
    (i32.or
     (i32.eqz
      (tee_local $2
       (i32.load8_s
        (get_local $0)
       )
      )
     )
     (i32.ne
      (get_local $2)
      (tee_local $3
       (i32.load8_s
        (get_local $1)
       )
      )
     )
    )
    (block (result i32)
     (set_local $1
      (get_local $2)
     )
     (get_local $3)
    )
    (loop $while-in (result i32)
     (if (result i32)
      (i32.or
       (i32.eqz
        (tee_local $2
         (i32.load8_s
          (tee_local $0
           (i32.add
            (get_local $0)
            (i32.const 1)
           )
          )
         )
        )
       )
       (i32.ne
        (get_local $2)
        (tee_local $3
         (i32.load8_s
          (tee_local $1
           (i32.add
            (get_local $1)
            (i32.const 1)
           )
          )
         )
        )
       )
      )
      (block (result i32)
       (set_local $1
        (get_local $2)
       )
       (get_local $3)
      )
      (br $while-in)
     )
    )
   )
  )
  (i32.sub
   (i32.and
    (get_local $1)
    (i32.const 255)
   )
   (i32.and
    (get_local $0)
    (i32.const 255)
   )
  )
 )
 (func $_pthread_self (; 33 ;) (; has Stack IR ;) (result i32)
  (i32.const 3536)
 )
 (func $_strlen (; 34 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (block $__rjto$0
   (if
    (i32.and
     (tee_local $2
      (get_local $0)
     )
     (i32.const 3)
    )
    (block
     (set_local $0
      (tee_local $1
       (get_local $2)
      )
     )
     (loop $while-in
      (br_if $__rjto$0
       (i32.eqz
        (i32.load8_s
         (get_local $1)
        )
       )
      )
      (br_if $while-in
       (i32.and
        (tee_local $0
         (tee_local $1
          (i32.add
           (get_local $1)
           (i32.const 1)
          )
         )
        )
        (i32.const 3)
       )
      )
     )
     (set_local $0
      (get_local $1)
     )
    )
   )
   (loop $while-in1
    (set_local $1
     (i32.add
      (get_local $0)
      (i32.const 4)
     )
    )
    (if
     (i32.eqz
      (i32.and
       (i32.xor
        (i32.and
         (tee_local $3
          (i32.load
           (get_local $0)
          )
         )
         (i32.const -2139062144)
        )
        (i32.const -2139062144)
       )
       (i32.add
        (get_local $3)
        (i32.const -16843009)
       )
      )
     )
     (block
      (set_local $0
       (get_local $1)
      )
      (br $while-in1)
     )
    )
   )
   (if
    (i32.and
     (get_local $3)
     (i32.const 255)
    )
    (loop $while-in3
     (br_if $while-in3
      (i32.load8_s
       (tee_local $0
        (i32.add
         (get_local $0)
         (i32.const 1)
        )
       )
      )
     )
    )
   )
  )
  (i32.sub
   (get_local $0)
   (get_local $2)
  )
 )
 (func $_memchr (; 35 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (set_local $4
   (i32.and
    (get_local $1)
    (i32.const 255)
   )
  )
  (block $__rjto$3
   (block $__rjti$3
    (block $__rjti$2
     (if
      (i32.and
       (tee_local $3
        (i32.ne
         (get_local $2)
         (i32.const 0)
        )
       )
       (i32.ne
        (i32.and
         (get_local $0)
         (i32.const 3)
        )
        (i32.const 0)
       )
      )
      (block
       (set_local $5
        (i32.and
         (get_local $1)
         (i32.const 255)
        )
       )
       (loop $while-in
        (br_if $__rjti$2
         (i32.eq
          (i32.load8_u
           (get_local $0)
          )
          (get_local $5)
         )
        )
        (br_if $while-in
         (i32.and
          (tee_local $3
           (i32.ne
            (tee_local $2
             (i32.add
              (get_local $2)
              (i32.const -1)
             )
            )
            (i32.const 0)
           )
          )
          (i32.ne
           (i32.and
            (tee_local $0
             (i32.add
              (get_local $0)
              (i32.const 1)
             )
            )
            (i32.const 3)
           )
           (i32.const 0)
          )
         )
        )
       )
      )
     )
     (br_if $__rjti$3
      (i32.eqz
       (get_local $3)
      )
     )
    )
    (if
     (i32.eq
      (i32.load8_u
       (get_local $0)
      )
      (tee_local $1
       (i32.and
        (get_local $1)
        (i32.const 255)
       )
      )
     )
     (if
      (get_local $2)
      (br $__rjto$3)
      (br $__rjti$3)
     )
    )
    (set_local $3
     (i32.mul
      (get_local $4)
      (i32.const 16843009)
     )
    )
    (block $__rjto$0
     (if
      (i32.gt_u
       (get_local $2)
       (i32.const 3)
      )
      (loop $while-in3
       (br_if $__rjto$0
        (i32.and
         (i32.xor
          (i32.and
           (tee_local $4
            (i32.xor
             (i32.load
              (get_local $0)
             )
             (get_local $3)
            )
           )
           (i32.const -2139062144)
          )
          (i32.const -2139062144)
         )
         (i32.add
          (get_local $4)
          (i32.const -16843009)
         )
        )
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 4)
        )
       )
       (br_if $while-in3
        (i32.gt_u
         (tee_local $2
          (i32.add
           (get_local $2)
           (i32.const -4)
          )
         )
         (i32.const 3)
        )
       )
      )
     )
     (br_if $__rjti$3
      (i32.eqz
       (get_local $2)
      )
     )
    )
    (loop $while-in5
     (br_if $__rjto$3
      (i32.eq
       (i32.load8_u
        (get_local $0)
       )
       (i32.and
        (get_local $1)
        (i32.const 255)
       )
      )
     )
     (set_local $0
      (i32.add
       (get_local $0)
       (i32.const 1)
      )
     )
     (br_if $while-in5
      (tee_local $2
       (i32.add
        (get_local $2)
        (i32.const -1)
       )
      )
     )
    )
   )
   (set_local $0
    (i32.const 0)
   )
  )
  (get_local $0)
 )
 (func $_isdigit (; 36 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (i32.lt_u
   (i32.add
    (get_local $0)
    (i32.const -48)
   )
   (i32.const 10)
  )
 )
 (func $_strcpy (; 37 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (drop
   (call $___stpcpy
    (get_local $0)
    (get_local $1)
   )
  )
  (get_local $0)
 )
 (func $___stpcpy (; 38 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (block $__rjto$0
   (if
    (i32.eqz
     (i32.and
      (i32.xor
       (get_local $1)
       (get_local $0)
      )
      (i32.const 3)
     )
    )
    (block
     (if
      (i32.and
       (get_local $1)
       (i32.const 3)
      )
      (loop $while-in
       (i32.store8
        (get_local $0)
        (tee_local $2
         (i32.load8_s
          (get_local $1)
         )
        )
       )
       (br_if $__rjto$0
        (i32.eqz
         (get_local $2)
        )
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 1)
        )
       )
       (br_if $while-in
        (i32.and
         (tee_local $1
          (i32.add
           (get_local $1)
           (i32.const 1)
          )
         )
         (i32.const 3)
        )
       )
      )
     )
     (if
      (i32.eqz
       (i32.and
        (i32.xor
         (i32.and
          (tee_local $2
           (i32.load
            (get_local $1)
           )
          )
          (i32.const -2139062144)
         )
         (i32.const -2139062144)
        )
        (i32.add
         (get_local $2)
         (i32.const -16843009)
        )
       )
      )
      (loop $while-in1
       (set_local $3
        (i32.add
         (get_local $0)
         (i32.const 4)
        )
       )
       (i32.store
        (get_local $0)
        (get_local $2)
       )
       (set_local $0
        (if (result i32)
         (i32.and
          (i32.xor
           (i32.and
            (tee_local $2
             (i32.load
              (tee_local $1
               (i32.add
                (get_local $1)
                (i32.const 4)
               )
              )
             )
            )
            (i32.const -2139062144)
           )
           (i32.const -2139062144)
          )
          (i32.add
           (get_local $2)
           (i32.const -16843009)
          )
         )
         (get_local $3)
         (block
          (set_local $0
           (get_local $3)
          )
          (br $while-in1)
         )
        )
       )
      )
     )
    )
   )
   (i32.store8
    (get_local $0)
    (tee_local $2
     (i32.load8_s
      (get_local $1)
     )
    )
   )
   (if
    (get_local $2)
    (loop $while-in3
     (i32.store8
      (tee_local $0
       (i32.add
        (get_local $0)
        (i32.const 1)
       )
      )
      (tee_local $2
       (i32.load8_s
        (tee_local $1
         (i32.add
          (get_local $1)
          (i32.const 1)
         )
        )
       )
      )
     )
     (br_if $while-in3
      (get_local $2)
     )
    )
   )
  )
  (get_local $0)
 )
 (func $_strerror (; 39 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (call $___strerror_l
   (get_local $0)
   (i32.load offset=188
    (call $___pthread_self_623)
   )
  )
 )
 (func $___pthread_self_623 (; 40 ;) (; has Stack IR ;) (result i32)
  (call $_pthread_self)
 )
 (func $___strerror_l (; 41 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (block $__rjto$1
   (block $__rjti$1
    (block $__rjti$0
     (loop $while-in
      (block $while-out
       (br_if $__rjti$0
        (i32.eq
         (i32.load8_u
          (i32.add
           (get_local $2)
           (i32.const 1024)
          )
         )
         (get_local $0)
        )
       )
       (br_if $while-in
        (i32.ne
         (tee_local $2
          (i32.add
           (get_local $2)
           (i32.const 1)
          )
         )
         (i32.const 87)
        )
       )
       (set_local $2
        (i32.const 87)
       )
       (br $__rjti$1)
      )
     )
    )
    (br_if $__rjti$1
     (get_local $2)
    )
    (set_local $0
     (i32.const 1120)
    )
    (br $__rjto$1)
   )
   (set_local $0
    (i32.const 1120)
   )
   (loop $while-in1
    (set_local $3
     (get_local $0)
    )
    (loop $while-in3
     (set_local $0
      (i32.add
       (get_local $3)
       (i32.const 1)
      )
     )
     (if
      (i32.load8_s
       (get_local $3)
      )
      (block
       (set_local $3
        (get_local $0)
       )
       (br $while-in3)
      )
     )
    )
    (br_if $while-in1
     (tee_local $2
      (i32.add
       (get_local $2)
       (i32.const -1)
      )
     )
    )
   )
  )
  (call $___lctrans
   (get_local $0)
   (i32.load offset=20
    (get_local $1)
   )
  )
 )
 (func $_wctomb (; 42 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (if (result i32)
   (get_local $0)
   (call $_wcrtomb
    (get_local $0)
    (get_local $1)
    (i32.const 0)
   )
   (i32.const 0)
  )
 )
 (func $_wcrtomb (; 43 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (block $do-once (result i32)
   (if (result i32)
    (get_local $0)
    (block (result i32)
     (if
      (i32.lt_u
       (get_local $1)
       (i32.const 128)
      )
      (block
       (i32.store8
        (get_local $0)
        (get_local $1)
       )
       (br $do-once
        (i32.const 1)
       )
      )
     )
     (if
      (i32.eqz
       (i32.load
        (i32.load offset=188
         (call $___pthread_self_623)
        )
       )
      )
      (if
       (i32.eq
        (i32.and
         (get_local $1)
         (i32.const -128)
        )
        (i32.const 57216)
       )
       (block
        (i32.store8
         (get_local $0)
         (get_local $1)
        )
        (br $do-once
         (i32.const 1)
        )
       )
       (block
        (i32.store
         (call $___errno_location)
         (i32.const 84)
        )
        (br $do-once
         (i32.const -1)
        )
       )
      )
     )
     (if
      (i32.lt_u
       (get_local $1)
       (i32.const 2048)
      )
      (block
       (i32.store8
        (get_local $0)
        (i32.or
         (i32.shr_u
          (get_local $1)
          (i32.const 6)
         )
         (i32.const 192)
        )
       )
       (i32.store8 offset=1
        (get_local $0)
        (i32.or
         (i32.and
          (get_local $1)
          (i32.const 63)
         )
         (i32.const 128)
        )
       )
       (br $do-once
        (i32.const 2)
       )
      )
     )
     (if
      (i32.or
       (i32.lt_u
        (get_local $1)
        (i32.const 55296)
       )
       (i32.eq
        (i32.and
         (get_local $1)
         (i32.const -8192)
        )
        (i32.const 57344)
       )
      )
      (block
       (i32.store8
        (get_local $0)
        (i32.or
         (i32.shr_u
          (get_local $1)
          (i32.const 12)
         )
         (i32.const 224)
        )
       )
       (i32.store8 offset=1
        (get_local $0)
        (i32.or
         (i32.and
          (i32.shr_u
           (get_local $1)
           (i32.const 6)
          )
          (i32.const 63)
         )
         (i32.const 128)
        )
       )
       (i32.store8 offset=2
        (get_local $0)
        (i32.or
         (i32.and
          (get_local $1)
          (i32.const 63)
         )
         (i32.const 128)
        )
       )
       (br $do-once
        (i32.const 3)
       )
      )
     )
     (if (result i32)
      (i32.lt_u
       (i32.add
        (get_local $1)
        (i32.const -65536)
       )
       (i32.const 1048576)
      )
      (block (result i32)
       (i32.store8
        (get_local $0)
        (i32.or
         (i32.shr_u
          (get_local $1)
          (i32.const 18)
         )
         (i32.const 240)
        )
       )
       (i32.store8 offset=1
        (get_local $0)
        (i32.or
         (i32.and
          (i32.shr_u
           (get_local $1)
           (i32.const 12)
          )
          (i32.const 63)
         )
         (i32.const 128)
        )
       )
       (i32.store8 offset=2
        (get_local $0)
        (i32.or
         (i32.and
          (i32.shr_u
           (get_local $1)
           (i32.const 6)
          )
          (i32.const 63)
         )
         (i32.const 128)
        )
       )
       (i32.store8 offset=3
        (get_local $0)
        (i32.or
         (i32.and
          (get_local $1)
          (i32.const 63)
         )
         (i32.const 128)
        )
       )
       (i32.const 4)
      )
      (block (result i32)
       (i32.store
        (call $___errno_location)
        (i32.const 84)
       )
       (i32.const -1)
      )
     )
    )
    (i32.const 1)
   )
  )
 )
 (func $_vsnprintf (; 44 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (set_local $4
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 128)
   )
  )
  (set_local $5
   (i32.add
    (get_local $4)
    (i32.const 124)
   )
  )
  (i64.store align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3780)
   )
  )
  (i64.store offset=8 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3788)
   )
  )
  (i64.store offset=16 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3796)
   )
  )
  (i64.store offset=24 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3804)
   )
  )
  (i64.store offset=32 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3812)
   )
  )
  (i64.store offset=40 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3820)
   )
  )
  (i64.store offset=48 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3828)
   )
  )
  (i64.store offset=56 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3836)
   )
  )
  (i64.store align=4
   (i32.sub
    (get_local $4)
    (i32.const -64)
   )
   (i64.load align=4
    (i32.const 3844)
   )
  )
  (i64.store offset=72 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3852)
   )
  )
  (i64.store offset=80 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3860)
   )
  )
  (i64.store offset=88 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3868)
   )
  )
  (i64.store offset=96 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3876)
   )
  )
  (i64.store offset=104 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3884)
   )
  )
  (i64.store offset=112 align=4
   (get_local $4)
   (i64.load align=4
    (i32.const 3892)
   )
  )
  (i32.store offset=120
   (get_local $4)
   (i32.load
    (i32.const 3900)
   )
  )
  (block $__rjto$0
   (block $__rjti$0
    (br_if $__rjti$0
     (i32.le_u
      (i32.add
       (get_local $1)
       (i32.const -1)
      )
      (i32.const 2147483646)
     )
    )
    (set_local $0
     (if (result i32)
      (get_local $1)
      (block (result i32)
       (i32.store
        (call $___errno_location)
        (i32.const 75)
       )
       (i32.const -1)
      )
      (block
       (set_local $0
        (get_local $5)
       )
       (set_local $1
        (i32.const 1)
       )
       (br $__rjti$0)
      )
     )
    )
    (br $__rjto$0)
   )
   (i32.store offset=48
    (get_local $4)
    (tee_local $1
     (select
      (tee_local $5
       (i32.sub
        (i32.const -2)
        (get_local $0)
       )
      )
      (get_local $1)
      (i32.gt_u
       (get_local $1)
       (get_local $5)
      )
     )
    )
   )
   (i32.store
    (tee_local $5
     (i32.add
      (get_local $4)
      (i32.const 20)
     )
    )
    (get_local $0)
   )
   (i32.store offset=44
    (get_local $4)
    (get_local $0)
   )
   (i32.store
    (tee_local $6
     (i32.add
      (get_local $4)
      (i32.const 16)
     )
    )
    (tee_local $0
     (i32.add
      (get_local $0)
      (get_local $1)
     )
    )
   )
   (i32.store offset=28
    (get_local $4)
    (get_local $0)
   )
   (set_local $0
    (call $_vfprintf
     (get_local $4)
     (get_local $2)
     (get_local $3)
    )
   )
   (if
    (get_local $1)
    (i32.store8
     (i32.add
      (tee_local $1
       (i32.load
        (get_local $5)
       )
      )
      (i32.shr_s
       (i32.shl
        (i32.eq
         (get_local $1)
         (i32.load
          (get_local $6)
         )
        )
        (i32.const 31)
       )
       (i32.const 31)
      )
     )
     (i32.const 0)
    )
   )
  )
  (set_global $STACKTOP
   (get_local $4)
  )
  (get_local $0)
 )
 (func $_vfprintf (; 45 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
  (set_local $3
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 224)
   )
  )
  (i64.store
   (tee_local $4
    (i32.add
     (get_local $3)
     (i32.const 160)
    )
   )
   (i64.const 0)
  )
  (i64.store offset=8
   (get_local $4)
   (i64.const 0)
  )
  (i64.store offset=16
   (get_local $4)
   (i64.const 0)
  )
  (i64.store offset=24
   (get_local $4)
   (i64.const 0)
  )
  (i64.store offset=32
   (get_local $4)
   (i64.const 0)
  )
  (i32.store
   (tee_local $5
    (i32.add
     (get_local $3)
     (i32.const 208)
    )
   )
   (i32.load
    (get_local $2)
   )
  )
  (set_local $0
   (if (result i32)
    (i32.lt_s
     (call $_printf_core
      (i32.const 0)
      (get_local $1)
      (get_local $5)
      (tee_local $2
       (i32.add
        (get_local $3)
        (i32.const 80)
       )
      )
      (get_local $4)
     )
     (i32.const 0)
    )
    (i32.const -1)
    (block (result i32)
     (set_local $11
      (if (result i32)
       (i32.gt_s
        (i32.load offset=76
         (get_local $0)
        )
        (i32.const -1)
       )
       (call $___lockfile
        (get_local $0)
       )
       (i32.const 0)
      )
     )
     (set_local $6
      (i32.load
       (get_local $0)
      )
     )
     (if
      (i32.lt_s
       (i32.load8_s offset=74
        (get_local $0)
       )
       (i32.const 1)
      )
      (i32.store
       (get_local $0)
       (i32.and
        (get_local $6)
        (i32.const -33)
       )
      )
     )
     (if
      (i32.load
       (tee_local $7
        (i32.add
         (get_local $0)
         (i32.const 48)
        )
       )
      )
      (set_local $1
       (call $_printf_core
        (get_local $0)
        (get_local $1)
        (get_local $5)
        (get_local $2)
        (get_local $4)
       )
      )
      (block
       (set_local $9
        (i32.load
         (tee_local $8
          (i32.add
           (get_local $0)
           (i32.const 44)
          )
         )
        )
       )
       (i32.store
        (get_local $8)
        (get_local $3)
       )
       (i32.store
        (tee_local $12
         (i32.add
          (get_local $0)
          (i32.const 28)
         )
        )
        (get_local $3)
       )
       (i32.store
        (tee_local $10
         (i32.add
          (get_local $0)
          (i32.const 20)
         )
        )
        (get_local $3)
       )
       (i32.store
        (get_local $7)
        (i32.const 80)
       )
       (i32.store
        (tee_local $13
         (i32.add
          (get_local $0)
          (i32.const 16)
         )
        )
        (i32.add
         (get_local $3)
         (i32.const 80)
        )
       )
       (set_local $1
        (call $_printf_core
         (get_local $0)
         (get_local $1)
         (get_local $5)
         (get_local $2)
         (get_local $4)
        )
       )
       (if
        (get_local $9)
        (block
         (drop
          (call_indirect (type $FUNCSIG$iiii)
           (get_local $0)
           (i32.const 0)
           (i32.const 0)
           (i32.add
            (i32.and
             (i32.load offset=36
              (get_local $0)
             )
             (i32.const 3)
            )
            (i32.const 2)
           )
          )
         )
         (set_local $1
          (select
           (get_local $1)
           (i32.const -1)
           (i32.load
            (get_local $10)
           )
          )
         )
         (i32.store
          (get_local $8)
          (get_local $9)
         )
         (i32.store
          (get_local $7)
          (i32.const 0)
         )
         (i32.store
          (get_local $13)
          (i32.const 0)
         )
         (i32.store
          (get_local $12)
          (i32.const 0)
         )
         (i32.store
          (get_local $10)
          (i32.const 0)
         )
        )
       )
      )
     )
     (i32.store
      (get_local $0)
      (i32.or
       (tee_local $2
        (i32.load
         (get_local $0)
        )
       )
       (i32.and
        (get_local $6)
        (i32.const 32)
       )
      )
     )
     (if
      (get_local $11)
      (call $___unlockfile
       (get_local $0)
      )
     )
     (select
      (i32.const -1)
      (get_local $1)
      (i32.and
       (get_local $2)
       (i32.const 32)
      )
     )
    )
   )
  )
  (set_global $STACKTOP
   (get_local $3)
  )
  (get_local $0)
 )
 (func $_printf_core (; 46 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32) (result i32)
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
  (local $17 i64)
  (local $18 i32)
  (local $19 i32)
  (local $20 i32)
  (local $21 i32)
  (local $22 i32)
  (local $23 i32)
  (local $24 i32)
  (local $25 i32)
  (local $26 i32)
  (set_local $18
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.sub
    (get_global $STACKTOP)
    (i32.const -64)
   )
  )
  (set_local $11
   (i32.add
    (get_local $18)
    (i32.const 40)
   )
  )
  (set_local $23
   (i32.add
    (get_local $18)
    (i32.const 60)
   )
  )
  (i32.store
   (tee_local $12
    (i32.add
     (get_local $18)
     (i32.const 56)
    )
   )
   (get_local $1)
  )
  (set_local $20
   (i32.ne
    (get_local $0)
    (i32.const 0)
   )
  )
  (set_local $21
   (tee_local $22
    (i32.add
     (get_local $18)
     (i32.const 40)
    )
   )
  )
  (set_local $24
   (i32.add
    (get_local $18)
    (i32.const 39)
   )
  )
  (set_local $26
   (i32.add
    (tee_local $25
     (i32.add
      (get_local $18)
      (i32.const 48)
     )
    )
    (i32.const 4)
   )
  )
  (set_local $1
   (i32.const 0)
  )
  (block $label$break$L125
   (block $__rjti$11
    (loop $label$continue$L1
     (block $label$break$L1
      (loop $while-in
       (if
        (i32.gt_s
         (get_local $9)
         (i32.const -1)
        )
        (set_local $9
         (if (result i32)
          (i32.gt_s
           (get_local $1)
           (i32.sub
            (i32.const 2147483647)
            (get_local $9)
           )
          )
          (block (result i32)
           (i32.store
            (call $___errno_location)
            (i32.const 75)
           )
           (i32.const -1)
          )
          (i32.add
           (get_local $1)
           (get_local $9)
          )
         )
        )
       )
       (br_if $__rjti$11
        (i32.eqz
         (tee_local $8
          (i32.load8_s
           (tee_local $10
            (i32.load
             (get_local $12)
            )
           )
          )
         )
        )
       )
       (set_local $1
        (get_local $10)
       )
       (block $label$break$L15
        (block $__rjti$0
         (loop $label$continue$L12
          (block $label$break$L12
           (block $switch
            (block $switch-case0
             (br_if $label$break$L12
              (i32.eqz
               (tee_local $8
                (i32.shr_s
                 (i32.shl
                  (get_local $8)
                  (i32.const 24)
                 )
                 (i32.const 24)
                )
               )
              )
             )
             (br_if $switch
              (i32.ne
               (get_local $8)
               (i32.const 37)
              )
             )
             (br $__rjti$0)
            )
           )
           (i32.store
            (get_local $12)
            (tee_local $1
             (i32.add
              (get_local $1)
              (i32.const 1)
             )
            )
           )
           (set_local $8
            (i32.load8_s
             (get_local $1)
            )
           )
           (br $label$continue$L12)
          )
         )
         (br $label$break$L15)
        )
        (set_local $8
         (get_local $1)
        )
        (loop $while-in3
         (br_if $label$break$L15
          (i32.ne
           (i32.load8_s offset=1
            (get_local $8)
           )
           (i32.const 37)
          )
         )
         (set_local $1
          (i32.add
           (get_local $1)
           (i32.const 1)
          )
         )
         (i32.store
          (get_local $12)
          (tee_local $8
           (i32.add
            (get_local $8)
            (i32.const 2)
           )
          )
         )
         (br_if $while-in3
          (i32.eq
           (i32.load8_s
            (get_local $8)
           )
           (i32.const 37)
          )
         )
        )
       )
       (set_local $1
        (i32.sub
         (get_local $1)
         (get_local $10)
        )
       )
       (if
        (get_local $20)
        (call $_out
         (get_local $0)
         (get_local $10)
         (get_local $1)
        )
       )
       (br_if $while-in
        (get_local $1)
       )
      )
      (set_local $8
       (i32.eqz
        (call $_isdigit
         (i32.load8_s offset=1
          (i32.load
           (get_local $12)
          )
         )
        )
       )
      )
      (i32.store
       (get_local $12)
       (tee_local $1
        (i32.add
         (tee_local $1
          (i32.load
           (get_local $12)
          )
         )
         (tee_local $8
          (if (result i32)
           (get_local $8)
           (block (result i32)
            (set_local $14
             (i32.const -1)
            )
            (i32.const 1)
           )
           (if (result i32)
            (i32.eq
             (i32.load8_s offset=2
              (get_local $1)
             )
             (i32.const 36)
            )
            (block (result i32)
             (set_local $14
              (i32.add
               (i32.load8_s offset=1
                (get_local $1)
               )
               (i32.const -48)
              )
             )
             (set_local $5
              (i32.const 1)
             )
             (i32.const 3)
            )
            (block (result i32)
             (set_local $14
              (i32.const -1)
             )
             (i32.const 1)
            )
           )
          )
         )
        )
       )
      )
      (if
       (i32.or
        (i32.gt_u
         (tee_local $8
          (i32.add
           (tee_local $6
            (i32.load8_s
             (get_local $1)
            )
           )
           (i32.const -32)
          )
         )
         (i32.const 31)
        )
        (i32.eqz
         (i32.and
          (i32.shl
           (i32.const 1)
           (get_local $8)
          )
          (i32.const 75913)
         )
        )
       )
       (set_local $8
        (i32.const 0)
       )
       (block
        (set_local $6
         (i32.const 0)
        )
        (loop $while-in5
         (set_local $8
          (i32.or
           (i32.shl
            (i32.const 1)
            (get_local $8)
           )
           (get_local $6)
          )
         )
         (i32.store
          (get_local $12)
          (tee_local $1
           (i32.add
            (get_local $1)
            (i32.const 1)
           )
          )
         )
         (if
          (i32.eqz
           (i32.or
            (i32.gt_u
             (tee_local $7
              (i32.add
               (tee_local $6
                (i32.load8_s
                 (get_local $1)
                )
               )
               (i32.const -32)
              )
             )
             (i32.const 31)
            )
            (i32.eqz
             (i32.and
              (i32.shl
               (i32.const 1)
               (get_local $7)
              )
              (i32.const 75913)
             )
            )
           )
          )
          (block
           (set_local $6
            (get_local $8)
           )
           (set_local $8
            (get_local $7)
           )
           (br $while-in5)
          )
         )
        )
       )
      )
      (if
       (i32.eq
        (i32.and
         (get_local $6)
         (i32.const 255)
        )
        (i32.const 42)
       )
       (block
        (set_local $5
         (block $__rjto$1 (result i32)
          (block $__rjti$1
           (br_if $__rjti$1
            (i32.eqz
             (call $_isdigit
              (i32.load8_s offset=1
               (get_local $1)
              )
             )
            )
           )
           (br_if $__rjti$1
            (i32.ne
             (i32.load8_s offset=2
              (tee_local $7
               (i32.load
                (get_local $12)
               )
              )
             )
             (i32.const 36)
            )
           )
           (i32.store
            (i32.add
             (get_local $4)
             (i32.shl
              (i32.add
               (i32.load8_s
                (tee_local $1
                 (i32.add
                  (get_local $7)
                  (i32.const 1)
                 )
                )
               )
               (i32.const -48)
              )
              (i32.const 2)
             )
            )
            (i32.const 10)
           )
           (set_local $1
            (i32.wrap/i64
             (i64.load
              (i32.add
               (get_local $3)
               (i32.shl
                (i32.add
                 (i32.load8_s
                  (get_local $1)
                 )
                 (i32.const -48)
                )
                (i32.const 3)
               )
              )
             )
            )
           )
           (set_local $6
            (i32.const 1)
           )
           (br $__rjto$1
            (i32.add
             (get_local $7)
             (i32.const 3)
            )
           )
          )
          (if
           (get_local $5)
           (block
            (set_local $9
             (i32.const -1)
            )
            (br $label$break$L1)
           )
          )
          (if
           (get_local $20)
           (block
            (set_local $1
             (i32.load
              (tee_local $5
               (i32.and
                (i32.add
                 (i32.load
                  (get_local $2)
                 )
                 (i32.const 3)
                )
                (i32.const -4)
               )
              )
             )
            )
            (i32.store
             (get_local $2)
             (i32.add
              (get_local $5)
              (i32.const 4)
             )
            )
           )
           (set_local $1
            (i32.const 0)
           )
          )
          (set_local $6
           (i32.const 0)
          )
          (i32.add
           (i32.load
            (get_local $12)
           )
           (i32.const 1)
          )
         )
        )
        (i32.store
         (get_local $12)
         (get_local $5)
        )
        (set_local $15
         (select
          (i32.sub
           (i32.const 0)
           (get_local $1)
          )
          (get_local $1)
          (tee_local $1
           (i32.lt_s
            (get_local $1)
            (i32.const 0)
           )
          )
         )
        )
        (set_local $16
         (select
          (i32.or
           (get_local $8)
           (i32.const 8192)
          )
          (get_local $8)
          (get_local $1)
         )
        )
        (set_local $8
         (get_local $6)
        )
       )
       (block
        (if
         (i32.lt_s
          (tee_local $15
           (call $_getint
            (get_local $12)
           )
          )
          (i32.const 0)
         )
         (block
          (set_local $9
           (i32.const -1)
          )
          (br $label$break$L1)
         )
        )
        (set_local $16
         (get_local $8)
        )
        (set_local $8
         (get_local $5)
        )
        (set_local $5
         (i32.load
          (get_local $12)
         )
        )
       )
      )
      (block $do-once6
       (if
        (i32.eq
         (i32.load8_s
          (get_local $5)
         )
         (i32.const 46)
        )
        (block
         (if
          (i32.ne
           (i32.load8_s
            (tee_local $1
             (i32.add
              (get_local $5)
              (i32.const 1)
             )
            )
           )
           (i32.const 42)
          )
          (block
           (i32.store
            (get_local $12)
            (get_local $1)
           )
           (set_local $1
            (call $_getint
             (get_local $12)
            )
           )
           (set_local $5
            (i32.load
             (get_local $12)
            )
           )
           (br $do-once6)
          )
         )
         (if
          (call $_isdigit
           (i32.load8_s offset=2
            (get_local $5)
           )
          )
          (if
           (i32.eq
            (i32.load8_s offset=3
             (tee_local $5
              (i32.load
               (get_local $12)
              )
             )
            )
            (i32.const 36)
           )
           (block
            (i32.store
             (i32.add
              (get_local $4)
              (i32.shl
               (i32.add
                (i32.load8_s
                 (tee_local $1
                  (i32.add
                   (get_local $5)
                   (i32.const 2)
                  )
                 )
                )
                (i32.const -48)
               )
               (i32.const 2)
              )
             )
             (i32.const 10)
            )
            (set_local $1
             (i32.wrap/i64
              (i64.load
               (i32.add
                (get_local $3)
                (i32.shl
                 (i32.add
                  (i32.load8_s
                   (get_local $1)
                  )
                  (i32.const -48)
                 )
                 (i32.const 3)
                )
               )
              )
             )
            )
            (i32.store
             (get_local $12)
             (tee_local $5
              (i32.add
               (get_local $5)
               (i32.const 4)
              )
             )
            )
            (br $do-once6)
           )
          )
         )
         (if
          (get_local $8)
          (block
           (set_local $9
            (i32.const -1)
           )
           (br $label$break$L1)
          )
         )
         (if
          (get_local $20)
          (block
           (set_local $1
            (i32.load
             (tee_local $5
              (i32.and
               (i32.add
                (i32.load
                 (get_local $2)
                )
                (i32.const 3)
               )
               (i32.const -4)
              )
             )
            )
           )
           (i32.store
            (get_local $2)
            (i32.add
             (get_local $5)
             (i32.const 4)
            )
           )
          )
          (set_local $1
           (i32.const 0)
          )
         )
         (i32.store
          (get_local $12)
          (tee_local $5
           (i32.add
            (i32.load
             (get_local $12)
            )
            (i32.const 2)
           )
          )
         )
        )
        (set_local $1
         (i32.const -1)
        )
       )
      )
      (set_local $13
       (i32.const 0)
      )
      (loop $while-in9
       (if
        (i32.gt_u
         (i32.add
          (i32.load8_s
           (get_local $5)
          )
          (i32.const -65)
         )
         (i32.const 57)
        )
        (block
         (set_local $9
          (i32.const -1)
         )
         (br $label$break$L1)
        )
       )
       (i32.store
        (get_local $12)
        (tee_local $6
         (i32.add
          (get_local $5)
          (i32.const 1)
         )
        )
       )
       (if
        (i32.lt_u
         (i32.add
          (tee_local $5
           (i32.and
            (tee_local $7
             (i32.load8_s
              (i32.add
               (i32.add
                (i32.mul
                 (get_local $13)
                 (i32.const 58)
                )
                (i32.load8_s
                 (get_local $5)
                )
               )
               (i32.const 2863)
              )
             )
            )
            (i32.const 255)
           )
          )
          (i32.const -1)
         )
         (i32.const 8)
        )
        (block
         (set_local $13
          (get_local $5)
         )
         (set_local $5
          (get_local $6)
         )
         (br $while-in9)
        )
       )
      )
      (if
       (i32.eqz
        (get_local $7)
       )
       (block
        (set_local $9
         (i32.const -1)
        )
        (br $label$break$L1)
       )
      )
      (set_local $19
       (i32.gt_s
        (get_local $14)
        (i32.const -1)
       )
      )
      (block $label$break$L77
       (block $__rjti$10
        (block $__rjti$9
         (if
          (i32.eq
           (get_local $7)
           (i32.const 19)
          )
          (if
           (get_local $19)
           (block
            (set_local $9
             (i32.const -1)
            )
            (br $label$break$L1)
           )
          )
          (block
           (if
            (get_local $19)
            (block
             (i32.store
              (i32.add
               (get_local $4)
               (i32.shl
                (get_local $14)
                (i32.const 2)
               )
              )
              (get_local $5)
             )
             (i64.store
              (get_local $11)
              (i64.load
               (i32.add
                (get_local $3)
                (i32.shl
                 (get_local $14)
                 (i32.const 3)
                )
               )
              )
             )
             (br $__rjti$9)
            )
           )
           (if
            (i32.eqz
             (get_local $20)
            )
            (block
             (set_local $9
              (i32.const 0)
             )
             (br $label$break$L1)
            )
           )
           (call $_pop_arg
            (get_local $11)
            (get_local $5)
            (get_local $2)
           )
           (set_local $6
            (i32.load
             (get_local $12)
            )
           )
           (br $__rjti$10)
          )
         )
        )
        (br_if $__rjti$10
         (get_local $20)
        )
        (set_local $1
         (i32.const 0)
        )
        (br $label$break$L77)
       )
       (set_local $5
        (select
         (tee_local $7
          (i32.and
           (get_local $16)
           (i32.const -65537)
          )
         )
         (get_local $16)
         (i32.and
          (get_local $16)
          (i32.const 8192)
         )
        )
       )
       (block $__rjto$8
        (block $__rjti$8
         (block $__rjti$7
          (block $__rjti$6
           (block $__rjti$5
            (block $__rjti$4
             (block $__rjti$3
              (block $switch-default45
               (block $switch-case37
                (block $switch-case36
                 (block $switch-case35
                  (block $switch-case34
                   (block $switch-case33
                    (block $switch-case32
                     (block $switch-case31
                      (block $switch-case29
                       (block $switch-case28
                        (block $switch-case26
                         (block $switch-case25
                          (block $switch-case24
                           (br_table $switch-case37 $switch-default45 $switch-case35 $switch-default45 $switch-case37 $switch-case37 $switch-case37 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-case36 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-case26 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-default45 $switch-case37 $switch-default45 $switch-case32 $switch-case29 $switch-case37 $switch-case37 $switch-case37 $switch-default45 $switch-case29 $switch-default45 $switch-default45 $switch-default45 $switch-case33 $switch-case24 $switch-case28 $switch-case25 $switch-default45 $switch-default45 $switch-case34 $switch-default45 $switch-case31 $switch-default45 $switch-default45 $switch-case26 $switch-default45
                            (i32.sub
                             (tee_local $6
                              (select
                               (i32.and
                                (tee_local $6
                                 (i32.load8_s
                                  (i32.add
                                   (get_local $6)
                                   (i32.const -1)
                                  )
                                 )
                                )
                                (i32.const -33)
                               )
                               (get_local $6)
                               (i32.and
                                (i32.ne
                                 (get_local $13)
                                 (i32.const 0)
                                )
                                (i32.eq
                                 (i32.and
                                  (get_local $6)
                                  (i32.const 15)
                                 )
                                 (i32.const 3)
                                )
                               )
                              )
                             )
                             (i32.const 65)
                            )
                           )
                          )
                          (block $switch15
                           (block $switch-default23
                            (block $switch-case22
                             (block $switch-case21
                              (block $switch-case20
                               (block $switch-case19
                                (block $switch-case18
                                 (block $switch-case17
                                  (block $switch-case16
                                   (br_table $switch-case16 $switch-case17 $switch-case18 $switch-case19 $switch-case20 $switch-default23 $switch-case21 $switch-case22 $switch-default23
                                    (i32.shr_s
                                     (i32.shl
                                      (i32.and
                                       (get_local $13)
                                       (i32.const 255)
                                      )
                                      (i32.const 24)
                                     )
                                     (i32.const 24)
                                    )
                                   )
                                  )
                                  (i32.store
                                   (i32.load
                                    (get_local $11)
                                   )
                                   (get_local $9)
                                  )
                                  (set_local $1
                                   (i32.const 0)
                                  )
                                  (br $label$break$L77)
                                 )
                                 (i32.store
                                  (i32.load
                                   (get_local $11)
                                  )
                                  (get_local $9)
                                 )
                                 (set_local $1
                                  (i32.const 0)
                                 )
                                 (br $label$break$L77)
                                )
                                (i64.store
                                 (i32.load
                                  (get_local $11)
                                 )
                                 (i64.extend_s/i32
                                  (get_local $9)
                                 )
                                )
                                (set_local $1
                                 (i32.const 0)
                                )
                                (br $label$break$L77)
                               )
                               (i32.store16
                                (i32.load
                                 (get_local $11)
                                )
                                (get_local $9)
                               )
                               (set_local $1
                                (i32.const 0)
                               )
                               (br $label$break$L77)
                              )
                              (i32.store8
                               (i32.load
                                (get_local $11)
                               )
                               (get_local $9)
                              )
                              (set_local $1
                               (i32.const 0)
                              )
                              (br $label$break$L77)
                             )
                             (i32.store
                              (i32.load
                               (get_local $11)
                              )
                              (get_local $9)
                             )
                             (set_local $1
                              (i32.const 0)
                             )
                             (br $label$break$L77)
                            )
                            (i64.store
                             (i32.load
                              (get_local $11)
                             )
                             (i64.extend_s/i32
                              (get_local $9)
                             )
                            )
                            (set_local $1
                             (i32.const 0)
                            )
                            (br $label$break$L77)
                           )
                           (set_local $1
                            (i32.const 0)
                           )
                           (br $label$break$L77)
                          )
                         )
                         (set_local $6
                          (i32.const 120)
                         )
                         (set_local $1
                          (select
                           (get_local $1)
                           (i32.const 8)
                           (i32.gt_u
                            (get_local $1)
                            (i32.const 8)
                           )
                          )
                         )
                         (set_local $5
                          (i32.or
                           (get_local $5)
                           (i32.const 8)
                          )
                         )
                         (br $__rjti$3)
                        )
                        (br $__rjti$3)
                       )
                       (set_local $10
                        (i32.const 0)
                       )
                       (set_local $7
                        (i32.const 3935)
                       )
                       (set_local $1
                        (select
                         (get_local $1)
                         (i32.add
                          (tee_local $6
                           (i32.sub
                            (get_local $21)
                            (tee_local $13
                             (call $_fmt_o
                              (tee_local $17
                               (i64.load
                                (get_local $11)
                               )
                              )
                              (get_local $22)
                             )
                            )
                           )
                          )
                          (i32.const 1)
                         )
                         (i32.or
                          (i32.eqz
                           (i32.and
                            (get_local $5)
                            (i32.const 8)
                           )
                          )
                          (i32.gt_s
                           (get_local $1)
                           (get_local $6)
                          )
                         )
                        )
                       )
                       (br $__rjti$7)
                      )
                      (if
                       (i64.lt_s
                        (tee_local $17
                         (i64.load
                          (get_local $11)
                         )
                        )
                        (i64.const 0)
                       )
                       (block
                        (i64.store
                         (get_local $11)
                         (tee_local $17
                          (i64.sub
                           (i64.const 0)
                           (get_local $17)
                          )
                         )
                        )
                        (set_local $10
                         (i32.const 1)
                        )
                        (set_local $7
                         (i32.const 3935)
                        )
                       )
                       (block
                        (set_local $10
                         (i32.ne
                          (i32.and
                           (get_local $5)
                           (i32.const 2049)
                          )
                          (i32.const 0)
                         )
                        )
                        (set_local $7
                         (select
                          (i32.const 3936)
                          (select
                           (i32.const 3937)
                           (i32.const 3935)
                           (i32.and
                            (get_local $5)
                            (i32.const 1)
                           )
                          )
                          (i32.and
                           (get_local $5)
                           (i32.const 2048)
                          )
                         )
                        )
                       )
                      )
                      (br $__rjti$4)
                     )
                     (set_local $10
                      (i32.const 0)
                     )
                     (set_local $7
                      (i32.const 3935)
                     )
                     (set_local $17
                      (i64.load
                       (get_local $11)
                      )
                     )
                     (br $__rjti$4)
                    )
                    (i64.store8
                     (get_local $24)
                     (i64.load
                      (get_local $11)
                     )
                    )
                    (set_local $6
                     (get_local $24)
                    )
                    (set_local $10
                     (i32.const 0)
                    )
                    (set_local $16
                     (i32.const 3935)
                    )
                    (set_local $13
                     (i32.const 1)
                    )
                    (set_local $5
                     (get_local $7)
                    )
                    (set_local $1
                     (get_local $21)
                    )
                    (br $__rjto$8)
                   )
                   (set_local $6
                    (call $_strerror
                     (i32.load
                      (call $___errno_location)
                     )
                    )
                   )
                   (br $__rjti$5)
                  )
                  (set_local $6
                   (select
                    (tee_local $5
                     (i32.load
                      (get_local $11)
                     )
                    )
                    (i32.const 3945)
                    (get_local $5)
                   )
                  )
                  (br $__rjti$5)
                 )
                 (i64.store32
                  (get_local $25)
                  (i64.load
                   (get_local $11)
                  )
                 )
                 (i32.store
                  (get_local $26)
                  (i32.const 0)
                 )
                 (i32.store
                  (get_local $11)
                  (get_local $25)
                 )
                 (set_local $10
                  (i32.const -1)
                 )
                 (br $__rjti$6)
                )
                (if
                 (get_local $1)
                 (block
                  (set_local $10
                   (get_local $1)
                  )
                  (br $__rjti$6)
                 )
                 (block
                  (call $_pad_666
                   (get_local $0)
                   (i32.const 32)
                   (get_local $15)
                   (i32.const 0)
                   (get_local $5)
                  )
                  (set_local $1
                   (i32.const 0)
                  )
                  (br $__rjti$8)
                 )
                )
               )
               (set_local $1
                (call $_fmt_fp
                 (get_local $0)
                 (f64.load
                  (get_local $11)
                 )
                 (get_local $15)
                 (get_local $1)
                 (get_local $5)
                 (get_local $6)
                )
               )
               (br $label$break$L77)
              )
              (set_local $6
               (get_local $10)
              )
              (set_local $10
               (i32.const 0)
              )
              (set_local $16
               (i32.const 3935)
              )
              (set_local $13
               (get_local $1)
              )
              (set_local $1
               (get_local $21)
              )
              (br $__rjto$8)
             )
             (set_local $7
              (i32.or
               (i32.eqz
                (i32.and
                 (get_local $5)
                 (i32.const 8)
                )
               )
               (i64.eq
                (tee_local $17
                 (i64.load
                  (get_local $11)
                 )
                )
                (i64.const 0)
               )
              )
             )
             (set_local $13
              (call $_fmt_x
               (get_local $17)
               (get_local $22)
               (i32.and
                (get_local $6)
                (i32.const 32)
               )
              )
             )
             (set_local $10
              (select
               (i32.const 0)
               (i32.const 2)
               (get_local $7)
              )
             )
             (set_local $7
              (select
               (i32.const 3935)
               (i32.add
                (i32.shr_u
                 (get_local $6)
                 (i32.const 4)
                )
                (i32.const 3935)
               )
               (get_local $7)
              )
             )
             (br $__rjti$7)
            )
            (set_local $13
             (call $_fmt_u
              (get_local $17)
              (get_local $22)
             )
            )
            (br $__rjti$7)
           )
           (set_local $19
            (i32.eqz
             (tee_local $14
              (call $_memchr
               (get_local $6)
               (i32.const 0)
               (get_local $1)
              )
             )
            )
           )
           (set_local $10
            (i32.const 0)
           )
           (set_local $16
            (i32.const 3935)
           )
           (set_local $13
            (select
             (get_local $1)
             (i32.sub
              (get_local $14)
              (get_local $6)
             )
             (get_local $19)
            )
           )
           (set_local $5
            (get_local $7)
           )
           (set_local $1
            (select
             (i32.add
              (get_local $6)
              (get_local $1)
             )
             (get_local $14)
             (get_local $19)
            )
           )
           (br $__rjto$8)
          )
          (set_local $6
           (i32.load
            (get_local $11)
           )
          )
          (set_local $1
           (i32.const 0)
          )
          (block $__rjto$2
           (block $__rjti$2
            (loop $while-in48
             (if
              (tee_local $7
               (i32.load
                (get_local $6)
               )
              )
              (block
               (br_if $__rjti$2
                (i32.or
                 (tee_local $13
                  (i32.lt_s
                   (tee_local $7
                    (call $_wctomb
                     (get_local $23)
                     (get_local $7)
                    )
                   )
                   (i32.const 0)
                  )
                 )
                 (i32.gt_u
                  (get_local $7)
                  (i32.sub
                   (get_local $10)
                   (get_local $1)
                  )
                 )
                )
               )
               (set_local $6
                (i32.add
                 (get_local $6)
                 (i32.const 4)
                )
               )
               (br_if $while-in48
                (i32.gt_u
                 (get_local $10)
                 (tee_local $1
                  (i32.add
                   (get_local $7)
                   (get_local $1)
                  )
                 )
                )
               )
              )
             )
            )
            (br $__rjto$2)
           )
           (if
            (get_local $13)
            (block
             (set_local $9
              (i32.const -1)
             )
             (br $label$break$L1)
            )
           )
          )
          (call $_pad_666
           (get_local $0)
           (i32.const 32)
           (get_local $15)
           (get_local $1)
           (get_local $5)
          )
          (if
           (get_local $1)
           (block
            (set_local $6
             (i32.load
              (get_local $11)
             )
            )
            (set_local $10
             (i32.const 0)
            )
            (loop $while-in50
             (br_if $__rjti$8
              (i32.eqz
               (tee_local $7
                (i32.load
                 (get_local $6)
                )
               )
              )
             )
             (br_if $__rjti$8
              (i32.gt_s
               (tee_local $10
                (i32.add
                 (tee_local $7
                  (call $_wctomb
                   (get_local $23)
                   (get_local $7)
                  )
                 )
                 (get_local $10)
                )
               )
               (get_local $1)
              )
             )
             (set_local $6
              (i32.add
               (get_local $6)
               (i32.const 4)
              )
             )
             (call $_out
              (get_local $0)
              (get_local $23)
              (get_local $7)
             )
             (br_if $while-in50
              (i32.lt_u
               (get_local $10)
               (get_local $1)
              )
             )
            )
            (br $__rjti$8)
           )
           (block
            (set_local $1
             (i32.const 0)
            )
            (br $__rjti$8)
           )
          )
         )
         (set_local $6
          (select
           (get_local $13)
           (get_local $22)
           (tee_local $19
            (i32.or
             (i32.ne
              (get_local $1)
              (i32.const 0)
             )
             (tee_local $14
              (i64.ne
               (get_local $17)
               (i64.const 0)
              )
             )
            )
           )
          )
         )
         (set_local $16
          (get_local $7)
         )
         (set_local $13
          (select
           (select
            (get_local $1)
            (tee_local $7
             (i32.add
              (i32.sub
               (get_local $21)
               (get_local $13)
              )
              (i32.and
               (i32.xor
                (get_local $14)
                (i32.const 1)
               )
               (i32.const 1)
              )
             )
            )
            (i32.gt_s
             (get_local $1)
             (get_local $7)
            )
           )
           (i32.const 0)
           (get_local $19)
          )
         )
         (set_local $5
          (select
           (i32.and
            (get_local $5)
            (i32.const -65537)
           )
           (get_local $5)
           (i32.gt_s
            (get_local $1)
            (i32.const -1)
           )
          )
         )
         (set_local $1
          (get_local $21)
         )
         (br $__rjto$8)
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 32)
         (get_local $15)
         (get_local $1)
         (i32.xor
          (get_local $5)
          (i32.const 8192)
         )
        )
        (set_local $1
         (select
          (get_local $15)
          (get_local $1)
          (i32.gt_s
           (get_local $15)
           (get_local $1)
          )
         )
        )
        (br $label$break$L77)
       )
       (call $_pad_666
        (get_local $0)
        (i32.const 32)
        (tee_local $1
         (select
          (tee_local $7
           (i32.add
            (tee_local $13
             (select
              (tee_local $14
               (i32.sub
                (get_local $1)
                (get_local $6)
               )
              )
              (get_local $13)
              (i32.lt_s
               (get_local $13)
               (get_local $14)
              )
             )
            )
            (get_local $10)
           )
          )
          (get_local $15)
          (i32.lt_s
           (get_local $15)
           (get_local $7)
          )
         )
        )
        (get_local $7)
        (get_local $5)
       )
       (call $_out
        (get_local $0)
        (get_local $16)
        (get_local $10)
       )
       (call $_pad_666
        (get_local $0)
        (i32.const 48)
        (get_local $1)
        (get_local $7)
        (i32.xor
         (get_local $5)
         (i32.const 65536)
        )
       )
       (call $_pad_666
        (get_local $0)
        (i32.const 48)
        (get_local $13)
        (get_local $14)
        (i32.const 0)
       )
       (call $_out
        (get_local $0)
        (get_local $6)
        (get_local $14)
       )
       (call $_pad_666
        (get_local $0)
        (i32.const 32)
        (get_local $1)
        (get_local $7)
        (i32.xor
         (get_local $5)
         (i32.const 8192)
        )
       )
      )
      (set_local $5
       (get_local $8)
      )
      (br $label$continue$L1)
     )
    )
    (br $label$break$L125)
   )
   (if
    (i32.eqz
     (get_local $0)
    )
    (if
     (get_local $5)
     (block
      (set_local $0
       (i32.const 1)
      )
      (loop $while-in53
       (if
        (tee_local $1
         (i32.load
          (i32.add
           (get_local $4)
           (i32.shl
            (get_local $0)
            (i32.const 2)
           )
          )
         )
        )
        (block
         (call $_pop_arg
          (i32.add
           (get_local $3)
           (i32.shl
            (get_local $0)
            (i32.const 3)
           )
          )
          (get_local $1)
          (get_local $2)
         )
         (br_if $while-in53
          (i32.lt_u
           (tee_local $0
            (i32.add
             (get_local $0)
             (i32.const 1)
            )
           )
           (i32.const 10)
          )
         )
         (set_local $9
          (i32.const 1)
         )
         (br $label$break$L125)
        )
       )
      )
      (loop $while-in55
       (if
        (i32.load
         (i32.add
          (get_local $4)
          (i32.shl
           (get_local $0)
           (i32.const 2)
          )
         )
        )
        (block
         (set_local $9
          (i32.const -1)
         )
         (br $label$break$L125)
        )
       )
       (br_if $while-in55
        (i32.lt_u
         (tee_local $0
          (i32.add
           (get_local $0)
           (i32.const 1)
          )
         )
         (i32.const 10)
        )
       )
      )
      (set_local $9
       (i32.const 1)
      )
     )
     (set_local $9
      (i32.const 0)
     )
    )
   )
  )
  (set_global $STACKTOP
   (get_local $18)
  )
  (get_local $9)
 )
 (func $___lockfile (; 47 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (i32.const 0)
 )
 (func $___unlockfile (; 48 ;) (; has Stack IR ;) (param $0 i32)
  (nop)
 )
 (func $_out (; 49 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32)
  (if
   (i32.eqz
    (i32.and
     (i32.load
      (get_local $0)
     )
     (i32.const 32)
    )
   )
   (drop
    (call $___fwritex
     (get_local $1)
     (get_local $2)
     (get_local $0)
    )
   )
  )
 )
 (func $_getint (; 50 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (if
   (call $_isdigit
    (i32.load8_s
     (i32.load
      (get_local $0)
     )
    )
   )
   (loop $while-in
    (set_local $1
     (i32.add
      (i32.add
       (i32.mul
        (get_local $1)
        (i32.const 10)
       )
       (i32.const -48)
      )
      (i32.load8_s
       (tee_local $2
        (i32.load
         (get_local $0)
        )
       )
      )
     )
    )
    (i32.store
     (get_local $0)
     (tee_local $2
      (i32.add
       (get_local $2)
       (i32.const 1)
      )
     )
    )
    (br_if $while-in
     (call $_isdigit
      (i32.load8_s
       (get_local $2)
      )
     )
    )
   )
  )
  (get_local $1)
 )
 (func $_pop_arg (; 51 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32)
  (local $3 i32)
  (local $4 f64)
  (local $5 i64)
  (block $label$break$L1
   (if
    (i32.le_u
     (get_local $1)
     (i32.const 20)
    )
    (block $switch-default
     (block $switch-case9
      (block $switch-case8
       (block $switch-case7
        (block $switch-case6
         (block $switch-case5
          (block $switch-case4
           (block $switch-case3
            (block $switch-case2
             (block $switch-case1
              (block $switch-case
               (br_table $switch-case $switch-case1 $switch-case2 $switch-case3 $switch-case4 $switch-case5 $switch-case6 $switch-case7 $switch-case8 $switch-case9 $switch-default
                (i32.sub
                 (get_local $1)
                 (i32.const 9)
                )
               )
              )
              (set_local $3
               (i32.load
                (tee_local $1
                 (i32.and
                  (i32.add
                   (i32.load
                    (get_local $2)
                   )
                   (i32.const 3)
                  )
                  (i32.const -4)
                 )
                )
               )
              )
              (i32.store
               (get_local $2)
               (i32.add
                (get_local $1)
                (i32.const 4)
               )
              )
              (i32.store
               (get_local $0)
               (get_local $3)
              )
              (br $label$break$L1)
             )
             (set_local $3
              (i32.load
               (tee_local $1
                (i32.and
                 (i32.add
                  (i32.load
                   (get_local $2)
                  )
                  (i32.const 3)
                 )
                 (i32.const -4)
                )
               )
              )
             )
             (i32.store
              (get_local $2)
              (i32.add
               (get_local $1)
               (i32.const 4)
              )
             )
             (i64.store
              (get_local $0)
              (i64.extend_s/i32
               (get_local $3)
              )
             )
             (br $label$break$L1)
            )
            (set_local $3
             (i32.load
              (tee_local $1
               (i32.and
                (i32.add
                 (i32.load
                  (get_local $2)
                 )
                 (i32.const 3)
                )
                (i32.const -4)
               )
              )
             )
            )
            (i32.store
             (get_local $2)
             (i32.add
              (get_local $1)
              (i32.const 4)
             )
            )
            (i64.store
             (get_local $0)
             (i64.extend_u/i32
              (get_local $3)
             )
            )
            (br $label$break$L1)
           )
           (set_local $5
            (i64.load
             (tee_local $1
              (i32.and
               (i32.add
                (i32.load
                 (get_local $2)
                )
                (i32.const 7)
               )
               (i32.const -8)
              )
             )
            )
           )
           (i32.store
            (get_local $2)
            (i32.add
             (get_local $1)
             (i32.const 8)
            )
           )
           (i64.store
            (get_local $0)
            (get_local $5)
           )
           (br $label$break$L1)
          )
          (set_local $3
           (i32.load
            (tee_local $1
             (i32.and
              (i32.add
               (i32.load
                (get_local $2)
               )
               (i32.const 3)
              )
              (i32.const -4)
             )
            )
           )
          )
          (i32.store
           (get_local $2)
           (i32.add
            (get_local $1)
            (i32.const 4)
           )
          )
          (i64.store
           (get_local $0)
           (i64.extend_s/i32
            (i32.shr_s
             (i32.shl
              (i32.and
               (get_local $3)
               (i32.const 65535)
              )
              (i32.const 16)
             )
             (i32.const 16)
            )
           )
          )
          (br $label$break$L1)
         )
         (set_local $3
          (i32.load
           (tee_local $1
            (i32.and
             (i32.add
              (i32.load
               (get_local $2)
              )
              (i32.const 3)
             )
             (i32.const -4)
            )
           )
          )
         )
         (i32.store
          (get_local $2)
          (i32.add
           (get_local $1)
           (i32.const 4)
          )
         )
         (i64.store
          (get_local $0)
          (i64.extend_u/i32
           (i32.and
            (get_local $3)
            (i32.const 65535)
           )
          )
         )
         (br $label$break$L1)
        )
        (set_local $3
         (i32.load
          (tee_local $1
           (i32.and
            (i32.add
             (i32.load
              (get_local $2)
             )
             (i32.const 3)
            )
            (i32.const -4)
           )
          )
         )
        )
        (i32.store
         (get_local $2)
         (i32.add
          (get_local $1)
          (i32.const 4)
         )
        )
        (i64.store
         (get_local $0)
         (i64.extend_s/i32
          (i32.shr_s
           (i32.shl
            (i32.and
             (get_local $3)
             (i32.const 255)
            )
            (i32.const 24)
           )
           (i32.const 24)
          )
         )
        )
        (br $label$break$L1)
       )
       (set_local $3
        (i32.load
         (tee_local $1
          (i32.and
           (i32.add
            (i32.load
             (get_local $2)
            )
            (i32.const 3)
           )
           (i32.const -4)
          )
         )
        )
       )
       (i32.store
        (get_local $2)
        (i32.add
         (get_local $1)
         (i32.const 4)
        )
       )
       (i64.store
        (get_local $0)
        (i64.extend_u/i32
         (i32.and
          (get_local $3)
          (i32.const 255)
         )
        )
       )
       (br $label$break$L1)
      )
      (set_local $4
       (f64.load
        (tee_local $1
         (i32.and
          (i32.add
           (i32.load
            (get_local $2)
           )
           (i32.const 7)
          )
          (i32.const -8)
         )
        )
       )
      )
      (i32.store
       (get_local $2)
       (i32.add
        (get_local $1)
        (i32.const 8)
       )
      )
      (f64.store
       (get_local $0)
       (get_local $4)
      )
      (br $label$break$L1)
     )
     (set_local $4
      (f64.load
       (tee_local $1
        (i32.and
         (i32.add
          (i32.load
           (get_local $2)
          )
          (i32.const 7)
         )
         (i32.const -8)
        )
       )
      )
     )
     (i32.store
      (get_local $2)
      (i32.add
       (get_local $1)
       (i32.const 8)
      )
     )
     (f64.store
      (get_local $0)
      (get_local $4)
     )
    )
   )
  )
 )
 (func $_fmt_x (; 52 ;) (; has Stack IR ;) (param $0 i64) (param $1 i32) (param $2 i32) (result i32)
  (if
   (i64.ne
    (get_local $0)
    (i64.const 0)
   )
   (loop $while-in
    (i32.store8
     (tee_local $1
      (i32.add
       (get_local $1)
       (i32.const -1)
      )
     )
     (i32.or
      (i32.load8_u
       (i32.add
        (i32.and
         (i32.wrap/i64
          (get_local $0)
         )
         (i32.const 15)
        )
        (i32.const 3392)
       )
      )
      (get_local $2)
     )
    )
    (br_if $while-in
     (i64.ne
      (tee_local $0
       (i64.shr_u
        (get_local $0)
        (i64.const 4)
       )
      )
      (i64.const 0)
     )
    )
   )
  )
  (get_local $1)
 )
 (func $_fmt_o (; 53 ;) (; has Stack IR ;) (param $0 i64) (param $1 i32) (result i32)
  (if
   (i64.ne
    (get_local $0)
    (i64.const 0)
   )
   (loop $while-in
    (i32.store8
     (tee_local $1
      (i32.add
       (get_local $1)
       (i32.const -1)
      )
     )
     (i32.or
      (i32.and
       (i32.wrap/i64
        (get_local $0)
       )
       (i32.const 7)
      )
      (i32.const 48)
     )
    )
    (br_if $while-in
     (i64.ne
      (tee_local $0
       (i64.shr_u
        (get_local $0)
        (i64.const 3)
       )
      )
      (i64.const 0)
     )
    )
   )
  )
  (get_local $1)
 )
 (func $_fmt_u (; 54 ;) (; has Stack IR ;) (param $0 i64) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i64)
  (local $4 i32)
  (set_local $2
   (i32.wrap/i64
    (get_local $0)
   )
  )
  (if
   (i64.gt_u
    (get_local $0)
    (i64.const 4294967295)
   )
   (block
    (loop $while-in
     (i32.store8
      (tee_local $1
       (i32.add
        (get_local $1)
        (i32.const -1)
       )
      )
      (i32.or
       (i32.and
        (i32.wrap/i64
         (i64.sub
          (get_local $0)
          (i64.mul
           (tee_local $3
            (i64.div_u
             (get_local $0)
             (i64.const 10)
            )
           )
           (i64.const 10)
          )
         )
        )
        (i32.const 255)
       )
       (i32.const 48)
      )
     )
     (if
      (i64.gt_u
       (get_local $0)
       (i64.const 42949672959)
      )
      (block
       (set_local $0
        (get_local $3)
       )
       (br $while-in)
      )
     )
    )
    (set_local $2
     (i32.wrap/i64
      (get_local $3)
     )
    )
   )
  )
  (if
   (get_local $2)
   (loop $while-in1
    (i32.store8
     (tee_local $1
      (i32.add
       (get_local $1)
       (i32.const -1)
      )
     )
     (i32.or
      (i32.sub
       (get_local $2)
       (i32.mul
        (tee_local $4
         (i32.div_u
          (get_local $2)
          (i32.const 10)
         )
        )
        (i32.const 10)
       )
      )
      (i32.const 48)
     )
    )
    (if
     (i32.ge_u
      (get_local $2)
      (i32.const 10)
     )
     (block
      (set_local $2
       (get_local $4)
      )
      (br $while-in1)
     )
    )
   )
  )
  (get_local $1)
 )
 (func $_pad_666 (; 55 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (param $4 i32)
  (local $5 i32)
  (set_local $5
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 256)
   )
  )
  (if
   (i32.and
    (i32.gt_s
     (get_local $2)
     (get_local $3)
    )
    (i32.eqz
     (i32.and
      (get_local $4)
      (i32.const 73728)
     )
    )
   )
   (block
    (drop
     (call $_memset
      (get_local $5)
      (i32.shr_s
       (i32.shl
        (get_local $1)
        (i32.const 24)
       )
       (i32.const 24)
      )
      (select
       (tee_local $1
        (i32.sub
         (get_local $2)
         (get_local $3)
        )
       )
       (i32.const 256)
       (i32.lt_u
        (get_local $1)
        (i32.const 256)
       )
      )
     )
    )
    (if
     (i32.gt_u
      (get_local $1)
      (i32.const 255)
     )
     (block
      (set_local $2
       (i32.sub
        (get_local $2)
        (get_local $3)
       )
      )
      (loop $while-in
       (call $_out
        (get_local $0)
        (get_local $5)
        (i32.const 256)
       )
       (br_if $while-in
        (i32.gt_u
         (tee_local $1
          (i32.add
           (get_local $1)
           (i32.const -256)
          )
         )
         (i32.const 255)
        )
       )
      )
      (set_local $1
       (i32.and
        (get_local $2)
        (i32.const 255)
       )
      )
     )
    )
    (call $_out
     (get_local $0)
     (get_local $5)
     (get_local $1)
    )
   )
  )
  (set_global $STACKTOP
   (get_local $5)
  )
 )
 (func $_fmt_fp (; 56 ;) (; has Stack IR ;) (param $0 i32) (param $1 f64) (param $2 i32) (param $3 i32) (param $4 i32) (param $5 i32) (result i32)
  (local $6 i32)
  (local $7 i32)
  (local $8 i32)
  (local $9 i32)
  (local $10 i32)
  (local $11 i32)
  (local $12 i32)
  (local $13 i32)
  (local $14 i32)
  (local $15 f64)
  (local $16 i32)
  (local $17 i32)
  (local $18 i32)
  (local $19 i32)
  (local $20 i32)
  (local $21 i64)
  (local $22 i32)
  (local $23 i32)
  (local $24 i32)
  (local $25 i32)
  (local $26 i64)
  (local $27 i64)
  (local $28 i32)
  (set_local $22
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 560)
   )
  )
  (i32.store
   (tee_local $11
    (i32.add
     (get_local $22)
     (i32.const 536)
    )
   )
   (i32.const 0)
  )
  (set_local $20
   (if (result i32)
    (i64.lt_s
     (tee_local $21
      (call $___DOUBLE_BITS_667
       (get_local $1)
      )
     )
     (i64.const 0)
    )
    (block (result i32)
     (set_local $1
      (tee_local $15
       (f64.neg
        (get_local $1)
       )
      )
     )
     (set_local $19
      (i32.const 3952)
     )
     (set_local $21
      (call $___DOUBLE_BITS_667
       (get_local $15)
      )
     )
     (i32.const 1)
    )
    (block (result i32)
     (set_local $19
      (select
       (i32.const 3955)
       (select
        (i32.const 3958)
        (i32.const 3953)
        (i32.and
         (get_local $4)
         (i32.const 1)
        )
       )
       (i32.and
        (get_local $4)
        (i32.const 2048)
       )
      )
     )
     (i32.ne
      (i32.and
       (get_local $4)
       (i32.const 2049)
      )
      (i32.const 0)
     )
    )
   )
  )
  (set_local $7
   (i32.add
    (get_local $22)
    (i32.const 32)
   )
  )
  (set_local $18
   (tee_local $13
    (get_local $22)
   )
  )
  (set_local $16
   (i32.add
    (tee_local $9
     (i32.add
      (get_local $13)
      (i32.const 540)
     )
    )
    (i32.const 12)
   )
  )
  (set_local $0
   (block $do-once (result i32)
    (if (result i32)
     (i64.eq
      (i64.and
       (get_local $21)
       (i64.const 9218868437227405312)
      )
      (i64.const 9218868437227405312)
     )
     (block (result i32)
      (call $_pad_666
       (get_local $0)
       (i32.const 32)
       (get_local $2)
       (tee_local $3
        (i32.add
         (get_local $20)
         (i32.const 3)
        )
       )
       (i32.and
        (get_local $4)
        (i32.const -65537)
       )
      )
      (call $_out
       (get_local $0)
       (get_local $19)
       (get_local $20)
      )
      (call $_out
       (get_local $0)
       (select
        (select
         (i32.const 3979)
         (i32.const 3983)
         (tee_local $5
          (i32.ne
           (i32.and
            (get_local $5)
            (i32.const 32)
           )
           (i32.const 0)
          )
         )
        )
        (select
         (i32.const 3971)
         (i32.const 3975)
         (get_local $5)
        )
        (f64.ne
         (get_local $1)
         (get_local $1)
        )
       )
       (i32.const 3)
      )
      (call $_pad_666
       (get_local $0)
       (i32.const 32)
       (get_local $2)
       (get_local $3)
       (i32.xor
        (get_local $4)
        (i32.const 8192)
       )
      )
      (get_local $3)
     )
     (block (result i32)
      (if
       (tee_local $6
        (f64.ne
         (tee_local $1
          (f64.mul
           (call $_frexpl
            (get_local $1)
            (get_local $11)
           )
           (f64.const 2)
          )
         )
         (f64.const 0)
        )
       )
       (i32.store
        (get_local $11)
        (i32.add
         (i32.load
          (get_local $11)
         )
         (i32.const -1)
        )
       )
      )
      (if
       (i32.eq
        (tee_local $14
         (i32.or
          (get_local $5)
          (i32.const 32)
         )
        )
        (i32.const 97)
       )
       (block
        (set_local $8
         (select
          (i32.add
           (get_local $19)
           (i32.const 9)
          )
          (get_local $19)
          (tee_local $12
           (i32.and
            (get_local $5)
            (i32.const 32)
           )
          )
         )
        )
        (if
         (i32.eqz
          (i32.or
           (i32.gt_u
            (get_local $3)
            (i32.const 11)
           )
           (i32.eqz
            (tee_local $7
             (i32.sub
              (i32.const 12)
              (get_local $3)
             )
            )
           )
          )
         )
         (block
          (set_local $15
           (f64.const 8)
          )
          (loop $while-in
           (set_local $15
            (f64.mul
             (get_local $15)
             (f64.const 16)
            )
           )
           (br_if $while-in
            (tee_local $7
             (i32.add
              (get_local $7)
              (i32.const -1)
             )
            )
           )
          )
          (set_local $1
           (if (result f64)
            (i32.eq
             (i32.load8_s
              (get_local $8)
             )
             (i32.const 45)
            )
            (f64.neg
             (f64.add
              (get_local $15)
              (f64.sub
               (f64.neg
                (get_local $1)
               )
               (get_local $15)
              )
             )
            )
            (f64.sub
             (f64.add
              (get_local $1)
              (get_local $15)
             )
             (get_local $15)
            )
           )
          )
         )
        )
        (if
         (i32.eq
          (tee_local $7
           (call $_fmt_u
            (i64.extend_s/i32
             (select
              (i32.sub
               (i32.const 0)
               (tee_local $6
                (i32.load
                 (get_local $11)
                )
               )
              )
              (get_local $6)
              (i32.lt_s
               (get_local $6)
               (i32.const 0)
              )
             )
            )
            (get_local $16)
           )
          )
          (get_local $16)
         )
         (i32.store8
          (tee_local $7
           (i32.add
            (get_local $9)
            (i32.const 11)
           )
          )
          (i32.const 48)
         )
        )
        (set_local $10
         (i32.or
          (get_local $20)
          (i32.const 2)
         )
        )
        (i32.store8
         (i32.add
          (get_local $7)
          (i32.const -1)
         )
         (i32.add
          (i32.and
           (i32.shr_s
            (get_local $6)
            (i32.const 31)
           )
           (i32.const 2)
          )
          (i32.const 43)
         )
        )
        (i32.store8
         (tee_local $7
          (i32.add
           (get_local $7)
           (i32.const -2)
          )
         )
         (i32.add
          (get_local $5)
          (i32.const 15)
         )
        )
        (set_local $9
         (i32.lt_s
          (get_local $3)
          (i32.const 1)
         )
        )
        (set_local $11
         (i32.eqz
          (i32.and
           (get_local $4)
           (i32.const 8)
          )
         )
        )
        (set_local $5
         (get_local $13)
        )
        (loop $while-in3
         (i32.store8
          (get_local $5)
          (i32.or
           (get_local $12)
           (i32.load8_u
            (i32.add
             (tee_local $6
              (i32.trunc_s/f64
               (get_local $1)
              )
             )
             (i32.const 3392)
            )
           )
          )
         )
         (set_local $1
          (f64.mul
           (f64.sub
            (get_local $1)
            (f64.convert_s/i32
             (get_local $6)
            )
           )
           (f64.const 16)
          )
         )
         (set_local $5
          (if (result i32)
           (i32.eq
            (i32.sub
             (tee_local $6
              (i32.add
               (get_local $5)
               (i32.const 1)
              )
             )
             (get_local $18)
            )
            (i32.const 1)
           )
           (if (result i32)
            (i32.and
             (get_local $11)
             (i32.and
              (get_local $9)
              (f64.eq
               (get_local $1)
               (f64.const 0)
              )
             )
            )
            (get_local $6)
            (block (result i32)
             (i32.store8
              (get_local $6)
              (i32.const 46)
             )
             (i32.add
              (get_local $5)
              (i32.const 2)
             )
            )
           )
           (get_local $6)
          )
         )
         (br_if $while-in3
          (f64.ne
           (get_local $1)
           (f64.const 0)
          )
         )
        )
        (set_local $3
         (block $__rjto$0 (result i32)
          (if
           (i32.eqz
            (i32.or
             (i32.eqz
              (get_local $3)
             )
             (i32.ge_s
              (i32.add
               (i32.sub
                (i32.const -2)
                (get_local $18)
               )
               (get_local $5)
              )
              (get_local $3)
             )
            )
           )
           (block
            (set_local $9
             (i32.sub
              (i32.add
               (i32.add
                (get_local $3)
                (i32.const 2)
               )
               (get_local $16)
              )
              (get_local $7)
             )
            )
            (br $__rjto$0
             (get_local $7)
            )
           )
          )
          (set_local $9
           (i32.add
            (i32.sub
             (i32.sub
              (get_local $16)
              (get_local $18)
             )
             (get_local $7)
            )
            (get_local $5)
           )
          )
          (get_local $7)
         )
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 32)
         (get_local $2)
         (tee_local $6
          (i32.add
           (get_local $9)
           (get_local $10)
          )
         )
         (get_local $4)
        )
        (call $_out
         (get_local $0)
         (get_local $8)
         (get_local $10)
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 48)
         (get_local $2)
         (get_local $6)
         (i32.xor
          (get_local $4)
          (i32.const 65536)
         )
        )
        (call $_out
         (get_local $0)
         (get_local $13)
         (tee_local $5
          (i32.sub
           (get_local $5)
           (get_local $18)
          )
         )
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 48)
         (i32.sub
          (get_local $9)
          (i32.add
           (get_local $5)
           (tee_local $3
            (i32.sub
             (get_local $16)
             (get_local $3)
            )
           )
          )
         )
         (i32.const 0)
         (i32.const 0)
        )
        (call $_out
         (get_local $0)
         (get_local $7)
         (get_local $3)
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 32)
         (get_local $2)
         (get_local $6)
         (i32.xor
          (get_local $4)
          (i32.const 8192)
         )
        )
        (br $do-once
         (get_local $6)
        )
       )
      )
      (if
       (get_local $6)
       (block
        (i32.store
         (get_local $11)
         (tee_local $8
          (i32.add
           (i32.load
            (get_local $11)
           )
           (i32.const -28)
          )
         )
        )
        (set_local $1
         (f64.mul
          (get_local $1)
          (f64.const 268435456)
         )
        )
       )
       (set_local $8
        (i32.load
         (get_local $11)
        )
       )
      )
      (set_local $6
       (tee_local $9
        (select
         (get_local $7)
         (i32.add
          (get_local $7)
          (i32.const 288)
         )
         (i32.lt_s
          (get_local $8)
          (i32.const 0)
         )
        )
       )
      )
      (loop $while-in5
       (i32.store
        (get_local $6)
        (tee_local $7
         (i32.trunc_u/f64
          (get_local $1)
         )
        )
       )
       (set_local $6
        (i32.add
         (get_local $6)
         (i32.const 4)
        )
       )
       (br_if $while-in5
        (f64.ne
         (tee_local $1
          (f64.mul
           (f64.sub
            (get_local $1)
            (f64.convert_u/i32
             (get_local $7)
            )
           )
           (f64.const 1e9)
          )
         )
         (f64.const 0)
        )
       )
      )
      (if
       (i32.gt_s
        (get_local $8)
        (i32.const 0)
       )
       (block
        (set_local $7
         (get_local $9)
        )
        (loop $while-in7
         (set_local $12
          (select
           (get_local $8)
           (i32.const 29)
           (i32.lt_s
            (get_local $8)
            (i32.const 29)
           )
          )
         )
         (if
          (i32.ge_u
           (tee_local $8
            (i32.add
             (get_local $6)
             (i32.const -4)
            )
           )
           (get_local $7)
          )
          (block
           (set_local $26
            (i64.extend_u/i32
             (get_local $12)
            )
           )
           (set_local $10
            (i32.const 0)
           )
           (loop $while-in9
            (set_local $21
             (i64.div_u
              (tee_local $27
               (i64.add
                (i64.shl
                 (i64.extend_u/i32
                  (i32.load
                   (get_local $8)
                  )
                 )
                 (get_local $26)
                )
                (i64.extend_u/i32
                 (get_local $10)
                )
               )
              )
              (i64.const 1000000000)
             )
            )
            (i64.store32
             (get_local $8)
             (i64.sub
              (get_local $27)
              (i64.mul
               (get_local $21)
               (i64.const 1000000000)
              )
             )
            )
            (set_local $10
             (i32.wrap/i64
              (get_local $21)
             )
            )
            (br_if $while-in9
             (i32.ge_u
              (tee_local $8
               (i32.add
                (get_local $8)
                (i32.const -4)
               )
              )
              (get_local $7)
             )
            )
           )
           (if
            (get_local $10)
            (i32.store
             (tee_local $7
              (i32.add
               (get_local $7)
               (i32.const -4)
              )
             )
             (get_local $10)
            )
           )
          )
         )
         (block $label$break$L57
          (if
           (i32.gt_u
            (get_local $6)
            (get_local $7)
           )
           (loop $while-in12
            (br_if $label$break$L57
             (i32.load
              (tee_local $8
               (i32.add
                (get_local $6)
                (i32.const -4)
               )
              )
             )
            )
            (set_local $6
             (if (result i32)
              (i32.gt_u
               (get_local $8)
               (get_local $7)
              )
              (block
               (set_local $6
                (get_local $8)
               )
               (br $while-in12)
              )
              (get_local $8)
             )
            )
           )
          )
         )
         (i32.store
          (get_local $11)
          (tee_local $8
           (i32.sub
            (i32.load
             (get_local $11)
            )
            (get_local $12)
           )
          )
         )
         (br_if $while-in7
          (i32.gt_s
           (get_local $8)
           (i32.const 0)
          )
         )
        )
       )
       (set_local $7
        (get_local $9)
       )
      )
      (set_local $12
       (select
        (i32.const 6)
        (get_local $3)
        (i32.lt_s
         (get_local $3)
         (i32.const 0)
        )
       )
      )
      (if
       (i32.lt_s
        (get_local $8)
        (i32.const 0)
       )
       (block
        (set_local $17
         (i32.add
          (i32.div_s
           (i32.add
            (get_local $12)
            (i32.const 25)
           )
           (i32.const 9)
          )
          (i32.const 1)
         )
        )
        (set_local $23
         (i32.eq
          (get_local $14)
          (i32.const 102)
         )
        )
        (set_local $3
         (get_local $6)
        )
        (loop $while-in14
         (set_local $10
          (select
           (tee_local $6
            (i32.sub
             (i32.const 0)
             (get_local $8)
            )
           )
           (i32.const 9)
           (i32.lt_s
            (get_local $6)
            (i32.const 9)
           )
          )
         )
         (set_local $8
          (select
           (i32.add
            (tee_local $7
             (select
              (get_local $9)
              (tee_local $3
               (if (result i32)
                (i32.lt_u
                 (get_local $7)
                 (get_local $3)
                )
                (block (result i32)
                 (set_local $24
                  (i32.add
                   (i32.shl
                    (i32.const 1)
                    (get_local $10)
                   )
                   (i32.const -1)
                  )
                 )
                 (set_local $25
                  (i32.shr_u
                   (i32.const 1000000000)
                   (get_local $10)
                  )
                 )
                 (set_local $8
                  (i32.const 0)
                 )
                 (set_local $6
                  (get_local $7)
                 )
                 (loop $while-in16
                  (i32.store
                   (get_local $6)
                   (i32.add
                    (i32.shr_u
                     (tee_local $28
                      (i32.load
                       (get_local $6)
                      )
                     )
                     (get_local $10)
                    )
                    (get_local $8)
                   )
                  )
                  (set_local $8
                   (i32.mul
                    (i32.and
                     (get_local $28)
                     (get_local $24)
                    )
                    (get_local $25)
                   )
                  )
                  (br_if $while-in16
                   (i32.lt_u
                    (tee_local $6
                     (i32.add
                      (get_local $6)
                      (i32.const 4)
                     )
                    )
                    (get_local $3)
                   )
                  )
                 )
                 (set_local $7
                  (select
                   (get_local $7)
                   (i32.add
                    (get_local $7)
                    (i32.const 4)
                   )
                   (i32.load
                    (get_local $7)
                   )
                  )
                 )
                 (if
                  (get_local $8)
                  (block
                   (i32.store
                    (get_local $3)
                    (get_local $8)
                   )
                   (set_local $6
                    (i32.add
                     (get_local $3)
                     (i32.const 4)
                    )
                   )
                  )
                  (set_local $6
                   (get_local $3)
                  )
                 )
                 (get_local $7)
                )
                (block (result i32)
                 (set_local $6
                  (get_local $3)
                 )
                 (select
                  (get_local $7)
                  (i32.add
                   (get_local $7)
                   (i32.const 4)
                  )
                  (i32.load
                   (get_local $7)
                  )
                 )
                )
               )
              )
              (get_local $23)
             )
            )
            (i32.shl
             (get_local $17)
             (i32.const 2)
            )
           )
           (get_local $6)
           (i32.gt_s
            (i32.shr_s
             (i32.sub
              (get_local $6)
              (get_local $7)
             )
             (i32.const 2)
            )
            (get_local $17)
           )
          )
         )
         (i32.store
          (get_local $11)
          (tee_local $6
           (i32.add
            (i32.load
             (get_local $11)
            )
            (get_local $10)
           )
          )
         )
         (if
          (i32.lt_s
           (get_local $6)
           (i32.const 0)
          )
          (block
           (set_local $7
            (get_local $3)
           )
           (set_local $3
            (get_local $8)
           )
           (set_local $8
            (get_local $6)
           )
           (br $while-in14)
          )
         )
        )
       )
       (block
        (set_local $3
         (get_local $7)
        )
        (set_local $8
         (get_local $6)
        )
       )
      )
      (set_local $11
       (get_local $9)
      )
      (if
       (i32.lt_u
        (get_local $3)
        (get_local $8)
       )
       (block
        (set_local $7
         (i32.mul
          (i32.shr_s
           (i32.sub
            (get_local $11)
            (get_local $3)
           )
           (i32.const 2)
          )
          (i32.const 9)
         )
        )
        (if
         (i32.ge_u
          (tee_local $9
           (i32.load
            (get_local $3)
           )
          )
          (i32.const 10)
         )
         (block
          (set_local $6
           (i32.const 10)
          )
          (loop $while-in18
           (set_local $7
            (i32.add
             (get_local $7)
             (i32.const 1)
            )
           )
           (br_if $while-in18
            (i32.ge_u
             (get_local $9)
             (tee_local $6
              (i32.mul
               (get_local $6)
               (i32.const 10)
              )
             )
            )
           )
          )
         )
        )
       )
       (set_local $7
        (i32.const 0)
       )
      )
      (set_local $7
       (if (result i32)
        (i32.lt_s
         (tee_local $6
          (i32.add
           (i32.sub
            (get_local $12)
            (select
             (i32.const 0)
             (get_local $7)
             (i32.eq
              (get_local $14)
              (i32.const 102)
             )
            )
           )
           (i32.shr_s
            (i32.shl
             (i32.and
              (tee_local $23
               (i32.ne
                (get_local $12)
                (i32.const 0)
               )
              )
              (tee_local $24
               (i32.eq
                (get_local $14)
                (i32.const 103)
               )
              )
             )
             (i32.const 31)
            )
            (i32.const 31)
           )
          )
         )
         (i32.add
          (i32.mul
           (i32.shr_s
            (i32.sub
             (get_local $8)
             (get_local $11)
            )
            (i32.const 2)
           )
           (i32.const 9)
          )
          (i32.const -9)
         )
        )
        (block (result i32)
         (set_local $14
          (i32.div_s
           (tee_local $6
            (i32.add
             (get_local $6)
             (i32.const 9216)
            )
           )
           (i32.const 9)
          )
         )
         (if
          (i32.lt_s
           (tee_local $6
            (i32.sub
             (get_local $6)
             (i32.mul
              (get_local $14)
              (i32.const 9)
             )
            )
           )
           (i32.const 8)
          )
          (block
           (set_local $9
            (i32.const 10)
           )
           (loop $while-in20
            (set_local $10
             (i32.add
              (get_local $6)
              (i32.const 1)
             )
            )
            (set_local $9
             (i32.mul
              (get_local $9)
              (i32.const 10)
             )
            )
            (if
             (i32.lt_s
              (get_local $6)
              (i32.const 7)
             )
             (block
              (set_local $6
               (get_local $10)
              )
              (br $while-in20)
             )
            )
           )
          )
          (set_local $9
           (i32.const 10)
          )
         )
         (set_local $17
          (i32.div_u
           (tee_local $14
            (i32.load
             (tee_local $6
              (i32.add
               (i32.add
                (get_local $11)
                (i32.shl
                 (get_local $14)
                 (i32.const 2)
                )
               )
               (i32.const -4092)
              )
             )
            )
           )
           (get_local $9)
          )
         )
         (if
          (i32.eqz
           (i32.and
            (tee_local $25
             (i32.eq
              (i32.add
               (get_local $6)
               (i32.const 4)
              )
              (get_local $8)
             )
            )
            (i32.eqz
             (tee_local $10
              (i32.sub
               (get_local $14)
               (i32.mul
                (get_local $17)
                (get_local $9)
               )
              )
             )
            )
           )
          )
          (block
           (set_local $1
            (select
             (f64.const 9007199254740994)
             (f64.const 9007199254740992)
             (i32.and
              (get_local $17)
              (i32.const 1)
             )
            )
           )
           (set_local $15
            (select
             (f64.const 0.5)
             (select
              (f64.const 1)
              (f64.const 1.5)
              (i32.and
               (get_local $25)
               (i32.eq
                (get_local $10)
                (tee_local $17
                 (i32.shr_u
                  (get_local $9)
                  (i32.const 1)
                 )
                )
               )
              )
             )
             (i32.lt_u
              (get_local $10)
              (get_local $17)
             )
            )
           )
           (if
            (get_local $20)
            (block
             (set_local $15
              (select
               (f64.neg
                (get_local $15)
               )
               (get_local $15)
               (tee_local $17
                (i32.eq
                 (i32.load8_s
                  (get_local $19)
                 )
                 (i32.const 45)
                )
               )
              )
             )
             (set_local $1
              (select
               (f64.neg
                (get_local $1)
               )
               (get_local $1)
               (get_local $17)
              )
             )
            )
           )
           (i32.store
            (get_local $6)
            (tee_local $10
             (i32.sub
              (get_local $14)
              (get_local $10)
             )
            )
           )
           (if
            (f64.ne
             (f64.add
              (get_local $1)
              (get_local $15)
             )
             (get_local $1)
            )
            (block
             (i32.store
              (get_local $6)
              (tee_local $7
               (i32.add
                (get_local $10)
                (get_local $9)
               )
              )
             )
             (if
              (i32.gt_u
               (get_local $7)
               (i32.const 999999999)
              )
              (loop $while-in22
               (i32.store
                (get_local $6)
                (i32.const 0)
               )
               (if
                (i32.lt_u
                 (tee_local $6
                  (i32.add
                   (get_local $6)
                   (i32.const -4)
                  )
                 )
                 (get_local $3)
                )
                (i32.store
                 (tee_local $3
                  (i32.add
                   (get_local $3)
                   (i32.const -4)
                  )
                 )
                 (i32.const 0)
                )
               )
               (i32.store
                (get_local $6)
                (tee_local $7
                 (i32.add
                  (i32.load
                   (get_local $6)
                  )
                  (i32.const 1)
                 )
                )
               )
               (br_if $while-in22
                (i32.gt_u
                 (get_local $7)
                 (i32.const 999999999)
                )
               )
              )
             )
             (set_local $7
              (i32.mul
               (i32.shr_s
                (i32.sub
                 (get_local $11)
                 (get_local $3)
                )
                (i32.const 2)
               )
               (i32.const 9)
              )
             )
             (if
              (i32.ge_u
               (tee_local $10
                (i32.load
                 (get_local $3)
                )
               )
               (i32.const 10)
              )
              (block
               (set_local $9
                (i32.const 10)
               )
               (loop $while-in24
                (set_local $7
                 (i32.add
                  (get_local $7)
                  (i32.const 1)
                 )
                )
                (br_if $while-in24
                 (i32.ge_u
                  (get_local $10)
                  (tee_local $9
                   (i32.mul
                    (get_local $9)
                    (i32.const 10)
                   )
                  )
                 )
                )
               )
              )
             )
            )
           )
          )
         )
         (set_local $10
          (get_local $7)
         )
         (set_local $6
          (select
           (tee_local $7
            (i32.add
             (get_local $6)
             (i32.const 4)
            )
           )
           (get_local $8)
           (i32.gt_u
            (get_local $8)
            (get_local $7)
           )
          )
         )
         (get_local $3)
        )
        (block (result i32)
         (set_local $10
          (get_local $7)
         )
         (set_local $6
          (get_local $8)
         )
         (get_local $3)
        )
       )
      )
      (set_local $17
       (i32.sub
        (i32.const 0)
        (get_local $10)
       )
      )
      (block $label$break$L109
       (if
        (i32.gt_u
         (get_local $6)
         (get_local $7)
        )
        (block
         (set_local $3
          (get_local $6)
         )
         (loop $while-in27
          (if
           (i32.load
            (tee_local $6
             (i32.add
              (get_local $3)
              (i32.const -4)
             )
            )
           )
           (block
            (set_local $6
             (get_local $3)
            )
            (set_local $14
             (i32.const 1)
            )
            (br $label$break$L109)
           )
          )
          (set_local $14
           (if (result i32)
            (i32.gt_u
             (get_local $6)
             (get_local $7)
            )
            (block
             (set_local $3
              (get_local $6)
             )
             (br $while-in27)
            )
            (i32.const 0)
           )
          )
         )
        )
        (set_local $14
         (i32.const 0)
        )
       )
      )
      (set_local $3
       (if (result i32)
        (get_local $24)
        (block (result i32)
         (set_local $5
          (if (result i32)
           (i32.and
            (i32.gt_s
             (tee_local $3
              (i32.add
               (get_local $12)
               (i32.xor
                (get_local $23)
                (i32.const 1)
               )
              )
             )
             (get_local $10)
            )
            (i32.gt_s
             (get_local $10)
             (i32.const -5)
            )
           )
           (block (result i32)
            (set_local $8
             (i32.sub
              (i32.add
               (get_local $3)
               (i32.const -1)
              )
              (get_local $10)
             )
            )
            (i32.add
             (get_local $5)
             (i32.const -1)
            )
           )
           (block (result i32)
            (set_local $8
             (i32.add
              (get_local $3)
              (i32.const -1)
             )
            )
            (i32.add
             (get_local $5)
             (i32.const -2)
            )
           )
          )
         )
         (if (result i32)
          (i32.and
           (get_local $4)
           (i32.const 8)
          )
          (get_local $8)
          (block (result i32)
           (if
            (get_local $14)
            (if
             (tee_local $12
              (i32.load
               (i32.add
                (get_local $6)
                (i32.const -4)
               )
              )
             )
             (if
              (i32.rem_u
               (get_local $12)
               (i32.const 10)
              )
              (set_local $3
               (i32.const 0)
              )
              (block
               (set_local $3
                (i32.const 0)
               )
               (set_local $9
                (i32.const 10)
               )
               (loop $while-in31
                (set_local $3
                 (i32.add
                  (get_local $3)
                  (i32.const 1)
                 )
                )
                (br_if $while-in31
                 (i32.eqz
                  (i32.rem_u
                   (get_local $12)
                   (tee_local $9
                    (i32.mul
                     (get_local $9)
                     (i32.const 10)
                    )
                   )
                  )
                 )
                )
               )
              )
             )
             (set_local $3
              (i32.const 9)
             )
            )
            (set_local $3
             (i32.const 9)
            )
           )
           (set_local $9
            (i32.add
             (i32.mul
              (i32.shr_s
               (i32.sub
                (get_local $6)
                (get_local $11)
               )
               (i32.const 2)
              )
              (i32.const 9)
             )
             (i32.const -9)
            )
           )
           (if (result i32)
            (i32.eq
             (i32.or
              (get_local $5)
              (i32.const 32)
             )
             (i32.const 102)
            )
            (select
             (get_local $8)
             (tee_local $3
              (select
               (tee_local $3
                (i32.sub
                 (get_local $9)
                 (get_local $3)
                )
               )
               (i32.const 0)
               (i32.gt_s
                (get_local $3)
                (i32.const 0)
               )
              )
             )
             (i32.lt_s
              (get_local $8)
              (get_local $3)
             )
            )
            (select
             (get_local $8)
             (tee_local $3
              (select
               (tee_local $3
                (i32.sub
                 (i32.add
                  (get_local $9)
                  (get_local $10)
                 )
                 (get_local $3)
                )
               )
               (i32.const 0)
               (i32.gt_s
                (get_local $3)
                (i32.const 0)
               )
              )
             )
             (i32.lt_s
              (get_local $8)
              (get_local $3)
             )
            )
           )
          )
         )
        )
        (get_local $12)
       )
      )
      (set_local $5
       (if (result i32)
        (tee_local $12
         (i32.eq
          (i32.or
           (get_local $5)
           (i32.const 32)
          )
          (i32.const 102)
         )
        )
        (block (result i32)
         (set_local $8
          (i32.const 0)
         )
         (select
          (get_local $10)
          (i32.const 0)
          (i32.gt_s
           (get_local $10)
           (i32.const 0)
          )
         )
        )
        (block (result i32)
         (if
          (i32.lt_s
           (i32.sub
            (get_local $16)
            (tee_local $9
             (call $_fmt_u
              (i64.extend_s/i32
               (select
                (get_local $17)
                (get_local $10)
                (i32.lt_s
                 (get_local $10)
                 (i32.const 0)
                )
               )
              )
              (get_local $16)
             )
            )
           )
           (i32.const 2)
          )
          (loop $while-in33
           (i32.store8
            (tee_local $9
             (i32.add
              (get_local $9)
              (i32.const -1)
             )
            )
            (i32.const 48)
           )
           (br_if $while-in33
            (i32.lt_s
             (i32.sub
              (get_local $16)
              (get_local $9)
             )
             (i32.const 2)
            )
           )
          )
         )
         (i32.store8
          (i32.add
           (get_local $9)
           (i32.const -1)
          )
          (i32.add
           (i32.and
            (i32.shr_s
             (get_local $10)
             (i32.const 31)
            )
            (i32.const 2)
           )
           (i32.const 43)
          )
         )
         (i32.store8
          (tee_local $8
           (i32.add
            (get_local $9)
            (i32.const -2)
           )
          )
          (get_local $5)
         )
         (i32.sub
          (get_local $16)
          (get_local $8)
         )
        )
       )
      )
      (call $_pad_666
       (get_local $0)
       (i32.const 32)
       (get_local $2)
       (tee_local $10
        (i32.add
         (i32.add
          (i32.add
           (i32.add
            (get_local $20)
            (i32.const 1)
           )
           (get_local $3)
          )
          (select
           (i32.const 1)
           (i32.and
            (i32.shr_u
             (get_local $4)
             (i32.const 3)
            )
            (i32.const 1)
           )
           (tee_local $17
            (i32.ne
             (get_local $3)
             (i32.const 0)
            )
           )
          )
         )
         (get_local $5)
        )
       )
       (get_local $4)
      )
      (call $_out
       (get_local $0)
       (get_local $19)
       (get_local $20)
      )
      (call $_pad_666
       (get_local $0)
       (i32.const 48)
       (get_local $2)
       (get_local $10)
       (i32.xor
        (get_local $4)
        (i32.const 65536)
       )
      )
      (if
       (get_local $12)
       (block
        (set_local $8
         (tee_local $9
          (i32.add
           (get_local $13)
           (i32.const 9)
          )
         )
        )
        (set_local $16
         (i32.add
          (get_local $13)
          (i32.const 8)
         )
        )
        (set_local $7
         (tee_local $12
          (select
           (get_local $11)
           (get_local $7)
           (i32.gt_u
            (get_local $7)
            (get_local $11)
           )
          )
         )
        )
        (loop $while-in35
         (set_local $5
          (call $_fmt_u
           (i64.extend_u/i32
            (i32.load
             (get_local $7)
            )
           )
           (get_local $9)
          )
         )
         (if
          (i32.eq
           (get_local $7)
           (get_local $12)
          )
          (if
           (i32.eq
            (get_local $5)
            (get_local $9)
           )
           (block
            (i32.store8
             (get_local $16)
             (i32.const 48)
            )
            (set_local $5
             (get_local $16)
            )
           )
          )
          (if
           (i32.gt_u
            (get_local $5)
            (get_local $13)
           )
           (block
            (drop
             (call $_memset
              (get_local $13)
              (i32.const 48)
              (i32.sub
               (get_local $5)
               (get_local $18)
              )
             )
            )
            (loop $while-in37
             (br_if $while-in37
              (i32.gt_u
               (tee_local $5
                (i32.add
                 (get_local $5)
                 (i32.const -1)
                )
               )
               (get_local $13)
              )
             )
            )
           )
          )
         )
         (call $_out
          (get_local $0)
          (get_local $5)
          (i32.sub
           (get_local $8)
           (get_local $5)
          )
         )
         (if
          (i32.le_u
           (tee_local $5
            (i32.add
             (get_local $7)
             (i32.const 4)
            )
           )
           (get_local $11)
          )
          (block
           (set_local $7
            (get_local $5)
           )
           (br $while-in35)
          )
         )
        )
        (if
         (i32.eqz
          (i32.and
           (i32.eqz
            (i32.and
             (get_local $4)
             (i32.const 8)
            )
           )
           (i32.xor
            (get_local $17)
            (i32.const 1)
           )
          )
         )
         (call $_out
          (get_local $0)
          (i32.const 3987)
          (i32.const 1)
         )
        )
        (if
         (i32.and
          (i32.lt_u
           (get_local $5)
           (get_local $6)
          )
          (i32.gt_s
           (get_local $3)
           (i32.const 0)
          )
         )
         (loop $while-in39
          (if
           (i32.gt_u
            (tee_local $7
             (call $_fmt_u
              (i64.extend_u/i32
               (i32.load
                (get_local $5)
               )
              )
              (get_local $9)
             )
            )
            (get_local $13)
           )
           (block
            (drop
             (call $_memset
              (get_local $13)
              (i32.const 48)
              (i32.sub
               (get_local $7)
               (get_local $18)
              )
             )
            )
            (loop $while-in41
             (br_if $while-in41
              (i32.gt_u
               (tee_local $7
                (i32.add
                 (get_local $7)
                 (i32.const -1)
                )
               )
               (get_local $13)
              )
             )
            )
           )
          )
          (call $_out
           (get_local $0)
           (get_local $7)
           (select
            (get_local $3)
            (i32.const 9)
            (i32.lt_s
             (get_local $3)
             (i32.const 9)
            )
           )
          )
          (set_local $7
           (i32.add
            (get_local $3)
            (i32.const -9)
           )
          )
          (set_local $3
           (if (result i32)
            (i32.and
             (i32.lt_u
              (tee_local $5
               (i32.add
                (get_local $5)
                (i32.const 4)
               )
              )
              (get_local $6)
             )
             (i32.gt_s
              (get_local $3)
              (i32.const 9)
             )
            )
            (block
             (set_local $3
              (get_local $7)
             )
             (br $while-in39)
            )
            (get_local $7)
           )
          )
         )
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 48)
         (i32.add
          (get_local $3)
          (i32.const 9)
         )
         (i32.const 9)
         (i32.const 0)
        )
       )
       (block
        (if
         (i32.and
          (i32.lt_u
           (get_local $7)
           (tee_local $20
            (select
             (get_local $6)
             (i32.add
              (get_local $7)
              (i32.const 4)
             )
             (get_local $14)
            )
           )
          )
          (i32.gt_s
           (get_local $3)
           (i32.const -1)
          )
         )
         (block
          (set_local $19
           (i32.eqz
            (i32.and
             (get_local $4)
             (i32.const 8)
            )
           )
          )
          (set_local $14
           (tee_local $11
            (i32.add
             (get_local $13)
             (i32.const 9)
            )
           )
          )
          (set_local $18
           (i32.sub
            (i32.const 0)
            (get_local $18)
           )
          )
          (set_local $12
           (i32.add
            (get_local $13)
            (i32.const 8)
           )
          )
          (set_local $5
           (get_local $3)
          )
          (set_local $6
           (get_local $7)
          )
          (loop $while-in43
           (if
            (i32.eq
             (tee_local $3
              (call $_fmt_u
               (i64.extend_u/i32
                (i32.load
                 (get_local $6)
                )
               )
               (get_local $11)
              )
             )
             (get_local $11)
            )
            (block
             (i32.store8
              (get_local $12)
              (i32.const 48)
             )
             (set_local $3
              (get_local $12)
             )
            )
           )
           (block $do-once44
            (if
             (i32.eq
              (get_local $6)
              (get_local $7)
             )
             (block
              (set_local $9
               (i32.add
                (get_local $3)
                (i32.const 1)
               )
              )
              (call $_out
               (get_local $0)
               (get_local $3)
               (i32.const 1)
              )
              (if
               (i32.and
                (get_local $19)
                (i32.lt_s
                 (get_local $5)
                 (i32.const 1)
                )
               )
               (block
                (set_local $3
                 (get_local $9)
                )
                (br $do-once44)
               )
              )
              (call $_out
               (get_local $0)
               (i32.const 3987)
               (i32.const 1)
              )
              (set_local $3
               (get_local $9)
              )
             )
             (block
              (br_if $do-once44
               (i32.le_u
                (get_local $3)
                (get_local $13)
               )
              )
              (drop
               (call $_memset
                (get_local $13)
                (i32.const 48)
                (i32.add
                 (get_local $3)
                 (get_local $18)
                )
               )
              )
              (loop $while-in47
               (br_if $while-in47
                (i32.gt_u
                 (tee_local $3
                  (i32.add
                   (get_local $3)
                   (i32.const -1)
                  )
                 )
                 (get_local $13)
                )
               )
              )
             )
            )
           )
           (call $_out
            (get_local $0)
            (get_local $3)
            (select
             (tee_local $3
              (i32.sub
               (get_local $14)
               (get_local $3)
              )
             )
             (get_local $5)
             (i32.gt_s
              (get_local $5)
              (get_local $3)
             )
            )
           )
           (br_if $while-in43
            (i32.and
             (i32.lt_u
              (tee_local $6
               (i32.add
                (get_local $6)
                (i32.const 4)
               )
              )
              (get_local $20)
             )
             (i32.gt_s
              (tee_local $5
               (i32.sub
                (get_local $5)
                (get_local $3)
               )
              )
              (i32.const -1)
             )
            )
           )
          )
          (set_local $3
           (get_local $5)
          )
         )
        )
        (call $_pad_666
         (get_local $0)
         (i32.const 48)
         (i32.add
          (get_local $3)
          (i32.const 18)
         )
         (i32.const 18)
         (i32.const 0)
        )
        (call $_out
         (get_local $0)
         (get_local $8)
         (i32.sub
          (get_local $16)
          (get_local $8)
         )
        )
       )
      )
      (call $_pad_666
       (get_local $0)
       (i32.const 32)
       (get_local $2)
       (get_local $10)
       (i32.xor
        (get_local $4)
        (i32.const 8192)
       )
      )
      (get_local $10)
     )
    )
   )
  )
  (set_global $STACKTOP
   (get_local $22)
  )
  (select
   (get_local $2)
   (get_local $0)
   (i32.lt_s
    (get_local $0)
    (get_local $2)
   )
  )
 )
 (func $___DOUBLE_BITS_667 (; 57 ;) (; has Stack IR ;) (param $0 f64) (result i64)
  (i64.reinterpret/f64
   (get_local $0)
  )
 )
 (func $_frexpl (; 58 ;) (; has Stack IR ;) (param $0 f64) (param $1 i32) (result f64)
  (call $_frexp
   (get_local $0)
   (get_local $1)
  )
 )
 (func $_frexp (; 59 ;) (; has Stack IR ;) (param $0 f64) (param $1 i32) (result f64)
  (local $2 i32)
  (local $3 i64)
  (local $4 i64)
  (block $switch
   (block $switch-default
    (block $switch-case0
     (if
      (tee_local $2
       (i32.and
        (i32.wrap/i64
         (tee_local $4
          (i64.shr_u
           (tee_local $3
            (i64.reinterpret/f64
             (get_local $0)
            )
           )
           (i64.const 52)
          )
         )
        )
        (i32.const 2047)
       )
      )
      (if
       (i32.eq
        (get_local $2)
        (i32.const 2047)
       )
       (br $switch)
       (br $switch-default)
      )
     )
     (i32.store
      (get_local $1)
      (tee_local $2
       (if (result i32)
        (f64.ne
         (get_local $0)
         (f64.const 0)
        )
        (block (result i32)
         (set_local $0
          (call $_frexp
           (f64.mul
            (get_local $0)
            (f64.const 18446744073709551615)
           )
           (get_local $1)
          )
         )
         (i32.add
          (i32.load
           (get_local $1)
          )
          (i32.const -64)
         )
        )
        (i32.const 0)
       )
      )
     )
     (br $switch)
    )
   )
   (i32.store
    (get_local $1)
    (i32.add
     (i32.and
      (i32.wrap/i64
       (get_local $4)
      )
      (i32.const 2047)
     )
     (i32.const -1022)
    )
   )
   (set_local $0
    (f64.reinterpret/i64
     (i64.or
      (i64.and
       (get_local $3)
       (i64.const -9218868437227405313)
      )
      (i64.const 4602678819172646912)
     )
    )
   )
  )
  (get_local $0)
 )
 (func $___fwritex (; 60 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (block $label$break$L5
   (block $__rjti$0
    (br_if $__rjti$0
     (tee_local $3
      (i32.load
       (tee_local $4
        (i32.add
         (get_local $2)
         (i32.const 16)
        )
       )
      )
     )
    )
    (set_local $2
     (if (result i32)
      (call $___towrite
       (get_local $2)
      )
      (i32.const 0)
      (block
       (set_local $3
        (i32.load
         (get_local $4)
        )
       )
       (br $__rjti$0)
      )
     )
    )
    (br $label$break$L5)
   )
   (if
    (i32.lt_u
     (i32.sub
      (get_local $3)
      (tee_local $4
       (i32.load
        (tee_local $5
         (i32.add
          (get_local $2)
          (i32.const 20)
         )
        )
       )
      )
     )
     (get_local $1)
    )
    (block
     (set_local $2
      (call_indirect (type $FUNCSIG$iiii)
       (get_local $2)
       (get_local $0)
       (get_local $1)
       (i32.add
        (i32.and
         (i32.load offset=36
          (get_local $2)
         )
         (i32.const 3)
        )
        (i32.const 2)
       )
      )
     )
     (br $label$break$L5)
    )
   )
   (block $label$break$L10
    (if
     (i32.or
      (i32.lt_s
       (i32.load8_s offset=75
        (get_local $2)
       )
       (i32.const 0)
      )
      (i32.eqz
       (get_local $1)
      )
     )
     (set_local $3
      (i32.const 0)
     )
     (block
      (set_local $3
       (get_local $1)
      )
      (loop $while-in
       (if
        (i32.ne
         (i32.load8_s
          (i32.add
           (get_local $0)
           (tee_local $6
            (i32.add
             (get_local $3)
             (i32.const -1)
            )
           )
          )
         )
         (i32.const 10)
        )
        (if
         (get_local $6)
         (block
          (set_local $3
           (get_local $6)
          )
          (br $while-in)
         )
         (block
          (set_local $3
           (i32.const 0)
          )
          (br $label$break$L10)
         )
        )
       )
      )
      (br_if $label$break$L5
       (i32.lt_u
        (tee_local $2
         (call_indirect (type $FUNCSIG$iiii)
          (get_local $2)
          (get_local $0)
          (get_local $3)
          (i32.add
           (i32.and
            (i32.load offset=36
             (get_local $2)
            )
            (i32.const 3)
           )
           (i32.const 2)
          )
         )
        )
        (get_local $3)
       )
      )
      (set_local $0
       (i32.add
        (get_local $0)
        (get_local $3)
       )
      )
      (set_local $1
       (i32.sub
        (get_local $1)
        (get_local $3)
       )
      )
      (set_local $4
       (i32.load
        (get_local $5)
       )
      )
     )
    )
   )
   (drop
    (call $_memcpy
     (get_local $4)
     (get_local $0)
     (get_local $1)
    )
   )
   (i32.store
    (get_local $5)
    (i32.add
     (i32.load
      (get_local $5)
     )
     (get_local $1)
    )
   )
   (set_local $2
    (i32.add
     (get_local $3)
     (get_local $1)
    )
   )
  )
  (get_local $2)
 )
 (func $___towrite (; 61 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (set_local $1
   (i32.load8_s
    (tee_local $2
     (i32.add
      (get_local $0)
      (i32.const 74)
     )
    )
   )
  )
  (i32.store8
   (get_local $2)
   (i32.or
    (i32.add
     (get_local $1)
     (i32.const 255)
    )
    (get_local $1)
   )
  )
  (tee_local $0
   (if (result i32)
    (i32.and
     (tee_local $1
      (i32.load
       (get_local $0)
      )
     )
     (i32.const 8)
    )
    (block (result i32)
     (i32.store
      (get_local $0)
      (i32.or
       (get_local $1)
       (i32.const 32)
      )
     )
     (i32.const -1)
    )
    (block (result i32)
     (i32.store offset=8
      (get_local $0)
      (i32.const 0)
     )
     (i32.store offset=4
      (get_local $0)
      (i32.const 0)
     )
     (i32.store offset=28
      (get_local $0)
      (tee_local $1
       (i32.load offset=44
        (get_local $0)
       )
      )
     )
     (i32.store offset=20
      (get_local $0)
      (get_local $1)
     )
     (i32.store offset=16
      (get_local $0)
      (i32.add
       (get_local $1)
       (i32.load offset=48
        (get_local $0)
       )
      )
     )
     (i32.const 0)
    )
   )
  )
 )
 (func $_sn_write (; 62 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (set_local $3
   (select
    (get_local $2)
    (tee_local $3
     (i32.sub
      (i32.load offset=16
       (get_local $0)
      )
      (tee_local $4
       (i32.load
        (tee_local $0
         (i32.add
          (get_local $0)
          (i32.const 20)
         )
        )
       )
      )
     )
    )
    (i32.gt_u
     (get_local $3)
     (get_local $2)
    )
   )
  )
  (drop
   (call $_memcpy
    (get_local $4)
    (get_local $1)
    (get_local $3)
   )
  )
  (i32.store
   (get_local $0)
   (i32.add
    (i32.load
     (get_local $0)
    )
    (get_local $3)
   )
  )
  (get_local $2)
 )
 (func $_sprintf (; 63 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (set_local $3
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 16)
   )
  )
  (i32.store
   (get_local $3)
   (get_local $2)
  )
  (set_local $0
   (call $_vsprintf
    (get_local $0)
    (get_local $1)
    (get_local $3)
   )
  )
  (set_global $STACKTOP
   (get_local $3)
  )
  (get_local $0)
 )
 (func $_vsprintf (; 64 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (call $_vsnprintf
   (get_local $0)
   (i32.const 2147483647)
   (get_local $1)
   (get_local $2)
  )
 )
 (func $_fputs (; 65 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (i32.shr_s
   (i32.shl
    (i32.ne
     (call $_fwrite
      (get_local $0)
      (i32.const 1)
      (tee_local $2
       (call $_strlen
        (get_local $0)
       )
      )
      (get_local $1)
     )
     (get_local $2)
    )
    (i32.const 31)
   )
   (i32.const 31)
  )
 )
 (func $_fwrite (; 66 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (result i32)
  (local $4 i32)
  (local $5 i32)
  (set_local $4
   (i32.mul
    (get_local $2)
    (get_local $1)
   )
  )
  (if
   (i32.gt_s
    (i32.load offset=76
     (get_local $3)
    )
    (i32.const -1)
   )
   (block
    (set_local $5
     (i32.eqz
      (call $___lockfile
       (get_local $3)
      )
     )
    )
    (set_local $0
     (call $___fwritex
      (get_local $0)
      (get_local $4)
      (get_local $3)
     )
    )
    (if
     (i32.eqz
      (get_local $5)
     )
     (call $___unlockfile
      (get_local $3)
     )
    )
   )
   (set_local $0
    (call $___fwritex
     (get_local $0)
     (get_local $4)
     (get_local $3)
    )
   )
  )
  (set_local $2
   (select
    (get_local $2)
    (i32.const 0)
    (get_local $1)
   )
  )
  (if
   (i32.ne
    (get_local $0)
    (get_local $4)
   )
   (set_local $2
    (i32.div_u
     (get_local $0)
     (get_local $1)
    )
   )
  )
  (get_local $2)
 )
 (func $runPostSets (; 67 ;) (; has Stack IR ;)
  (nop)
 )
 (func $_llvm_bswap_i32 (; 68 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (i32.or
   (i32.or
    (i32.or
     (i32.shl
      (i32.and
       (get_local $0)
       (i32.const 255)
      )
      (i32.const 24)
     )
     (i32.shl
      (i32.and
       (i32.shr_s
        (get_local $0)
        (i32.const 8)
       )
       (i32.const 255)
      )
      (i32.const 16)
     )
    )
    (i32.shl
     (i32.and
      (i32.shr_s
       (get_local $0)
       (i32.const 16)
      )
      (i32.const 255)
     )
     (i32.const 8)
    )
   )
   (i32.shr_u
    (get_local $0)
    (i32.const 24)
   )
  )
 )
 (func $_memcpy (; 69 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (if
   (i32.ge_s
    (get_local $2)
    (i32.const 8192)
   )
   (return
    (call $_emscripten_memcpy_big
     (get_local $0)
     (get_local $1)
     (get_local $2)
    )
   )
  )
  (set_local $4
   (get_local $0)
  )
  (set_local $3
   (i32.add
    (get_local $0)
    (get_local $2)
   )
  )
  (if
   (i32.eq
    (i32.and
     (get_local $0)
     (i32.const 3)
    )
    (i32.and
     (get_local $1)
     (i32.const 3)
    )
   )
   (block
    (loop $while-in
     (if
      (i32.and
       (get_local $0)
       (i32.const 3)
      )
      (block
       (if
        (i32.eqz
         (get_local $2)
        )
        (return
         (get_local $4)
        )
       )
       (i32.store8
        (get_local $0)
        (i32.load8_s
         (get_local $1)
        )
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 1)
        )
       )
       (set_local $1
        (i32.add
         (get_local $1)
         (i32.const 1)
        )
       )
       (set_local $2
        (i32.sub
         (get_local $2)
         (i32.const 1)
        )
       )
       (br $while-in)
      )
     )
    )
    (set_local $5
     (i32.add
      (tee_local $2
       (i32.and
        (get_local $3)
        (i32.const -4)
       )
      )
      (i32.const -64)
     )
    )
    (loop $while-in1
     (if
      (i32.le_s
       (get_local $0)
       (get_local $5)
      )
      (block
       (i32.store
        (get_local $0)
        (i32.load
         (get_local $1)
        )
       )
       (i32.store offset=4
        (get_local $0)
        (i32.load offset=4
         (get_local $1)
        )
       )
       (i32.store offset=8
        (get_local $0)
        (i32.load offset=8
         (get_local $1)
        )
       )
       (i32.store offset=12
        (get_local $0)
        (i32.load offset=12
         (get_local $1)
        )
       )
       (i32.store offset=16
        (get_local $0)
        (i32.load offset=16
         (get_local $1)
        )
       )
       (i32.store offset=20
        (get_local $0)
        (i32.load offset=20
         (get_local $1)
        )
       )
       (i32.store offset=24
        (get_local $0)
        (i32.load offset=24
         (get_local $1)
        )
       )
       (i32.store offset=28
        (get_local $0)
        (i32.load offset=28
         (get_local $1)
        )
       )
       (i32.store offset=32
        (get_local $0)
        (i32.load offset=32
         (get_local $1)
        )
       )
       (i32.store offset=36
        (get_local $0)
        (i32.load offset=36
         (get_local $1)
        )
       )
       (i32.store offset=40
        (get_local $0)
        (i32.load offset=40
         (get_local $1)
        )
       )
       (i32.store offset=44
        (get_local $0)
        (i32.load offset=44
         (get_local $1)
        )
       )
       (i32.store offset=48
        (get_local $0)
        (i32.load offset=48
         (get_local $1)
        )
       )
       (i32.store offset=52
        (get_local $0)
        (i32.load offset=52
         (get_local $1)
        )
       )
       (i32.store offset=56
        (get_local $0)
        (i32.load offset=56
         (get_local $1)
        )
       )
       (i32.store offset=60
        (get_local $0)
        (i32.load offset=60
         (get_local $1)
        )
       )
       (set_local $0
        (i32.sub
         (get_local $0)
         (i32.const -64)
        )
       )
       (set_local $1
        (i32.sub
         (get_local $1)
         (i32.const -64)
        )
       )
       (br $while-in1)
      )
     )
    )
    (loop $while-in3
     (if
      (i32.lt_s
       (get_local $0)
       (get_local $2)
      )
      (block
       (i32.store
        (get_local $0)
        (i32.load
         (get_local $1)
        )
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 4)
        )
       )
       (set_local $1
        (i32.add
         (get_local $1)
         (i32.const 4)
        )
       )
       (br $while-in3)
      )
     )
    )
   )
   (block
    (set_local $2
     (i32.sub
      (get_local $3)
      (i32.const 4)
     )
    )
    (loop $while-in5
     (if
      (i32.lt_s
       (get_local $0)
       (get_local $2)
      )
      (block
       (i32.store8
        (get_local $0)
        (i32.load8_s
         (get_local $1)
        )
       )
       (i32.store8 offset=1
        (get_local $0)
        (i32.load8_s offset=1
         (get_local $1)
        )
       )
       (i32.store8 offset=2
        (get_local $0)
        (i32.load8_s offset=2
         (get_local $1)
        )
       )
       (i32.store8 offset=3
        (get_local $0)
        (i32.load8_s offset=3
         (get_local $1)
        )
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 4)
        )
       )
       (set_local $1
        (i32.add
         (get_local $1)
         (i32.const 4)
        )
       )
       (br $while-in5)
      )
     )
    )
   )
  )
  (loop $while-in7
   (if
    (i32.lt_s
     (get_local $0)
     (get_local $3)
    )
    (block
     (i32.store8
      (get_local $0)
      (i32.load8_s
       (get_local $1)
      )
     )
     (set_local $0
      (i32.add
       (get_local $0)
       (i32.const 1)
      )
     )
     (set_local $1
      (i32.add
       (get_local $1)
       (i32.const 1)
      )
     )
     (br $while-in7)
    )
   )
  )
  (get_local $4)
 )
 (func $_memset (; 70 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (set_local $4
   (i32.add
    (get_local $0)
    (get_local $2)
   )
  )
  (set_local $1
   (i32.and
    (get_local $1)
    (i32.const 255)
   )
  )
  (if
   (i32.ge_s
    (get_local $2)
    (i32.const 67)
   )
   (block
    (loop $while-in
     (if
      (i32.and
       (get_local $0)
       (i32.const 3)
      )
      (block
       (i32.store8
        (get_local $0)
        (get_local $1)
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 1)
        )
       )
       (br $while-in)
      )
     )
    )
    (set_local $6
     (i32.add
      (tee_local $5
       (i32.and
        (get_local $4)
        (i32.const -4)
       )
      )
      (i32.const -64)
     )
    )
    (set_local $3
     (i32.or
      (i32.or
       (i32.or
        (get_local $1)
        (i32.shl
         (get_local $1)
         (i32.const 8)
        )
       )
       (i32.shl
        (get_local $1)
        (i32.const 16)
       )
      )
      (i32.shl
       (get_local $1)
       (i32.const 24)
      )
     )
    )
    (loop $while-in1
     (if
      (i32.le_s
       (get_local $0)
       (get_local $6)
      )
      (block
       (i32.store
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=4
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=8
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=12
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=16
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=20
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=24
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=28
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=32
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=36
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=40
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=44
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=48
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=52
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=56
        (get_local $0)
        (get_local $3)
       )
       (i32.store offset=60
        (get_local $0)
        (get_local $3)
       )
       (set_local $0
        (i32.sub
         (get_local $0)
         (i32.const -64)
        )
       )
       (br $while-in1)
      )
     )
    )
    (loop $while-in3
     (if
      (i32.lt_s
       (get_local $0)
       (get_local $5)
      )
      (block
       (i32.store
        (get_local $0)
        (get_local $3)
       )
       (set_local $0
        (i32.add
         (get_local $0)
         (i32.const 4)
        )
       )
       (br $while-in3)
      )
     )
    )
   )
  )
  (loop $while-in5
   (if
    (i32.lt_s
     (get_local $0)
     (get_local $4)
    )
    (block
     (i32.store8
      (get_local $0)
      (get_local $1)
     )
     (set_local $0
      (i32.add
       (get_local $0)
       (i32.const 1)
      )
     )
     (br $while-in5)
    )
   )
  )
  (i32.sub
   (get_local $4)
   (get_local $2)
  )
 )
 (func $_sbrk (; 71 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (if
   (i32.or
    (i32.and
     (i32.gt_s
      (get_local $0)
      (i32.const 0)
     )
     (i32.lt_s
      (tee_local $0
       (i32.add
        (tee_local $1
         (i32.load
          (get_global $DYNAMICTOP_PTR)
         )
        )
        (get_local $0)
       )
      )
      (get_local $1)
     )
    )
    (i32.lt_s
     (get_local $0)
     (i32.const 0)
    )
   )
   (block
    (drop
     (call $abortOnCannotGrowMemory)
    )
    (call $___setErrNo
     (i32.const 12)
    )
    (return
     (i32.const -1)
    )
   )
  )
  (i32.store
   (get_global $DYNAMICTOP_PTR)
   (get_local $0)
  )
  (set_local $2
   (call $getTotalMemory)
  )
  (if
   (i32.gt_s
    (get_local $0)
    (get_local $2)
   )
   (if
    (i32.eqz
     (call $enlargeMemory)
    )
    (block
     (i32.store
      (get_global $DYNAMICTOP_PTR)
      (get_local $1)
     )
     (call $___setErrNo
      (i32.const 12)
     )
     (return
      (i32.const -1)
     )
    )
   )
  )
  (get_local $1)
 )
 (func $dynCall_ii (; 72 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (result i32)
  (call_indirect (type $FUNCSIG$ii)
   (get_local $1)
   (i32.and
    (get_local $0)
    (i32.const 1)
   )
  )
 )
 (func $dynCall_iiii (; 73 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (result i32)
  (call_indirect (type $FUNCSIG$iiii)
   (get_local $1)
   (get_local $2)
   (get_local $3)
   (i32.add
    (i32.and
     (get_local $0)
     (i32.const 3)
    )
    (i32.const 2)
   )
  )
 )
 (func $b0 (; 74 ;) (; has Stack IR ;) (param $0 i32) (result i32)
  (call $abort
   (i32.const 0)
  )
  (i32.const 0)
 )
 (func $b1 (; 75 ;) (; has Stack IR ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (call $abort
   (i32.const 1)
  )
  (i32.const 0)
 )
)
