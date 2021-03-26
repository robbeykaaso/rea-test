#include "rea.h"

namespace rea4{

class pipelineJS : public rea4::pipeline{
public:
    Q_OBJECT
public:
    void execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, bool aNeedFuture = true) override;
    Q_INVOKABLE void pipeAdded(const QString& aName, const QString& aType);
signals:
    void postStream(const QString& aName, const QJsonObject& aStream);
private:
    QHash<QString, QString> m_data_type;
};

}
