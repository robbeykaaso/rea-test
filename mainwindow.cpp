/****************************************************************************
ref: https://www.jianshu.com/p/3c3888329732
****************************************************************************/

#include "mainwindow.h"

#include "ui_mainwindow.h"
#include <QtWebEngineWidgets>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QMessageBox>
#include <QTextStream>
#include <QWebChannel>
#include "reaJS.h"
#include <sstream>


static int test_sum = 0;
static int test_pass = 0;

namespace rea4 {

static QHash<QString, std::function<void(void)>> m_tests;

void test(const QString& aName){
    m_tests.value(aName)();
}

#define addTest(aName, aTest) \
    rea4::m_tests.insert(#aName, [](){aTest});

}

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "7777");
    ui->setupUi(this);
    m_webView = new QWebEngineView(this);
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(m_webView);

    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    /*m_inspector = nullptr;
    connect(m_webView, &QWidget::customContextMenuRequested, this, [this]() {
        QMenu* menu = new QMenu(this);
        QAction* action = menu->addAction("Inspect");
        connect(action, &QAction::triggered, this, [this](){
            if(m_inspector == nullptr)
            {
                m_inspector = new Inspector(this);
            }
            else
            {
                m_inspector->show();
            }
        });
        menu->exec(QCursor::pos());
    });*/

    m_jsContext = new JsContext();
    m_webChannel = new QWebChannel();
    m_webChannel->registerObject("context", m_jsContext);
    m_webChannel->registerObject("Pipeline", rea4::pipeline::instance("js"));
    m_webView->page()->setWebChannel(m_webChannel);
    connect(m_jsContext, &JsContext::recvdMsg, this, [this](const QJsonObject& msg) {
        for (auto i : msg.keys())
            test_sum += msg.value(i).toInt();
        for (auto i : msg.keys())
            rea4::test(i);
        auto sw = "Received message: hello";
        ui->statusBar->showMessage(sw);
    });
    m_webView->setUrl(QUrl("file:/html/test.html"));


    unitTest();
}

