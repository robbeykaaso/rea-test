#ifndef REA_H_
#define REA_H_

#include "util.h"
#include <vector>
#include <functional>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QThread>
#include <QThreadPool>
#include <QRunnable>
#include <QEvent>
#include <QEventLoop>
#include <QCoreApplication>

namespace rea4 {

class pipeline;

class stream0;
template <typename T>
class stream;
template <typename T>
using pipeFunc = std::function<void(stream<T>*)>;

template <typename T, typename F = pipeFunc<T>>
class pipe;

class typeTrait0{
public:
    virtual ~typeTrait0(){

    }
    virtual QString name(){
        return "";
    }
    virtual QVariant QData(stream0*){
        return QVariant();
    }
};

template <typename T>
class typeTrait : public typeTrait0{

};

class scopeCache{
public:
    scopeCache(){
    }
    scopeCache(const QJsonObject& aData);
    template<typename T>
    scopeCache* cache(const QString& aName, T aData){
        m_data.insert(aName, std::make_shared<stream<T>>(aData));
        return this;
    }
    template<typename T>
    T data(const QString& aName){
        if (m_data.contains(aName))
            return std::dynamic_pointer_cast<stream<T>>(m_data.value(aName))->data();
        else
            return T();
    }
    QVariantList toList();
    std::shared_ptr<stream0> dataStream(const QString& aName){
        return m_data.value(aName);
    }
private:
    QHash<QString, std::shared_ptr<stream0>> m_data;
    friend stream0;
};

class stream0 : public std::enable_shared_from_this<stream0>{
public:
    stream0(const QString& aTag = "") {
        m_tag = aTag;
        m_scope = nullptr;
    }
    stream0(const stream0&) = default;
    stream0(stream0&&) = default;
    stream0& operator=(const stream0&) = default;
    stream0& operator=(stream0&&) = default;
    virtual ~stream0(){

    }
    QString tag(){
        return m_tag;
    }
    virtual QString dataType(){return "";}

    std::shared_ptr<scopeCache> scope(bool aNew = false){
        if (!m_scope || aNew)
            m_scope = std::make_shared<scopeCache>();
        return m_scope;
    }

    virtual QVariant QData(){
        return QVariant();
    }
protected:
    pipeline* m_parent;
    QString m_tag;
    std::shared_ptr<scopeCache> m_scope;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<stream0>>>> m_outs = nullptr;
    friend class pipe0;
    friend pipeline;
    template<typename T, typename F>
    friend class pipe;
    template <typename T>
    friend class stream;
};

class pipeFuture;
template <typename T, typename F = pipeFunc<T>>
class pipeDelegate;
template <typename T, typename F>
class pipeParallel;

class pipe0 : public QObject{
public:
    enum AspectType {AspectBefore, AspectAround, AspectAfter};
public:
    virtual ~pipe0(){}
    virtual QString actName() {return m_name;}

    template <typename T, template<class, typename> class P = pipe>
    pipe0* nextF(pipeFunc<T> aNextFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject()){
        return nextF0(m_parent, this, aNextFunc, aTag, aParam);
    }
    virtual pipe0* nextP(pipe0* aNext, const QString& aTag = "");
    virtual pipe0* next(const QString& aName, const QString& aTag = "");

    template <typename T>
    pipe0* nextFB(pipeFunc<T> aNextFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject()){
        nextF<T>(aNextFunc, aTag, aParam);
        return this;
    }
    virtual pipe0* nextPB(pipe0* aNext, const QString& aTag = "");
    virtual pipe0* nextB(const QString& aName, const QString& aTag = "");

    virtual void removeNext(const QString& aName);
    void removeAspect(pipe0::AspectType aType, const QString& aAspect = "");
    virtual void execute(std::shared_ptr<stream0> aStream);

