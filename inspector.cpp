#include "inspector.h"
#include "ui_inspector.h"

Inspector::Inspector(QWidget *parent) :
                                        QDialog(parent),
                                        ui(new Ui::Inspector)
{
    ui->setupUi(this);
    m_webView = new QWebEngineView(this);
    QStackedLayout* layout = new QStackedLayout(ui->frame);
    ui->frame->setLayout(layout);
    layout->addWidget(m_webView);
    m_webView->load(QUrl("http://localhost:7777"));
    QDialog::show();
}

void Inspector::show()
{
    m_webView->reload();
    QDialog::show();
}

Inspector::~Inspector()
{
    delete ui;
}
