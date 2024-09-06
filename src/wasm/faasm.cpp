#include <faabric/executor/ExecutorContext.h>
#include <faabric/transport/PointToPointBroker.h>
#include <faabric/util/func.h>
#include <faabric/util/snapshot.h>
#include <wasm/WasmExecutionContext.h>
#include <wasm/WasmModule.h>
#include <wasm/faasm.h>

namespace wasm {
static std::shared_ptr<faabric::transport::PointToPointGroup>
getPointToPointGroup()
{
    faabric::Message msg = faabric::executor::ExecutorContext::get()->getMsg();
    return faabric::transport::PointToPointGroup::getOrAwaitGroup(
      msg.groupid());
}

void doFaasmSmCriticalLocal()
{
    SPDLOG_DEBUG("S - sm_critical_local");

    getPointToPointGroup()->localLock();
}

void doFaasmSmCriticalLocalEnd()
{
    SPDLOG_DEBUG("S - sm_critical_local_end");

    getPointToPointGroup()->localUnlock();
}

static std::pair<uint32_t, faabric::util::SnapshotDataType>
extractSnapshotDataType(int32_t varType)
{
    switch (varType) {
        case (faabric::util::SnapshotDataType::Raw): {
            SPDLOG_ERROR("Cannot declare untyped merge regions from code");
        }
        case (faabric::util::SnapshotDataType::Bool): {
            return { sizeof(int8_t), faabric::util::SnapshotDataType::Bool };
        }
        case (faabric::util::SnapshotDataType::Int): {
            return { sizeof(int32_t), faabric::util::SnapshotDataType::Int };
        }
        case (faabric::util::SnapshotDataType::Long): {
            return { sizeof(int64_t), faabric::util::SnapshotDataType::Long };
        }
        case (faabric::util::SnapshotDataType::Float): {
            return { sizeof(float), faabric::util::SnapshotDataType::Float };
        }
        case (faabric::util::SnapshotDataType::Double): {
            return { sizeof(double), faabric::util::SnapshotDataType::Double };
        }
        default: {
            SPDLOG_ERROR("Unrecognised memory data type: {}", varType);
            throw std::runtime_error("Unrecognised shared memory data type");
        }
    }
}

static faabric::util::SnapshotMergeOperation extractSnapshotMergeOp(
  int32_t mergeOp)
{
    if (faabric::util::SnapshotMergeOperation::Bytewise <= mergeOp &&
        mergeOp <= faabric::util::SnapshotMergeOperation::Min) {
        return static_cast<faabric::util::SnapshotMergeOperation>(mergeOp);
    }

    SPDLOG_ERROR("Unrecognised merge operation: {}", mergeOp);
    throw std::runtime_error("Unrecognised merge operation");
}

void doFaasmSmReduce(int32_t varPtr,
                     int32_t varType,
                     int32_t reduceOp,
                     int32_t currentBatch)
{
    // Here we have two scenarios, the second of which differs in behaviour when
    // we're in single host mode:
    //
    // 1. We're being notified of a reduction variable in an *upcoming* batch of
    // threads. This means the snapshot doesn't yet exist, and we don't know
    // whether it will be in single host mode or not. Therefore, we always keep
    // the merge region in a list.
    //
    // 2. We're being notified of a reduction variable in the *current* batch of
    // threads. If this is in single host mode, we can ignore it, as there won't
    // be any snapshotting at all, otherwise we add it to the snapshot.

    bool isCurrentBatch = currentBatch == 1;
    bool isSingleHost = faabric::executor::ExecutorContext::get()
                          ->getBatchRequest()
                          ->singlehost();

    // Here we can ignore if we're in the current batch, and it's in single host
    // mode.
    if (isCurrentBatch && isSingleHost) {
        SPDLOG_DEBUG("S - sm_reduce - {} {} {} {} (ignored, single host)",
                     varPtr,
                     varType,
                     reduceOp,
                     currentBatch);
        return;
    }

    SPDLOG_DEBUG(
      "S - sm_reduce - {} {} {} {}", varPtr, varType, reduceOp, currentBatch);

    auto dataType = extractSnapshotDataType(varType);
    faabric::util::SnapshotMergeOperation mergeOp =
      extractSnapshotMergeOp(reduceOp);

    faabric::Message* msg =
      &faabric::executor::ExecutorContext::get()->getMsg();
    SPDLOG_DEBUG("Registering reduction variable {}-{} for {} {}",
                 varPtr,
                 varPtr + dataType.first,
                 faabric::util::funcToString(*msg, false),
                 isCurrentBatch ? "this batch" : "next batch");

    if (isCurrentBatch) {
        faabric::executor::Executor* executor =
          faabric::executor::ExecutorContext::get()->getExecutor();
        auto snap = executor->getMainThreadSnapshot(*msg, false);
        snap->addMergeRegion(varPtr, dataType.first, dataType.second, mergeOp);
    } else {
        wasm::WasmModule* module = getExecutingModule();
        module->addMergeRegionForNextThreads(
          varPtr, dataType.first, dataType.second, mergeOp);
    }
}

int32_t doFaasmReadInput(char* inBuff, int32_t inBuffLen)
{
    SPDLOG_DEBUG("S - faasm_read_input {} {}", inBuff, inBuffLen);

    faabric::Message& call =
      faabric::executor::ExecutorContext::get()->getMsg();
    std::vector<uint8_t> inputBytes =
      faabric::util::stringToBytes(call.inputdata());

    // If nothing, return nothing
    if (inputBytes.empty()) {
        return 0;
    }

    // If buffer has zero size, return the input size
    if (inBuffLen == 0) {
        return inputBytes.size();
    }

    // Write to the wasm buffer
    int inputSize = faabric::util::safeCopyToBuffer(
      inputBytes, reinterpret_cast<uint8_t*>(inBuff), inBuffLen);

    return inputSize;
}
}