void MainWindow::unitTest(){
    rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
        test_pass++;
        std::cout << QString("Success: %1 (%2/%3)")
                         .arg(aInput->data())
                         .arg(test_pass)
                         .arg(test_sum)
                         .toStdString() << std::endl;
        aInput->out();
    }, rea::Json("name", "testSuccess"));

    rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
        test_pass--;
        std::cout << QString("Fail: %1 (%2/%3)")
                         .arg(aInput->data())
                         .arg(test_pass)
                         .arg(test_sum)
                         .toStdString() << std::endl;
        aInput->out();
    }, rea::Json("name", "testFail"));

    addTest(test4,
            rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
                assert(aInput->data() == 4.0);
                aInput->setData(aInput->data() + 1)->out();
            }, rea::Json("name", "test4_", "external", true));

            rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
                assert(aInput->data() == 5.0);
                aInput->setData(aInput->data() + 1)->out();
            }, rea::Json("name", "test4__", "external", true));
        )
    addTest(test5,
            rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
                assert(aInput->data() == "hello");
                aInput->setData("world")->out();
            }, rea::Json("name", "test5", "external", true));
        )

    rea4::m_tests.insert("test6",[](){
        rea4::pipeline::find("test6__")->removeNext("test_6");
        rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
            aInput->out();
        }, rea::Json("name", "test6"))
            ->next("test6_")
            ->next("test6__")
            ->next<double>([](rea4::stream<double>* aInput){
                assert(aInput->data() == 6.0);
                aInput->outs<QString>("Pass: test6", "testSuccess");
            }, "", rea::Json("name", "test_6"));

        rea4::pipeline::run<double>("test6", 4);
    });

    addTest(test7,
            rea4::pipeline::find("test7")->removeNext("test_7");
            rea4::pipeline::find("test7")
                ->next<QString>([](rea4::stream<QString>* aInput){
                    assert(aInput->data() == "world");
                    aInput->outs<QString>("Pass: test7", "testSuccess");
                }, "", rea::Json("name", "test_7"));

            rea4::pipeline::run<QString>("test7", "hello");
        )

    addTest(test8,
            rea4::pipeline::run<QString>("test8", "hello");
        )

    addTest(test9,
            rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
                assert(aInput->data() == "hello");
                aInput->outs<QString>("Pass: test9", "testSuccess");
            }, rea::Json("name", "test9", "external", true));
        )

    addTest(test11,
            rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
                assert(aInput->data() == 3);
                aInput->out();
            }, rea::Json("name", "test11"))
                ->next(rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
                    assert(aInput->data() == 3);
                    aInput->outs<QString>("Pass: test11", "testSuccess");
                }))
                ->next("testSuccess");

            rea4::pipeline::run<int>("test11", 3);
        )

    addTest(test12,
            rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
                assert(aInput->data() == 4);
                std::stringstream ss;
                ss << std::this_thread::get_id();
                aInput->outs<std::string>(ss.str(), "test12_0");
            }, rea::Json("name", "test12"))
                ->next(rea4::pipeline::add<std::string>([](rea4::stream<std::string>* aInput){
                    std::stringstream ss;
                    ss << std::this_thread::get_id();
                    assert(ss.str() != aInput->data());
                    aInput->outs<QString>("Pass: test12", "testSuccess");
                }, rea::Json("name", "test12_0", "thread", 2)))
                ->next("testSuccess");

            rea4::pipeline::run<int>("test12", 4);
        )

    addTest(test13,
            rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
                assert(aInput->data() == 66);
                aInput->outs<QString>("test13", "test13_0");
            }, rea::Json("name", "test13"))
                ->next("test13_0")
                ->next("testSuccess");

            rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
                aInput->out();
            }, rea::Json("name", "test13_1"))
                ->next("test13__")
                ->next("testSuccess");

            rea4::pipeline::find("test13_0")
                ->next<QString>([](rea4::stream<QString>* aInput){
                    aInput->out();
                }, "", rea::Json("name", "test13__"))
                ->next("testSuccess");

            rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
                assert(aInput->data() == "test13");
                aInput->outs<QString>("Pass: test13", "testSuccess");
                aInput->outs<QString>("Pass: test13_", "test13__");
            }, rea::Json("name", "test13_0"));

            rea4::pipeline::run<int>("test13", 66);
            rea4::pipeline::run<QString>("test13_1", "Pass: test13__");
        )

    rea4::m_tests.insert("test14", [](){
        rea4::pipeline::add<int, rea4::pipePartial>([](rea4::stream<int>* aInput){
            assert(aInput->data() == 66);
            aInput->setData(77)->out();
        }, rea::Json("name", "test14"))
            ->nextB(rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
                        assert(aInput->data() == 77);
                        aInput->outs<QString>("Pass: test14", "testSuccess");
                    }), "test14")
            ->next(rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
                       assert(aInput->data() == 77);
                       aInput->outs<QString>("Fail: test14", "testFail");
                   }), "test14_");

            rea4::pipeline::run<int>("test14", 66, "test14");
        });

    addTest(test16,
            rea4::pipeline::find("test16")->removeNext("test16_");
            rea4::pipeline::find("test16")->removeNext("test16__");

            rea4::pipeline::find("test16")
                ->nextB(rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
                            assert(aInput->data() == 77.0);
                            aInput->outs<QString>("Pass: test16", "testSuccess");
                        }, rea::Json("name", "test16_")), "test16")
                ->next(rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
                           assert(aInput->data() == 77.0);
                           aInput->outs<QString>("Fail: test16", "testFail");
                       }, rea::Json("name", "test16__")), "test16_");

            rea4::pipeline::run<double>("test16", 66, "test16");
        )

    rea4::m_tests.insert("test17", [](){
        rea4::pipeline::add<double, rea4::pipePartial>([](rea4::stream<double>* aInput){
            assert(aInput->data() == 66);
            aInput->setData(77)->out();
        }, rea::Json("name", "test17", "external", true));
    });

    rea4::m_tests.insert("test18", [](){
        rea4::pipeline::add<int, rea4::pipeDelegate>([](rea4::stream<int>* aInput){
            assert(aInput->data() == 66);
            aInput->out();
        }, rea::Json("name", "test18_0", "delegate", "test18"))
        ->next("testSuccess");

        rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
            assert(aInput->data() == 56);
            aInput->outs<QString>("Pass: test18", "testSuccess");
        }, rea::Json("name", "test18"));

        rea4::pipeline::run<int>("test18_0", 66);
        rea4::pipeline::run<int>("test18", 56);
    });

    rea4::m_tests.insert("test20", [](){
        rea4::pipeline::add<double, rea4::pipeDelegate>([](rea4::stream<double>* aInput){
            assert(aInput->data() == 66.0);
            aInput->out();
        }, rea::Json("name", "test20_0", "delegate", "test20"))
            ->nextB("testSuccess")
            ->next("testSuccessJS");

        rea4::pipeline::run<double>("test20_0", 66);
        rea4::pipeline::run<double>("test20", 56);
    });

    rea4::m_tests.insert("test21", [](){
        rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
            assert(aInput->data() == 56.0);
            aInput->outs<QString>("Pass: test21");
        }, rea::Json("name", "test21", "external", true));
    });

    rea4::m_tests.insert("test22", [](){
        rea4::pipeline::input<int>(0, "test22")
            ->asyncCall<int>([](rea4::stream<int>* aInput){
                aInput->setData(aInput->data() + 1)->out();
            }, rea::Json("thread", 1))
            ->asyncCall<QString>([](rea4::stream<int>* aInput){
                assert(aInput->data() == 1);
                aInput->outs<QString>("world");
            }, rea::Json("thread", 2))
            ->asyncCall<QString>([](rea4::stream<QString>* aInput){
                assert(aInput->data() == "world");
                aInput->setData("Pass: test22")->out();
            })
            ->asyncCall("testSuccess");
    });

    rea4::m_tests.insert("test24", [](){
        rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
            assert(aInput->data() == 24.0);
            aInput->outs<QString>("Pass: test24");
        }, rea::Json("name", "test24", "thread", 5, "external", true));
    });

    rea4::m_tests.insert("test25", [](){
        rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
            rea4::pipeline::input<double>(25, "test25")
                ->asyncCall<QString>("test25")
                ->asyncCall("testSuccess");
        }, rea::Json("name", "test25_", "thread", 1));
        rea4::pipeline::run<double>("test25_", 0);
    });

    rea4::test("test4");
    rea4::test("test5");
    rea4::test("test9");
    rea4::test("test17");
    rea4::test("test21");
    rea4::test("test24");
}

MainWindow::~MainWindow()
{
    delete ui;
}

JsContext::JsContext(QObject*){

}

void JsContext::sendMsg(QWebEnginePage* page, const QString& msg){
    page->runJavaScript(QString("recvMessage('%1');").arg(msg));
}

void JsContext::onMsg(const QJsonValue &msg)
{
    if (msg.isObject())
        emit recvdMsg(msg.toObject());
}

/*static rea::regPip<QQmlApplicationEngine*> reg_web([](rea::stream<QQmlApplicationEngine*>* aInput){
    static auto m_webView = new QWebEngineView();
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(m_webView);

    aInput->out();
}, QJsonObject(), "regQML");*/
