#include "reaQML0.h"
#include <QFile>
#include <QJsonDocument>

namespace rea4 {

QQmlApplicationEngine* qml_engine = nullptr;

qmlScopeCache::qmlScopeCache(std::shared_ptr<scopeCache> aScope){
    m_scope = aScope;
}

QVariant qmlScopeCache::cache(const QString& aName, QJSValue aData){
    if (aData.isArray())
        m_scope->cache<QJsonArray>(aName, valType<QJsonArray>::data(aData));
    else if (aData.isObject())
        m_scope->cache<QJsonObject>(aName, valType<QJsonObject>::data(aData));
    else if (aData.isNumber())
        m_scope->cache<double>(aName, valType<double>::data(aData));
    else if (aData.isBool())
        m_scope->cache<bool>(aName, valType<bool>::data(aData));
    else if (aData.isString())
        m_scope->cache<QString>(aName, valType<QString>::data(aData));
    else
        qFatal("Invalid data type in qmlScopeCache");
    return QVariant::fromValue<QObject*>(this);
}

QJSValue qmlScopeCache::data(const QString& aName){
    return qml_engine->toScriptValue(m_scope->dataStream(aName)->QData());
    /*auto dt = m_scope->dataStream(aName);
    if (dt->dataType() == "object")
        qml_engine->toScriptValue(std::dynamic_pointer_cast<stream<QJsonObject>>(dt)->data());
    else if (dt->dataType() == "array")
        qml_engine->toScriptValue(std::dynamic_pointer_cast<stream<QJsonArray>>(dt)->data());
    else if (dt->dataType() == "bool")
        qml_engine->toScriptValue(std::dynamic_pointer_cast<stream<bool>>(dt)->data());
    else if (dt->dataType() == "string")
        qml_engine->toScriptValue(std::dynamic_pointer_cast<stream<QString>>(dt)->data());
    else if (dt->dataType() == "number")
        qml_engine->toScriptValue(std::dynamic_pointer_cast<stream<double>>(dt)->data());
    else
        qFatal("Invalid data type in qmlScopeCache");
    return QJSValue();*/
}

QVariant qmlStream::scope(bool aNew){
    if (!m_scope || aNew)
        m_scope = std::make_shared<scopeCache>();
    return QVariant::fromValue<QObject*>(new qmlScopeCache(m_scope));
}

QVariant qmlStream::asyncCall(const QString& aName, const QString& aType){
#define DOCALL(TYPE) \
    if (aType == "object") \
        doCall<TYPE, QJsonObject>(aName, valType<TYPE>::data(m_data)); \
    else if (aType == "array") \
        doCall<TYPE, QJsonArray>(aName, valType<TYPE>::data(m_data)); \
    else if (aType == "bool") \
        doCall<TYPE, bool>(aName, valType<TYPE>::data(m_data)); \
    else if (aType == "number") \
        doCall<TYPE, double>(aName, valType<TYPE>::data(m_data)); \
    else if (aType == "string") \
        doCall<TYPE, QString>(aName, valType<TYPE>::data(m_data)); \
    else \
        doCall<TYPE>(aName, valType<TYPE>::data(m_data));

    if (m_data.isString())
        DOCALL(QString)
    else if (m_data.isBool())
        DOCALL(bool)
    else if (m_data.isNumber())
        DOCALL(double)
    else if (m_data.isArray())
        DOCALL(QJsonArray)
    else
        DOCALL(QJsonObject)
    return QVariant::fromValue<QObject*>(this);
}

QVariant qmlStream::asyncCall(QJSValue aFunc, const QJsonObject& aParam){
    QString tp = "";
    if (m_data.isString())
        tp = "string";
    else if (m_data.isBool())
        tp = "bool";
    else if (m_data.isNumber())
        tp = "number";
    else if (m_data.isArray())
        tp = "array";
    else if (m_data.isObject())
        tp = "object";
    else
        assert(0);
    auto tp2 = aParam.value("vtype").toString();
    auto prm = aParam;
    auto pip = qmlPipe::createPipe(aFunc, rea::Json(prm, "vtype", tp));
    auto ret = asyncCall(pip->actName(), tp2);
    pipeline::instance()->remove(pip->actName());
    pip->deleteLater();
    return ret;
}

QVariant qmlPipe::next(QJSValue aNext, const QString& aTag, const QJsonObject& aPipeParam){
    auto ret = createPipe(aNext, aPipeParam);
    pipeline::instance()->find(m_pipe)->next(ret->m_pipe, aTag);
    return QVariant::fromValue<QObject*>(ret);
}

QVariant qmlPipe::nextB(QJSValue aNext, const QString& aTag, const QJsonObject& aPipeParam){
    next(aNext, aTag, aPipeParam);
    return QVariant::fromValue<QObject*>(this);
}

QVariant qmlPipe::next(const QString& aName, const QString& aTag){
    qmlPipe* ret = new qmlPipe();
    ret->m_pipe = pipeline::instance()->find(m_pipe)->next(aName, aTag)->actName();
    return QVariant::fromValue<QObject*>(ret);
}

QVariant qmlPipe::nextB(const QString& aName, const QString& aTag){
    next(aName, aTag);
    return QVariant::fromValue<QObject*>(this);
}

void qmlPipe::removeNext(const QString& aName){
    pipeline::instance()->find(m_pipe)->removeNext(aName);
}

void qmlPipe::removeAspect(const QString& aType, const QString& aAspect){
    if (aType == "before")
        pipeline::instance()->find(m_pipe)->removeAspect(pipe0::AspectType::AspectBefore, aAspect);
    else if (aType == "around")
        pipeline::instance()->find(m_pipe)->removeAspect(pipe0::AspectType::AspectAround, aAspect);
    else if (aType == "after")
        pipeline::instance()->find(m_pipe)->removeAspect(pipe0::AspectType::AspectAfter, aAspect);
    else
        throw "aspect type error";
}

qmlPipe* qmlPipe::createPipe(QJSValue aFunc, const QJsonObject& aParam){
    qmlPipe* ret = new qmlPipe();
    auto tp = aParam.value("type").toString();
    auto prm = std::make_shared<ICreateQMLPipe>(aParam, aFunc);
    pipeline::instance()->run<std::shared_ptr<ICreateQMLPipe>>("createQMLPipe_" + tp, prm);
    if (prm->param.contains("actname"))
        ret->m_pipe = prm->param.value("actname").toString();
    else
        assert(0);

    return ret;
}

void pipelineQML::run(const QString& aName, const QJSValue& aInput, const QString& aTag, const QJsonObject& aScopeCache){
    if (aInput.isString())
        pipeline::instance()->run<QString>(aName, aInput.toString(), aTag, std::make_shared<scopeCache>(aScopeCache));
    else if (aInput.isBool())
        pipeline::instance()->run<bool>(aName, aInput.toBool(), aTag, std::make_shared<scopeCache>(aScopeCache));
    else if (aInput.isNumber())
        pipeline::instance()->run<double>(aName, aInput.toNumber(), aTag, std::make_shared<scopeCache>(aScopeCache));
    else if (aInput.isArray())
        pipeline::instance()->run<QJsonArray>(aName, QJsonArray::fromVariantList(aInput.toVariant().toList()), aTag, std::make_shared<scopeCache>(aScopeCache));
    else
        pipeline::instance()->run<QJsonObject>(aName, QJsonObject::fromVariantMap(aInput.toVariant().toMap()), aTag, std::make_shared<scopeCache>(aScopeCache));
}

QVariant pipelineQML::input(const QJSValue& aInput, const QString& aTag, const QJsonObject& aScopeCache){
    auto id = aTag == "" ? rea::generateUUID() : aTag;
    auto stm = new qmlStream(aInput, id, std::make_shared<scopeCache>(aScopeCache));
    return QVariant::fromValue<QObject*>(stm);
}

void pipelineQML::call(const QString& aName, const QJSValue& aInput){
    if (aInput.isString())
        pipeline::instance()->call<QString>(aName, aInput.toString());
    else if (aInput.isBool())
        pipeline::instance()->call<bool>(aName, aInput.toBool());
    else if (aInput.isNumber())
        pipeline::instance()->call<double>(aName, aInput.toNumber());
    else if (aInput.isArray())
        pipeline::instance()->call<QJsonArray>(aName, QJsonArray::fromVariantList(aInput.toVariant().toList()));
    else
        pipeline::instance()->call<QJsonObject>(aName, QJsonObject::fromVariantMap(aInput.toVariant().toMap()));
}

QVariant pipelineQML::asyncCall(const QString& aName, const QJSValue& aInput){
    auto id = rea::generateUUID();
    auto stm = new qmlStream(aInput, id);
    return stm->asyncCall(aName);
}

void pipelineQML::remove(const QString& aName){
    pipeline::instance()->remove(aName);
}

QVariant pipelineQML::add(QJSValue aFunc, const QJsonObject& aPipeParam){
    auto ret = qmlPipe::createPipe(aFunc, aPipeParam);
    return QVariant::fromValue<QObject*>(ret);
}

QVariant pipelineQML::find(const QString& aName){
    qmlPipe* ret = new qmlPipe();
    ret->m_pipe = aName;
    return QVariant::fromValue<QObject*>(ret);
}

static bool m_language_updated;
static QJsonObject translates;

pipelineQML::pipelineQML(){
    m_language_updated = false;
    QFile fl(".language");
    if (fl.open(QFile::ReadOnly)){
        translates = QJsonDocument::fromJson(fl.readAll()).object();
        fl.close();
    }
}

pipelineQML::~pipelineQML(){
    if (m_language_updated){
        QFile fl(".language");
        if (fl.open(QFile::WriteOnly)){
            fl.write(QJsonDocument(translates).toJson());
            fl.close();
        }
    }
}

QString tr0(const QString& aOrigin){
    auto key = aOrigin.trimmed();
    if (!translates.contains(key)){
        translates.insert(key, aOrigin);
        m_language_updated = true;
    }
    return translates.value(key).toString(aOrigin);
}

QVariant pipelineQML::tr(const QString& aOrigin){
    return tr0(aOrigin);
}

#define regCreateQMLPipe(Name) \
static regPip<std::shared_ptr<ICreateQMLPipe>> reg_createQMLPipe_##Name([](stream<std::shared_ptr<ICreateQMLPipe>>* aInput){ \
    auto dt = aInput->data(); \
    auto prm = dt->param; \
    auto tp = prm.value("vtype").toString("object"); \
    if (tp == "object"){ \
        dt->param.insert("actname", pipeline::instance()->add<QJsonObject, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    }else if (tp == "string") \
        dt->param.insert("actname", pipeline::instance()->add<QString, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else if (tp == "number") \
        dt->param.insert("actname", pipeline::instance()->add<double, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else if (tp == "bool") \
        dt->param.insert("actname", pipeline::instance()->add<bool, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else if (tp == "array") \
        dt->param.insert("actname", pipeline::instance()->add<QJsonArray, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else \
        assert(0); \
}, rea::Json("name", STR(createQMLPipe_##Name)));

regCreateQMLPipe(Partial)
regCreateQMLPipe(Delegate)
regCreateQMLPipe()

static regPip<QQmlApplicationEngine*> reg_recative2_qml([](stream<QQmlApplicationEngine*>* aInput){
    //ref from: https://stackoverflow.com/questions/25403363/how-to-implement-a-singleton-provider-for-qmlregistersingletontype
    qml_engine = aInput->data();
    qmlRegisterSingletonType<pipelineQML>("Pipeline2", 1, 0, "Pipeline2", &pipelineQML::qmlInstance);
    aInput->out();
}, rea::Json("name", "install0_QML"), "regQML");

}
