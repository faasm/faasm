#include <string>
#include <iostream>
#include <boost/filesystem.hpp>
#include <util/util.h>

#include <faasm/matrix.h>


using namespace boost::filesystem;

int main() {
    std::string targetCategory = "CCAT";
    path outputDir("/home/scs17/faasm/reuters");
    path downloadDir("/home/scs17/faasm/rcv1rcv2aminigoutte");

    int rowCount = 18758;
    int colCount = 21531;

    path enFile = downloadDir;
    enFile.append("EN");
    enFile.append("Index_EN-EN");

    std::cout << "Reading from " << enFile << std::endl;

    std::ifstream input(enFile.c_str());

    std::string line;

    Eigen::SparseMatrix<double> mat(rowCount, colCount);
    std::vector<Eigen::Triplet<double>> triplets;

    int count = 0;
    while (getline(input, line)) {
        printf("%i: %s\n", count, line.c_str());
        count ++;

        const std::vector<std::string> lineTokens = util::tokeniseString(line, ' ');
        std::string cat = lineTokens[0];

        for(int i = 1; i < lineTokens.size(); i++) {
            // Ignore empty token
            std::basic_string<char> thisToken = lineTokens[i];
            if(util::isAllWhitespace(thisToken)) {
                continue;
            }

            // Split up index:value part
            const std::vector<std::string> valueTokens = util::tokeniseString(thisToken, ':');
            int col = std::stoi(valueTokens[0]);
            double weight = std::stof(valueTokens[1]);

            // Add to matrix
            triplets.emplace_back(Eigen::Triplet<double>(i, col, weight));
        };

        if(count > 10) {
            break;
        }
    }
    
    input.close();

    return 0;
}
