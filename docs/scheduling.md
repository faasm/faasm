# Scheduling

Multiple instances of the Faasm runtime communicate to form a distributed 
work-sharing scheduler. This means function calls can be distributed randomly, 
and the runtime will hand them off to another runtime instance if it is better 
suited to execute it. 

This means functions will always be run by the same runtime instance, and hence 
able to share local in-memory state, provided resources are available.