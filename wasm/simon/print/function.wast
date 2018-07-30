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
 (import "env" "___syscall54" (func $___syscall54 (param i32 i32) (result i32)))
 (import "env" "___syscall6" (func $___syscall6 (param i32 i32) (result i32)))
 (import "env" "_abort" (func $_abort))
 (import "env" "_emscripten_memcpy_big" (func $_emscripten_memcpy_big (param i32 i32 i32) (result i32)))
 (global $DYNAMICTOP_PTR (mut i32) (get_global $DYNAMICTOP_PTR$asm2wasm$import))
 (global $STACKTOP (mut i32) (get_global $STACKTOP$asm2wasm$import))
 (global $STACK_MAX (mut i32) (get_global $STACK_MAX$asm2wasm$import))
 (global $__THREW__ (mut i32) (i32.const 0))
 (global $threwValue (mut i32) (i32.const 0))
 (global $tempRet0 (mut i32) (i32.const 0))
 (elem (get_global $tableBase) $b0 $___stdio_close $b1 $___stdout_write $___stdio_seek $___stdio_write)
 (data (i32.const 1024) "\04\04\00\00\05")
 (data (i32.const 1040) "\01")
 (data (i32.const 1064) "\01\00\00\00\02\00\00\00\98\04\00\00\00\04")
 (data (i32.const 1088) "\01")
 (data (i32.const 1103) "\n\ff\ff\ff\ff")
 (data (i32.const 1152) "FOOBAR")
 (export "___errno_location" (func $___errno_location))
 (export "_free" (func $_free))
 (export "_main" (func $_main))
 (export "_malloc" (func $_malloc))
 (export "_memcpy" (func $_memcpy))
 (export "_memset" (func $_memset))
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
 (func $stackAlloc (; 11 ;) (param $0 i32) (result i32)
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
 (func $stackSave (; 12 ;) (result i32)
  (get_global $STACKTOP)
 )
 (func $stackRestore (; 13 ;) (param $0 i32)
  (set_global $STACKTOP
   (get_local $0)
  )
 )
 (func $establishStackSpace (; 14 ;) (param $0 i32) (param $1 i32)
  (set_global $STACKTOP
   (get_local $0)
  )
  (set_global $STACK_MAX
   (get_local $1)
  )
 )
 (func $setThrew (; 15 ;) (param $0 i32) (param $1 i32)
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
 (func $setTempRet0 (; 16 ;) (param $0 i32)
  (set_global $tempRet0
   (get_local $0)
  )
 )
 (func $getTempRet0 (; 17 ;) (result i32)
  (get_global $tempRet0)
 )
 (func $_main (; 18 ;) (result i32)
  ;;@ ../func/function_print.c:4:0
  (drop
   (call $_puts
    (i32.const 1152)
   )
  )
  ;;@ ../func/function_print.c:5:0
  (i32.const 0)
 )
 (func $_malloc (; 19 ;) (param $0 i32) (result i32)
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
  (local $23 i32)
  (block $folding-inner1
   (block $folding-inner0
    (set_local $14
     (get_global $STACKTOP)
    )
    (set_global $STACKTOP
     (i32.add
      (get_global $STACKTOP)
      (i32.const 16)
     )
    )
    (set_local $23
     (get_local $14)
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
              (i32.const 2208)
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
                      (i32.const 2248)
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
            (i32.const 2208)
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
               (i32.const 2224)
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
            (i32.const 2216)
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
                        (i32.const 2248)
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
              (i32.const 2208)
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
                 (i32.const 2224)
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
                (i32.const 2228)
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
                (i32.const 2248)
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
                  (i32.const 2224)
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
                 (i32.const 2208)
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
             (i32.const 2216)
             (get_local $5)
            )
            (i32.store
             (i32.const 2228)
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
             (i32.const 2212)
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
            (set_local $3
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
                (i32.const 2512)
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
               (i32.eqz
                (tee_local $0
                 (i32.load offset=16
                  (get_local $3)
                 )
                )
               )
               (br_if $while-out
                (i32.eqz
                 (tee_local $0
                  (i32.load offset=20
                   (get_local $3)
                  )
                 )
                )
               )
              )
              (set_local $3
               (get_local $0)
              )
              (set_local $4
               (select
                (get_local $0)
                (get_local $4)
                (tee_local $6
                 (i32.lt_u
                  (tee_local $0
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
                (get_local $0)
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
                (i32.const 2224)
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
                    (i32.const 2512)
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
                    (i32.const 2212)
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
                   (i32.const 2224)
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
                   (i32.const 2224)
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
                   (i32.const 2224)
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
                  (i32.const 2228)
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
                  (i32.const 2248)
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
                    (i32.const 2224)
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
                   (i32.const 2208)
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
               (i32.const 2216)
               (get_local $10)
              )
              (i32.store
               (i32.const 2228)
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
            (i32.const 2212)
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
                 (i32.const 2512)
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
                    (set_local $1
                     (get_local $0)
                    )
                    (set_local $2
                     (i32.const 0)
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
             (if
              (i32.eqz
               (i32.or
                (get_local $5)
                (get_local $0)
               )
              )
              (block
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
               (set_local $5
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
                  (i32.const 2512)
                 )
                )
               )
              )
             )
             (set_local $5
              (if (result i32)
               (get_local $5)
               (block
                (set_local $1
                 (get_local $0)
                )
                (set_local $0
                 (get_local $5)
                )
                (br $__rjti$1)
               )
               (get_local $0)
              )
             )
             (br $__rjto$1)
            )
            (set_local $5
             (get_local $1)
            )
            (set_local $1
             (get_local $2)
            )
            (loop $while-in17
             (set_local $12
              (i32.load offset=4
               (get_local $0)
              )
             )
             (if
              (i32.eqz
               (tee_local $2
                (i32.load offset=16
                 (get_local $0)
                )
               )
              )
              (set_local $2
               (i32.load offset=20
                (get_local $0)
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
               (get_local $1)
              )
             )
             (set_local $1
              (select
               (get_local $16)
               (get_local $1)
               (get_local $12)
              )
             )
             (set_local $5
              (select
               (get_local $0)
               (get_local $5)
               (get_local $12)
              )
             )
             (set_local $2
              (if (result i32)
               (get_local $2)
               (block
                (set_local $0
                 (get_local $2)
                )
                (br $while-in17)
               )
               (get_local $1)
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
                (i32.const 2216)
               )
               (get_local $4)
              )
             )
             (block
              (if
               (i32.gt_u
                (tee_local $17
                 (i32.load
                  (i32.const 2224)
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
                      (i32.const 2512)
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
                      (i32.const 2212)
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
                     (i32.const 2224)
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
                     (i32.const 2224)
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
                     (i32.const 2224)
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
                     (i32.const 2248)
                    )
                   )
                   (if
                    (i32.and
                     (tee_local $3
                      (i32.load
                       (i32.const 2208)
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
                       (i32.const 2224)
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
                      (i32.const 2208)
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
                   (i32.const 2512)
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
                    (i32.const 2212)
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
                       (i32.const 2224)
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
                      (i32.const 2224)
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
        (i32.const 2216)
       )
      )
      (get_local $3)
     )
     (block
      (set_local $0
       (i32.load
        (i32.const 2228)
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
         (i32.const 2228)
         (tee_local $4
          (i32.add
           (get_local $0)
           (get_local $3)
          )
         )
        )
        (i32.store
         (i32.const 2216)
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
         (i32.const 2216)
         (i32.const 0)
        )
        (i32.store
         (i32.const 2228)
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
        (i32.const 2220)
       )
      )
      (get_local $3)
     )
     (block
      (i32.store
       (i32.const 2220)
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
             (i32.const 2680)
            )
            (i32.load
             (i32.const 2688)
            )
            (block (result i32)
             (i32.store
              (i32.const 2688)
              (i32.const 4096)
             )
             (i32.store
              (i32.const 2684)
              (i32.const 4096)
             )
             (i32.store
              (i32.const 2692)
              (i32.const -1)
             )
             (i32.store
              (i32.const 2696)
              (i32.const -1)
             )
             (i32.store
              (i32.const 2700)
              (i32.const 0)
             )
             (i32.store
              (i32.const 2652)
              (i32.const 0)
             )
             (i32.store
              (i32.const 2680)
              (i32.xor
               (i32.and
                (get_local $23)
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
       (i32.const 2648)
      )
     )
     (if
      (i32.or
       (i32.le_u
        (tee_local $10
         (i32.add
          (tee_local $2
           (i32.load
            (i32.const 2640)
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
         (i32.const 2652)
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
               (i32.const 2232)
              )
             )
            )
           )
           (set_local $2
            (i32.const 2656)
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
                         (i32.const 2684)
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
                 (i32.const 2640)
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
                  (i32.const 2648)
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
                (i32.const 2688)
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
         (i32.const 2652)
         (i32.or
          (i32.load
           (i32.const 2652)
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
      (i32.const 2640)
      (tee_local $2
       (i32.add
        (i32.load
         (i32.const 2640)
        )
        (get_local $1)
       )
      )
     )
     (if
      (i32.gt_u
       (get_local $2)
       (i32.load
        (i32.const 2644)
       )
      )
      (i32.store
       (i32.const 2644)
       (get_local $2)
      )
     )
     (block $label$break$L294
      (if
       (tee_local $6
        (i32.load
         (i32.const 2232)
        )
       )
       (block
        (set_local $2
         (i32.const 2656)
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
                 (i32.const 2220)
                )
                (get_local $1)
               )
              )
              (get_local $2)
             )
            )
            (i32.store
             (i32.const 2232)
             (get_local $0)
            )
            (i32.store
             (i32.const 2220)
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
             (i32.const 2236)
             (i32.load
              (i32.const 2696)
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
            (i32.const 2224)
           )
          )
         )
         (block
          (i32.store
           (i32.const 2224)
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
         (i32.const 2656)
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
               (i32.const 2220)
               (tee_local $0
                (i32.add
                 (i32.load
                  (i32.const 2220)
                 )
                 (get_local $4)
                )
               )
              )
              (i32.store
               (i32.const 2232)
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
                 (i32.const 2228)
                )
                (get_local $1)
               )
               (block
                (i32.store
                 (i32.const 2216)
                 (tee_local $0
                  (i32.add
                   (i32.load
                    (i32.const 2216)
                   )
                   (get_local $4)
                  )
                 )
                )
                (i32.store
                 (i32.const 2228)
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
                         (i32.const 2248)
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
                       (i32.const 2208)
                       (i32.and
                        (i32.load
                         (i32.const 2208)
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
                          (i32.const 2512)
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
                        (i32.const 2212)
                        (i32.and
                         (i32.load
                          (i32.const 2212)
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
                         (i32.const 2224)
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
                        (i32.const 2224)
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
                       (i32.const 2224)
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
                  (i32.const 2248)
                 )
                )
                (block $do-once58
                 (if
                  (i32.and
                   (tee_local $1
                    (i32.load
                     (i32.const 2208)
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
                      (i32.const 2224)
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
                    (i32.const 2208)
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
                (i32.const 2512)
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
                   (i32.const 2212)
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
                 (i32.const 2212)
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
                    (i32.const 2224)
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
                   (i32.const 2224)
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
         (i32.const 2656)
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
         (i32.const 2232)
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
         (i32.const 2220)
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
         (i32.const 2236)
         (i32.load
          (i32.const 2696)
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
          (i32.const 2656)
         )
        )
        (i64.store offset=8 align=4
         (get_local $4)
         (i64.load align=4
          (i32.const 2664)
         )
        )
        (i32.store
         (i32.const 2656)
         (get_local $0)
        )
        (i32.store
         (i32.const 2660)
         (get_local $1)
        )
        (i32.store
         (i32.const 2668)
         (i32.const 0)
        )
        (i32.store
         (i32.const 2664)
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
              (i32.const 2248)
             )
            )
            (if
             (i32.and
              (tee_local $2
               (i32.load
                (i32.const 2208)
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
                (i32.const 2224)
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
               (i32.const 2208)
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
            (i32.const 2512)
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
               (i32.const 2212)
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
             (i32.const 2212)
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
                (i32.const 2224)
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
               (i32.const 2224)
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
             (i32.const 2224)
            )
           )
          )
          (i32.lt_u
           (get_local $0)
           (get_local $2)
          )
         )
         (i32.store
          (i32.const 2224)
          (get_local $0)
         )
        )
        (i32.store
         (i32.const 2656)
         (get_local $0)
        )
        (i32.store
         (i32.const 2660)
         (get_local $1)
        )
        (i32.store
         (i32.const 2668)
         (i32.const 0)
        )
        (i32.store
         (i32.const 2244)
         (i32.load
          (i32.const 2680)
         )
        )
        (i32.store
         (i32.const 2240)
         (i32.const -1)
        )
        (i32.store
         (i32.const 2260)
         (i32.const 2248)
        )
        (i32.store
         (i32.const 2256)
         (i32.const 2248)
        )
        (i32.store
         (i32.const 2268)
         (i32.const 2256)
        )
        (i32.store
         (i32.const 2264)
         (i32.const 2256)
        )
        (i32.store
         (i32.const 2276)
         (i32.const 2264)
        )
        (i32.store
         (i32.const 2272)
         (i32.const 2264)
        )
        (i32.store
         (i32.const 2284)
         (i32.const 2272)
        )
        (i32.store
         (i32.const 2280)
         (i32.const 2272)
        )
        (i32.store
         (i32.const 2292)
         (i32.const 2280)
        )
        (i32.store
         (i32.const 2288)
         (i32.const 2280)
        )
        (i32.store
         (i32.const 2300)
         (i32.const 2288)
        )
        (i32.store
         (i32.const 2296)
         (i32.const 2288)
        )
        (i32.store
         (i32.const 2308)
         (i32.const 2296)
        )
        (i32.store
         (i32.const 2304)
         (i32.const 2296)
        )
        (i32.store
         (i32.const 2316)
         (i32.const 2304)
        )
        (i32.store
         (i32.const 2312)
         (i32.const 2304)
        )
        (i32.store
         (i32.const 2324)
         (i32.const 2312)
        )
        (i32.store
         (i32.const 2320)
         (i32.const 2312)
        )
        (i32.store
         (i32.const 2332)
         (i32.const 2320)
        )
        (i32.store
         (i32.const 2328)
         (i32.const 2320)
        )
        (i32.store
         (i32.const 2340)
         (i32.const 2328)
        )
        (i32.store
         (i32.const 2336)
         (i32.const 2328)
        )
        (i32.store
         (i32.const 2348)
         (i32.const 2336)
        )
        (i32.store
         (i32.const 2344)
         (i32.const 2336)
        )
        (i32.store
         (i32.const 2356)
         (i32.const 2344)
        )
        (i32.store
         (i32.const 2352)
         (i32.const 2344)
        )
        (i32.store
         (i32.const 2364)
         (i32.const 2352)
        )
        (i32.store
         (i32.const 2360)
         (i32.const 2352)
        )
        (i32.store
         (i32.const 2372)
         (i32.const 2360)
        )
        (i32.store
         (i32.const 2368)
         (i32.const 2360)
        )
        (i32.store
         (i32.const 2380)
         (i32.const 2368)
        )
        (i32.store
         (i32.const 2376)
         (i32.const 2368)
        )
        (i32.store
         (i32.const 2388)
         (i32.const 2376)
        )
        (i32.store
         (i32.const 2384)
         (i32.const 2376)
        )
        (i32.store
         (i32.const 2396)
         (i32.const 2384)
        )
        (i32.store
         (i32.const 2392)
         (i32.const 2384)
        )
        (i32.store
         (i32.const 2404)
         (i32.const 2392)
        )
        (i32.store
         (i32.const 2400)
         (i32.const 2392)
        )
        (i32.store
         (i32.const 2412)
         (i32.const 2400)
        )
        (i32.store
         (i32.const 2408)
         (i32.const 2400)
        )
        (i32.store
         (i32.const 2420)
         (i32.const 2408)
        )
        (i32.store
         (i32.const 2416)
         (i32.const 2408)
        )
        (i32.store
         (i32.const 2428)
         (i32.const 2416)
        )
        (i32.store
         (i32.const 2424)
         (i32.const 2416)
        )
        (i32.store
         (i32.const 2436)
         (i32.const 2424)
        )
        (i32.store
         (i32.const 2432)
         (i32.const 2424)
        )
        (i32.store
         (i32.const 2444)
         (i32.const 2432)
        )
        (i32.store
         (i32.const 2440)
         (i32.const 2432)
        )
        (i32.store
         (i32.const 2452)
         (i32.const 2440)
        )
        (i32.store
         (i32.const 2448)
         (i32.const 2440)
        )
        (i32.store
         (i32.const 2460)
         (i32.const 2448)
        )
        (i32.store
         (i32.const 2456)
         (i32.const 2448)
        )
        (i32.store
         (i32.const 2468)
         (i32.const 2456)
        )
        (i32.store
         (i32.const 2464)
         (i32.const 2456)
        )
        (i32.store
         (i32.const 2476)
         (i32.const 2464)
        )
        (i32.store
         (i32.const 2472)
         (i32.const 2464)
        )
        (i32.store
         (i32.const 2484)
         (i32.const 2472)
        )
        (i32.store
         (i32.const 2480)
         (i32.const 2472)
        )
        (i32.store
         (i32.const 2492)
         (i32.const 2480)
        )
        (i32.store
         (i32.const 2488)
         (i32.const 2480)
        )
        (i32.store
         (i32.const 2500)
         (i32.const 2488)
        )
        (i32.store
         (i32.const 2496)
         (i32.const 2488)
        )
        (i32.store
         (i32.const 2508)
         (i32.const 2496)
        )
        (i32.store
         (i32.const 2504)
         (i32.const 2496)
        )
        (i32.store
         (i32.const 2232)
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
         (i32.const 2220)
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
         (i32.const 2236)
         (i32.load
          (i32.const 2696)
         )
        )
       )
      )
     )
     (if
      (i32.gt_u
       (tee_local $0
        (i32.load
         (i32.const 2220)
        )
       )
       (get_local $3)
      )
      (block
       (i32.store
        (i32.const 2220)
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
    (i32.const 2232)
    (tee_local $2
     (i32.add
      (tee_local $0
       (i32.load
        (i32.const 2232)
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
 (func $_free (; 20 ;) (param $0 i32)
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
      (i32.const 2224)
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
        (i32.const 2228)
       )
       (get_local $0)
      )
      (block
       (if
        (i32.ne
         (i32.and
          (tee_local $3
           (i32.load
            (tee_local $1
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
         (set_local $3
          (get_local $0)
         )
         (set_local $1
          (get_local $4)
         )
         (set_local $5
          (get_local $0)
         )
         (br $label$break$L10)
        )
       )
       (i32.store
        (i32.const 2216)
        (get_local $4)
       )
       (i32.store
        (get_local $1)
        (i32.and
         (get_local $3)
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
       (set_local $3
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
         (tee_local $1
          (i32.add
           (i32.shl
            (get_local $2)
            (i32.const 3)
           )
           (i32.const 2248)
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
         (get_local $3)
         (get_local $5)
        )
        (block
         (i32.store
          (i32.const 2208)
          (i32.and
           (i32.load
            (i32.const 2208)
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
         (set_local $3
          (get_local $0)
         )
         (set_local $1
          (get_local $4)
         )
         (set_local $5
          (get_local $0)
         )
         (br $label$break$L10)
        )
       )
       (if
        (i32.eq
         (get_local $3)
         (get_local $1)
        )
        (set_local $6
         (i32.add
          (get_local $3)
          (i32.const 8)
         )
        )
        (block
         (if
          (i32.gt_u
           (get_local $12)
           (get_local $3)
          )
          (call $_abort)
         )
         (if
          (i32.eq
           (i32.load
            (tee_local $1
             (i32.add
              (get_local $3)
              (i32.const 8)
             )
            )
           )
           (get_local $0)
          )
          (set_local $6
           (get_local $1)
          )
          (call $_abort)
         )
        )
       )
       (i32.store offset=12
        (get_local $5)
        (get_local $3)
       )
       (i32.store
        (get_local $6)
        (get_local $5)
       )
       (set_local $3
        (get_local $0)
       )
       (set_local $1
        (get_local $4)
       )
       (set_local $5
        (get_local $0)
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
            (i32.const 2512)
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
            (i32.const 2212)
            (i32.and
             (i32.load
              (i32.const 2212)
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
           (set_local $3
            (get_local $0)
           )
           (set_local $1
            (get_local $4)
           )
           (set_local $5
            (get_local $0)
           )
           (br $label$break$L10)
          )
         )
        )
        (if
         (i32.gt_u
          (i32.load
           (i32.const 2224)
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
            (set_local $3
             (get_local $0)
            )
            (set_local $1
             (get_local $4)
            )
            (set_local $5
             (get_local $0)
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
           (i32.const 2224)
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
           (i32.const 2224)
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
          (set_local $3
           (get_local $0)
          )
          (set_local $1
           (get_local $4)
          )
          (set_local $5
           (get_local $0)
          )
         )
        )
        (block
         (set_local $3
          (get_local $0)
         )
         (set_local $1
          (get_local $4)
         )
         (set_local $5
          (get_local $0)
         )
        )
       )
      )
      (block
       (set_local $3
        (get_local $0)
       )
       (set_local $1
        (get_local $4)
       )
       (set_local $5
        (get_local $0)
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
          (i32.const 2232)
         )
         (get_local $7)
        )
        (block
         (i32.store
          (i32.const 2220)
          (tee_local $0
           (i32.add
            (i32.load
             (i32.const 2220)
            )
            (get_local $1)
           )
          )
         )
         (i32.store
          (i32.const 2232)
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
            (i32.const 2228)
           )
          )
          (return)
         )
         (i32.store
          (i32.const 2228)
          (i32.const 0)
         )
         (i32.store
          (i32.const 2216)
          (i32.const 0)
         )
         (return)
        )
       )
       (if
        (i32.eq
         (i32.load
          (i32.const 2228)
         )
         (get_local $7)
        )
        (block
         (i32.store
          (i32.const 2216)
          (tee_local $0
           (i32.add
            (i32.load
             (i32.const 2216)
            )
            (get_local $1)
           )
          )
         )
         (i32.store
          (i32.const 2228)
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
              (i32.const 2248)
             )
            )
           )
           (block
            (if
             (i32.gt_u
              (i32.load
               (i32.const 2224)
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
             (i32.const 2208)
             (i32.and
              (i32.load
               (i32.const 2208)
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
               (i32.const 2224)
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
                (i32.const 2224)
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
                (i32.const 2224)
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
                 (i32.const 2512)
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
                 (i32.const 2212)
                 (i32.and
                  (i32.load
                   (i32.const 2212)
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
                (i32.const 2224)
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
                (i32.const 2224)
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
                (i32.const 2224)
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
          (i32.const 2228)
         )
        )
        (block
         (i32.store
          (i32.const 2216)
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
      (i32.const 2248)
     )
    )
    (if
     (i32.and
      (tee_local $5
       (i32.load
        (i32.const 2208)
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
        (i32.const 2224)
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
       (i32.const 2208)
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
    (i32.const 2512)
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
       (i32.const 2212)
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
           (i32.const 2224)
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
          (i32.const 2224)
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
      (i32.const 2212)
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
   (i32.const 2240)
   (tee_local $0
    (i32.add
     (i32.load
      (i32.const 2240)
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
   (i32.const 2664)
  )
  (loop $while-in19
   (set_local $0
    (i32.add
     (tee_local $1
      (i32.load
       (get_local $0)
      )
     )
     (i32.const 8)
    )
   )
   (br_if $while-in19
    (get_local $1)
   )
  )
  (i32.store
   (i32.const 2240)
   (i32.const -1)
  )
 )
 (func $___stdio_close (; 21 ;) (param $0 i32) (result i32)
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
   (call $_dummy_314
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
 (func $___stdout_write (; 22 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (set_local $4
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 32)
   )
  )
  (set_local $5
   (i32.add
    (tee_local $3
     (get_local $4)
    )
    (i32.const 16)
   )
  )
  (i32.store offset=36
   (get_local $0)
   (i32.const 3)
  )
  (if
   (i32.eqz
    (i32.and
     (i32.load
      (get_local $0)
     )
     (i32.const 64)
    )
   )
   (block
    (i32.store
     (get_local $3)
     (i32.load offset=60
      (get_local $0)
     )
    )
    (i32.store offset=4
     (get_local $3)
     (i32.const 21523)
    )
    (i32.store offset=8
     (get_local $3)
     (get_local $5)
    )
    (if
     (call $___syscall54
      (i32.const 54)
      (get_local $3)
     )
     (i32.store8 offset=75
      (get_local $0)
      (i32.const -1)
     )
    )
   )
  )
  (set_local $0
   (call $___stdio_write
    (get_local $0)
    (get_local $1)
    (get_local $2)
   )
  )
  (set_global $STACKTOP
   (get_local $4)
  )
  (get_local $0)
 )
 (func $___stdio_seek (; 23 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (local $3 i32)
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
  (i32.store
   (tee_local $3
    (get_local $4)
   )
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
   (get_local $4)
  )
  (get_local $0)
 )
 (func $___syscall_ret (; 24 ;) (param $0 i32) (result i32)
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
 (func $___errno_location (; 25 ;) (result i32)
  (i32.const 2704)
 )
 (func $___stdio_write (; 26 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
 (func $_dummy_314 (; 27 ;) (param $0 i32) (result i32)
  (get_local $0)
 )
 (func $_strlen (; 28 ;) (param $0 i32) (result i32)
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
     (set_local $1
      (get_local $0)
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
 (func $___lockfile (; 29 ;) (param $0 i32) (result i32)
  (i32.const 0)
 )
 (func $___unlockfile (; 30 ;) (param $0 i32)
  (nop)
 )
 (func $___fwritex (; 31 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
   (set_local $2
    (block $label$break$L10 (result i32)
     (if (result i32)
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
      (i32.const 0)
      (block (result i32)
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
          (br $label$break$L10
           (i32.const 0)
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
       (get_local $3)
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
     (get_local $2)
     (get_local $1)
    )
   )
  )
  (get_local $2)
 )
 (func $___towrite (; 32 ;) (param $0 i32) (result i32)
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
 (func $_fwrite (; 33 ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (result i32)
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
 (func $___overflow (; 34 ;) (param $0 i32) (param $1 i32) (result i32)
  (local $2 i32)
  (local $3 i32)
  (local $4 i32)
  (local $5 i32)
  (local $6 i32)
  (local $7 i32)
  (set_local $3
   (get_global $STACKTOP)
  )
  (set_global $STACKTOP
   (i32.add
    (get_global $STACKTOP)
    (i32.const 16)
   )
  )
  (i32.store8
   (tee_local $4
    (get_local $3)
   )
   (tee_local $7
    (i32.and
     (get_local $1)
     (i32.const 255)
    )
   )
  )
  (block $do-once
   (block $__rjti$0
    (br_if $__rjti$0
     (tee_local $5
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
    (set_local $1
     (if (result i32)
      (call $___towrite
       (get_local $0)
      )
      (i32.const -1)
      (block
       (set_local $5
        (i32.load
         (get_local $2)
        )
       )
       (br $__rjti$0)
      )
     )
    )
    (br $do-once)
   )
   (if
    (i32.lt_u
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
     (get_local $5)
    )
    (if
     (i32.ne
      (tee_local $1
       (i32.and
        (get_local $1)
        (i32.const 255)
       )
      )
      (i32.load8_s offset=75
       (get_local $0)
      )
     )
     (block
      (i32.store
       (get_local $2)
       (i32.add
        (get_local $6)
        (i32.const 1)
       )
      )
      (i32.store8
       (get_local $6)
       (get_local $7)
      )
      (br $do-once)
     )
    )
   )
   (set_local $1
    (if (result i32)
     (i32.eq
      (call_indirect (type $FUNCSIG$iiii)
       (get_local $0)
       (get_local $4)
       (i32.const 1)
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
      (i32.const 1)
     )
     (i32.load8_u
      (get_local $4)
     )
     (i32.const -1)
    )
   )
  )
  (set_global $STACKTOP
   (get_local $3)
  )
  (get_local $1)
 )
 (func $_fputs (; 35 ;) (param $0 i32) (param $1 i32) (result i32)
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
 (func $_puts (; 36 ;) (param $0 i32) (result i32)
  (local $1 i32)
  (local $2 i32)
  (local $3 i32)
  (set_local $2
   (if (result i32)
    (i32.gt_s
     (i32.load offset=76
      (tee_local $1
       (i32.load
        (i32.const 1024)
       )
      )
     )
     (i32.const -1)
    )
    (call $___lockfile
     (get_local $1)
    )
    (i32.const 0)
   )
  )
  (set_local $0
   (block $do-once (result i32)
    (if (result i32)
     (i32.lt_s
      (call $_fputs
       (get_local $0)
       (get_local $1)
      )
      (i32.const 0)
     )
     (i32.const -1)
     (block (result i32)
      (if
       (i32.ne
        (i32.load8_s offset=75
         (get_local $1)
        )
        (i32.const 10)
       )
       (if
        (i32.lt_u
         (tee_local $0
          (i32.load
           (tee_local $3
            (i32.add
             (get_local $1)
             (i32.const 20)
            )
           )
          )
         )
         (i32.load offset=16
          (get_local $1)
         )
        )
        (block
         (i32.store
          (get_local $3)
          (i32.add
           (get_local $0)
           (i32.const 1)
          )
         )
         (i32.store8
          (get_local $0)
          (i32.const 10)
         )
         (br $do-once
          (i32.const 0)
         )
        )
       )
      )
      (i32.shr_s
       (call $___overflow
        (get_local $1)
        (i32.const 10)
       )
       (i32.const 31)
      )
     )
    )
   )
  )
  (if
   (get_local $2)
   (call $___unlockfile
    (get_local $1)
   )
  )
  (get_local $0)
 )
 (func $runPostSets (; 37 ;)
  (nop)
 )
 (func $_memcpy (; 38 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
 (func $_memset (; 39 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
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
 (func $_sbrk (; 40 ;) (param $0 i32) (result i32)
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
 (func $dynCall_ii (; 41 ;) (param $0 i32) (param $1 i32) (result i32)
  (call_indirect (type $FUNCSIG$ii)
   (get_local $1)
   (i32.and
    (get_local $0)
    (i32.const 1)
   )
  )
 )
 (func $dynCall_iiii (; 42 ;) (param $0 i32) (param $1 i32) (param $2 i32) (param $3 i32) (result i32)
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
 (func $b0 (; 43 ;) (param $0 i32) (result i32)
  (call $abort
   (i32.const 0)
  )
  (i32.const 0)
 )
 (func $b1 (; 44 ;) (param $0 i32) (param $1 i32) (param $2 i32) (result i32)
  (call $abort
   (i32.const 1)
  )
  (i32.const 0)
 )
)
