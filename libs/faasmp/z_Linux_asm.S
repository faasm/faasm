//  z_Linux_asm.S:  - microtasking routines specifically
//                    written for Intel platforms running Linux* OS

// Clang includes this symbol for ABI compatibility with libgomp
// Since it is preceeded by a `.` we have to include it in assembly
// We only include it too to satify the linker and avoid to modify the codegen in Clang
// It is therefore only named and not used because we do not care about
// compatibility with any runtime
	.hidden	.gomp_critical_user_.reduction.var # @.gomp_critical_user_.reduction.var
	.type	.gomp_critical_user_.reduction.var,@object
	.section	.data..gomp_critical_user_.reduction.var,"",@
	.globl	.gomp_critical_user_.reduction.var
	.p2align	4
.gomp_critical_user_.reduction.var:
	.int64	42                      # 0x2a
	.size	.gomp_critical_user_.reduction.var, 8

// Variable for custom reductions
	.hidden	.gomp_critical_user_.atomic_reduction.var # @.gomp_critical_user_.atomic_reduction.var
	.type	.gomp_critical_user_.atomic_reduction.var,@object
	.section	.data..gomp_critical_user_.atomic_reduction.var,"",@
	.globl	.gomp_critical_user_.atomic_reduction.var
	.p2align	4
.gomp_critical_user_.atomic_reduction.var:
	.int64	42                      # 0x2a
	.size	.gomp_critical_user_.atomic_reduction.var, 8

// Variable for the critical section
	.hidden	.gomp_critical_user_.var # @.gomp_critical_user_.atomic_reduction.var
	.type	.gomp_critical_user_.var,@object
	.section	.data..gomp_critical_user_.var,"",@
	.globl	.gomp_critical_user_.var
	.p2align	4
.gomp_critical_user_.var:
	.int64	42                      # 0x2a
	.size	.gomp_critical_user_.var, 8

// GPU stuff
	.hidden .omp_offloading.entries_begin
	.type	.omp_offloading.entries_begin,@object
	.section	.data..omp_offloading.entries_begin,"",@
	.globl	.omp_offloading.entries_begin
	.p2align	4
.omp_offloading.entries_begin:
	.int64	42                      # 0x2a
	.size	.omp_offloading.entries_begin, 8

	.hidden .omp_offloading.entries_end
	.type	.omp_offloading.entries_end,@object
	.section	.data..omp_offloading.entries_end,"",@
	.globl	.omp_offloading.entries_end
	.p2align	4
.omp_offloading.entries_end:
	.int64	42                      # 0x2a
	.size	.omp_offloading.entries_end, 8
