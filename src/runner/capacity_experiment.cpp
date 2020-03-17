#include <util/config.h>
#include <wasm/WasmModule.h>
#include <iostream>

int main() {
    message::Message call;
    call.set_user("demo");
    call.set_function("noop");

    util::SystemConfig &conf = util::getSystemConfig();
    conf.logLevel = "off";

    // Create a lock file
    const char *lockFile = "/tmp/capacity.lock";
    int fd = open(lockFile, O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        printf("Failed to open lock file: %s\n", lockFile);
        return 1;
    }

    // Each process seems to be able to handle ~2000
    int nModules = 2000;
    int nProcesses = 50;
    for (int p = 0; p < nProcesses; p++) {
        pid_t pid = fork();

        if (pid == 0) {
            printf("Child worker spawning %i modules\n", nModules);

            // Make sure modules stay in scope and don't get GCed
            std::vector<wasm::WasmModule> modules;
            modules.reserve(nModules);
            for (int i = 0; i < nModules; i++) {
                if (i > 0 && i % 100 == 0) {
                    std::cout << "Created " << i << " modules" << std::endl;
                }
                modules.emplace_back();
                modules.back().bindToFunction(call);
            }

            std::cout << "Created " << nModules << " modules" << std::endl;

            // Wait while lock file exists
            FILE *thisFp = fopen(lockFile, "r");
            while(thisFp != nullptr) {
                printf("Child worker waiting for lock to go\n");
                sleep(10);

                fclose(thisFp);
                thisFp = fopen(lockFile, "r");
            }

            // Done (never executes as parent dies)
            printf("Child worker finishing\n");
            return 0;
        }
    }

    printf("Parent process waiting\n");
    sleep(20);

    // Remove the lock file
    close(fd);
    remove(lockFile);
}