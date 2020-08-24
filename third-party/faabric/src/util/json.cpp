#include "json.h"
#include "timing.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

using namespace rapidjson;

namespace faabric::util {
    std::string messageToJson(const faabric::Message &msg) {
        Document d;
        d.SetObject();
        Document::AllocatorType &a = d.GetAllocator();

        // Need to be explicit with strings here to make a copy _and_ make sure we specify the length
        // to include any null-terminators from bytes
        d.AddMember("id", msg.id(), a);
        d.AddMember("user", Value(msg.user().c_str(), msg.user().size(), a).Move(), a);
        d.AddMember("function", Value(msg.function().c_str(), msg.function().size(), a).Move(), a);
        d.AddMember("index", msg.idx(), a);
        d.AddMember("hops", msg.hops(), a);
        if(!msg.executedhost().empty()) d.AddMember("exec_host", Value(msg.executedhost().c_str(), msg.executedhost().size(), a).Move(), a);
        if(msg.finishtimestamp() > 0)   d.AddMember("finished", msg.finishtimestamp(), a);

        if(msg.timestamp() > 0)         d.AddMember("timestamp", msg.timestamp(), a);

        if(!msg.snapshotkey().empty())  d.AddMember("snapshot_key", Value(msg.snapshotkey().c_str(), msg.snapshotkey().size(), a).Move(), a);
        if(msg.snapshotsize() > 0)      d.AddMember("snapshot_size", msg.snapshotsize(), a);
        if(msg.funcptr() > 0)           d.AddMember("func_ptr", msg.funcptr(), a);

        if(!msg.pythonuser().empty())       d.AddMember("py_user", Value(msg.pythonuser().c_str(), msg.pythonuser().size(), a).Move(), a);
        if(!msg.pythonfunction().empty())   d.AddMember("py_func", Value(msg.pythonfunction().c_str(), msg.pythonfunction().size(), a).Move(), a);
        if(!msg.pythonentry().empty())      d.AddMember("py_entry", Value(msg.pythonentry().c_str(), msg.pythonentry().size(), a).Move(), a);

        if(!msg.inputdata().empty())    d.AddMember("input_data", Value(msg.inputdata().c_str(), msg.inputdata().size(), a).Move(), a);
        if(!msg.outputdata().empty())   d.AddMember("output_data", Value(msg.outputdata().c_str(), msg.outputdata().size(), a).Move(), a);

        if(msg.isasync())               d.AddMember("async", msg.isasync(), a);
        if(msg.ispython())              d.AddMember("python", msg.ispython(), a);
        if(msg.istypescript())          d.AddMember("typescript", msg.istypescript(), a);
        if(msg.isstatusrequest())       d.AddMember("status", msg.isstatusrequest(), a);
        if(msg.isexecgraphrequest())    d.AddMember("exec_graph", msg.isexecgraphrequest(), a);
        if(msg.isflushrequest())        d.AddMember("flush", msg.isflushrequest(), a);

        if(!msg.resultkey().empty())    d.AddMember("result_key", Value(msg.resultkey().c_str(), msg.resultkey().size()).Move(), a);
        if(!msg.statuskey().empty())    d.AddMember("status_key", Value(msg.statuskey().c_str(), msg.statuskey().size()).Move(), a);

        if(msg.coldstartinterval() > 0) d.AddMember("cold_start_interval", msg.coldstartinterval(), a);

        if(msg.ismpi())                 d.AddMember("mpi", msg.ismpi(), a);
        if(msg.mpiworldid() > 0)        d.AddMember("mpi_world_id", msg.mpiworldid(), a);
        if(msg.mpirank() > 0)           d.AddMember("mpi_rank", msg.mpirank(), a);
        if(msg.mpiworldsize() > 0)      d.AddMember("mpi_world_size", msg.mpiworldsize(), a);

        if(!msg.cmdline().empty())      d.AddMember("cmdline", Value(msg.cmdline().c_str(), msg.cmdline().size()).Move(), a);

        if(msg.issgx())                 d.AddMember("sgx", msg.issgx(), a);

        StringBuffer sb;
        Writer<StringBuffer> writer(sb);
        d.Accept(writer);

        return sb.GetString();
    }

