#include "json.h"
#include "timing.h"

#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include <iostream>

using namespace rapidjson;

namespace util {
    std::string messageToJson(const message::Message &msg) {
        Document d;
        d.SetObject();

        d.AddMember("id", msg.id(), d.GetAllocator());

        d.AddMember("user", StringRef(msg.user().c_str()), d.GetAllocator());
        d.AddMember("function", StringRef(msg.function().c_str()), d.GetAllocator());

        d.AddMember("input_data", StringRef(msg.inputdata().c_str()), d.GetAllocator());
        d.AddMember("output_data", StringRef(msg.outputdata().c_str()), d.GetAllocator());

        d.AddMember("async", msg.isasync(), d.GetAllocator());
        d.AddMember("python", msg.ispython(), d.GetAllocator());
        d.AddMember("typescript", msg.istypescript(), d.GetAllocator());

        d.AddMember("result_key", StringRef(msg.resultkey().c_str()), d.GetAllocator());
        d.AddMember("status_key", StringRef(msg.statuskey().c_str()), d.GetAllocator());

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

    int getIntFromJson(Document &doc, const std::string &key, int dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetInt();
    }

    std::string getStringFromJson(Document &doc, const std::string &key, const std::string &dflt) {
        Value::MemberIterator it = doc.FindMember(key.c_str());
        if (it == doc.MemberEnd()) {
            return dflt;
        }

        return it->value.GetString();
    }

    message::Message jsonToMessage(const std::string &jsonIn) {
        PROF_START(jsonDecode)

        Document d;
        d.Parse(jsonIn.c_str());

        message::Message msg;
        msg.set_id(getIntFromJson(d, "id", 0));

        msg.set_user(getStringFromJson(d, "user", ""));
        msg.set_function(getStringFromJson(d, "function", ""));

        msg.set_inputdata(getStringFromJson(d, "input_data", ""));
        msg.set_outputdata(getStringFromJson(d, "output_data", ""));

        msg.set_isasync(getBoolFromJson(d, "async", false));
        msg.set_ispython(getBoolFromJson(d, "python", false));
        msg.set_istypescript(getBoolFromJson(d, "typescript", false));

        msg.set_resultkey(getStringFromJson(d, "result_key", ""));
        msg.set_statuskey(getStringFromJson(d, "status_key", ""));

        msg.set_type(message::Message_MessageType_CALL);

        PROF_END(jsonDecode)

        return msg;
    }
}

