#include "KnativeEndpoint.h"

#ifdef WITH_BOOST
#include <boost/program_options.hpp>
namespace po = boost::program_options;
#endif

#include <util/logging.h>
#include <worker/WorkerMain.h>

using namespace knative;

int main(int argc, char **argv) {

    int port = 8080;

#ifdef WITH_BOOST
    // Parses command line args and report errors without the logger because they are destined to the user
    try {
        po::options_description desc("Allowed options");
        desc.add_options()
                ("help,h", "print help message")
                ("port,p", po::value<int>()->default_value(port), "port the worker will listen on")
                ;
        po::variables_map vm;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        }

        port = vm["port"].as<int>();
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
#else
    if (argc != 1) {
        std::cerr << "cmd line options only supported when compiling with boost program options" << std::endl;
        return EXIT_FAILURE;
    }
#endif

    util::initLogging ();
    const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

    // Start the worker pool
    logger->info("Starting worker pool in the background");
    worker::WorkerMain w;
    w.startBackground();

    // Start endpoint (will also have multiple threads)
    logger->info("Starting knative endpoint");
    KnativeEndpoint endpoint(port);
    endpoint.start();

    logger->info("Shutting down knative endpoint");
    w.shutdown();

    return EXIT_SUCCESS;
}
