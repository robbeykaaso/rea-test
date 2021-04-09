#include "rea.h"
#include <QJSValue>
#include <QQmlEngine>
#include <QQmlApplicationEngine>

namespace rea4 {

class pipelineQML : public rea4::pipeline{
public:
    Q_OBJECT
public:
    pipelineQML();
protected:
    void execute(const QString& aName, std::shared_ptr<rea4::stream0> aStream, const QJsonObject& aSync = QJsonObject(), bool aFromOutside = false) override;
    void tryExecutePipeOutside(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync = QJsonObject()) override;
};

extern QQmlApplicationEngine* qml_engine;

class qmlScopeCache : public QObject{
    Q_OBJECT
public:
    qmlScopeCache(std::shared_ptr<scopeCache> aScope);
    Q_INVOKABLE QJSValue cache(const QString& aName, QJSValue aData);
    Q_INVOKABLE QJSValue data(const QString& aName);
private:
    std::shared_ptr<scopeCache> m_scope;
};

class qmlStream : public QObject{
    Q_OBJECT
public:
    qmlStream(){}
    qmlStream(std::shared_ptr<stream<QJSValue>> aStream){
        m_stream = aStream;
    }
    Q_INVOKABLE QJSValue setData(QJSValue aData);
    Q_INVOKABLE QJSValue scope(bool aNew = false);
    Q_INVOKABLE QJSValue data();
    Q_INVOKABLE QString tag();
    Q_INVOKABLE QJSValue out(const QString& aTag = "");
    Q_INVOKABLE QJSValue outs(QJSValue aOut, const QString& aNext = "", const QString& aTag = "");
    Q_INVOKABLE QJSValue asyncCall(const QString& aName, const QString& aPipeline = "qml");
    Q_INVOKABLE QJSValue asyncCallF(QJSValue aFunc, const QJsonObject& aParam = QJsonObject(), const QString& aPipeline = "qml");
private:
    std::shared_ptr<stream<QJSValue>> m_stream;
};

template <typename T>
class valType{
public:
    static T data(const QJSValue&){
        return T();
    }
};

template <>
class valType<QJsonObject>{
public:
    static QJsonObject data(const QJSValue& aValue){
        return QJsonObject::fromVariantMap(aValue.toVariant().toMap());
    }
};

template <>
class valType<QJsonArray>{
public:
    static QJsonArray data(const QJSValue& aValue){
        return QJsonArray::fromVariantList(aValue.toVariant().toList());
    }
};

template <>
class valType<QString>{
public:
    static QString data(const QJSValue& aValue){
        return aValue.toString();
    }
};

template <>
class valType<double>{
public:
    static double data(const QJSValue& aValue){
        return aValue.toNumber();
    }
};

template <>
class valType<bool>{
public:
    static bool data(const QJSValue& aValue){
        return aValue.toBool();
    }
};

template <>
class funcType<QJSValue, QJSValue>{
public:
    void doEvent(QJSValue aFunc, std::shared_ptr<stream<QJSValue>> aStream){
        if (!aFunc.equals(QJSValue::NullValue)){
            QJSValueList paramlist;
            qmlStream stm(aStream);
            paramlist.append(qml_engine->toScriptValue(&stm));
            aFunc.call(paramlist);
        }
    }
};

class qmlPipeline;

class qmlPipe : public QObject
{
    Q_OBJECT
public:
    qmlPipe(pipeline* aParent, const QString& aName);
public:
    Q_INVOKABLE QString actName() {return m_name;}
    Q_INVOKABLE void resetTopo();
    Q_INVOKABLE QJSValue next(const QString& aName, const QString& aTag = "");
    Q_INVOKABLE QJSValue nextB(const QString& aName, const QString& aTag = "");
    Q_INVOKABLE QJSValue nextF(QJSValue aFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject());
    Q_INVOKABLE QJSValue nextFB(QJSValue aFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject());
    Q_INVOKABLE void removeNext(const QString& aName);
private:
    pipeline* m_parent;
    QString m_name;
    QJsonObject m_param;
    friend qmlPipeline;
};

class qmlPipeline : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(qmlPipeline)
public:
    qmlPipeline();
    ~qmlPipeline();
public:
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine);
    static Q_INVOKABLE void run(const QString& aName, const QJSValue& aInput, const QString& aTag = "", const QJsonObject& aScopeCache = QJsonObject());
    static Q_INVOKABLE void call(const QString& aName, const QJSValue& aInput);
    static Q_INVOKABLE QJSValue input(const QJSValue& aInput, const QString& aTag = "", const QJsonObject& aScopeCache = QJsonObject());
    static Q_INVOKABLE void remove(const QString& aName);
    static Q_INVOKABLE QJSValue add(QJSValue aFunc, const QJsonObject& aParam = QJsonObject());
    static Q_INVOKABLE QJSValue find(const QString& aName);
    static Q_INVOKABLE QJSValue asyncCall(const QString& aName, const QJSValue& aInput);
};

}
