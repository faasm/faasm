#pragma once

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace runner {
po::variables_map parseRunnerCmdLine(int argc, char* argv[]);
}
