#include "faasm/matrix.h"
#include <stdio.h>

using namespace Eigen;

namespace faasm {

    MatrixXd randomDenseMatrix(int rows, int cols) {
        MatrixXd mat = MatrixXd::Random(rows, cols);

        return mat;
    }

    SparseMatrix<double> randomSparseMatrix(int rows, int cols) {
        // Random distribution
        std::default_random_engine gen;
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Create a list of triplets
        std::vector<Triplet<double>> triplets;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Get random value
                auto v_ij = dist(gen);

                // If below threshold, add the triplet to the list
                if (v_ij < 0.1) {
                    const Triplet<double, int> &triplet = Triplet<double>(i, j, v_ij);
                    triplets.push_back(triplet);
                }
            }
        }

        // Create a sparse matrix from the list of triplets
        SparseMatrix<double> mat(rows, cols);
        mat.setFromTriplets(triplets.begin(), triplets.end());

        return mat;
    }

    /**
     * Serialises a matrix to a byte array
     */
    uint8_t *matrixToBytes(const MatrixXd &mat) {
        long rows = mat.rows();
        long cols = mat.cols();

        long nDoubles = rows * cols;
        auto doubleArray = new double[nDoubles];
        Map<MatrixXd>(&doubleArray[0], rows, cols) = mat;

        auto byteArray = reinterpret_cast<uint8_t *>(&doubleArray[0]);

        return byteArray;
    }

    /**
     * Deserialises a byte array to a matrix
     */
    MatrixXd bytesToMatrix(uint8_t *byteArray, long rows, long columns) {
        auto doubleArray = reinterpret_cast<double *>(&byteArray[0]);

        Map<MatrixXd> mat(&doubleArray[0], rows, columns);

        return mat;
    }

    struct SparseKeys {
        char *valueKey;
        char *innerKey;
        char *outerKey;
        char *sizeKey;

        ~SparseKeys() {
            delete[] valueKey;
            delete[] innerKey;
            delete[] outerKey;
            delete[] sizeKey;
        }
    };

    struct SparseSizes {
        long cols;
        long rows;
        long nNonZeros;

        size_t valuesLen;
        size_t innerLen;
        size_t outerLen;
    };

    SparseKeys getSparseKeys(const char *key) {
        size_t keyLen = strlen(key);

        SparseKeys keys{};
        keys.valueKey = new char[keyLen + 6];
        keys.innerKey = new char[keyLen + 6];
        keys.outerKey = new char[keyLen + 6];
        keys.sizeKey = new char[keyLen + 6];

        sprintf(keys.valueKey, "%s_vals", key);
        sprintf(keys.innerKey, "%s_innr", key);
        sprintf(keys.outerKey, "%s_outr", key);
        sprintf(keys.sizeKey, "%s_size", key);

        return keys;
    }

    void writeSparseMatrixToState(FaasmMemory *memory, const SparseMatrix<double> &mat, const char *key) {
        if (!mat.isCompressed()) {
            throw std::runtime_error("Sparse matrices must be compressed before serializing");
        }

        // Need to store three arrays: values, inner indices and outer starts
        // Values = the actual values (doubles)
        // Inner index = index of each value in its respective column (ints)
        // Outer index = position of the first value for that column in the other 2 arrays (ints)
        //
        // Both the values and inner arrays will have one entry for each value in the matrix
        // Outer index will have one entry for each column
        size_t nValueBytes = mat.nonZeros() * sizeof(double);
        size_t nInnerBytes = mat.nonZeros() * sizeof(int);
        size_t nOuterBytes = mat.outerSize() * sizeof(int);

        // Build array to specify sizes
        SparseSizes sizes{};
        sizes.cols = mat.cols();
        sizes.rows = (size_t) mat.rows();
        sizes.nNonZeros = mat.nonZeros();
        sizes.valuesLen = nValueBytes;
        sizes.innerLen = nInnerBytes;
        sizes.outerLen = nOuterBytes;

        auto sizeBytes = reinterpret_cast<uint8_t *>(&sizes);

        // Write everything
        SparseKeys keys = getSparseKeys(key);
        memory->writeState(keys.valueKey, (uint8_t *) mat.valuePtr(), nValueBytes);
        memory->writeState(keys.innerKey, (uint8_t *) mat.innerIndexPtr(), nInnerBytes);
        memory->writeState(keys.outerKey, (uint8_t *) mat.outerIndexPtr(), nOuterBytes);
        memory->writeState(keys.sizeKey, sizeBytes, sizeof(SparseSizes));
    }

    SparseSizes readSparseSizes(FaasmMemory *memory, const SparseKeys &keys) {
        uint8_t sizeBuffer[sizeof(SparseSizes)];
        memory->readState(keys.sizeKey, sizeBuffer, sizeof(SparseSizes));
        auto sizes = reinterpret_cast<SparseSizes *>(sizeBuffer);

        return *sizes;
    }

    SparseMatrix<double> readSparseMatrixFromState(FaasmMemory *memory, const char *key) {
        SparseKeys keys = getSparseKeys(key);
        SparseSizes sizes = readSparseSizes(memory, keys);

        // Create new matrix and copy data in
        SparseMatrix<double> mat(sizes.rows, sizes.cols);
        mat.makeCompressed();
        mat.resizeNonZeros(sizes.nNonZeros);

        // Read data from state straight into matrix
        memory->readState(keys.valueKey, (uint8_t *) mat.valuePtr(), sizes.valuesLen);
        memory->readState(keys.innerKey, (uint8_t *) mat.innerIndexPtr(), sizes.innerLen);
        memory->readState(keys.outerKey, (uint8_t *) mat.outerIndexPtr(), sizes.outerLen);

        mat.finalize();

        return mat;
    }

    SparseMatrix<double> readSparseMatrixColumnsFromState(FaasmMemory *memory, const char *key,
            long colStart, long colEnd) {

        // Read in the full matrix properties
        SparseKeys keys = getSparseKeys(key);
        SparseSizes sizes = readSparseSizes(memory, keys);

        // Read in the outer indices, this gives the sizes of each column and thus the elements we want
        long nCols = colEnd - colStart;
        size_t nOuterBytes = nCols * sizeof(int);
        size_t offsetOuterBytes = colStart * sizeof(int);
        auto outerBuffer = new uint8_t[nOuterBytes];
        memory->readStateOffset(keys.outerKey, offsetOuterBytes, outerBuffer, nOuterBytes);
        int * outerIndices = reinterpret_cast<int*>(outerBuffer);
        
        // Work out which values and inner indices correspond to our columns
        // Note, these are held as offsets into the _full_ matrix so need to be
        // offset when dealing with the real matrix
        int startIdx = outerIndices[0];
        int endIdx = outerIndices[nCols - 1];
        int nValues = endIdx - startIdx;
        size_t nValueBytes = nValues * sizeof(double);
        size_t nInnerBytes = nValues * sizeof(int);

        SparseMatrix<double> mat(sizes.rows, nCols);
        mat.makeCompressed();

        // Drop out if nothing to do
        if(nValues == 0) {
            return mat;
        }

        mat.resizeNonZeros(nValues);

        // We already have the outer indices in memory, just need to rebase them relative
        // to our new index
        int* relativeOuterIndices = new int[nCols];
        for(int i = 0; i < nCols; i++) {
            relativeOuterIndices[i] = outerIndices[i] - outerIndices[0];
        }
        memcpy(mat.outerIndexPtr(), relativeOuterIndices, nOuterBytes);

        // Read the others into memory
        memory->readStateOffset(keys.valueKey, startIdx, (uint8_t *) mat.valuePtr(), nValueBytes);
        memory->readStateOffset(keys.innerKey, startIdx, (uint8_t *) mat.innerIndexPtr(), nInnerBytes);

        mat.finalize();

        delete[] outerBuffer;

        return mat;
    }


    /**
     * Writes a matrix to state
     */
    void writeMatrixToState(FaasmMemory *memory, const char *key, const MatrixXd &matrix) {
        size_t nBytes = matrix.rows() * matrix.cols() * sizeof(double);
        uint8_t *serialisedData = matrixToBytes(matrix);

        memory->writeState(key, serialisedData, nBytes);

        delete[] serialisedData;
    }

    /**
     * Reads a matrix from state
     */
    MatrixXd readMatrixFromState(FaasmMemory *memory, const char *key, long rows, long cols) {
        size_t nBytes = rows * cols * sizeof(double);

        auto buffer = new uint8_t[nBytes];
        memory->readState(key, buffer, nBytes);

        Eigen::MatrixXd deserialised = bytesToMatrix(buffer, rows, cols);
        delete[] buffer;
        return deserialised;
    }

    long matrixByteIndex(long row, long col, long nRows) {
        // Work out the position of this element
        // Note that matrices are stored in column-major order by default
        long byteIdx = ((col * nRows) + row) * sizeof(double);

        return byteIdx;
    }

    /** 
     * Updates a specific element in state 
     */
    void writeMatrixToStateElement(FaasmMemory *memory, const char *key, const MatrixXd &matrix, long row, long col) {
        // Work out the position of this element
        // Note that matrices are stored in column-major order by default
        long byteIdx = matrixByteIndex(row, col, matrix.rows());

        double value = matrix(row, col);
        auto byteValue = reinterpret_cast<uint8_t *>(&value);
        size_t nBytes = sizeof(double);

        memory->writeStateOffset(key, (size_t) byteIdx, byteValue, nBytes);
    }

    /**
     * Reads a subset of full columns from state (note that column numbers are inclusive
     */
    MatrixXd readMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart, long colEnd, long nRows) {
        // Note, inclusive
        long nCols = colEnd - colStart + 1;

        long startIdx = matrixByteIndex(0, colStart, nRows);
        long endIdx = matrixByteIndex(nRows, colEnd, nRows);

        long bufferLen = endIdx - startIdx;
        auto buffer = new uint8_t[bufferLen];

        memory->readStateOffset(key, startIdx, buffer, bufferLen);

        MatrixXd result = bytesToMatrix(buffer, nRows, nCols);
        delete[] buffer;
        return result;
    }

    /**
     * Shuffles the columns of a matrix
     */
    void shuffleMatrixColumns(MatrixXd &matrix) {
        int nCols = (int) matrix.cols();

        for (int i = nCols - 1; i > 0; i--) {
            int r = randomInteger(0, nCols - 1);
            matrix.col(i).swap(matrix.col(r));
        }
    }

    /**
     * Shuffles the columns of two matrices in the same order
     */
    void shufflePairedMatrixColumns(MatrixXd &a, MatrixXd &b) {
        int nCols = (int) a.cols();

        for (int i = nCols - 1; i > 0; i--) {
            int r = randomInteger(0, nCols - 1);
            a.col(i).swap(a.col(r));
            b.col(i).swap(b.col(r));
        }
    }

    /**
     * Prints the given matrix
     */
    void printMatrix(MatrixXd &mat) {
        for (int r = 0; r < mat.rows(); r++) {
            for (int c = 0; c < mat.cols(); c++) {
                double val = mat.coeff(r, c);
                printf("%.2f, ", val);
            }

            printf("\n");
        }
    }

    /**
     * Sums the squared error between two vectors when the value in the first is non-zero
     */
    double calculateSquaredError(const MatrixXd &a, const MatrixXd &b) {
        MatrixXd diff = a - b;

        double squaredError = 0;
        for (long r = 0; r < diff.rows(); r++) {
            for (long c = 0; c < diff.cols(); c++) {
                double e = diff.coeff(r, c);
                squaredError += pow(e, 2);
            }
        }

        return squaredError;
    }

    /**
     * Finds the mean squared error between two matrices
     */
    double calculateRootMeanSquaredError(const MatrixXd &a, const MatrixXd &b) {
        double squaredError = calculateSquaredError(a, b);

        long nElements = a.cols() * a.rows();
        double rmse = sqrt(squaredError / nElements);
        return rmse;
    }
}
