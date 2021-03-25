#ifndef INSPECTOR_H
#define INSPECTOR_H

#include <QDialog>
#include <QtWebEngineWidgets>

namespace Ui {
class Inspector;
}

class Inspector : public QDialog
{
    Q_OBJECT

public:
    explicit Inspector(QWidget *parent = nullptr);
    ~Inspector();
public Q_SLOTS:
    void show();
private:
    Ui::Inspector *ui;
    QWebEngineView* m_webView;
};

#endif // INSPECTOR_H