    virtual void resetTopo();
private:
    friend pipeline;
protected:
    class streamEvent : public QEvent{
    public:
        static const Type type = static_cast<Type>(QEvent::User + 1);
    public:
        streamEvent(const QString& aName, std::shared_ptr<stream0> aStream) : QEvent(type) {
            m_name = aName;
            m_stream = aStream;
        }
        QString getName() {return m_name;}
        std::shared_ptr<stream0> getStream() {return m_stream;}
    private:
        QString m_name;
        std::shared_ptr<stream0> m_stream;
    };
    pipe0(pipeline* aParent, const QString& aName = "", int aThreadNo = 0, bool aReplace = false);
    virtual void insertNext(const QString& aName, const QString& aTag) {
        m_next.insert(aName, aTag);
    }
protected:
    void doNextEvent(const QMap<QString, QString>& aNexts, std::shared_ptr<stream0> aStream);
    void setAspect(QString& aTarget, const QString& aAspect);
protected:
    QString m_name;
    QMap<QString, QString> m_next;
    QString m_before = "", m_around = "", m_after = "";
    QString m_external = "";
    pipeline* m_parent;
    QThread* m_thread = QThread::currentThread();
private:
    friend pipeFuture;
    void tryExecutePipe(const QString& aName, std::shared_ptr<stream0> aStream);
    template<typename, typename>
    friend class pipeDelegate;
};

class pipeFuture : public pipe0 {
public:
    QString actName() override {return m_act_name;}
    void execute(std::shared_ptr<stream0> aStream) override;
    void removeNext(const QString& aName) override;
    void resetTopo() override;
protected:
    pipeFuture(pipeline* aParent, const QString& aName);
    void insertNext(const QString& aName, const QString& aTag) override;
private:
    QString m_act_name;
    QVector<QPair<QString, QString>> m_next2;
    friend pipeline;
};

class pipeline : public QObject{
public:
    static pipeline* instance(const QString& aName = "");
public:
    pipeline(const QString& aName = "");
    pipeline(pipeline&&) = delete;
    QString name(){return m_name;}
    virtual ~pipeline();

    virtual void remove(const QString& aName, bool aOutside = false);

    template<typename T, template<class, typename> class P = pipe, typename F = pipeFunc<T>, typename S = pipeFunc<T>>
    pipe0* add(F aFunc, const QJsonObject& aParam = QJsonObject()){
        auto nm = aParam.value("name").toString();
        auto tmp = new P<T, S>(this, nm, aParam.value("thread").toInt(), aParam.value("replace").toBool());  //https://stackoverflow.com/questions/213761/what-are-some-uses-of-template-template-parameters
        if (nm != ""){
            auto ad = tmp->actName() + "_pipe_add";
            call<int>(ad, 0);
            remove(ad);
        }
        tmp->initialize(aFunc, aParam);
        pipe0* ret = tmp;

        auto bf = aParam.value("before").toString();
        if (bf != ""){
            auto joint = find(bf);
            joint->setAspect(joint->m_before, ret->actName());
        }
        auto ar = aParam.value("around").toString();
        if (ar != ""){
            auto joint = find(ar);
            joint->setAspect(joint->m_around, ret->actName());
        }
        auto af = aParam.value("after").toString();
        if (af != ""){
            auto joint = find(af);
            joint->setAspect(joint->m_after, ret->actName());
        }
        return ret;
    }

    pipe0* find(const QString& aName, bool aNeedFuture = true) {
        auto ret = m_pipes.value(aName);
        if (!ret && aNeedFuture){
            ret = new pipeFuture(this, aName);
        }
        return ret;
    }

    template<typename T>
    void run(const QString& aName, T aInput, const QString& aTag = "", std::shared_ptr<scopeCache> aScope = nullptr){
        auto stm = std::make_shared<stream<T>>(aInput, aTag, aScope);
        execute(aName, stm);
    }

    template<typename T, typename F = pipeFunc<T>>
    void call(const QString& aName, T aInput = T()){
        auto pip = m_pipes.value(aName);
        if (pip){
            auto pip2 = dynamic_cast<pipe<T, F>*>(pip);
            auto stm = std::make_shared<stream<T>>(aInput);
            pip2->doEvent(stm);
        }
    }

    template<typename T>
    std::shared_ptr<stream<T>> asyncCall(const QString& aName, T aInput = T()){
        return input<T>(aInput)->asyncCall(aName, this);
    }

    template<typename T>
    std::shared_ptr<stream<T>> input(T aInput = T(), const QString& aTag = "", std::shared_ptr<scopeCache> aScope = nullptr){
        auto tag = aTag == "" ? rea::generateUUID() : aTag;
        return std::make_shared<stream<T>>(aInput, tag, aScope);
    }

