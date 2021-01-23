#include "reaC++.h"
#include "imagePool.h"
#include <QImage>

static rea::regPip<QJsonObject> test_qsg([](rea::stream<QJsonObject>* aInput){
    if (!aInput->data().value("qsg").toBool()){
        aInput->out();
        return;
    }

    auto pth = "D:/mywork/qsgboardtest/微信图片_20200916112142.png";
    rea::pipeline::add<QJsonObject>([pth](rea::stream<QJsonObject>* aInput){
        QImage img(pth);
        rea::imagePool::cacheImage(pth, img);
        auto cfg = rea::Json("width", img.width() ? img.width() : 600,
                             "height", img.height() ? img.height() : 600,
                             "arrow", rea::Json("visible", false,
                                                "pole", true),
                             "face", 200,
                             "text", rea::Json("visible", false,
                                               "size", rea::JArray(100, 50),
                                               "location", "bottom"),
                             "color", "blue",
                             "objects", rea::Json(
                                            "img_2", rea::Json(
                                                         "type", "image",
                                                         "range", rea::JArray(0, 0, img.width(), img.height()),
                                                         "path", pth,
                                                         "caption", "Text",
                                                         "color", "green"//,
                                                                          // "text", rea::Json("visible", true,
                                                                          //                   "size", rea::JArray(100, 50),
                                                                          //                   "location", "cornerLT")
                                                                                                                      ),
                                            "shp_0", rea::Json(
                                                         "type", "poly",
                                                         "points", rea::JArray(QJsonArray(),
                                                                               rea::JArray(50, 50, 200, 50, 200, 200, 50, 200, 50, 50),
                                                                               rea::JArray(80, 70, 120, 100, 120, 70, 80, 70)),
                                                         "color", "red",
                                                         "width", 3,
                                                         "caption", "poly",
                                                         "style", "dash",
                                                         "face", 50,
                                                         "text", rea::Json("visible", true,
                                                                           "size", rea::JArray(100, 50))
                                                                                                                      ),
                                            "shp_1", rea::Json(
                                                         "type", "ellipse",
                                                         "center", rea::JArray(400, 400),
                                                         "radius", rea::JArray(300, 200),
                                                         "width", 5,
                                                         "style", "dash",
                                                         "ccw", false,
                                                         "angle", 30,
                                                         "caption", "ellipse"
                                                                                                                      )
                                                                                                ));

        auto view = aInput->data();
        for (auto i : view.keys())
            cfg.insert(i, view.value(i));

        aInput->outs<QJsonObject>(cfg, "updateQSGModel_testbrd");
    }, rea::Json("name", "testQSGShow"));

    rea::pipeline::add<QJsonObject>([pth](rea::stream<QJsonObject>* aInput){
        QImage img(pth);
        auto cfg = rea::Json("width", img.width() ? img.width() : 600,
                             "height", img.height() ? img.height() : 600,
                             "objects", rea::Json(
                                            "img_2", rea::Json(
                                                         "type", "image",
                                                         "range", rea::JArray(0, 0, img.width(), img.height()),
                                                         "path", pth)));
        for (int i = 0; i < 2000; ++i){
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 100));
            rea::imagePool::cacheImage(pth, img);
            rea::pipeline::run<QJsonObject>("updateQSGModel_testbrd", cfg);
        }
    }, rea::Json("name", "testFPS", "thread", 5));

    rea::pipeline::add<rea::transaction*>([](rea::stream<rea::transaction*>* aInput){
        auto rt = aInput->data();
        if (rt->getName() != "updateQSGPos_testbrd;")
            aInput->out();
    }, rea::Json("name", "filterUpdateQSGPosS", "before", "transactionStart"));

    rea::pipeline::add<QJsonObject>([](rea::stream<QJsonObject>* aInput){
        auto dt = aInput->data();
        if (dt.value("name") != "updateQSGPos_testbrd;")
            aInput->out();
    }, rea::Json("name", "filterUpdateQSGPosE", "before", "transactionEnd"));
}, QJsonObject(), "unitTest");
