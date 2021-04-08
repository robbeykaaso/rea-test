#include <reaC++.h>
#include <sstream>

using namespace rea;

void test1(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 3);
        aInput->out();
    }, Json("name", "test1"))
        ->next(pipeline::add<int>([](stream<int>* aInput){
            assert(aInput->data() == 3);
            aInput->outs<QString>("Pass: test1", "testSuccess");
        }))
        ->next("testSuccess");

    pipeline::instance()->run("test1", 3);
}

void test2(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 4);
        std::stringstream ss;
        ss << std::this_thread::get_id();
        aInput->outs<std::string>(ss.str(), "test2_0");
    }, Json("name", "test2"))
        ->next(pipeline::add<std::string>([](stream<std::string>* aInput){
            std::stringstream ss;
            ss << std::this_thread::get_id();
            assert(ss.str() != aInput->data());
            aInput->outs<QString>("Pass: test2", "testSuccess");
        }, Json("name", "test2_0", "thread", 2)))
        ->next("testSuccess");

    pipeline::instance()->run("test2", 4);
}

static rea::regPip<QJsonObject> reg_test3([](rea::stream<QJsonObject>* aInput){
    if (!aInput->data().value("rea").toBool()){
        aInput->out();
        return;
    }
    aInput->outs<QString>("Pass: test3___", "testSuccess");
    aInput->out();
}, QJsonObject(), "unitTest");

void test3(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 66);
        aInput->outs<QString>("test3", "test3_0");
    }, Json("name", "test3"))
        ->next("test3_0")
        ->next("testSuccess");

    pipeline::add<QString>([](rea::stream<QString>* aInput){
        aInput->out();
    }, rea::Json("name", "test3_1"))
        ->next(rea::local("test3__"))
        ->next("testSuccess");

    rea::pipeline::find("test3_0")
        ->nextF<QString>([](rea::stream<QString>* aInput){
            aInput->out();
        }, "", Json("name", "test3__"))
        ->next("testSuccess");

    pipeline::add<QString>([](stream<QString>* aInput){
        assert(aInput->data() == "test3");
        aInput->outs<QString>("Pass: test3", "testSuccess");
        aInput->outs<QString>("Pass: test3_", "test3__");
    }, Json("name", "test3_0"));

    pipeline::instance()->run<int>("test3", 66);
    pipeline::instance()->run<QString>("test3_1", "Pass: test3__");
}

void test4(){
    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 56);
        aInput->outs<QString>("8", "");
    }, Json("name", "test4"))
        ->nextB(local("test4_0")
                    ->nextB(rea::pipeline::add<QString>([](rea::stream<QString>* aInput){
                                assert(aInput->data() == "Pass: test4");
                                aInput->outs<QString>("Pass: test4__");
                            })->nextB(local("testSuccess"))))
        //->next("test4_0")
        ->next(local("test4_0"))
        ->next(local("testSuccess"));

    pipeline::add<QString>([](stream<QString>* aInput){
        assert(aInput->data() == "8");
        //aInput->setData("Pass: test4");
        aInput->outs<QString>("Pass: test4");
    }, Json("name", "test4_0"))
        ->next("testFail");

    pipeline::instance()->run("test4", 56);

    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        assert(aInput->data().value("56") == "56");
        aInput->out();
    }, Json("name", "test4_"))
        //->next("test4_1");
        ->next(local("test4_1"))
        ->next(local("testSuccess"));
    pipeline::instance()->run("test4_", Json("56", "56"));
}

void test5(){
    pipeline::add<int, pipeDelegate>([](stream<int>* aInput){
        assert(aInput->data() == 66);
        aInput->out();
    }, Json("name", "test5_0", "delegate", "test5"))
        ->next("testSuccess");

    pipeline::add<int>([](stream<int>* aInput){
        assert(aInput->data() == 56);
        aInput->outs<QString>("Pass: test5", "testSuccess");
    }, Json("name", "test5"));

    pipeline::instance()->run("test5_0", 66);
    pipeline::instance()->run("test5", 56, "", false);
}

