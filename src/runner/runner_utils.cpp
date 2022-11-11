#include <runner/runner_utils.h>

namespace po = boost::program_options;

namespace runner {
po::variables_map parseRunnerCmdLine(int argc, char* argv[])
{
    // Define command line arguments
    po::options_description desc("Allowed options");
    desc.add_options()(
      "user", po::value<std::string>(), "function's user name (required)")(
      "function", po::value<std::string>(), "function name (required)")(
      "input-data", po::value<std::string>(), "input data for the function")(
      "cmdline",
      po::value<std::string>(),
      "command line arguments to pass the function");

    // Mark user and function as positional arguments
    po::positional_options_description p;
    p.add("user", 1);
    p.add("function", 2);

    // Parse command line arguments
    po::variables_map vm;
    po::store(
      po::command_line_parser(argc, argv).options(desc).positional(p).run(),
      vm);
    po::notify(vm);

    return vm;
}
}