    template<typename T>
    void supportType(){
        m_types.insert(typeid (T).name(), std::make_shared<typeTrait<T>>());
    }
protected:
    virtual void execute(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync = QJsonObject(),
                         bool aFromOutside = false);
    virtual void removePipeOutside(const QString& aName);
    virtual void tryExecutePipeOutside(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync, const QString& aFlag);
private:
    QThread* findThread(int aNo);
    QString m_name;
    QHash<QString, pipe0*> m_pipes;
    QHash<int, std::shared_ptr<QThread>> m_threads;
    QHash<QString, std::shared_ptr<typeTrait0>> m_types;
    friend pipe0;
    friend pipeFuture;
    friend stream0;
    template<typename T, typename F>
    friend class pipe;
    template<typename T>
    friend class stream;
};

template <typename T>
class stream : public stream0{
public:
    stream() : stream0(){}
    stream(T aInput, const QString& aTag = "", std::shared_ptr<scopeCache> aScope = nullptr) : stream0(aTag){
        m_data = aInput;
        m_scope = aScope;
    }
    stream<T>* setData(T aData) {
        m_data = aData;
        return this;
    }
    T data() {return m_data;}

    QVariant QData() override{
        auto tp = typeid (T).name();
        if (pipeline::instance()->m_types.contains(tp))
            return pipeline::instance()->m_types.value(tp)->QData(this);
        else
            return QVariant();
    }

    QString dataType() override{
        auto tp = typeid (T).name();
        if (pipeline::instance()->m_types.contains(tp))
            return pipeline::instance()->m_types.value(tp)->name();
        else
            return "";
    }

    stream<T>* out(const QString& aTag = ""){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<stream0>>>>();
        if (aTag != "")
            m_tag = aTag;
        return this;
    }

    void noOut(){
        m_outs = nullptr;
    }

    template<typename S>
    stream<S>* outs(S aOut = S(), const QString& aNext = "", const QString& aTag = ""){
        if (!m_outs)
            m_outs = std::make_shared<std::vector<std::pair<QString, std::shared_ptr<stream0>>>>();
        auto ot = std::make_shared<stream<S>>(aOut, aTag == "" ? m_tag : aTag, m_scope);
        m_outs->push_back(std::pair<QString, std::shared_ptr<stream0>>(aNext, ot));
        return ot.get();
    }

    template<typename S>
    stream<T>* outsB(S aOut = S(), const QString& aNext = "", const QString& aTag = ""){
        outs<S>(aOut, aNext, aTag);
        return this;
    }

    template<typename S>
    std::shared_ptr<stream<S>> map(S aInput = S()){
        return std::make_shared<stream<S>>(aInput, m_tag, m_scope);
    }

    template<typename S = T>
    std::shared_ptr<stream<S>> asyncCall(const QString& aName, pipeline* aPipeline = pipeline::instance()){
        std::shared_ptr<stream<S>> ret = nullptr;
        QEventLoop loop;
        bool timeout = false;
        auto monitor = aPipeline->find(aName)->nextF<S>([&loop, &timeout, &ret, this](stream<S>* aInput){
            ret = map<S>(aInput->data());
            if (loop.isRunning()){
                loop.quit();
            }else
                timeout = true;
        }, m_tag);
        aPipeline->execute(aName, shared_from_this());
        if (!timeout)
            loop.exec();
        aPipeline->find(aName)->removeNext(monitor->actName());
        aPipeline->remove(monitor->actName(), true);
        return ret; //std::dynamic_pointer_cast<stream<T>>(shared_from_this());
    }

