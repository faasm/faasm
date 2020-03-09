# Faasm and Threading

The role of threads in a serverless context is unclear. Serverless is inherently 
focused on high volumes of small, single threaded, distributed functions, not on low 
volumes of big, multi-threaded, single-host ones. The underlying hardware reflects this,
with most serverless platforms offering low function resources on commodity VMs with 
<=4 cores.

To take advantage of distributed serverless parallelism, Faasm supports threads, but
executes them as parallel distributed _functions_, across multiple hosts.