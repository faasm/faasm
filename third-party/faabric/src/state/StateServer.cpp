#include "StateServer.h"

#include <util/logging.h>
#include <util/config.h>
#include <state/State.h>
#include <state/InMemoryStateKeyValue.h>

#include <grpcpp/grpcpp.h>
#include <util/macros.h>
#include <proto/macros.h>

#define KV_FROM_REQUEST(request) auto kv = std::static_pointer_cast<InMemoryStateKeyValue>( \
    state.getKV(request->user(), request->key()) \
    );

namespace state {
    StateServer::StateServer(State &stateIn) :
            RPCServer(DEFAULT_RPC_HOST, STATE_PORT),
            state(stateIn) {
    }

    void StateServer::doStart(const std::string &serverAddr) {
        // Build the server
        ServerBuilder builder;
        builder.AddListeningPort(serverAddr, InsecureServerCredentials());
        builder.RegisterService(this);

        // Start it
        server = builder.BuildAndStart();
        util::getLogger()->info("State server listening on {}", serverAddr);

        server->Wait();
    }

    Status StateServer::Pull(
            ServerContext *context,
            ServerReaderWriter<faabric::StateChunk, faabric::StateChunkRequest> *stream) {

        // Iterate through streamed requests
        faabric::StateChunkRequest request;
        while (stream->Read(&request)) {
            util::getLogger()->debug(
                    "Pull {}/{} ({}->{})",
                    request.user(), request.key(),
                    request.offset(), request.offset() + request.chunksize()
            );

            // Write the response
            KV_FROM_REQUEST((&request))
            faabric::StateChunk response;

            uint64_t chunkOffset = request.offset();
            uint64_t chunkLen = request.chunksize();
            uint8_t *chunk = kv->getChunk(chunkOffset, chunkLen);

            response.set_user(request.user());
            response.set_key(request.key());
            response.set_offset(chunkOffset);

            // TODO: avoid copying here
            response.set_data(chunk, chunkLen);

            stream->Write(response);
        }

        return Status::OK;
    }

    Status StateServer::Push(
            ServerContext *context,
            ServerReader<faabric::StateChunk> *reader,
            faabric::StateResponse *response) {

        // Assume user and key are same throughout
        std::string user;
        std::string key;

        faabric::StateChunk request;
        while (reader->Read(&request)) {
            util::getLogger()->debug(
                    "Push {}/{} ({}->{})",
                    request.user(), request.key(),
                    request.offset(), request.offset() + request.data().size()
            );

            KV_FROM_REQUEST((&request))
            kv->setChunk(request.offset(), BYTES_CONST(request.data().c_str()), request.data().size());

            if (user.empty()) {
                user = kv->user;
                key = kv->key;
            }
        }

        response->set_user(user);
        response->set_key(key);

        return Status::OK;
    }

    Status StateServer::Size(
            ServerContext *context,
            const faabric::StateRequest *request,
            faabric::StateSizeResponse *response) {
        util::getLogger()->debug("Size {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)
        response->set_user(kv->user);
        response->set_key(kv->key);
        response->set_statesize(kv->size());

        return Status::OK;
    }

    Status StateServer::Append(
            ServerContext *context,
            const faabric::StateRequest *request,
            faabric::StateResponse *response) {
        util::getLogger()->debug("Append {}/{}", request->user(), request->key());
        KV_FROM_REQUEST(request)

        auto data = BYTES_CONST(request->data().c_str());
        uint64_t dataLen = request->data().size();
        kv->append(data, dataLen);

        response->set_user(request->user());
        response->set_key(request->key());

        return Status::OK;
    }

    Status StateServer::ClearAppended(
            ServerContext *context,
            const ::faabric::StateRequest *request,
            faabric::StateResponse *response) {
        util::getLogger()->debug("Clear appended {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)

        kv->clearAppended();

        return Status::OK;
    }

    Status StateServer::PullAppended(
            grpc::ServerContext *context,
            const ::faabric::StateAppendedRequest *request,
            faabric::StateAppendedResponse *response) {
        util::getLogger()->debug("Pull appended {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)

        response->set_user(request->user());
        response->set_key(request->key());

        for (uint32_t i = 0; i < request->nvalues(); i++) {
            AppendedInMemoryState &value = kv->getAppendedValue(i);
            auto appendedValue = response->add_values();
            appendedValue->set_data(reinterpret_cast<char *>(value.data.get()), value.length);
        }

        return Status::OK;
    }

    Status StateServer::Lock(
            grpc::ServerContext *context,
            const faabric::StateRequest *request,
            faabric::StateResponse *response) {
        util::getLogger()->debug("Lock {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)
        kv->lockWrite();

        return Status::OK;
    }

    Status StateServer::Unlock(
            grpc::ServerContext *context,
            const faabric::StateRequest *request,
            faabric::StateResponse *response) {
        util::getLogger()->debug("Unlock {}/{}", request->user(), request->key());

        KV_FROM_REQUEST(request)
        kv->unlockWrite();

        return Status::OK;
    }

    Status StateServer::Delete(grpc::ServerContext *context, const faabric::StateRequest *request,
                               faabric::StateResponse *response) {

        util::getLogger()->debug("Delete {}/{}", request->user(), request->key());

        state.deleteKV(request->user(), request->key());

        return Status::OK;
    }
}