    template<typename S = T, template<class, typename> class P = pipe, typename F = pipeFunc<T>, typename R = pipeFunc<T>>
    std::shared_ptr<stream<S>> asyncCallF(F aFunc, const QJsonObject& aParam = QJsonObject(), pipeline* aPipeline = pipeline::instance()){
        auto pip = aPipeline->add<T, P, F, R>(aFunc, aParam);
        auto ret = asyncCall<S>(pip->actName(), aPipeline);
        aPipeline->remove(pip->actName(), true);
        return ret;
    }
private:
    T m_data;
    template<typename T, typename F>
    friend class funcType;
    template <typename T, typename F>
    friend class pipeDelegate;
};

template <>
class typeTrait<double> : public typeTrait0{
public:
    QString name() override{
        return "number";
    }
    QVariant QData(stream0 *aData) override{
        return QVariant::fromValue(reinterpret_cast<stream<double>*>(aData)->data());
    }
};

template <>
class typeTrait<QString> : public typeTrait0{
public:
    QString name() override{
        return "string";
    }
    QVariant QData(stream0 *aData) override{
        return QVariant::fromValue(reinterpret_cast<stream<QString>*>(aData)->data());
    }
};

template <>
class typeTrait<QJsonObject> : public typeTrait0{
public:
    QString name() override{
        return "object";
    }
    QVariant QData(stream0 *aData) override{
        return QVariant::fromValue(reinterpret_cast<stream<QJsonObject>*>(aData)->data());
    }
};

template <>
class typeTrait<bool> : public typeTrait0{
public:
    QString name() override{
        return "bool";
    }
    QVariant QData(stream0 *aData) override{
        return QVariant::fromValue(reinterpret_cast<stream<bool>*>(aData)->data());
    }
};

template <>
class typeTrait<QJsonArray> : public typeTrait0{
public:
    QString name() override{
        return "array";
    }
    QVariant QData(stream0 *aData) override{
        return QVariant::fromValue(reinterpret_cast<stream<QJsonArray>*>(aData)->data());
    }
};

template <typename T>
pipe0* nextF0(pipeline* aPipeline, pipe0* aPipe, pipeFunc<T> aNextFunc, const QString& aTag, const QJsonObject& aParam){
    return aPipe->next(aPipeline->add<T>(aNextFunc, aParam)->actName(), aTag);
}

template<typename T, typename F>
class funcType{
public:
    void doEvent(F aFunc, std::shared_ptr<stream<T>> aStream){
        aFunc(aStream.get());
    }
};

template <typename T, typename F>
class pipe : public pipe0{
protected:
    pipe(pipeline* aParent, const QString& aName = "", int aThreadNo = 0, bool aReplace = false) : pipe0(aParent, aName, aThreadNo, aReplace) {}
    virtual pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()){
        m_func = aFunc;
        m_external = aParam.value("external").toString(m_parent->name());
        auto bf = aParam.value("befored").toString();
        if (bf != "")
            setAspect(m_before, bf);
        auto ed = aParam.value("aftered").toString();
        if (ed != "")
            setAspect(m_after, ed);
        return this;
    }
    bool event( QEvent* e) override{
        if(e->type()== streamEvent::type){
            auto eve = reinterpret_cast<streamEvent*>(e);
            if (eve->getName() == m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                doEvent(stm);
                doNextEvent(m_next, stm);
            }
        }
        return true;
    }
    void doEvent(const std::shared_ptr<stream<T>> aStream){
        if (doAspect(m_before, aStream)){
            if (m_around != "")
                doAspect(m_around, aStream);
            else
                funcType<T, F>().doEvent(m_func, aStream);
        }
        if (aStream->m_outs)
            doAspect(m_after, aStream);
    }
protected:
    F m_func;
    friend pipeParallel<T, F>;
    friend pipeline;
private:
    bool doAspect(const QString& aName, std::shared_ptr<stream<T>> aStream){
        if (aName == "")
            return true;
        bool ret = false;
        auto nms = aName.split(";");
        for (auto i : nms){
            auto pip = m_parent->m_pipes.value(i);
            if (pip){
                auto pip2 = dynamic_cast<pipe<T, F>*>(pip);
                pip2->doEvent(aStream);
                if (aStream->m_outs)
                    ret = true;
            }
        }
        return ret;
    }
};

template <typename T, typename F>
class pipeAsync : public pipe<T, F>{
protected:
    pipeAsync(pipeline* aParent, const QString& aName = "", int aThreadNo = 0) : pipe<T, F>(aParent, aName, aThreadNo){

    }
    void execute(std::shared_ptr<stream0> aStream) override{
        auto nxt_eve = std::make_unique<pipe0::streamEvent>(pipe0::m_name, aStream);
        QCoreApplication::postEvent(this, nxt_eve.release());
    }
    friend pipeline;
};

