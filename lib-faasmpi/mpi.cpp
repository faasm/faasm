#include "mpi.h"
#include "faasmpi/faasmpi.h"

struct faasmpi_communicator_t faasmpi_comm_world{.id=FAASMPI_COMM_WORLD};

struct faasmpi_datatype_t faasmpi_type_int{.id=FAASMPI_INT};
