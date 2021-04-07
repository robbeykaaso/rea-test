#include "rea.h"
#include <QJSValue>
#include <QQmlEngine>
#include <QQmlApplicationEngine>

namespace rea4 {

extern QQmlApplicationEngine* qml_engine;

class qmlScopeCache : public QObject{
    Q_OBJECT
public:
    qmlScopeCache(std::shared_ptr<scopeCache> aScope);
    Q_INVOKABLE QVariant cache(const QString& aName, QJSValue aData);
    Q_INVOKABLE QJSValue data(const QString& aName);
private:
    std::shared_ptr<scopeCache> m_scope;
};

class qmlStream : public QObject{
    Q_OBJECT
public:
    qmlStream(){}
    qmlStream(QJSValue aInput, const QString& aTag = "", std::shared_ptr<scopeCache> aScope = nullptr){
        m_data = aInput;
        m_tag = aTag;
        m_scope = aScope;
    }
    Q_INVOKABLE QVariant scope(bool aNew = false);
    Q_INVOKABLE QVariant setData(QJSValue aData){
        m_data = aData;
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QJSValue data(){
        return m_data;
    }
    Q_INVOKABLE QVariant out(const QString& aTag = ""){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>>();
        m_tag = aTag;
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE QVariant outs(QJSValue aOut, const QString& aNext = "", const QString& aTag = ""){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>>();
        auto ot = std::make_shared<qmlStream>(aOut, aTag, m_scope);
        m_outs->push_back(std::pair<QString, std::shared_ptr<qmlStream>>(aNext, ot));
        return QVariant::fromValue<QObject*>(ot.get());
    }
    Q_INVOKABLE QVariant outsB(QJSValue aOut, const QString& aNext = "", const QString& aTag = ""){
        outs(aOut, aNext, aTag);
        return QVariant::fromValue<QObject*>(this);
    }
    Q_INVOKABLE void noOut(){m_outs = nullptr;}
    Q_INVOKABLE QVariant asyncCall(const QString& aName, const QString& aType = "");
    Q_INVOKABLE QVariant asyncCall(QJSValue aFunc, const QJsonObject& aParam = QJsonObject());
    Q_INVOKABLE QString tag(){
        return m_tag;
    }
private:
    template<typename T, typename S = T>
    void doCall(const QString& aName, const T& aData){
        auto pip = pipeline::find(aName, false);
        if (!pip)
            return;
        QEventLoop loop;
        bool timeout = false;
        auto monitor = pipeline::find(aName)->next<S>([&loop, &timeout, this](stream<S>* aInput){
            m_data = qml_engine->toScriptValue(aInput->data());
            m_scope = aInput->scope();
            if (loop.isRunning()){
                loop.quit();
            }else
                timeout = true;
        }, m_tag);
        pip->execute(std::make_shared<stream<T>>(aData, m_tag, m_scope));
        if (!timeout)
            loop.exec();
        pipeline::instance()->remove(monitor->actName());
    }

    QJSValue m_data;
    QString m_tag;
    std::shared_ptr<scopeCache> m_scope;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<qmlStream>>>> m_outs = nullptr;
    template<typename T, typename F>
    friend class funcType;
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

template<typename T>
class funcType<T, QJSValue>{
public:
    void doEvent(QJSValue aFunc, std::shared_ptr<stream<T>> aStream){
        if (qml_engine && !aFunc.equals(QJsonValue::Null)){
            QJSValueList paramList;
            qmlStream stm(qml_engine->toScriptValue(aStream->data()), aStream->tag(), aStream->m_scope);
            paramList.append(qml_engine->toScriptValue(QVariant::fromValue<QObject*>(&stm)));
            aFunc.call(paramList);
            aStream->setData(valType<T>::data(stm.data()));
            if (stm.m_outs){
                aStream->out(stm.m_tag);
                for (auto i : *stm.m_outs){
                    if (i.second->data().isArray()){  //isArray must isObject
                        auto ot = aStream->template outs<QJsonArray>(valType<QJsonArray>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_scope != aStream->m_scope)
                            ot->m_scope = i.second->m_scope;
                    }else if (i.second->data().isObject()){
                        auto ot = aStream->template outs<QJsonObject>(valType<QJsonObject>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_scope != aStream->m_scope)
                            ot->m_scope = i.second->m_scope;
                    }else if (i.second->data().isNumber()){
                        auto ot = aStream->template outs<double>(valType<double>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_scope != aStream->m_scope)
                            ot->m_scope = i.second->m_scope;
                    }else if (i.second->data().isBool()){
                        auto ot = aStream->template outs<bool>(valType<bool>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_scope != aStream->m_scope)
                            ot->m_scope = i.second->m_scope;
                    }else if (i.second->data().isString()){
                        auto ot = aStream->template outs<QString>(valType<QString>::data(i.second->data()), i.first, i.second->m_tag);
                        if (i.second->m_scope != aStream->m_scope)
                            ot->m_scope = i.second->m_scope;
                    }else if (i.second->data().isQObject()){

                    }
                    else
                        qFatal("Invalid data type in qmlStream!");
                }
            }
        }
    }
    std::shared_ptr<stream0> createStreamList(std::vector<T>& aDataList, std::shared_ptr<stream<T>> aStream){
        QJsonArray lst;
        for (int i = 0; i < aDataList.size(); ++i)
            lst.push_back(QJsonValue(aDataList[i]));
        auto stms = std::make_shared<stream<QJsonArray>>(lst, "", aStream->m_cache, aStream->m_transaction);
        stms->out();
        return std::move(stms);
    }
};


class pipelineQML;

class qmlPipe : public QObject
{
    Q_OBJECT
public:
    qmlPipe(){}
public:
    Q_INVOKABLE QString actName() {return m_pipe;}
    Q_INVOKABLE QVariant next(QJSValue aNext, const QString& aTag = "", const QJsonObject& aPipeParam = QJsonObject());
    Q_INVOKABLE QVariant nextB(QJSValue aNext, const QString& aTag = "", const QJsonObject& aPipeParam = QJsonObject());
    Q_INVOKABLE QVariant next(const QString& aName, const QString& aTag = "");
    Q_INVOKABLE QVariant nextB(const QString& aName, const QString& aTag = "");
    Q_INVOKABLE void removeNext(const QString& aName);
    Q_INVOKABLE void removeAspect(const QString& aType, const QString& aAspect = "");
    static qmlPipe* createPipe(QJSValue aFunc, const QJsonObject& aParam);
private:
    QString m_pipe;
    QJsonObject m_param;
    friend pipelineQML;
};

class pipelineQML : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(pipelineQML)
public:
    pipelineQML();
    ~pipelineQML();
public:
    static QObject *qmlInstance(QQmlEngine *engine, QJSEngine *scriptEngine)
    {
        Q_UNUSED(engine);
        Q_UNUSED(scriptEngine);

        return new pipelineQML();
    }
    static Q_INVOKABLE void run(const QString& aName, const QJSValue& aInput, const QString& aTag = "", const QJsonObject& aScopeCache = QJsonObject());
    static Q_INVOKABLE void call(const QString& aName, const QJSValue& aInput);
    static Q_INVOKABLE QVariant asyncCall(const QString& aName, const QJSValue& aInput);
    static Q_INVOKABLE QVariant input(const QJSValue& aInput, const QString& aTag = "", const QJsonObject& aScopeCache = QJsonObject());
    static Q_INVOKABLE void remove(const QString& aName);
    static Q_INVOKABLE QVariant add(QJSValue aFunc, const QJsonObject& aPipeParam = QJsonObject());
    static Q_INVOKABLE QVariant find(const QString& aName);
    static Q_INVOKABLE QVariant tr(const QString& aOrigin);
};

struct ICreateQMLPipe{
public:
    ICreateQMLPipe(const QJsonObject& aParam, QJSValue aFunc) {
        param = aParam;
        func = aFunc;
    }
    QJsonObject param;
    QJSValue func;
};

QString tr0(const QString& aOrigin);

}
