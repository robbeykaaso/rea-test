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

template <typename T>
class stream;
template <typename T>
using pipeFunc = std::function<void(stream<T>*)>;

template <typename T>
class pipe;

class stream0 : public std::enable_shared_from_this<stream0>{
public:
    stream0(const QString& aTag = "") {
        m_tag = aTag;
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
    QString dataType(){return m_data_type;}
    QString id(){return m_id;}
protected:
    QString m_data_type;
    QString m_tag;
    QString m_id;
    std::shared_ptr<std::vector<std::pair<QString, std::shared_ptr<stream0>>>> m_outs = nullptr;
    friend class pipe0;
    friend pipeline;
    template<typename T>
    friend class pipe;
    template <typename T>
    friend class stream;
};

class pipeFuture;
template <typename T>
class pipeDelegate;

class pipe0 : public QObject{
public:
    enum AspectType {AspectBefore, AspectAround, AspectAfter};
public:
    virtual ~pipe0(){}
    virtual QString actName() {return m_name;}

    template <typename T>
    pipe0* next(pipeFunc<T> aNextFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject()){
        return nextF0(this, aNextFunc, aTag, aParam);
    }
    virtual pipe0* next(pipe0* aNext, const QString& aTag = "");
    virtual pipe0* next(const QString& aName, const QString& aTag = "");

    template <typename T>
    pipe0* nextB(pipeFunc<T> aNextFunc, const QString& aTag = "", const QJsonObject& aParam = QJsonObject()){
        next<T>(aNextFunc, aTag, aParam);
        return this;
    }
    virtual pipe0* nextB(pipe0* aNext, const QString& aTag = "");
    virtual pipe0* nextB(const QString& aName, const QString& aTag = "");

    virtual void removeNext(const QString& aName);
    void removeAspect(pipe0::AspectType aType, const QString& aAspect = "");
    virtual void execute(std::shared_ptr<stream0> aStream);
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
    pipe0(const QString& aName = "", int aThreadNo = 0);
    virtual void insertNext(const QString& aName, const QString& aTag) {
        m_next.insert(aName, aTag);
    }
    virtual void resetTopo();
protected:
    void doNextEvent(const QMap<QString, QString>& aNexts, std::shared_ptr<stream0> aStream);
    void setAspect(QString& aTarget, const QString& aAspect);
protected:
    QString m_name;
    QMap<QString, QString> m_next;
    QString m_before = "", m_around = "", m_after = "";
    bool m_external = false;
    QThread* m_thread = QThread::currentThread();
private:
    friend pipeFuture;
    void tryExecutePipe(const QString& aName, std::shared_ptr<stream0> aStream);
    template<typename>
    friend class pipeDelegate;
};

class pipeFuture : public pipe0 {
public:
    QString actName() override {return m_act_name;}
    void execute(std::shared_ptr<stream0> aStream) override;
    void removeNext(const QString& aName) override;
protected:
    pipeFuture(const QString& aName);
    void resetTopo() override;
    void insertNext(const QString& aName, const QString& aTag) override;
private:
    QString m_act_name;
    QVector<QPair<QString, QString>> m_next2;
    friend pipeline;
};

class transaction{
public:
    transaction(const QString& aID);
    virtual ~transaction(){

    }
    virtual void pipeIn(pipe0* aPipe){
        m_remained.insert(aPipe);
    }
    virtual void pipeOut(pipe0* aPipe, bool aFinished){
        m_remained.remove(aPipe);
        if (!m_remained.size() && aFinished){

        }
    }
private:
    QString m_id;
    QSet<pipe0*> m_remained;
    QHash<QString, std::shared_ptr<stream0>> m_scope_cache;
};

class pipeline : public QObject{
public:
    static pipeline* instance(const QString& aName = "");
public:
    pipeline();
    pipeline(pipeline&&) = delete;
    virtual ~pipeline();

    virtual void remove(const QString& aName, bool aOutside = false);

