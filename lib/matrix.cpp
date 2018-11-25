#include "faasm/matrix.h"

#include <random>

using namespace Eigen;

namespace faasm {

    MatrixXd randomDenseMatrix(int rows, int cols) {
        MatrixXd mat = MatrixXd::Random(rows, cols);

        return mat;
    }

    SparseMatrix<double> randomSparseMatrix(int rows, int cols, double threshold) {
        // Random distribution
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Create a list of triplets
        std::vector<Triplet<double>> triplets;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                // Get random value
                auto v_ij = dist(gen);

                // If below threshold, add the triplet to the list
                if (v_ij < threshold) {
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
        char *nonZeroKey;

        ~SparseKeys() {
            delete[] valueKey;
            delete[] innerKey;
            delete[] outerKey;
            delete[] sizeKey;
            delete[] nonZeroKey;
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
        keys.nonZeroKey = new char[keyLen + 6];

        sprintf(keys.valueKey, "%s_vals", key);
        sprintf(keys.innerKey, "%s_innr", key);
        sprintf(keys.outerKey, "%s_outr", key);
        sprintf(keys.sizeKey, "%s_size", key);
        sprintf(keys.nonZeroKey, "%s_nonz", key);

        return keys;
    }

    void writeSparseMatrixToState(FaasmMemory *memory, const char *key, const SparseMatrix<double> &mat) {
        if (!mat.isCompressed()) {
            throw std::runtime_error("Sparse matrices must be compressed before serializing");
        }

        // Eigen docs are useful in understanding the approach here
        // https://eigen.tuxfamily.org/dox/group__TutorialSparse.html
        // We need to store three arrays: values, inner indices and outer starts
        //
        // Values = the actual values (doubles)
        // Inner index = index of each value in its respective column (ints)
        // Outer index = number of non-zero entries in the columns _before_ this one (has length cols + 1)
        //
        // To make partial deserialisation easier, we also store a fourther array:
        //
        // Non-zero count = the number of non-zero entries in _this_ column
        //
        // Both the values and inner arrays will have one entry for each value in the matrix
        // Outer index and non-zero count will have one entry for each column
        long totalNonZeros = mat.nonZeros();
        long nCols = mat.cols();
        size_t nValueBytes = totalNonZeros * sizeof(double);
        size_t nInnerBytes = totalNonZeros * sizeof(int);
        size_t nOuterBytes = (mat.outerSize() + 1) * sizeof(int);
        size_t nNonZeroBytes = mat.outerSize() * sizeof(int);

        // The non-zero counts can worked out with the outer indices by subtracting the ith value from the (i+1)th
        int *nonZeroCounts = new int[nCols];
        const int *outerIndices = mat.outerIndexPtr();
        for (int i = 0; i < nCols; i++) {
            nonZeroCounts[i] = outerIndices[i + 1] - outerIndices[i];
        }

        // Build array to specify sizes
        SparseSizes sizes{};
        sizes.cols = nCols;
        sizes.rows = mat.rows();
        sizes.nNonZeros = totalNonZeros;
        sizes.valuesLen = nValueBytes;
        sizes.innerLen = nInnerBytes;
        sizes.outerLen = nOuterBytes;

        auto sizeBytes = reinterpret_cast<uint8_t *>(&sizes);

        // Write everything
        SparseKeys keys = getSparseKeys(key);
        memory->writeState(keys.valueKey, reinterpret_cast<const uint8_t *>(mat.valuePtr()), nValueBytes);
        memory->writeState(keys.innerKey, reinterpret_cast<const uint8_t *>(mat.innerIndexPtr()), nInnerBytes);
        memory->writeState(keys.outerKey, reinterpret_cast<const uint8_t *>(mat.outerIndexPtr()), nOuterBytes);
        memory->writeState(keys.nonZeroKey, reinterpret_cast<const uint8_t *>(nonZeroCounts), nNonZeroBytes);
        memory->writeState(keys.sizeKey, sizeBytes, sizeof(SparseSizes));

        delete[] nonZeroCounts;
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

        auto outerBytes = new uint8_t[sizes.outerLen];
        auto innerBytes = new uint8_t[sizes.innerLen];
        auto valuesBytes = new uint8_t[sizes.valuesLen];

        // Read data into buffers
        memory->readState(keys.outerKey, outerBytes, sizes.outerLen);
        memory->readState(keys.innerKey, innerBytes, sizes.innerLen);
        memory->readState(keys.valueKey, valuesBytes, sizes.valuesLen);

        auto outerPtr = reinterpret_cast<int *>(outerBytes);
        auto innerPtr = reinterpret_cast<int *>(innerBytes);
        auto valuePtr = reinterpret_cast<double *>(valuesBytes);

        // Use eigen to import from the buffers
        Map<SparseMatrix<double>> mat(
                sizes.rows,
                sizes.cols,
                sizes.nNonZeros,
                outerPtr,
                innerPtr,
                valuePtr
        );

        return mat;
    }

    /**
     *  Reads a subset of a sparse matrix from state. The start/ end columns are *exclusive*
     */
    SparseMatrix<double> readSparseMatrixColumnsFromState(FaasmMemory *memory, const char *key,
                                                          long colStart, long colEnd) {
        // This depends heavily on the Eigen sparse matrix representation which is documented here:
        // https://eigen.tuxfamily.org/dox/group__TutorialSparse.html

        // Read in the full matrix properties
        SparseKeys keys = getSparseKeys(key);
        SparseSizes sizes = readSparseSizes(memory, keys);

        long nCols = colEnd - colStart;
        size_t colBytes = nCols * sizeof(int);
        size_t colOffset = colStart * sizeof(int);

        // Load the number of non-zeros in each column
        auto nonZeroBuffer = new uint8_t[colBytes];
        memory->readStateOffset(keys.nonZeroKey, colOffset, nonZeroBuffer, colBytes);
        int *nonZeroCounts = reinterpret_cast<int *>(nonZeroBuffer);

        // Work out how many values we have in total
        int nValues = 0;
        for (int i = 0; i < nCols; i++) {
            nValues += nonZeroCounts[i];
        }

        // Load the outer indices (one longer than the number of columns)
        size_t outerBytes = colBytes + sizeof(int);
        auto outerBuffer = new uint8_t[outerBytes];
        memory->readStateOffset(keys.outerKey, colOffset, outerBuffer, outerBytes);
        int *outerIndices = reinterpret_cast<int *>(outerBuffer);

        // We need to rebase the outer indices to fit our newly created matrix
        int startIdx = outerIndices[0];
        for (int i = 0; i <= nCols; i++) {
            outerIndices[i] -= startIdx;
        }

        // Read in the values and inner indices
        size_t nValueBytes = nValues * sizeof(double);
        size_t offsetValueBytes = startIdx * sizeof(double);
        size_t nInnerBytes = nValues * sizeof(int);
        size_t offsetInnerBytes = startIdx * sizeof(int);

        // Read into buffers
        auto valueBytes = new uint8_t[nValueBytes];
        auto innerBytes = new uint8_t[nInnerBytes];

        // Do the reading from state
        memory->readStateOffset(keys.valueKey, offsetValueBytes, valueBytes, nValueBytes);
        memory->readStateOffset(keys.innerKey, offsetInnerBytes, innerBytes, nInnerBytes);

        auto valuePtr = reinterpret_cast<double *>(valueBytes);
        auto innerPtr = reinterpret_cast<int *>(innerBytes);

        // Use eigen to import from the buffers
        Map<SparseMatrix<double>> mat(
                sizes.rows,
                nCols,
                nValues,
                outerIndices,
                innerPtr,
                valuePtr
        );

        delete[] nonZeroBuffer;

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
     * Reads a subset of full columns from state. Columns are *exclusive*
     */
    MatrixXd readMatrixColumnsFromState(FaasmMemory *memory, const char *key, long colStart, long colEnd, long nRows) {
        long nCols = colEnd - colStart;

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
