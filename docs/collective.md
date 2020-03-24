# Faasm Collective Communication

To support efficient parallel processing, Faasm exposes _shared memory regions_ between functions 
colocated on the same host. These in-memory regions are referred to collectively as the _local tier_. 
Faasm synchronises these regions across hosts through the _global tier_, which is independent of any 
given host, and holds a single authoritative copy of the underlying data. Data held in the global tier 
can outlive any given function execution, and is useful for holding larger shared datasets which may be 
used as part of a composition of several functions. 

This two-tier architecture is flexible and works well for a range of applications. Synchronisation 
between the local tier and the global tier is crucial in enabling this flexibility, but does
introduce latency and network overheads. Certain classes of applications have well defined patterns
for sharing data, and don't require this much flexibility. Accordingly, we can introduce optimisations
in Faasm's state management specifically for such applications, hence avoiding unnecessary overheads in
certain cases. 

Collective communication is one such class of application. It involves several well-defined operations 
such as broadcast, reduce and gather, all of which have fixed modes of sharing _immutable_ data between a 
number of distributed functions. When these operations are performed, the Faasm runtime knows exactly which 
functions need to access which chunks of data, and can optimise their placement accordingly.

To do this, Faasm uses _point-to-point replication_. This is a special type of state synchronisation 
that occurs only through the local tier of each host, and _does not_ involve the global tier.

## Point-to-point replication

Whenever Faasm functions access shared state, they do so through their local tier, i.e. each host has a 
single replica of the data held in memory which is shared between all functions on that host. In a 
collective communication operation, Faasm knows exactly which functions need to access which pieces of data, 
hence can deduce which hosts need replicas in their local tier. To ensure minimal replication, the host 
initiating the operation can send one or more point-to-point messages, transmitting the required data to 
the target hosts. Hosts receiving such messages will copy this data into their local tier, thus making it 
accessible to every function on that host.

If many functions on a given host need to access the same data, only one point-to-point replication is required
to ensure they all have access.

## Collective operations

Collective operations can be categorised by the number of messages that must be sent, the level of data
sharing, and hence how many replications they require. If we assume a naive implementation of each 
operation, with a total of `n_funcs` involved across `n_hosts`, we get the following table:

Operation | Send data | Recv data | Messages | P2P replications
--- | --- | --- | --- | ---
Send | One | None | One | `<= 1` 
Recv | None | One | One | `<= 1`
Barrier | None | None | `n_funcs * 2` | None
Broadcast | Many | None | `n_funcs` | `n_hosts - 1` 
Reduce | None | Many | `n_funcs` | `<= n_funcs`


**TODO - finish these notes**