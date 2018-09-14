(module
  (type $0 (func (param i32) (result i32)))
  (type $1 (func (param i32 i32 i32 i32) (result i32)))
  (import "env" "memory" (memory $2 256 256))
  (import "env" "STACKTOP" (global $3 i32))
  (export "_exec" (func $5))
  (export "_run" (func $6))
  (global $4  (mut i32) (get_global $3))
  
  (func $5 (type $0)
    (param $0 i32)
    (result i32)
    (local $1 i32)
    get_local $0
    i32.const 4
    i32.add
    set_local $1
    i32.const 0
    set_local $0
    loop $loop
      get_local $0
      i32.const 5
      i32.ne
      if $if
        get_local $1
        i32.load
        get_local $0
        i32.add
        get_local $0
        i32.store8
        get_local $0
        i32.const 1
        i32.add
        set_local $0
        br $loop
      end ;; $if
    end ;; $loop
    i32.const 0
    )
  
  (func $6 (type $1)
    (param $0 i32)
    (param $1 i32)
    (param $2 i32)
    (param $3 i32)
    (result i32)
    (local $4 i32)
    get_global $4
    set_local $4
    get_global $4
    i32.const 32
    i32.add
    set_global $4
    get_local $4
    get_local $0
    i32.store
    get_local $4
    get_local $1
    i32.store offset=4
    get_local $4
    get_local $2
    i32.store offset=8
    get_local $4
    get_local $3
    i32.store offset=12
    get_local $4
    i32.const 0
    i32.store offset=16
    get_local $4
    call $5
    drop
    get_local $4
    set_global $4
    i32.const 0
    ))