    bool getBoolFromJson(Document &doc, const std::string &key, bool dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetBool();
    }

    std::vector<uint32_t> getUintArrayFromJson(Document &doc, const std::string &key) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        std::vector<uint32_t> result;
        if (it == doc.MemberEnd()) {
            return result;
        }

        for(const auto &i : it->value.GetArray()) {
            result.emplace_back(i.GetUint());
        }

        return result;
    }
    
    int getIntFromJson(Document &doc, const std::string &key, int dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetInt();
    }

    int64_t getInt64FromJson(Document &doc, const std::string &key, int dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetInt64();
    }

    std::string getStringFromJson(Document &doc, const std::string &key, const std::string &dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        const char *valuePtr = it->value.GetString();
        return std::string(valuePtr, valuePtr + it->value.GetStringLength());
    }

    faabric::Message jsonToMessage(const std::string &jsonIn) {
        PROF_START(jsonDecode)

        MemoryStream ms(jsonIn.c_str(), jsonIn.size());
        Document d;
        d.ParseStream(ms);

        faabric::Message msg;
        msg.set_timestamp(getInt64FromJson(d, "timestamp", 0));
        msg.set_id(getIntFromJson(d, "id", 0));
        msg.set_user(getStringFromJson(d, "user", ""));
        msg.set_function(getStringFromJson(d, "function", ""));
        msg.set_idx(getIntFromJson(d, "index", 0));
        msg.set_hops(getIntFromJson(d, "hops", 0));
        msg.set_executedhost(getStringFromJson(d, "exec_host", ""));
        msg.set_finishtimestamp(getInt64FromJson(d, "finished", 0));

        msg.set_snapshotkey(getStringFromJson(d, "snapshot_key", ""));
        msg.set_snapshotsize(getIntFromJson(d, "snapshot_size", 0));
        msg.set_funcptr(getIntFromJson(d, "func_ptr", 0));

        msg.set_pythonuser(getStringFromJson(d, "py_user", ""));
        msg.set_pythonfunction(getStringFromJson(d, "py_func", ""));
        msg.set_pythonentry(getStringFromJson(d, "py_entry", ""));

        msg.set_inputdata(getStringFromJson(d, "input_data", ""));
        msg.set_outputdata(getStringFromJson(d, "output_data", ""));

        msg.set_isasync(getBoolFromJson(d, "async", false));
        msg.set_ispython(getBoolFromJson(d, "python", false));
        msg.set_istypescript(getBoolFromJson(d, "typescript", false));
        msg.set_isstatusrequest(getBoolFromJson(d, "status", false));
        msg.set_isexecgraphrequest(getBoolFromJson(d, "exec_graph", false));
        msg.set_isflushrequest(getBoolFromJson(d, "flush", false));

        msg.set_resultkey(getStringFromJson(d, "result_key", ""));
        msg.set_statuskey(getStringFromJson(d, "status_key", ""));

        msg.set_coldstartinterval(getIntFromJson(d, "cold_start_interval", 0));

        msg.set_type(faabric::Message_MessageType_CALL);

        msg.set_ismpi(getBoolFromJson(d, "mpi", false));
        msg.set_mpiworldid(getIntFromJson(d, "mpi_world_id", 0));
        msg.set_mpirank(getIntFromJson(d, "mpi_rank", 0));
        msg.set_mpiworldsize(getIntFromJson(d, "mpi_world_size", 0));

        msg.set_cmdline(getStringFromJson(d, "cmdline", ""));

        msg.set_issgx(getBoolFromJson(d, "sgx", false));

        PROF_END(jsonDecode)

        return msg;
    }
    
    std::string getValueFromJsonString(const std::string &key, const std::string &jsonIn) {
        MemoryStream ms(jsonIn.c_str(), jsonIn.size());
        Document d;
        d.ParseStream(ms);

        std::string result = getStringFromJson(d, key, "");
        return result;
    }
}