template <typename T, typename F>
class pipeDelegate : public pipe<T, F>{
public:
    pipe0* nextP(pipe0* aNext, const QString& aTag = "") override{
        return pipe0::m_parent->find(m_delegate)->nextP(aNext, aTag);
    }
    pipe0* next(const QString& aName, const QString& aTag = "") override{
        return pipe0::m_parent->find(m_delegate)->next(aName, aTag);
    }
    void removeNext(const QString& aName) override{
        pipe0::m_parent->find(m_delegate)->removeNext(aName);
    }
protected:
    pipeDelegate(pipeline* aParent, const QString& aName = "", int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aParent, aName, aThreadNo, aReplace) {}
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm0 = eve->getStream();
                auto stm = std::dynamic_pointer_cast<stream<T>>(stm0);
                doEvent(stm);
            }
        }
        return true;
    }
    pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override{
        m_delegate = aParam.value("delegate").toString();
        auto del = pipe0::m_parent->find(m_delegate);
        for (auto i : m_next2)
            del->insertNext(i.first, i.second);
        return pipe<T, F>::initialize(aFunc, aParam);
    }
    void insertNext(const QString& aName, const QString& aTag) override{
        m_next2.push_back(QPair<QString, QString>(aName, aTag));
    }
private:
    QString m_delegate;
    QVector<QPair<QString, QString>> m_next2;
    friend pipeline;
};

template <typename T, typename F>
class pipePartial : public pipe<T, F> {
public:
    void removeNext(const QString& aName) override {
        for (auto i = m_next2.begin(); i != m_next2.end(); ++i)  //: for will not remove
            i.value().remove(aName);
    }
protected:
    pipePartial(pipeline* aParent, const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aParent, aName, aThreadNo, aReplace) {

    }
    void insertNext(const QString& aName, const QString& aTag) override {
        rea::tryFind(&m_next2, aTag)->insert(aName, aTag);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                doEvent(stm);
                doNextEvent(m_next2.value(stm->tag()), stm);
            }
        }
        return true;
    }
private:
    QHash<QString, QMap<QString, QString>> m_next2;
    friend pipeline;
};

template <typename T, typename F>
class parallelTask : public QRunnable{
public:
    parallelTask(pipeParallel<T, F>* aPipe, std::shared_ptr<stream<T>> aStream) : QRunnable(){
        m_pipe = aPipe;
        m_source = aStream;
    }
    void run() override{
        m_pipe->doEvent(m_source);
        m_pipe->doNextEvent(m_pipe->m_next, m_source);
    }
private:
    std::shared_ptr<stream<T>> m_source;
    pipeParallel<T, F>* m_pipe;
};

template <typename T, typename F = pipeFunc<T>>
class pipeParallel : public pipe<T, F> {
protected:
    pipeParallel(pipeline* aParent, const QString& aName, int aThreadNo = 0, bool aReplace = false) : pipe<T, F>(aParent, aName, aThreadNo, aReplace) {

    }
    ~pipeParallel() override{

    }
    pipe0* initialize(F aFunc, const QJsonObject& aParam = QJsonObject()) override {
        m_act_name = aParam.value("delegate").toString();
        m_init = aFunc != nullptr;
        return pipe<T, F>::initialize(aFunc, aParam);
    }
    bool event( QEvent* e) override{
        if(e->type()== pipe0::streamEvent::type){
            auto eve = reinterpret_cast<pipe0::streamEvent*>(e);
            if (eve->getName() == pipe0::m_name){
                if (!m_init){
                    auto pip = pipe0::m_parent->find(m_act_name, false);
                    if (pip)
                        pipe<T, F>::m_func = dynamic_cast<pipe<T, F>*>(pip)->m_func;
                    m_init = true;
                }
                auto stm = std::dynamic_pointer_cast<stream<T>>(eve->getStream());
                QThreadPool::globalInstance()->start(new parallelTask<T, F>(this, stm));
            }
        }
        return true;
    }
private:
    bool m_init = false;
    QString m_act_name;
    friend parallelTask<T, F>;
    friend pipeline;
};

template <typename T, template<class, typename> class P = pipe>
class regPip
{
public:
    regPip(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject(), const QString& aPrevious = ""){
        auto pip = pipeline::instance()->add<T, P>(aFunc, aParam);
        if (aPrevious != "")
            rea4::pipeline::instance()->find(aPrevious)->next(pip->actName());
    }
};

}

#endif

