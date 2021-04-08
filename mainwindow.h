/****************************************************************************
ref: https://www.jianshu.com/p/3c3888329732
****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QString>
#include "inspector.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class JsContext : public QObject
{
    Q_OBJECT
public:
    explicit JsContext(QObject *parent = nullptr);

signals:
    void recvdMsg(const QString& msg);
public:
    // 向页面发送消息
    void sendMsg(QWebEnginePage* page, const QString& msg);

public slots:
    // 接收到页面发送来的消息
    void onMsg(const QJsonValue& msg);
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QQmlApplicationEngine* aEngine, QWidget *parent = nullptr);
    ~MainWindow();
private:
    void unitTest();
    JsContext* m_jsContext;
    QWebChannel* m_webChannel;
    Ui::MainWindow *ui;
    QWebEngineView *m_webView;
    Inspector* m_inspector;
};



#endif // MAINWINDOW_H
