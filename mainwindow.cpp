/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
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
#include "reaC++.h"
#include "reaJS.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "7777");
    ui->setupUi(this);
    m_webView = new QWebEngineView(this);
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(m_webView);

    m_webView->setContextMenuPolicy(Qt::CustomContextMenu);
    m_inspector = nullptr;
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
    });

    m_jsContext = new JsContext();
    m_webChannel = new QWebChannel();
    m_webChannel->registerObject("context", m_jsContext);
    m_webChannel->registerObject("Pipeline", rea::pipelineJS::instance());
    m_webView->page()->setWebChannel(m_webChannel);
    connect(m_jsContext, &JsContext::recvdMsg, this, [this](const QString& msg) {
        rea::pipelineJS::instance()->trig(rea::Json("hello", "world"));
        auto sw = "Received message: " + msg;
        ui->statusBar->showMessage(sw);
    });
    m_webView->setUrl(QUrl("qrc:/test.html"));


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

static rea::regPip<QQmlApplicationEngine*> reg_web([](rea::stream<QQmlApplicationEngine*>* aInput){
    static MainWindow wd;
    rea::pipeline::add<double>([](rea::stream<double>* aInput){
        wd.show();
    }, rea::Json("name", "openWebWindow"));
    aInput->out();
}, QJsonObject(), "install0_QML");

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
