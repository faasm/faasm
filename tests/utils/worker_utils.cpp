#include <catch2/catch.hpp>

#include "utils.h"

#include <conf/FaasmConfig.h>
#include <faabric/proto/faabric.pb.h>
#include <faabric/runner/FaabricMain.h>
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
    auto& sch = faabric::scheduler::getScheduler();

    std::vector<faabric::Message> resultMsgs;

    for (const auto& msgId : msgIds) {
        if (isThreads) {
            int returnValue = sch.awaitThreadResult(msgId);
            if (requireSuccess) {
                REQUIRE(returnValue == 0);
            }
            faabric::Message result;
            result.set_id(msgId);
            result.set_returnvalue(returnValue);
            resultMsgs.push_back(result);
        } else {
            faabric::Message result =
              sch.getFunctionResult(appId, msgId, 20000);
            if (requireSuccess) {
                REQUIRE(result.returnvalue() == 0);
            }
            resultMsgs.push_back(result);
        }
    }

    return resultMsgs;
}

std::vector<faabric::Message> executeWithPool(
  std::shared_ptr<faabric::BatchExecuteRequest> req,
  int timeoutMs,
  bool requireSuccess)
{
    faabric::util::SystemConfig& conf = faabric::util::getSystemConfig();
    conf::FaasmConfig& faasmConf = conf::getFaasmConfig();
    conf.boundTimeout = 1000;
    faasmConf.chainedCallTimeout = 10000;

    faabric::scheduler::Scheduler& sch = faabric::scheduler::getScheduler();

    // Start up system
    auto fac = std::make_shared<faaslet::FaasletFactory>();
    faabric::runner::FaabricMain m(fac);
    m.startRunner();

    // Execute forcing local
    req->mutable_messages()->at(0).set_topologyhint("FORCE_LOCAL");
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

    sch.callFunctions(req);

    usleep(1000 * 500);

    // Wait for all functions to complete
    auto resultMsgs = waitForBatchResults(
      isThreads, appId, reqMsgIds, timeoutMs, requireSuccess);

    m.shutdown();

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
        for (auto& msg : *req->mutable_messages()) {
            msg.set_appid(appId);
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
