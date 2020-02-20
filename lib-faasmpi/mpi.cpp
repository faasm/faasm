#include <stdexcept>
#include "mpi.h"
#include "faasmpi/faasmpi.h"

struct faasmpi_communicator_t faasmpi_comm_world{.id=FAASMPI_COMM_WORLD};

struct faasmpi_datatype_t faasmpi_type_int{.id=FAASMPI_INT, .size=sizeof(int)};
struct faasmpi_datatype_t faasmpi_type_long{.id=FAASMPI_LONG, .size=sizeof(long)};
struct faasmpi_datatype_t faasmpi_type_float{.id=FAASMPI_FLOAT, .size=sizeof(float)};

struct faasmpi_info_t faasmpi_info_null{.id=FAASMPI_INFO_NULL};

struct faasmpi_op_t faasmpi_op_max{.id=FAASMPI_OP_MAX};
struct faasmpi_op_t faasmpi_op_min{.id=FAASMPI_OP_MIN};
struct faasmpi_op_t faasmpi_op_sum{.id=FAASMPI_OP_SUM};
struct faasmpi_op_t faasmpi_op_prod{.id=FAASMPI_OP_PROD};
struct faasmpi_op_t faasmpi_op_land{.id=FAASMPI_OP_LAND};
struct faasmpi_op_t faasmpi_op_lor{.id=FAASMPI_OP_LOR};
struct faasmpi_op_t faasmpi_op_band{.id=FAASMPI_OP_BAND};
struct faasmpi_op_t faasmpi_op_bor{.id=FAASMPI_OP_BOR};
struct faasmpi_op_t faasmpi_op_maxloc{.id=FAASMPI_OP_MAXLOC};
struct faasmpi_op_t faasmpi_op_minloc{.id=FAASMPI_OP_MINLOC};

faasmpi_datatype_t *getFaasmDatatypeFromId(int datatypeId) {
    switch (datatypeId) {
        case FAASMPI_INT:
            return MPI_INT;
        case FAASMPI_LONG:
            return MPI_LONG;
        case FAASMPI_FLOAT:
            return MPI_FLOAT;
        default:
            throw std::runtime_error("Unrecognised datatype ID\n");
    }
}
