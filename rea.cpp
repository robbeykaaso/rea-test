#include "rea.h"
#include <mutex>
#include <sstream>
#include <QFile>
#include <QJsonDocument>
#include "reaJS.h"

namespace rea4 {

transactionManager::transactionManager(){

    pipeline::add<transaction*>([this](stream<transaction*>* aInput){
        //std::cout << "***transaction start***: " << aInput->data().toStdString() << std::endl;
        auto rt = aInput->data();
        alive_transactions.insert(rt->getName(), rt);
    }, rea::Json("name", "transactionStart"));

    pipeline::add<QJsonObject>([this](stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        auto nm = dt.value("name").toString();
        transactions.push_back(dt.value("detail").toString());
        alive_transactions.remove(nm);
        //std::cout << "***transaction end***: " << dt.value("name").toString().toStdString() << std::endl;
    }, rea::Json("name", "transactionEnd"));

    pipeline::add<double>([this](stream<double>* aInput){
        if (aInput->data()){
            QString ret = "";
            for (auto i : transactions)
                ret += i;
            for (auto i : alive_transactions)
                ret += (i->print() + "running!\n");
            QFile sv("transaction.txt");
            if (sv.open(QFile::WriteOnly)){
                sv.write(ret.toUtf8());
                sv.close();
            }
        }else{
            for (auto i : transactions)
                std::cout << i.toStdString();
            for (auto i : alive_transactions)
                std::cout << (i->print().toStdString() + "running!\n");
        }
    }, rea::Json("name", "logTransaction"));
}

transactionManager::~transactionManager(){
    pipeline::remove("transactionStart");
    pipeline::remove("transactionEnd");
    pipeline::remove("logTransaction");
}

void transaction::executed(const QString& aPipe){
    auto cnt = m_candidates.value(aPipe) - 1;
    if (!cnt)
        m_candidates.remove(aPipe);
    else if (cnt > 0)
        m_candidates.insert(aPipe, cnt);
    //else
    //    m_logs.push_back("(" + aPipe + ")");
}

void transaction::addTrig(const QString& aStart, const QString& aNext){
    std::lock_guard<std::mutex> gd(m_mutex);
    m_logs.push_back(aStart + " > " + aNext);
    if (!m_candidates.contains(aNext))
        m_candidates.insert(aNext, 0);
    m_candidates.insert(aNext, m_candidates.value(aNext) + 1);
}

transaction::transaction(const QString& aName, const QString& aTag){
    m_name = aName + ";" + aTag;
    if (aName != "")
        addTrig(aTag + ":", aName);
}

transaction::~transaction(){
    pipeline::run<QJsonObject>("transactionEnd", rea::Json("name", m_name, "detail", print(), "fail", m_fail), "", false);
}

void transaction::log(const QString& aLog){
    std::lock_guard<std::mutex> gd(m_mutex);
    m_logs.push_back(aLog);
}

const QString transaction::print(){
    std::lock_guard<std::mutex> gd(m_mutex);
    QString ret = "******************** " + m_name + "\n";
    for (auto i : m_logs)
        ret += " " + i + "\n";
    for (auto i : m_candidates.keys())
        ret += " alive: " + i + "*" + QString::number(m_candidates.size()) + "\n";
    return ret;
}

void stream0::executed(const QString& aPipe){
    if (m_transaction)
        m_transaction->executed(aPipe);
}

QString stream0::cache(const QString& aID){
    auto id = aID;
    if (id == "")
        rea::generateUUID();
    pipeline::instance()->m_stream_cache.insert(id, this->shared_from_this());
    return id;
}

pipe0::pipe0(const QString& aName, int aThreadNo, bool aReplace){
    if (aName == "")
        m_name = rea::generateUUID();
    else
        m_name = aName;
    if (aThreadNo != 0){
        m_thread = pipeline::instance()->findThread(aThreadNo);
        moveToThread(m_thread);
    }
    auto old = pipeline::find(m_name, false);
    if (old){
        if (aReplace)
            m_next = old->m_next;
        pipeline::remove(m_name);
    }
    pipeline::instance()->m_pipes.insert(m_name, this);
}

pipe0* pipe0::next(pipe0* aNext, const QString& aTag){
    auto tags = aTag.split(";");
    for (auto i : tags)
        insertNext(aNext->actName(), i);
    return aNext;
}

pipe0* pipe0::next(const QString& aName, const QString& aTag){
    auto tags = aTag.split(";");
    for (auto i : tags)
        insertNext(aName, i);
    auto nxt = pipeline::find(aName);
    return nxt;
}

void pipe0::removeNext(const QString &aName){
    m_next.remove(aName);
}

void pipe0::removeAspect(pipe0::AspectType aType, const QString& aAspect){
    QString* tar = nullptr;
    if (aType == pipe0::AspectType::AspectAfter)
        tar = &m_after;
    else if (aType == pipe0::AspectType::AspectBefore)
        tar = &m_before;
    else if (aType == pipe0::AspectType::AspectAround)
        tar = &m_around;
    if (aAspect == "")
        *tar = "";
    else{
        auto idx = tar->indexOf(aAspect);
        if (idx > 0)
            *tar = tar->remove(idx - 1, aAspect.length());
        else if (!idx)
            *tar = tar->remove(idx, aAspect.length());
    }
}

pipe0* pipe0::nextB(pipe0* aNext, const QString& aTag){
    next(aNext, aTag);
    return this;
}

pipe0* pipe0::nextB(const QString& aName, const QString& aTag){
    next(aName, aTag);
    return this;
}

void pipe0::doNextEvent(const QMap<QString, QString>& aNexts, std::shared_ptr<stream0> aStream){
    auto outs = aStream->m_outs;
    aStream->m_outs = nullptr;
    if (outs){
        if (outs->size() > 0){
            for (auto i : *outs)
                if (i.first == ""){
                    for (auto j : aNexts.keys()){
                        auto pip = pipeline::find(j, false);
                        if (pip){
                            aStream->addTrig(actName(), pip->actName());
                            pip->execute(i.second);
                        }
                    }
                }else{
                    auto pip = pipeline::find(i.first, false);
                    if (pip){
                        aStream->addTrig(actName(), pip->actName());
                        pip->execute(i.second);
                    }
                }
        }else
            for (auto i : aNexts.keys()){
                auto pip = pipeline::find(i, false);
                if (pip){
                    aStream->addTrig(actName(), pip->actName());
                    pip->execute(aStream);
                }
            }
    }
}

void pipe0::setAspect(QString& aTarget, const QString& aAspect){
    if (aTarget != "")
        aTarget += ";";
    aTarget += aAspect;
}

void pipe0::execute(std::shared_ptr<stream0> aStream){
    if (QThread::currentThread() == m_thread){
        streamEvent nxt_eve(m_name, aStream);
        QCoreApplication::sendEvent(this, &nxt_eve);
    }else{
        auto nxt_eve = std::make_unique<streamEvent>(m_name, aStream);
        QCoreApplication::postEvent(this, nxt_eve.release());  //https://stackoverflow.com/questions/32583078/does-postevent-free-the-event-after-posting-double-free-or-corruption // still memory leak, reason is unknown
    }
}

void pipeFuture::insertNext(const QString& aName, const QString& aTag){
    m_next2.push_back(QPair<QString, QString>(aName, aTag));
}

class pipeFuture0 : public pipe0 {  //the next of pipePartial may be the same name but not the same previous
public:
    pipeFuture0(const QString& aName) : pipe0(aName){
    }
protected:
    void insertNext(const QString& aName, const QString& aTag) override{
        m_next2.push_back(QPair<QString, QString>(aName, aTag));
    }
private:
    QVector<QPair<QString, QString>> m_next2;
    QHash<QString, QJsonObject> m_locals;
    friend pipeFuture;
};

static QHash<QString, pipeline*> pipelines;

void pipeline::execute(const QString& aName, std::shared_ptr<stream0> aStream, bool aNeedFuture){
    auto pip = find(aName, aNeedFuture);
    if (pip)
        pip->execute(aStream);
}

void pipeline::tryExecutePipeOutside(const QString& aName, std::shared_ptr<stream0> aStream){
    for (auto i : pipelines)
        if (i != this)
            i->execute(aName, aStream, false);
}

void pipeFuture::execute(std::shared_ptr<stream0> aStream){
    pipeline::instance()->tryExecutePipeOutside(actName(), aStream);
}

pipeFuture::pipeFuture(const QString& aName) : pipe0 (){
    m_act_name = aName;

    if (pipeline::find(aName + "_pipe_add", false)){  //there will be another pipeFuture before, this future should inherit its records before it is removed
        auto pip = new pipeFuture0(aName);
        pipeline::call<int>(aName + "_pipe_add", 0);
        for (auto i : pip->m_next2)
            insertNext(i.first, i.second);
        setAspect(m_before, pip->m_before);
        setAspect(m_around, pip->m_around);
        setAspect(m_after, pip->m_after);
        pipeline::remove(aName);
    }
    pipeline::add<int>([this, aName](const stream<int>* aInput){
        auto this_event = pipeline::find(aName, false);
        for (auto i : m_next2)
            this_event->insertNext(i.first, i.second);
        setAspect(this_event->m_before, m_before);
        setAspect(this_event->m_around, m_around);
        setAspect(this_event->m_after, m_after);
        pipeline::remove(m_name);
    }, rea::Json("name", aName + "_pipe_add"));
}


void pipeline::remove(const QString& aName){
    auto pipe = instance()->m_pipes.value(aName);
    if (pipe){
        //std::cout << "pipe: " + aName.toStdString() + " is removed!" << std::endl;
        //run("re_log", STMJSON(dst::Json("msg", "pipe " + aName + " is removed")));
        instance()->m_pipes.remove(aName);
        delete pipe; //if aName is from pipe, this must be write in the end
    }
}

void pipeline::tryStartTransaction(std::shared_ptr<transaction> aTransaction){
    if (aTransaction){
        auto pip = m_pipes.value("transactionStart");
        if (pip)
            pip->execute(std::make_shared<stream<transaction*>>(aTransaction.get()));
    }
}

QThread* pipeline::findThread(int aNo){
    auto thread = m_threads.find(aNo);
    if (thread == m_threads.end()){
        auto tmp = std::make_shared<QThread>();
        tmp->start();
        m_threads.insert(aNo, tmp);
        thread = m_threads.find(aNo);
    }
    return thread->get();
}

pipeline* pipeline::instance(const QString& aName){
    if (!pipelines.contains(aName)){
        if (aName == "js")
            pipelines.insert(aName, new pipelineJS());
        else
            pipelines.insert(aName, new pipeline());
    }
    return pipelines.value(aName);
}

pipeline::pipeline(){

}

pipeline::~pipeline(){
    for (auto i : m_threads)
        if (i.get()->isRunning()){
            i.get()->terminate();
            i->wait();
        }
    for (auto i : m_pipes.values())
        delete i;
    m_pipes.clear();
}

}
