#include "reaJS.h"

namespace rea4 {

/*m_data = aInput;
m_tag = aTag;
if (aCache)
    m_cache = aCache;
else
    m_cache = std::make_shared<QHash<QString, std::shared_ptr<stream0>>>();
m_transaction = aTransaction;*/
void pipelineJS::execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, bool aNeedFuture){
    if (m_data_type.value(aName) == "string"){
        executeJSPipe(aName,  rea::Json("data", std::dynamic_pointer_cast<rea4::stream<QString>>(aStream)->data()));
    }
    auto stm = aStream;
}

void pipelineJS::pipeAdded(const QString& aName, const QString& aType){
    m_data_type.insert(aName, aType);
}

void pipelineJS::tryExecuteOutsidePipe(const QString& aName, const QJsonObject& aStream){
    //tryExecutePipeOutside(aName, );
}

}

