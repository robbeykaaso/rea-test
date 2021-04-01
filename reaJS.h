#include "rea.h"

namespace rea4{

class pipelineJS : public rea4::pipeline{
public:
    Q_OBJECT
public:
    Q_INVOKABLE void tryExecuteOutsidePipe(const QString& aName, const QJsonObject& aStream, const QJsonObject& aSync = QJsonObject());
signals:
    void executeJSPipe(const QString& aName, const QJsonObject& aStream, const QJsonObject& aSync);
protected:
    void execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync = QJsonObject()) override;
};

}
