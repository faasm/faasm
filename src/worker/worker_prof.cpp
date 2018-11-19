#include "worker.h"

int main() {
    // Executes single function for profiling purposes
    util::initLogging();

    worker::Worker w(1);
    w.runSingle();
}
