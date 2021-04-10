#include "reaJS.h"

namespace rea4 {

template <>
class rea4::typeTrait<pipelineJS*> : public typeTrait0{
public:
    QString name() override{
        return "jsPipeline";
    }
    QVariant QData(stream0* aStream) override{
        return QVariant::fromValue<QObject*>(reinterpret_cast<stream<pipelineJS*>*>(aStream)->data());
    }
};

pipelineJS::pipelineJS() : pipeline("js"){
    pipeline::instance()->supportType<pipelineJS*>();
    rea4::pipeline::instance()->add<double>([](rea4::stream<double>* aInput){
        auto pip_js = reinterpret_cast<rea4::pipelineJS*>(rea4::pipeline::instance("js"));
        aInput->outs<pipelineJS*>(pip_js)->scope()->cache<pipelineJS*>("pipeline", pip_js);
    }, rea::Json("name", "pipelineJSObject", "external", "qml"));
};

void pipelineJS::execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync, bool aFromOutside){
    if (aStream->dataType() == "")
        throw "not supported type";
    executeJSPipe(aName,  aStream->QData(), aStream->tag(), aStream->scope()->toList(), aSync, aFromOutside);
}

void pipelineJS::tryExecuteOutsidePipe(const QString& aName, const QVariant& aData, const QString& aTag, const QJsonObject& aScope, const QJsonObject& aSync, const QString& aFlag){
    if (aData.type() == QVariant::Type::Map)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonObject>>(aData.toJsonObject(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFlag);
    else if (aData.type() == QVariant::Type::List)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QJsonArray>>(aData.toJsonArray(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFlag);
    else if (aData.type() == QVariant::Type::String)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<QString>>(aData.toString(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFlag);
    else if (aData.type() == QVariant::Type::Bool)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<bool>>(aData.toBool(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFlag);
    else if (aData.type() == QVariant::Type::Double)
        tryExecutePipeOutside(aName, std::make_shared<rea4::stream<double>>(aData.toDouble(), aTag, std::make_shared<scopeCache>(aScope)), aSync, aFlag);
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

static regPip<std::shared_ptr<pipeline*>> reg_create_jspipeline([](stream<std::shared_ptr<pipeline*>>* aInput){
    *aInput->data() = new pipelineJS();
}, rea::Json("name", "createjspipeline"));

}

