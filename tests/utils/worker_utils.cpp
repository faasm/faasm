#include <catch2/catch.hpp>

#include "utils.h"

#include <conf/FaasmConfig.h>
#include <faabric/planner/PlannerClient.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
#include <faabric/util/batch.h>
#include <faabric/util/gids.h>
#include <faaslet/Faaslet.h>

using namespace faaslet;

namespace tests {

std::vector<faabric::Message> waitForBatchResults(bool isThreads,
                                                  int appId,
                                                  const std::set<int>& msgIds,
                                                  int timeoutMs,
                                                  bool requireSuccess)
{
    auto& plannerCli = faabric::planner::getPlannerClient();

    std::vector<faabric::Message> resultMsgs;

    for (const auto& msgId : msgIds) {
        faabric::Message result =
          plannerCli.getMessageResult(appId, msgId, 20000);
        if (requireSuccess) {
            REQUIRE(result.returnvalue() == 0);
        }
        resultMsgs.push_back(result);
    }

    return resultMsgs;
}

std::vector<faabric::Message> executeWithPool(
  std::shared_ptr<faabric::BatchExecuteRequest> req,
  int timeoutMs,
  bool requireSuccess)
{
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    faasmConf.chainedCallTimeout = 10000;

    // Execute forcing local
    bool isThreads = req->type() == faabric::BatchExecuteRequest::THREADS;

    // In the tests, the planner server runs in the same process than the
    // executor pool, thus calling functions and waiting for results on the
    // same shared pointer can lead to thread races. Instead, we take note of
    // all the message id's involved in the request, and wait on the pair
    // (appId, msgId)
    std::set<int> reqMsgIds;
    int appId = req->messages(0).appid();
    for (const auto& msg : req->messages()) {
        reqMsgIds.insert(msg.id());
    }

    auto& plannerCli = faabric::planner::getPlannerClient();
    plannerCli.callFunctions(req);

    // In the case of an MPI request, we want to wait for all the MPI messages,
    // not only the one with rank 0
    if (req->messages(0).ismpi()) {
        int maxRetries = 5;
        int numRetries = 0;
        int expectedWorldSize = req->messages(0).mpiworldsize();
        auto decision = plannerCli.getSchedulingDecision(req);
        while (decision.messageIds.size() != expectedWorldSize) {
            if (numRetries >= maxRetries) {
                SPDLOG_ERROR(
                  "Timed-out waiting for MPI messages to be scheduled ({}/{})",
                  decision.messageIds.size(),
                  expectedWorldSize);
                throw std::runtime_error("Timed-out waiting for MPI messges");
            }

            SPDLOG_DEBUG(
              "Waiting for MPI messages to be scheduled ({}/{}, app: {})",
              decision.messageIds.size(),
              expectedWorldSize,
              req->appid());
            SLEEP_MS(1000);

            numRetries += 1;
            decision = plannerCli.getSchedulingDecision(req);

            // If the decision has no app ID, it means that the app has
            // already finished, so we don't even have to wait for the messages
            if (decision.appId == 0) {
                auto berStatus = plannerCli.getBatchResults(req);
                return std::vector<faabric::Message>(
                  berStatus->mutable_messageresults()->begin(),
                  berStatus->mutable_messageresults()->end());
            }
        }

        // Finally, add the message IDs to the waiting set
        for (const auto& mid : decision.messageIds) {
            reqMsgIds.insert(mid);
        }
    }

    // Wait for all functions to complete
    auto resultMsgs = waitForBatchResults(
      isThreads, appId, reqMsgIds, timeoutMs, requireSuccess);

    return resultMsgs;
}

void executeWithPoolMultipleTimes(
  std::shared_ptr<faabric::BatchExecuteRequest> req,
  int numRepeats)
{
    for (int i = 0; i < numRepeats; i++) {
        // We must give each message in the request a different id, as faasm
        // expects message ids to be unique (and executing the same request
        // multiple times breaks this assumption)
        int appId = faabric::util::generateGid();
        faabric::util::updateBatchExecAppId(req, appId);
        for (auto& msg : *req->mutable_messages()) {
            msg.set_id(faabric::util::generateGid());
        }

        executeWithPool(req);
    }
}

bool executeWithPoolGetBooleanResult(
  std::shared_ptr<faabric::BatchExecuteRequest> req)
{
    auto resultMsg = executeWithPool(req).at(0);

    return resultMsg.outputdata() == "success";
}
}
