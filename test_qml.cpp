#include <reaC++.h>

using namespace rea;

static regPip<QJsonObject> test_qml([](stream<QJsonObject>* aInput){
    if (aInput->data().value("qml").toBool())
        pipeline::instance()->engine->load(QUrl(QStringLiteral("test2.qml")));
    aInput->out();
},  QJsonObject(), "unitTest");
