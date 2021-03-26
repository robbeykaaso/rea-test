#include "rea.h"

namespace rea4{

class pipelineJS : public rea4::pipeline{
public:
    Q_OBJECT
public:
    Q_INVOKABLE void pipeAdded(const QString& aName, const QString& aType);
    Q_INVOKABLE void tryExecuteOutsidePipe(const QString& aName, const QJsonObject& aStream);
signals:
    void executeJSPipe(const QString& aName, const QJsonObject& aStream);
protected:
    void execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, bool aNeedFuture = true) override;
private:
    QHash<QString, QString> m_data_type;
};

}
