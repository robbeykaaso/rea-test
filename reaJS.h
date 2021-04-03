#include "rea.h"

namespace rea4{

class pipelineJS : public rea4::pipeline{
public:
    Q_OBJECT
public:
    Q_INVOKABLE void tryExecuteOutsidePipe(const QString& aName, const QJsonObject& aStream, const QJsonObject& aSync = QJsonObject(), bool aFromOutside = true);
    Q_INVOKABLE void removePipeOutside(const QString& aName) override;
    void remove(const QString& aName, bool) override;
signals:
    void executeJSPipe(const QString& aName, const QJsonObject& aStream, const QJsonObject& aSync, bool aFromOutside);
    void removeJSPipe(const QString& aName);
protected:
    void execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync = QJsonObject(), bool aFromOutside = false) override;
};

}
