#include "rea.h"
#include <mutex>
#include <sstream>
#include <QFile>
#include <QJsonDocument>
#include "reaJS.h"

namespace rea4 {

pipe0::pipe0(const QString& aName, int aThreadNo){
    if (aName == "")
        m_name = rea::generateUUID();
    else
        m_name = aName;
    if (aThreadNo != 0){
        m_thread = pipeline::instance()->findThread(aThreadNo);
        moveToThread(m_thread);
    }
    if (pipeline::find(m_name, false)){
        pipeline::instance()->remove(m_name);
    }
    pipeline::instance()->m_pipes.insert(m_name, this);
}

void pipe0::resetTopo(){
    m_next.clear();
    m_before = "";
    m_around = "";
    m_after = "";
}

pipe0* pipe0::next(pipe0* aNext, const QString& aTag){
    assert(!this->m_external);
    auto tags = aTag.split(";");
    for (auto i : tags)
        insertNext(aNext->actName(), i);
    return aNext;
}

pipe0* pipe0::next(const QString& aName, const QString& aTag){
    assert(!this->m_external);
    auto tags = aTag.split(";");
    for (auto i : tags)
        insertNext(aName, i);
    auto nxt = pipeline::find(aName);
    return nxt;
}

void pipe0::removeNext(const QString &aName){
    assert(!this->m_external);
    m_next.remove(aName);
}

void pipe0::removeAspect(pipe0::AspectType aType, const QString& aAspect){
    assert(!this->m_external);
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

void pipe0::tryExecutePipe(const QString& aName, std::shared_ptr<stream0> aStream){
    auto pip = pipeline::find(aName);
    if (pip){
        if (pip->m_external)
            pipeline::instance()->tryExecutePipeOutside(pip->actName(), aStream, QJsonObject(), false);
        else
            pip->execute(aStream);
    }
}

void pipe0::doNextEvent(const QMap<QString, QString>& aNexts, std::shared_ptr<stream0> aStream){
    auto outs = aStream->m_outs;
    aStream->m_outs = nullptr;
    if (outs){
        if (outs->size() > 0){
            for (auto i : *outs)
                if (i.first == ""){
                    for (auto j : aNexts.keys())
                        tryExecutePipe(j, i.second);
                }else
                    tryExecutePipe(i.first, i.second);
        }else
            for (auto i : aNexts.keys())
                tryExecutePipe(i, aStream);
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

void pipeFuture::resetTopo(){
    m_next2.clear();
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
    friend pipeFuture;
};

transaction::transaction(const QString& aID){
    m_id = aID;
}

static QHash<QString, pipeline*> pipelines;

void pipeline::execute(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync, bool aFromOutside){
    auto pip = find(aName, !aFromOutside);
    if (pip){
        if (!aSync.empty()){
            pip->resetTopo();
            auto nxts = aSync.value("next").toArray();
            for (auto i : nxts){
                auto nxt = i.toArray();
                pip->insertNext(nxt[0].toString(), nxt[1].toString());
            }
            pip->setAspect(pip->m_before, aSync.value("before").toString());
            pip->setAspect(pip->m_after, aSync.value("after").toString());
            pip->setAspect(pip->m_around, aSync.value("around").toString());
        }
        pip->execute(aStream);
    }
}

void pipeline::tryExecutePipeOutside(const QString& aName, std::shared_ptr<stream0> aStream, const QJsonObject& aSync, bool aFromOutside){
    for (auto i : pipelines)
        if (i != this)
            i->execute(aName, aStream, aSync, aFromOutside);
}

void pipeFuture::execute(std::shared_ptr<stream0> aStream){
    QJsonObject sync;
    QJsonArray nxts;
    for (auto i : m_next2)
        nxts.push_back(rea::JArray(i.first, i.second));
    if (nxts.size() > 0)
        sync.insert("next", nxts);
    if (m_before != "")
        sync.insert("before", m_before);
    if (m_around != "")
        sync.insert("around", m_around);
    if (m_after != "")
        sync.insert("after", m_after);
    pipeline::instance()->tryExecutePipeOutside(actName(), aStream, sync);
}

void pipeFuture::removeNext(const QString& aName){
    for (auto i = m_next2.size() - 1; i >= 0; --i)
        if (m_next2[i].first == aName)
            m_next2.remove(i);
}

pipeFuture::pipeFuture(const QString& aName) : pipe0 (){
    m_act_name = aName;

    if (pipeline::find(aName + "_pipe_add", false)){  //there will be another pipeFuture before, this future should inherit its records before it is removed
        auto pip = new pipeFuture0(aName);
        pipeline::call<int>(aName + "_pipe_add", 0);
        for (auto i : pip->m_next2)
            insertNext(i.first, i.second);
        m_external = pip->m_external;
        setAspect(m_before, pip->m_before);
        setAspect(m_around, pip->m_around);
        setAspect(m_after, pip->m_after);
        pipeline::instance()->remove(aName);
    }
    pipeline::add<int>([this, aName](const stream<int>* aInput){
        auto this_event = pipeline::find(aName, false);
        for (auto i : m_next2)
            this_event->insertNext(i.first, i.second);
        this_event->m_external = m_external;
        setAspect(this_event->m_before, m_before);
        setAspect(this_event->m_around, m_around);
        setAspect(this_event->m_after, m_after);
        pipeline::instance()->remove(m_name);
    }, rea::Json("name", aName + "_pipe_add"));
}


void pipeline::remove(const QString& aName, bool aOutside){
    auto pipe = instance()->m_pipes.value(aName);
    if (pipe){
        //std::cout << "pipe: " + aName.toStdString() + " is removed!" << std::endl;
        //run("re_log", STMJSON(dst::Json("msg", "pipe " + aName + " is removed")));
        instance()->m_pipes.remove(aName);
        delete pipe; //if aName is from pipe, this must be write in the end
    }
    if (aOutside)
        instance()->removePipeOutside(aName);
}

void pipeline::removePipeOutside(const QString& aName){
    for (auto i : pipelines.values())
        if (i != this)
            i->remove(aName);
}

void pipeline::removeTransaction(const QString& aID){
    m_transactions.remove(aID);
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
