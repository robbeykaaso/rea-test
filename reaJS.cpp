#include "reaJS.h"

namespace rea4 {

/*m_data = aInput;
m_tag = aTag;
if (aCache)
    m_cache = aCache;
else
    m_cache = std::make_shared<QHash<QString, std::shared_ptr<stream0>>>();
m_transaction = aTransaction;*/
void pipelineJS::execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync){
    if (m_data_type.value(aName) == "object")
        executeJSPipe(aName,  rea::Json("data", std::dynamic_pointer_cast<rea4::stream<QJsonObject>>(aStream)->data(),
                                        "tag", aStream->tag()), aSync);
    else if (m_data_type.value(aName) == "array")
        executeJSPipe(aName,  rea::Json("data", std::dynamic_pointer_cast<rea4::stream<QJsonArray>>(aStream)->data(),
                                        "tag", aStream->tag()), aSync);
    else if (m_data_type.value(aName) == "string")
        executeJSPipe(aName,  rea::Json("data", std::dynamic_pointer_cast<rea4::stream<QString>>(aStream)->data(),
                                        "tag", aStream->tag()), aSync);
    else if (m_data_type.value(aName) == "bool"){
        executeJSPipe(aName,  rea::Json("data", std::dynamic_pointer_cast<rea4::stream<bool>>(aStream)->data(),
                                        "tag", aStream->tag()), aSync);
    }else if (m_data_type.value(aName) == "number"){
        executeJSPipe(aName,  rea::Json("data", std::dynamic_pointer_cast<rea4::stream<double>>(aStream)->data(),
                                        "tag", aStream->tag()), aSync);
    }else{
        //assert(0);
    }
    auto stm = aStream;
}

void pipelineJS::pipeAdded(const QString& aName, const QString& aType){
    if (aType == "")
        m_data_type.remove(aName);
    else
        m_data_type.insert(aName, aType);
}

void pipelineJS::tryExecuteOutsidePipe(const QString& aName, const QJsonObject& aStream, const QJsonObject& aSync){
    if (aStream.value("data").isObject())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonObject>>(aStream.value("data").toObject(), aStream.value("tag").toString()), aSync);
    else if (aStream.value("data").isArray())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonArray>>(aStream.value("data").toArray(), aStream.value("tag").toString()), aSync);
    else if (aStream.value("data").isString())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QString>>(aStream.value("data").toString(), aStream.value("tag").toString()), aSync);
    else if (aStream.value("data").isBool())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<bool>>(aStream.value("data").toBool(), aStream.value("tag").toString()), aSync);
    else if (aStream.value("data").isDouble())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<double>>(aStream.value("data").toDouble(), aStream.value("tag").toString()), aSync);
    else
        assert(0);
}

}

