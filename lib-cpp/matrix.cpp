#include "faasm/matrix.h"

#include <algorithm>
#include <random>

using namespace Eigen;

namespace faasm {

    MatrixXd randomDenseMatrix(int rows, int cols) {
        MatrixXd mat = MatrixXd::Random(rows, cols);

        return mat;
    }

    MatrixXd zeroMatrix(int rows, int cols) {
        MatrixXd mat = MatrixXd::Zero(rows, cols);

        return mat;
    }

    SparseMatrix<double> randomSparseMatrix(int rows, int cols, double threshold) {
        // Random distribution
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<double> dist(0.0, 1.0);

        // Create a list of triplets
        std::vector<Triplet<double>>
                triplets;

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

    SparseMatrixSerialiser::SparseMatrixSerialiser(const SparseMatrix<double> &matIn) : mat(matIn) {
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
        // To make partial deserialisation easier, we also store a further array:
        //
        // Non-zero count = the number of non-zero entries in _this_ column
        //
        // Both the values and inner arrays will have one entry for each value in the matrix
        // Outer index and non-zero count will have one entry for each column
        long totalNonZeros = mat.nonZeros();
        long nCols = mat.cols();
        nValueBytes = totalNonZeros * sizeof(double);
        nInnerBytes = totalNonZeros * sizeof(int);
        nOuterBytes = (mat.outerSize() + 1) * sizeof(int);
        nNonZeroBytes = mat.outerSize() * sizeof(int);

        // Set up bytes pointers to the relevant matrix data
        valueBytes = reinterpret_cast<const uint8_t *>(mat.valuePtr());
        innerBytes = reinterpret_cast<const uint8_t *>(mat.innerIndexPtr());
        outerBytes = reinterpret_cast<const uint8_t *>(mat.outerIndexPtr());

        // The non-zero counts can be worked out with the outer indices by subtracting the
        // ith value from the (i+1)th
        nonZeroCounts = new int[nCols];
        const int *outerIndices = mat.outerIndexPtr();
        for (int i = 0; i < nCols; i++) {
            nonZeroCounts[i] = outerIndices[i + 1] - outerIndices[i];
        }
        nonZeroBytes = reinterpret_cast<const uint8_t *>(nonZeroCounts);

        // Build array to specify sizes
        sizes.cols = nCols;
        sizes.rows = mat.rows();
        sizes.nNonZeros = totalNonZeros;
        sizes.valuesLen = nValueBytes;
        sizes.innerLen = nInnerBytes;
        sizes.outerLen = nOuterBytes;
        sizes.nonZeroLen = nNonZeroBytes;

        sizeBytes = reinterpret_cast<uint8_t *>(&sizes);
        nSizeBytes = sizeof(SparseSizes);
    }

    void SparseMatrixSerialiser::writeToState(const char *key) {
        SparseKeys keys = getSparseKeys(key);

        faasmWriteState(keys.valueKey, valueBytes, nValueBytes);
        faasmWriteState(keys.innerKey, innerBytes, nInnerBytes);
        faasmWriteState(keys.outerKey, outerBytes, nOuterBytes);
        faasmWriteState(keys.nonZeroKey, nonZeroBytes, nNonZeroBytes);
        faasmWriteState(keys.sizeKey, sizeBytes, nSizeBytes);
    }

    Map<const SparseMatrix<double>> SparseMatrixSerialiser::readFromBytes(
            const SparseSizes &sizes,
            uint8_t *outerBytes,
            uint8_t *innerBytes,
            uint8_t *valuesBytes
    ) {

        auto outerPtr = reinterpret_cast<int *>(outerBytes);
        auto innerPtr = reinterpret_cast<int *>(innerBytes);
        auto valuePtr = reinterpret_cast<double *>(valuesBytes);

        // Use eigen to import from the buffers
        Map<const SparseMatrix<double>> mat(
                sizes.rows,
                sizes.cols,
                sizes.nNonZeros,
                outerPtr,
                innerPtr,
                valuePtr
        );

        return mat;
    }

    SparseMatrixSerialiser::~SparseMatrixSerialiser() {
        delete[] nonZeroCounts;
    }

    void writeSparseMatrixToState(const char *key,
                                  const SparseMatrix<double> &mat) {
        SparseMatrixSerialiser serialiser(mat);
        serialiser.writeToState(key);
    }

    SparseSizes readSparseSizes(const SparseKeys &keys) {
        uint8_t sizeBuffer[sizeof(SparseSizes)];
        faasmReadState(keys.sizeKey, sizeBuffer, sizeof(SparseSizes));
        auto sizes = reinterpret_cast<SparseSizes *>(sizeBuffer);

        if(sizes->cols == 0 || sizes->rows == 0) {
            throw std::runtime_error("Loaded sparse matrix size zero");
        }
        return *sizes;
    }

    Map<const SparseMatrix<double>> readSparseMatrixFromState(const char *key) {
        SparseKeys keys = getSparseKeys(key);
        SparseSizes sizes = readSparseSizes(keys);

        auto outerBytes = new uint8_t[sizes.outerLen];
        auto innerBytes = new uint8_t[sizes.innerLen];
        auto valuesBytes = new uint8_t[sizes.valuesLen];

        // Read data into buffers
        faasmReadState(keys.outerKey, outerBytes, sizes.outerLen);
        faasmReadState(keys.innerKey, innerBytes, sizes.innerLen);
        faasmReadState(keys.valueKey, valuesBytes, sizes.valuesLen);

        return SparseMatrixSerialiser::readFromBytes(
                sizes,
                outerBytes,
                innerBytes,
                valuesBytes
        );
    }

    /**
     *  Reads a subset of a sparse matrix from state. The start/ end columns are *exclusive*
     */
    Map<const SparseMatrix<double>> readSparseMatrixColumnsFromState(const char *key,
                                                                     long colStart, long colEnd) {
        // This depends heavily on the Eigen sparse matrix representation which is documented here:
        // https://eigen.tuxfamily.org/dox/group__TutorialSparse.html

        // Read in the full matrix properties
        SparseKeys keys = getSparseKeys(key);
        SparseSizes sizes = readSparseSizes(keys);

        long nCols = colEnd - colStart;
        size_t colBytes = nCols * sizeof(int);
        size_t colOffset = colStart * sizeof(int);

        // Load the number of non-zeros in each column
        uint8_t *nonZeroBuffer = faasmReadStateOffsetPtr(keys.nonZeroKey, sizes.nonZeroLen, colOffset, colBytes);
        int *nonZeroCounts = reinterpret_cast<int *>(nonZeroBuffer);

        // Work out how many values we have in total
        int nValues = 0;
        for (int i = 0; i < nCols; i++) {
            nValues += nonZeroCounts[i];
        }

        // Load the outer indices (one longer than the number of columns)
        long nOuterIndices = nCols + 1;
        size_t outerBytes = nOuterIndices * sizeof(int);
        uint8_t *outerBuffer = faasmReadStateOffsetPtr(keys.outerKey, sizes.outerLen, colOffset, outerBytes);
        int *outerIndices = reinterpret_cast<int *>(outerBuffer);
        int startIdx = outerIndices[0];

        // We need to get these indices RELATIVE to the first index to fit our newly created matrix
        // BUT: we may be reusing THIS SAME ARRAY of outer indices so we must copy it first before modifying
        auto outerIndicesRebased = new int[nOuterIndices];
        std::copy(outerIndices, outerIndices + nOuterIndices, outerIndicesRebased);
        for (int i = 0; i <= nCols; i++) {
            outerIndicesRebased[i] -= startIdx;
        }

        // Read in the values and inner indices
        size_t nValueBytes = nValues * sizeof(double);
        size_t offsetValueBytes = startIdx * sizeof(double);
        size_t nInnerBytes = nValues * sizeof(int);
        size_t offsetInnerBytes = startIdx * sizeof(int);

        // Do the reading from state
        uint8_t *valueBytes = faasmReadStateOffsetPtr(keys.valueKey, sizes.valuesLen, offsetValueBytes, nValueBytes);

        uint8_t *innerBytes = faasmReadStateOffsetPtr(keys.innerKey, sizes.innerLen, offsetInnerBytes, nInnerBytes);

        auto valuePtr = reinterpret_cast<double *>(valueBytes);
        auto innerPtr = reinterpret_cast<int *>(innerBytes);

        // Use eigen to import from the buffers
        Map<const SparseMatrix<double>> mat(
                sizes.rows,
                nCols,
                nValues,
                outerIndicesRebased,
                innerPtr,
                valuePtr
        );

        return mat;
    }


    /**
     * Writes a matrix to state
     */
    void writeMatrixToState(const char *key, const MatrixXd &matrix) {
        size_t nBytes = matrix.rows() * matrix.cols() * sizeof(double);
        auto byteArray = reinterpret_cast<const uint8_t *>(matrix.data());

        faasmWriteState(key, byteArray, nBytes);
    }

    /**
     * Reads a matrix from state
     */
    Map<const MatrixXd> readMatrixFromState(const char *key, long rows, long cols) {
        long nDoubles = rows * cols;

        auto buffer = new double[nDoubles];
        readMatrixFromState(key, buffer, rows, cols);

        Map<const MatrixXd> mat(buffer, rows, cols);

        return mat;
    }

    /** 
     * Reads a matrix from state directly into the given buffer
     */
    void readMatrixFromState(const char *key, double *buffer, long rows, long cols) {

        size_t nBytes = rows * cols * sizeof(double);
        auto byteBuffer = reinterpret_cast<uint8_t *>(buffer);

        faasmReadState(key, byteBuffer, nBytes);
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
    void writeMatrixToStateElement(const char *key, const MatrixXd &matrix, long row, long col) {
        // Work out total size of this matrix
        size_t totalBytes = matrix.rows() * matrix.cols() * sizeof(double);

        // Work out the position of this element
        // Note that matrices are stored in column-major order by default
        long byteIdx = matrixByteIndex(row, col, matrix.rows());

        double value = matrix.coeff(row, col);
        auto byteValue = reinterpret_cast<uint8_t *>(&value);
        size_t nBytes = sizeof(double);

        faasmWriteStateOffset(key, totalBytes, (size_t) byteIdx, byteValue, nBytes);
    }

    /**
     * Reads a subset of full columns from state. Columns are *exclusive*
     */
    Map<const MatrixXd> readMatrixColumnsFromState(const char *key, long totalCols, long colStart,
                                                   long colEnd, long nRows) {
        long nCols = colEnd - colStart;

        long startIdx = matrixByteIndex(0, colStart, nRows);
        long endIdx = matrixByteIndex(nRows, colEnd, nRows);

        long bufferLen = endIdx - startIdx;
        long totalLen = totalCols * nRows * sizeof(double);
        uint8_t *buffer = faasmReadStateOffsetPtr(key, totalLen, startIdx, bufferLen);

        auto doubleArray = reinterpret_cast<double *>(buffer);
        Map<const MatrixXd> result(doubleArray, nRows, nCols);

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
     * Sums the squared error between two vectors
     */
    double calculateSquaredError(const MatrixXd &prediction, const MatrixXd &actual) {
        MatrixXd diff = prediction - actual;

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
     * Calculates the hinge error. This is used in classification problems.
     * The error will be zero if the classification is correct (i.e. the
     * prediction and the actual have the same sign), but will be non-zero
     * if they are different sign.
     */
    double calculateHingeError(const MatrixXd &prediction, const MatrixXd &actual) {
        double totalErr = 0;
        for (long r = 0; r < prediction.rows(); r++) {
            for (long c = 0; c < prediction.cols(); c++) {
                double thisProduct = prediction.coeff(r, c) * actual.coeff(c, r);

                // Product will be negative if prediction and actual have different sign
                totalErr += std::max(1.0 - thisProduct, 0.0);
            }
        }

        return totalErr;
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
