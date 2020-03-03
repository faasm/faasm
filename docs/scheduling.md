
# Scheduling and Work Sharing

Faasm workers schedule tasks via a standard work-sharing approach. This means function calls are distributed randomly, and workers will hand them off to another worker if that worker is better suited to execute it. This means functions will always be run by the same worker (hence able to share local in-memory state), provided resources are available.

In auto-scaling environments such as KNative, the workers will auto-scale to meet demand.
