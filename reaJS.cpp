#include "reaJS.h"

namespace rea4 {

void pipelineJS::execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync, bool aFromOutside){
    auto tp = aStream->dataType();
    if (tp == "object")
        doExecute<QJsonObject>(aName, aStream, aSync, aFromOutside);
    else if (tp == "array")
        doExecute<QJsonArray>(aName, aStream, aSync, aFromOutside);
    else if (tp == "string")
        doExecute<QString>(aName, aStream, aSync, aFromOutside);
    else if (tp == "bool")
        doExecute<bool>(aName, aStream, aSync, aFromOutside);
    else if (tp == "number")
        doExecute<double>(aName, aStream, aSync, aFromOutside);
    else{
        //assert(0);
    }
}

void pipelineJS::tryExecuteOutsidePipe(const QString& aName, const QJsonObject& aStream, const QJsonObject& aSync, bool aFromOutside){
    if (aStream.value("data").isObject())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonObject>>(aStream.value("data").toObject(), aStream.value("tag").toString(), std::make_shared<scopeCache>(aStream.value("scope").toObject())), aSync, aFromOutside);
    else if (aStream.value("data").isArray())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonArray>>(aStream.value("data").toArray(), aStream.value("tag").toString(), std::make_shared<scopeCache>(aStream.value("scope").toObject())), aSync, aFromOutside);
    else if (aStream.value("data").isString())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QString>>(aStream.value("data").toString(), aStream.value("tag").toString(), std::make_shared<scopeCache>(aStream.value("scope").toObject())), aSync, aFromOutside);
    else if (aStream.value("data").isBool())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<bool>>(aStream.value("data").toBool(), aStream.value("tag").toString(), std::make_shared<scopeCache>(aStream.value("scope").toObject())), aSync, aFromOutside);
    else if (aStream.value("data").isDouble())
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<double>>(aStream.value("data").toDouble(), aStream.value("tag").toString(), std::make_shared<scopeCache>(aStream.value("scope").toObject())), aSync, aFromOutside);
    else
        assert(0);
}

void pipelineJS::removePipeOutside(const QString& aName){
    pipeline::removePipeOutside(aName);
}

void pipelineJS::remove(const QString& aName, bool){
    removeJSPipe(aName);
}

}

