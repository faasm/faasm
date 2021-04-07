np,native,faasm
1,22,26
2,12,42
4,7,39

# Profiling Insights

## Profiling Results (time per process!)

1 process:
```
call,               times,  total,    avg_time
mpiInit,            1,      8.41,     8.41
mpiBcast,           93,     0.00,     0.0
mpiAllReduce,       6693,   1.40,     0.00
mpiBarrier,         7,      0.00,     0.0
# total time 34s (usually like 26s, may be due to profiling)
```

2 process:
```
call,               times,  total,    avg_time
mpiGetHostForRank,  83336,  0.73,     8.75e-06
mpiInit,            1,      1.56,     1.56
mpiBcast,           186,    3.10,     0.01
mpiAllReduce,       13362,  1662.94,  0.12
mpiBarrier,         14,     1.06,     0.07
mpiSendRecv,        7349,   1440.60,  0.19
mpiIRecv,           62472,  2646.76,  0.04
mpiSend,            62460,  3240.46,  0.05
mpiWait,            62398,  16840.72, 0.26
# total time: 47s vs 12s native
```

4 processes:
```
call,               times,  total,    avg_time
mpiGetHostForRank,  319924, 7.15,     2.23e-05
mpiInit,            3,      9.35,     3.11
mpiBcast,           372,    33.86,    0.09
mpiAllReduce,       26683,  6397.22,  0.23
mpiBarrier,         28,     4.42,     0.15
mpiSendRecv,        29495,  6870.03,  0.23
mpiIRecv,           249904, 11185.23, 0.04
mpiSend,            249826, 19032.88, 0.07
mpiWait,            249753, 43548.95, 0.17
# total time 50s vs 7s native
```

## Initialisation times

```csv
np,time
1,8ms
2,15.37
4,16.2
```

## Discarded Possibilities

1. Initialisation time (`MPI_Init`)
2. Barriers (`MPI_Barrier`)

## Call frequency + Average Time

```txt
MPI_Irecv 249904
MPI_Send 249904
MPI_Wait 249904
MPI_Wtime 245065
MPI_Sendrecv 29520
MPI_Allreduce 26768
MPI_Bcast 376
MPI_Comm_rank 48
MPI_Barrier 28
MPI_Comm_size 24
MPI_Reduce 24
MPI_Cart_rank 16
MPI_Cart_shift 12
MPI_Type_size 8
MPI_Cart_create 4
MPI_Cart_get 4
MPI_Comm_free 4
MPI_Scan 4
MPI_Finalize 4
```
