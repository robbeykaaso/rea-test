#include "reaJS.h"

namespace rea4 {

void pipelineJS::execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync, bool aFromOutside){
    if (aStream->dataType() == "")
        throw "not supported type";
    executeJSPipe(aName,  aStream->QData(), aStream->tag(), aStream->scope()->toList(), aSync, aFromOutside);
}

void pipelineJS::tryExecuteOutsidePipe(const QString& aName, const QVariant& aData, const QString& aTag, const QJsonObject& aScope, const QJsonObject& aSync, bool aFromOutside){
    if (aData.type() == QVariant::Type::Map)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonObject>>(aData.toJsonObject(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFromOutside);
    else if (aData.type() == QVariant::Type::List)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonArray>>(aData.toJsonArray(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFromOutside);
    else if (aData.type() == QVariant::Type::String)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QString>>(aData.toString(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFromOutside);
    else if (aData.type() == QVariant::Type::Bool)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<bool>>(aData.toBool(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFromOutside);
    else if (aData.type() == QVariant::Type::Double)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<double>>(aData.toDouble(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFromOutside);
    else{
        std::cout << aData.type() << std::endl;
        assert(0);
    }
}

void pipelineJS::removePipeOutside(const QString& aName){
    pipeline::removePipeOutside(aName);
}

void pipelineJS::remove(const QString& aName, bool){
    removeJSPipe(aName);
}

}