void test6(){
    pipeline::add<int, pipePartial>([](stream<int>* aInput){
        assert(aInput->data() == 66);
        aInput->setData(77);
        aInput->out();
        //aInput->out(dst::Json("tag", "service2"));
    }, Json("name", "test6"));

    pipeline::find("test6")
        ->nextB(pipeline::add<int>([](stream<int>* aInput){
                    assert(aInput->data() == 77);
                    aInput->outs<QString>("Pass: test6", "testSuccess");
                })->nextB("testSuccess"), "service1")
        ->next(pipeline::add<int>([](stream<int>* aInput){
                   assert(aInput->data() == 77);
                   aInput->outs<QString>("Fail: test6", "testFail");
               }), "service2")
        ->next("testFail");

    pipeline::instance()->run("test6", 66, "service1");
}

void test7(){
    pipeline::add<int>([](rea::stream<int>* aInput){
        aInput->outs<int>(66);
        aInput->outs<int>(68);
    }, rea::Json("name", "test7"))
        ->next(buffer<int>(2))
        ->next(pipeline::add<std::vector<int>>([](stream<std::vector<int>>* aInput){
            auto dt = aInput->data();
            assert(dt.size() == 2);
            assert(dt.at(0) == 66);
            assert(dt.at(1) == 68);
            aInput->outs<QString>("Pass: test7", "testSuccess");
        }))
        ->next("testSuccess");
    pipeline::instance()->run<int>("test7", 0);
    pipeline::instance()->run<QJsonObject>("test7_", QJsonObject());
}

void test8(){
    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        aInput->var<QString>("test8_var", "test8_var");
        aInput->out();
    }, Json("name", "test8"))->next("test8_");

    pipeline::add<QJsonObject>([](stream<QJsonObject>* aInput){
        assert(aInput->data().value("test8") == "test8");
        assert(aInput->varData<QString>("test8_var") == "test8_var_");
        aInput->outs<QString>("Pass: test8", "testSuccess");
    }, Json("name", "test8_0"))
        ->next("testSuccess");

    pipeline::instance()->run("test8", Json("test8", "test8"));
}

void test9(){
    static std::mutex mtx;
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        aInput->var<std::shared_ptr<QSet<QThread*>>>("threads", std::make_shared<QSet<QThread*>>())
            ->var<int>("count", 0);
        for (int i = 0; i < 200; ++i)
            aInput->outs<double>(i);
    }, rea::Json("name", "test9"))
        ->next(rea::pipeline::add<double, rea::pipeParallel>([](rea::stream<double>* aInput){
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            {
                std::lock_guard<std::mutex> gd(mtx);
                auto trds = aInput->varData<std::shared_ptr<QSet<QThread*>>>("threads");
                trds->insert(QThread::currentThread());
                aInput->var<int>("count", aInput->varData<int>("count") + 1);
            }
            aInput->out();
        }))->nextF<double>([](rea::stream<double>* aInput){
            std::lock_guard<std::mutex> gd(mtx);
            auto cnt = aInput->varData<int>("count");
            if (cnt == 200){
                aInput->var<int>("count", cnt + 1);
                assert(aInput->varData<std::shared_ptr<QSet<QThread*>>>("threads")->size() == 8);
                aInput->outs<QString>("Pass: test9", "testSuccess");
            }
        });
    rea::pipeline::instance()->run<double>("test9", 0);
}

void test10(){
    static int cnt = 0;
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        for (int i = 0; i < 100; ++i)
            aInput->setData(i)->outs<double>(0, "", "", false)->var<int>("count", i);
    }, rea::Json("name", "test10"))
        ->next(rea::pipeline::add<double, rea::pipeThrottle>([](rea::stream<double>* aInput) {
            aInput->out();
        }))
        ->nextF<double>([](rea::stream<double>* aInput){
            ++cnt;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (aInput->varData<int>("count") == 99){
                assert(cnt < 100);
                aInput->outs<QString>("Pass: test10", "testSuccess");
            }
        }, "", rea::Json("thread", 6));
    rea::pipeline::instance()->run<double>("test10", 0);
}

