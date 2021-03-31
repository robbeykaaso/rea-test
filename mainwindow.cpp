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
    connect(m_jsContext, &JsContext::recvdMsg, this, [this](const QString& msg) {
        //rea::pipelineJS::instance()->trig(rea::Json("hello", "world"));
        //rea4::pipeline::run<QString>("testJS", "hello");
        rea4::pipeline::run<double>("test6", 4);
        rea4::pipeline::run<QString>("test7", "hello");
        rea4::pipeline::run<QString>("test8", "hello");
        rea4::pipeline::run<int>("test11", 3);
        rea4::pipeline::run<int>("test12", 4);
        rea4::pipeline::run<int>("test13", 66);
        rea4::pipeline::run<QString>("test13_1", "Pass: test13__");
        auto sw = "Received message: " + msg;
        ui->statusBar->showMessage(sw);
    });
    m_webView->setUrl(QUrl("file:/html/test.html"));


    unitTest();
}

void MainWindow::unitTest(){
    rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
        std::cout << ("Success: " + aInput->data()).toStdString() << std::endl;
    }, rea::Json("name", "testSuccess"));

    rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
        assert(aInput->data() == 4);
        aInput->setData(aInput->data() + 1)->out();
    }, rea::Json("name", "test4_", "external", true));

    rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
        assert(aInput->data() == 5);
        aInput->setData(aInput->data() + 1)->out();
    }, rea::Json("name", "test4__", "external", true));

    rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
        assert(aInput->data() == "hello");
        aInput->setData("world")->out();
    }, rea::Json("name", "test5", "external", true));

    rea4::pipeline::find("test6__")->removeNext("test_6");
    rea4::pipeline::add<double>([](rea4::stream<double>* aInput){
        aInput->out();
    }, rea::Json("name", "test6"))
    ->next("test6_")
    ->next("test6__")
    ->next<double>([](rea4::stream<double>* aInput){
        assert(aInput->data() == 6.0);
        aInput->outs<QString>("Pass: test6", "testSuccess");
    }, "", rea::Json("name", "test_6", "external", true));

    rea4::pipeline::find("test7")->removeNext("test_7");
    rea4::pipeline::find("test7")
        ->next<QString>([](rea4::stream<QString>* aInput){
            assert(aInput->data() == "world");
            aInput->outs<QString>("Pass: test7", "testSuccess");
        }, "", rea::Json("name", "test_7"));

    rea4::pipeline::add<QString>([](rea4::stream<QString>* aInput){
        assert(aInput->data() == "hello");
        aInput->outs<QString>("Pass: test9", "testSuccess");
    }, rea::Json("name", "test9", "external", true));


    rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
        assert(aInput->data() == 3);
        aInput->out();
    }, rea::Json("name", "test11"))
        ->next(rea4::pipeline::add<int>([](rea4::stream<int>* aInput){
        assert(aInput->data() == 3);
        aInput->outs<QString>("Pass: test11", "testSuccess");
    }))
    ->next("testSuccess");


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

void JsContext::onMsg(const QString &msg)
{
    emit recvdMsg(msg);
}

/*static rea::regPip<QQmlApplicationEngine*> reg_web([](rea::stream<QQmlApplicationEngine*>* aInput){
    static auto m_webView = new QWebEngineView();
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(m_webView);

    aInput->out();
}, QJsonObject(), "regQML");*/
