#include "utils.h"

#include <state/InMemoryStateKeyValue.h>
#include <state/StateServer.h>
#include <emulator/emulator.h>

using namespace state;

namespace tests {
    DummyStateServer::DummyStateServer():remoteState(LOCALHOST) {

    }

    std::shared_ptr<StateKeyValue> DummyStateServer::getRemoteKv() {
        if(dummyData.empty()) {
            return remoteState.getKV(dummyUser, dummyKey);
        } else {
            return remoteState.getKV(dummyUser, dummyKey, dummyData.size());
        }
    }

    std::shared_ptr<StateKeyValue> DummyStateServer::getLocalKv() {
        if(dummyData.empty()) {
            return state::getGlobalState().getKV(dummyUser, dummyKey);
        } else {
            return state::getGlobalState().getKV(dummyUser, dummyKey, dummyData.size());
        }
    }

    std::vector<uint8_t> DummyStateServer::getRemoteKvValue() {
        std::vector<uint8_t> actual(dummyData.size(), 0);
        getRemoteKv()->get(actual.data());
        return actual;
    }

    std::vector<uint8_t> DummyStateServer::getLocalKvValue() {
        std::vector<uint8_t> actual(dummyData.size(), 0);
        getLocalKv()->get(actual.data());
        return actual;
    }
    
    void DummyStateServer::start(int nMessages) {
        // NOTE - in a real deployment each server would be running in its own
        // process on a separate host. To run it in a thread like this we need to
        // be careful to avoid sharing any global variables with the main thread.
        //
        // We force the server thread to have localhost IP, and the main thread
        // to be the "client" with a junk IP.

        serverThread = std::thread([this, nMessages] {
            const std::shared_ptr<spdlog::logger> &logger = util::getLogger();

            // Make sure any emulated state actions use this remote state
            setEmulatorState(&remoteState);

            // Override the host endpoint for the server thread. Must be localhost
            util::getSystemConfig().endpointHost = LOCALHOST;

            // Master the dummy data in this thread
            if(!dummyData.empty()) {
                const std::shared_ptr<state::StateKeyValue> &kv = remoteState.getKV(dummyUser, dummyKey, dummyData.size());
                std::shared_ptr<InMemoryStateKeyValue> inMemKv = std::static_pointer_cast<InMemoryStateKeyValue>(kv);

                // Check this kv "thinks" it's master
                if(!inMemKv->isMaster()) {
                    logger->error("Dummy state server not master for data");
                    throw std::runtime_error("Remote state server failed");
                }

                // Set the data
                kv->set(dummyData.data());
                logger->debug("Finished setting master for test {}/{}", kv->user, kv->key);
            }
            
            // Process the required number of messages
            StateServer server(remoteState);
            logger->debug("Running test state server for {} messages", nMessages);
            int processedMessages = 0;

            while (processedMessages < nMessages) {
                try {
                    processedMessages += server.poll();
                } catch (tcp::TCPShutdownException &ex) {
                    logger->debug("Shutting down test state server after {} messages", processedMessages);
                    break;
                }

                logger->debug("Test state server processed {} messages", processedMessages);
            }

            // Close the server
            server.close();
        });

        // Give it time to start
        usleep(500 * 1000);
    }

    void DummyStateServer::wait() {
        if(serverThread.joinable()) {
            serverThread.join();
        }
    }
}