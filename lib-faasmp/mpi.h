#ifndef FAASM_MPI_H
#define FAASM_MPI_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Custom Faasm MPI implementation
 * Official MPI spec: https://www.mpi-forum.org/docs/
 * Open MPI repo: https://github.com/open-mpi/ompi
 * You can probably find a version of mpi.h somewhere on your system, e.g. /usr/local/include/mpi.h
 */

#define MPI_MAX_OBJECT_NAME 128

/*
 * Behind the scenes structs (some parts of which are defined in the MPI specification)
 * Each can be extended with private fields as necessary
 */
struct faasmpi_status_public_t {
    // These MPI_XXX fields are defined in the spec
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
};

// Open MPI version: https://github.com/open-mpi/ompi/blob/master/ompi/datatype/ompi_datatype.h
struct faasmpi_datatype_t {
    char name[MPI_MAX_OBJECT_NAME];
};

// Open MPI version: https://github.com/open-mpi/ompi/blob/master/ompi/communicator/communicator.h
struct faasmpi_communicator_t {
    char name[MPI_MAX_OBJECT_NAME];
};

// Open MPI version: https://github.com/open-mpi/ompi/blob/master/ompi/message/message.h
struct faasmpi_message_t {
    int id;
};

/**
 * User-facing constants
 */
// MPI_Comms
struct faasmpi_communicator_t faasmpi_comm_world{.name="COMM_WORLD"};
#define MPI_COMM_WORLD (&faasmpi_comm_world)

// MPI_Datatypes
struct faasmpi_datatype_t faasmpi_type_int{.name="int"};
#define MPI_INT &faasmpi_type_int

// MPI_Statuses
#define MPI_STATUS_IGNORE (static_cast<MPI_Status *> (0))

/*
 * User-facing types
 */
typedef struct faasmpi_communicator_t *MPI_Comm;
typedef struct faasmpi_datatype_t *MPI_Datatype;
typedef struct faasmpi_status_public_t MPI_Status;
typedef struct faasmpi_message_t *MPI_Message;

/*
 * User-facing functions
 */
int MPI_Init(int *argc, char ***argv);

int MPI_Finalize(void);

int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);

int MPI_Comm_rank(MPI_Comm comm, int *rank);

int MPI_Comm_size(MPI_Comm comm, int *size);

#ifdef __cplusplus
}
#endif

#endif
