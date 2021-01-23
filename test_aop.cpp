#include "reaC++.h"
#include "util.h"

struct TimeAspect
{
    void Before(int i)
    {
        std::cout << "time before" << std::endl;
    }

    void After(int i)
    {
        std::cout <<"time after" << std::endl;
    }
};

struct LoggingAspect
{
    void Before(int i)
    {
        std::cout <<"log before"<< std::endl;
    }

    void After(int i)
    {
        std::cout <<"log after"<< std::endl;
    }
};

void foo(int a)
{
    std::cout <<"real function: " << a << std::endl;
}

static rea::regPip<QJsonObject> unit_test([](rea::stream<QJsonObject>* aInput){
    //https://www.cnblogs.com/qicosmos/p/4772389.html
    if (!aInput->data().value("aop").toBool()){
        aInput->out();
        return;
    }
    rea::Invoke<LoggingAspect, TimeAspect>(&foo, 1);
    std::cout <<"-----------------------"<< std::endl;
    rea::Invoke<TimeAspect, LoggingAspect>(&foo, 1);
    aInput->out();
}, QJsonObject(), "unitTest");
