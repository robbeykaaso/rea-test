#include "reaJS.h"
#include <QFile>
#include <QJsonDocument>

namespace rea {

QVariant jsStream::map(QJSValue aInput){
    auto stm = new jsStream(aInput, m_tag, m_cache, m_transaction);
    return QVariant::fromValue<QObject*>(stm);
}

QVariant jsStream::call(const QString& aName, const QString& aType){
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

QVariant jsStream::call(QJSValue aFunc, const QJsonObject& aParam){
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
    auto pip = jsPipe::createPipe(aFunc, rea::Json(prm, "vtype", tp));
    auto ret = call(pip->actName(), tp2);
    pipeline::remove(pip->actName());
    pip->deleteLater();
    return ret;
}

QVariant jsStream::var(const QString& aName, QJSValue aData){
    if (aData.isArray())
        m_cache->insert(aName, std::make_shared<stream<QJsonArray>>(QJsonArray::fromVariantList(aData.toVariant().toList())));
    else if (aData.isObject())
        m_cache->insert(aName, std::make_shared<stream<QJsonObject>>(QJsonObject::fromVariantMap(aData.toVariant().toMap())));
    else if (aData.isNumber())
        m_cache->insert(aName, std::make_shared<stream<double>>(aData.toNumber()));
    else if (aData.isBool())
        m_cache->insert(aName, std::make_shared<stream<bool>>(aData.toBool()));
    else if (aData.isString())
        m_cache->insert(aName, std::make_shared<stream<QString>>(aData.toString()));
    else
        qFatal("Invalid data type in qmlStream!");
    return QVariant::fromValue<QObject*>(this);
}

QJSValue jsStream::varData(const QString& aName, const QString& aType){
#define getVarData(TP) \
{ \
    auto ret = std::dynamic_pointer_cast<stream<TP>>(m_cache->value(aName)); \
    if (ret) \
        return pipeline::instance()->engine->toScriptValue(ret->data()); \
    else  \
        return pipeline::instance()->engine->toScriptValue(TP()); \
}

    if (aType == "object")
        getVarData(QJsonObject)
    else if (aType == "array")
        getVarData(QJsonArray)
    else if (aType == "string")
        getVarData(QString)
    else if (aType == "bool")
        getVarData(bool)
    else if (aType == "number")
        getVarData(double)
    else
        qFatal("Invalid data type in qmlStream!");
    return QJSValue();
}

QVariant jsPipe::nextP(QVariant aNext, const QString& aTag){
    auto nxt = qobject_cast<jsPipe*>(qvariant_cast<QObject*>(aNext));
    pipeline::find(m_pipe)->next(nxt->m_pipe, aTag);
    return aNext;
}

QVariant jsPipe::next(QJSValue aNext, const QString& aTag, const QJsonObject& aPipeParam){
    auto ret = createPipe(aNext, aPipeParam);
    pipeline::find(m_pipe)->next(ret->m_pipe, aTag);
    return QVariant::fromValue<QObject*>(ret);
}

QVariant jsPipe::nextB(QJSValue aNext, const QString& aTag, const QJsonObject& aPipeParam){
    next(aNext, aTag, aPipeParam);
    return QVariant::fromValue<QObject*>(this);
}

QVariant jsPipe::next(const QString& aName, const QString& aTag){
    jsPipe* ret = new jsPipe();
    ret->m_pipe = pipeline::find(m_pipe)->next(aName, aTag)->actName();
    return QVariant::fromValue<QObject*>(ret);
}

QVariant jsPipe::nextB(const QString& aName, const QString& aTag){
    next(aName, aTag);
    return QVariant::fromValue<QObject*>(this);
}

QVariant jsPipe::nextL(const QString& aName, const QString& aTag, const QJsonObject& aPipeParam){
    jsPipe* ret = new jsPipe();
    ret->m_pipe = pipeline::find(aName)->createLocal(aName, aPipeParam)->actName();
    pipeline::find(m_pipe)->next(ret->m_pipe, aTag);
    return QVariant::fromValue<QObject*>(ret);
}

void jsPipe::removeNext(const QString& aName){
    pipeline::find(m_pipe)->removeNext(aName);
}

jsPipe* jsPipe::createPipe(QJSValue aFunc, const QJsonObject& aParam){
    jsPipe* ret = new jsPipe();
    auto tp = aParam.value("type").toString();
    auto prm = std::make_shared<ICreateJSPipe>(aParam, aFunc);
    pipeline::run<std::shared_ptr<ICreateJSPipe>>("createJSPipe2_" + tp, prm, "", false);
    if (prm->param.contains("actname"))
        ret->m_pipe = prm->param.value("actname").toString();
    else
        assert(0);

    return ret;
}

static bool m_language_updated;
static QJsonObject translates;

pipelineJS::pipelineJS(){
    m_language_updated = false;
    QFile fl(".language");
    if (fl.open(QFile::ReadOnly)){
        translates = QJsonDocument::fromJson(fl.readAll()).object();
        fl.close();
    }
}

pipelineJS::~pipelineJS(){
    if (m_language_updated){
        QFile fl(".language");
        if (fl.open(QFile::WriteOnly)){
            fl.write(QJsonDocument(translates).toJson());
            fl.close();
        }
    }
}

std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> createScopeCache(const QJsonObject& aScopeCache){
    std::shared_ptr<QHash<QString, std::shared_ptr<stream0>>> ch = nullptr;
    if (!aScopeCache.empty()){
        ch = std::make_shared<QHash<QString, std::shared_ptr<stream0>>>();
        for (auto i : aScopeCache.keys()){
            auto val = aScopeCache.value(i);
            if (val.isString())
                ch->insert(i, std::make_shared<stream<QString>>(val.toString()));
            else if (val.isBool())
                ch->insert(i, std::make_shared<stream<bool>>(val.toBool()));
            else if (val.isDouble())
                ch->insert(i, std::make_shared<stream<double>>(val.toDouble()));
            else if (val.isArray())
                ch->insert(i, std::make_shared<stream<QJsonArray>>(val.toArray()));
            else
                ch->insert(i, std::make_shared<stream<QJsonObject>>(val.toObject()));
        }
    }
    return ch;
}

void pipelineJS::trigged(const QJsonValue& aInput){
    if (aInput.isObject()){
        auto ret = aInput.toObject();
    }
}

pipelineJS* pipelineJS::instance(){
    static std::mutex apipeline_mutex;
    std::lock_guard<std::mutex> lg(apipeline_mutex);
    static pipelineJS ret;  //if realized in .h, there will be multi objects in different dlls
    return &ret;
}

bool pipelineJS::run(const QString& aName, const QJSValue& aInput, const QString& aTag, bool aTransaction, const QJsonObject& aScopeCache){
    bool ret = false;
    auto ch = createScopeCache(aScopeCache);
    if (aInput.isString())
        ret = pipeline::run<QString>(aName, aInput.toString(), aTag, aTransaction, ch);
    else if (aInput.isBool())
        ret = pipeline::run<bool>(aName, aInput.toBool(), aTag, aTransaction, ch);
    else if (aInput.isNumber())
        ret = pipeline::run<double>(aName, aInput.toNumber(), aTag, aTransaction, ch);
    else if (aInput.isArray())
        ret = pipeline::run<QJsonArray>(aName, QJsonArray::fromVariantList(aInput.toVariant().toList()), aTag, aTransaction, ch);
    else
        ret = pipeline::run<QJsonObject>(aName, QJsonObject::fromVariantMap(aInput.toVariant().toMap()), aTag, aTransaction, ch);
    return ret;
}

void pipelineJS::runC(const QString& aName, const QJSValue& aInput, const QString& aStreamID, const QString& aTag){
    if (aInput.isString())
        pipeline::runC<QString>(aName, aInput.toString(), aStreamID, aTag);
    else if (aInput.isBool())
        pipeline::runC<bool>(aName, aInput.toBool(), aStreamID, aTag);
    else if (aInput.isNumber())
        pipeline::runC<double>(aName, aInput.toNumber(), aStreamID, aTag);
    else if (aInput.isArray())
        pipeline::runC<QJsonArray>(aName, QJsonArray::fromVariantList(aInput.toVariant().toList()), aStreamID, aTag);
    else
        pipeline::runC<QJsonObject>(aName, QJsonObject::fromVariantMap(aInput.toVariant().toMap()), aStreamID, aTag);
}

void pipelineJS::syncCall(const QString& aName, const QJSValue& aInput){
    if (aInput.isString())
        pipeline::syncCall<QString>(aName, aInput.toString());
    else if (aInput.isBool())
        pipeline::syncCall<bool>(aName, aInput.toBool());
    else if (aInput.isNumber())
        pipeline::syncCall<double>(aName, aInput.toNumber());
    else if (aInput.isArray())
        pipeline::syncCall<QJsonArray>(aName, QJsonArray::fromVariantList(aInput.toVariant().toList()));
    else
        pipeline::syncCall<QJsonObject>(aName, QJsonObject::fromVariantMap(aInput.toVariant().toMap()));
}

QVariant pipelineJS::call(const QString& aName, const QJSValue& aInput){
    auto id = generateUUID();
    auto rt = std::make_shared<transaction>("", id);
    rea::pipeline::instance()->tryStartTransaction(rt);
    auto stm = new jsStream(aInput, id, nullptr, rt);
    return stm->call(aName);
}

QVariant pipelineJS::input(const QJSValue& aInput, const QString& aTag, bool aTransaction, const QJsonObject& aScopeCache){
    auto id = aTag == "" ? generateUUID() : aTag;
    auto rt = aTransaction ? std::make_shared<transaction>("", id) : nullptr;
    rea::pipeline::instance()->tryStartTransaction(rt);
    auto stm = new jsStream(aInput, id, createScopeCache(aScopeCache), rt);
    return QVariant::fromValue<QObject*>(stm);
}

void pipelineJS::remove(const QString& aName){
    pipeline::remove(aName);
}

void pipelineJS::removeAspect(const QString& aPipe, pipe0::AspectType aType, const QString& aAspect){
    pipeline::removeAspect(aPipe, aType, aAspect);
}

QVariant pipelineJS::add(QJSValue aFunc, const QJsonObject& aPipeParam){
    auto ret = jsPipe::createPipe(aFunc, aPipeParam);
    return QVariant::fromValue<QObject*>(ret);
}

QVariant pipelineJS::find(const QString& aName){
    jsPipe* ret = new jsPipe();
    ret->m_pipe = aName;
    return QVariant::fromValue<QObject*>(ret);
}

QString tr0(const QString& aOrigin){
    auto key = aOrigin.trimmed();
    if (!translates.contains(key)){
        translates.insert(key, aOrigin);
        m_language_updated = true;
    }
    return translates.value(key).toString(aOrigin);
}

QVariant pipelineJS::tr(const QString& aOrigin){
    return tr0(aOrigin);
}

#define regCreateJSPipe(Name) \
static regPip<std::shared_ptr<ICreateJSPipe>> reg_createJSPipe_##Name([](stream<std::shared_ptr<ICreateJSPipe>>* aInput){ \
    auto dt = aInput->data(); \
    auto prm = dt->param; \
    auto tp = prm.value("vtype").toString("object"); \
    if (tp == "object"){ \
        dt->param.insert("actname", pipeline::add<QJsonObject, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    }else if (tp == "string") \
        dt->param.insert("actname", pipeline::add<QString, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else if (tp == "number") \
        dt->param.insert("actname", pipeline::add<double, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else if (tp == "bool") \
        dt->param.insert("actname", pipeline::add<bool, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else if (tp == "array") \
        dt->param.insert("actname", pipeline::add<QJsonArray, pipe##Name, QJSValue, QJSValue>(dt->func, prm)->actName()); \
    else \
        assert(0); \
}, Json("name", STR(createJSPipe2_##Name)));

regCreateJSPipe(Partial)
regCreateJSPipe(Delegate)
regCreateJSPipe(Buffer)
regCreateJSPipe(Local)
regCreateJSPipe(Throttle)
regCreateJSPipe()

void myMessageOutput(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    //https://stackoverflow.com/questions/54307407/why-am-i-getting-qwindowswindowsetgeometry-unable-to-set-geometry-warning-wit
    /*if (type != QtWarningMsg || !msg.startsWith("QWindowsWindow::setGeometry")) {
        QByteArray localMsg = msg.toLocal8Bit();
        fprintf(stdout, localMsg.constData());
    }*/
    //https://blog.csdn.net/liang19890820/article/details/51838096
    QByteArray localMsg = msg.toLocal8Bit();
    auto ret = QString(localMsg) + " (" + context.file + ":" + context.line + ", " + context.function + ")";
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "debug", "msg", "Debug: " + ret), "", false);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "info", "msg", "Info: " + ret), "", false);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "warning", "msg", "Warning: " + ret), "", false);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "critical", "msg", "Critical: " + ret), "", false);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        rea::pipeline::run<QJsonObject>("addLogRecord", rea::Json("type", "system", "level", "error", "msg", "Fatal: " + ret), "", false);
        abort();
    }
}

}