    template<typename T, template<typename> class P = pipe>
    static pipe0* add(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject()){
        auto nm = aParam.value("name").toString();
        auto tmp = new P<T>(nm, aParam.value("thread").toInt());  //https://stackoverflow.com/questions/213761/what-are-some-uses-of-template-template-parameters
        if (nm != ""){
            auto ad = tmp->actName() + "_pipe_add";
            pipeline::call<int>(ad, 0);
            pipeline::instance()->remove(ad);
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

    static pipe0* find(const QString& aName, bool needFuture = true) {
        auto ret = instance()->m_pipes.value(aName);
        if (!ret && needFuture){
            ret = new pipeFuture(aName);
        }
        return ret;
    }

    template<typename T>
    static void run(const QString& aName, T aInput, const QString& aTag = ""){
        auto stm = std::make_shared<stream<T>>(aInput, aTag, aName + ";" + aTag);
        instance()->execute(aName, stm);
    }

    template<typename T>
    static void call(const QString& aName, T aInput = T()){
        auto pip = instance()->m_pipes.value(aName);
        if (pip){
            auto pip2 = dynamic_cast<pipe<T>*>(pip);
            auto stm = std::make_shared<stream<T>>(aInput);
            pip2->doEvent(stm);
        }
    }

    template<typename T>
    static std::shared_ptr<stream<T>> asyncCall(const QString& aName, T aInput = T()){
        return input<T>(aInput)->asyncCall(aName);
    }

    template<typename T>
    static std::shared_ptr<stream<T>> input(T aInput = T(), const QString& aTag = ""){
        auto tag = aTag == "" ? rea::generateUUID() : aTag;
        return std::make_shared<stream<T>>(aInput, tag, ";" + tag);
    }
protected:
    virtual void execute(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync = QJsonObject(), bool aFromOutside = false);
    virtual void removePipeOutside(const QString& aName);
    void tryExecutePipeOutside(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync = QJsonObject(), bool aFromOutside = true);
private:
    void removeTransaction(const QString& aID);
private:
    QThread* findThread(int aNo);
    QHash<QString, pipe0*> m_pipes;
    QHash<int, std::shared_ptr<QThread>> m_threads;
    QHash<QString, transaction> m_transactions;
    friend pipe0;
    friend pipeFuture;
    friend stream0;
    template<typename T>
    friend class pipe;
    template<typename T>
    friend class stream;
};

template <typename T>
class typeTrait{
public:
    static QString name(){
        return "";
    }
};

template <>
class typeTrait<double>{
public:
    static QString name(){
        return "number";
    }
};

template <>
class typeTrait<QString>{
public:
    static QString name(){
        return "string";
    }
};

template <>
class typeTrait<QJsonObject>{
public:
    static QString name(){
        return "object";
    }
};

template <>
class typeTrait<bool>{
public:
    static QString name(){
        return "bool";
    }
};

template <>
class typeTrait<QJsonArray>{
public:
    static QString name(){
        return "array";
    }
};


template <typename T>
class stream : public stream0{
public:
    stream() : stream0(){}
    stream(T aInput, const QString& aTag = "", const QString& aID = "") : stream0(aTag){
        m_id = aID;
        m_data = aInput;
        m_data_type = typeTrait<T>::name();
    }
    stream<T>* setData(T aData) {
        m_data = aData;
        return this;
    }
    T data() {return m_data;}

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
        auto ot = std::make_shared<stream<S>>(aOut, aTag == "" ? m_tag : aTag, m_id);
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
        return std::make_shared<stream<S>>(aInput, m_tag, m_id);
    }

    template<typename S = T>
    std::shared_ptr<stream<S>> asyncCall(const QString& aName){
        std::shared_ptr<stream<S>> ret = nullptr;
        QEventLoop loop;
        bool timeout = false;
        auto monitor = pipeline::find(aName)->next<S>([&loop, &timeout, &ret, this](stream<S>* aInput){
            ret = map<S>(aInput->data());
            if (loop.isRunning()){
                loop.quit();
            }else
                timeout = true;
        }, m_tag);
        pipeline::instance()->execute(aName, shared_from_this());
        if (!timeout)
            loop.exec();
        pipeline::find(aName)->removeNext(monitor->actName());
        pipeline::instance()->remove(monitor->actName(), true);
        return ret; //std::dynamic_pointer_cast<stream<T>>(shared_from_this());
    }

    template<typename S = T, template<typename> class P = pipe>
    std::shared_ptr<stream<S>> asyncCall(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject()){
        auto pip = pipeline::add<T, P>(aFunc, aParam);
        auto ret = asyncCall<S>(pip->actName());
        pipeline::instance()->remove(pip->actName(), true);
        return ret;
    }
private:
    T m_data;
    template <typename T>
    friend class pipeDelegate;
};

template <typename T>
pipe0* nextF0(pipe0* aPipe, pipeFunc<T> aNextFunc, const QString& aTag, const QJsonObject& aParam){
    return aPipe->next(pipeline::add<T>(aNextFunc, aParam), aTag);
}

template <typename T>
class pipe : public pipe0{
protected:
    pipe(const QString& aName = "", int aThreadNo = 0) : pipe0(aName, aThreadNo) {}
    virtual pipe0* initialize(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject()){
        m_func = aFunc;
        m_external = aParam.value("external").toBool();
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
        if (doAspect(m_before, aStream, AspectType::AspectBefore)){
            if (m_around != "")
                doAspect(m_around, aStream, AspectType::AspectAround);
            else
                m_func(aStream.get());
        }
        if (aStream->m_outs)
            doAspect(m_after, aStream, AspectType::AspectAfter);
    }
protected:
    pipeFunc<T> m_func;
    friend pipeline;
private:
    bool doAspect(const QString& aName, std::shared_ptr<stream<T>> aStream, AspectType aType){
        if (aName == "")
            return true;
        bool ret = false;
        auto nms = aName.split(";");
        for (auto i : nms){
            auto pip = pipeline::instance()->m_pipes.value(i);
            if (pip){
                auto pip2 = dynamic_cast<pipe<T>*>(pip);
                pip2->doEvent(aStream);
                if (aStream->m_outs)
                    ret = true;
            }
        }
        return ret;
    }
};

template <typename T>
class pipeAsync : public pipe<T>{
protected:
    pipeAsync(const QString& aName = "", int aThreadNo = 0) : pipe<T>(aName, aThreadNo){

    }
    void execute(std::shared_ptr<stream0> aStream) override{
        auto nxt_eve = std::make_unique<pipe0::streamEvent>(pipe0::m_name, aStream);
        QCoreApplication::postEvent(this, nxt_eve.release());
    }
    friend pipeline;
};

template <typename T>
class pipeDelegate : public pipe<T>{
public:
    pipe0* next(pipe0* aNext, const QString& aTag = "") override{
        return pipeline::find(m_delegate)->next(aNext, aTag);
    }
    pipe0* next(const QString& aName, const QString& aTag = "") override{
        return pipeline::find(m_delegate)->next(aName, aTag);
    }
    void removeNext(const QString& aName) override{
        pipeline::find(m_delegate)->removeNext(aName);
    }
protected:
    pipeDelegate(const QString& aName = "", int aThreadNo = 0) : pipe<T>(aName, aThreadNo) {}
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
    pipe0* initialize(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject()) override{
        m_delegate = aParam.value("delegate").toString();
        auto del = pipeline::find(m_delegate);
        for (auto i : m_next2)
            del->insertNext(i.first, i.second);
        return pipe<T>::initialize(aFunc, aParam);
    }
    void insertNext(const QString& aName, const QString& aTag) override{
        m_next2.push_back(QPair<QString, QString>(aName, aTag));
    }
private:
    QString m_delegate;
    QVector<QPair<QString, QString>> m_next2;
    friend pipeline;
};

template <typename T>
class pipePartial : public pipe<T> {
public:
    void removeNext(const QString& aName) override {
        for (auto i = m_next2.begin(); i != m_next2.end(); ++i)  //: for will not remove
            i.value().remove(aName);
    }
protected:
    pipePartial(const QString& aName, int aThreadNo = 0) : pipe<T>(aName, aThreadNo) {

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

template <typename T, template<typename> class P = pipe>
class regPip
{
public:
    regPip(pipeFunc<T> aFunc, const QJsonObject& aParam = QJsonObject(), const QString& aPrevious = ""){
        auto pip = pipeline::add<T, P>(aFunc, aParam);
        actName = pip->actName();
        if (aPrevious != "")
            rea4::pipeline::find(aPrevious)->next(pip);
    }
    QString actName;
};

}

#endif

