# Perf shows that the loops are doing the same

### Perf stat:
```
 Performance counter stats for 'speed_difference':

      14530.607857      task-clock (msec)         #    1.000 CPUs utilized
                 7      context-switches          #    0.000 K/sec
                 0      cpu-migrations            #    0.000 K/sec
               589      page-faults               #    0.041 K/sec
    46,636,539,230      cycles                    #    3.210 GHz
   114,771,451,699      instructions              #    2.46  insn per cycle
    13,208,253,517      branches                  #  908.995 M/sec
           196,535      branch-misses             #    0.00% of all branches

      14.531197506 seconds time elapsed
```

```
 Performance counter stats for 'simple_runner demo speed_difference':

       4852.352536      task-clock (msec)         #    1.000 CPUs utilized
                 3      context-switches          #    0.001 K/sec
                 0      cpu-migrations            #    0.000 K/sec
             5,611      page-faults               #    0.001 M/sec
    14,531,719,357      cycles                    #    2.995 GHz
    35,368,260,394      instructions              #    2.43  insn per cycle
     4,120,953,595      branches                  #  849.269 M/sec
         5,078,676      branch-misses             #    0.12% of all branches

       4.852843140 seconds time elapsed
```

### Perf report

Symbol table:
```
functionDef35       log
functionDef2        __original_main
```

The global time spent in each section is very similar:
![Time division](time-division.png)

The loop bodies (pictures below) look quite similar -- unless the unknown/unknown:XX are something to worry about. However the Wasm code 
before the loop fiddles with the MM registers. Manually compiling with `clang++ -O3 -mavx2` but did not alter the native
speed. I think I have only found a bad example of speed difference where here MUSL's log is faster than GLibC's log.
When looking at the version ran in Wasm, it is a lot smaller than the one in GLibC which does not
explain why my experiement ran faster. I'll try to instead profile the mersenne_twister_engine on native and
Wasm and see how that one does because I think that'll be closer to my results.

![main native](main-native.png)

![main Wasm](main-wasm.png)