void test11(){
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 1.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test__11", "before", "test_11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 2.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test_11", "before", "test11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 3.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 4.0);
        aInput->setData(dt + 1)->out();
    }, rea::Json("name", "test11_", "after", "test11"));

    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        auto dt = aInput->data();
        assert(dt == 5.0);
        aInput->outs<QString>("Pass: test11", "testSuccess");
    }, rea::Json("name", "test11__", "after", "test11_"));

    rea::pipeline::instance()->run<double>("test11", 1);
}

void test12(){
    rea::pipeline::instance()->input<int>(0, "test12")
        ->call<int>([](rea::stream<int>* aInput){
            aInput->setData(aInput->data() + 1)->out();
        }, rea::Json("thread", 1))
        ->call<QString>([](rea::stream<int>* aInput){
            assert(aInput->data() == 1);
            aInput->outs<QString>("world");
        }, rea::Json("thread", 2))
        ->call<QString>([](rea::stream<QString>* aInput){
            assert(aInput->data() == "world");
            aInput->setData("Pass: test12")->out();
        })
        ->call("testSuccess");
}

class foo{
public:
    foo(int aStart){
        m_start = aStart;
    }
    void operator()(rea::stream<int>* aInput) {
        auto ret = aInput->data() + m_start;
        aInput->setData(ret)->out();
    }

    void memberFoo(rea::stream<int>* aInput){
        auto ret = aInput->data() + m_start;
        aInput->setData(ret)->out();
    }
private:
    int m_start;
};

void test13(){
    auto tmp = foo(6);
    rea::pipeline::add<int>(foo(2));
    rea::pipeline::instance()->input<int>(3)
        ->call<int>(foo(2))
        ->call<int>(std::bind1st(std::mem_fun(&foo::memberFoo), &tmp))
        ->call<QString>([](rea::stream<int>* aInput){
            assert(aInput->data() == 11);
            aInput->outs<QString>("Pass: test13")->out();
        })
        ->call("testSuccess");
}

void testReactive2(){
    test1(); // test anonymous next
    test2(); // test specific next and multithread
    test3(); // test pipe future
    test4(); // test pipe local
    test5(); // test pipe delegate and pipe param
    test6(); // test pipe partial and next/stream param and nextB
    test7(); // test pipe Buffer
    test8(); // test pipe QML
    test9(); // test pipe parallel
    test10(); // test pipe throttle
    test11(); //test pipe aop
    test12(); //test stream program
    test13(); //test functor
}

static regPip<QJsonObject> test_param([](stream<QJsonObject>* aInput){
    aInput->setData(rea::Json("qml", true,
                              "rea", true,
                              "qsg", true,
                              "stg", true,
                              "tcp", false,
                              "aop", false,
                              "modbus", false))->out();
}, rea::Json("before", "unitTest"));

static regPip<QJsonObject> test_rea([](stream<QJsonObject>* aInput){

    pipeline::add<QString>([](stream<QString>* aInput){
        std::cout << "Success:" << aInput->data().toStdString() << std::endl;
        aInput->out();
    }, Json("name", "testSuccess"));

    pipeline::add<QString>([](stream<QString>* aInput){
        std::cout << "Fail:" << aInput->data().toStdString() << std::endl;
        aInput->out();
    }, Json("name", "testFail"));

    if (!aInput->data().value("rea").toBool()){
        aInput->out();
        return;
    }

    rea::pipeline::add<double>([](stream<double>* aInput){
        testReactive2();
    }, rea::Json("name", "doUnitTest"));

    aInput->out();
}, rea::Json("name", "unitTest"));
