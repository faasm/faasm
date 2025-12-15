# Granny

Granny is a system that enables the fine-grained management of multi-threaded
and multi-process applications.

Granny executes unmodified OpenMP and MPI applications, and is able to take
control over their execution in order to add/remove threads to the computation,
or migrate processes across VMs with minimal overhead.

Granny executes threads and processes as Granules, a lightweight execution
abstraction based on WebAssembly. To learn more about Granny, consider reading
our NSDI'25 [paper](https://www.usenix.org/conference/nsdi25/presentation/segarra).

Granny build on top of the [Faasm](https://github.com/faasm/faasm) serverless
runtime and its Faaslet abstraction, see Faasm's ATC'20 [paper](
https://www.usenix.org/conference/atc20/presentation/shillaker) for more
details. Granny's main contribution on top of Faasm is [faabric](
https://github.com/faasm/faabric), a library for distributed message passing,
scheduling, and shared memory.

## Further Reading

For more details on how to use Granny, we refer to the following:
* [Faasm's Readme](./README_Faasm.md) - for quick-start instructions.
* [Granny Experiments](https://github.com/faasm/granny-experiments) - for end-to-end experiments with Granny.

## Acknowledgements

<img width="80px" src="https://cloudskin.eu/assets/img/europe.jpg" alt="European flag" />

Granny has been partially funded and developed under the umbrella of the
CLOUDSKIN project, which has received funding from the European Union’s Horizon
research and innovation programme under grant agreement No 101092646.

https://cloudskin.eu
