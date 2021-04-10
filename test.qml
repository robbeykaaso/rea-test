import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 1.4
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.12
import QtQuick.Controls.Universal 2.3
import QtWebEngine 1.8
import QtWebChannel 1.0
import "qml/gui/Basic"
import "qml/gui/Pipe"
import "qml/gui/Pipe/TreeNodeView"
import Pipeline 1.0
import QSGBoard 1.0
import Pipeline2 1.0

ApplicationWindow {
    width: 800
    height: 600
    visible: true
    property var view_cfg
    property int test_pass
    property int test_sum
    property var unit_test
    Universal.theme: Universal.Dark

    menuBar: MenuBar{
        id: mainmenu
        Menu{
            title: "rea"
            MenuItem{
                text: "TestRea"
                onClicked: Pipeline.run("doUnitTest", 0, "", false)
            }
            MenuItem{
                text: "TestQMLStorage"
                onClicked: Pipeline.run("writeJson2", "testFS2.json", "testQMLStg", true, {"testFS2.json": {hello: "world"}})
                Component.onCompleted: {
                    console.log("hi")
                    Pipeline.find("writeJson2")
                    .next(function(aInput){
                        var dt = aInput.varData("testFS2.json", "object")
                        console.assert(dt["hello"] === "world")
                        aInput.var("testFS2.json", {hello: "world2"}).outs(aInput.data(), "readJson2")
                        dt = aInput.varData("testFS2.json", "object")
                        console.assert(dt["hello"] === "world2")
                    }, "testQMLStg", {vtype: "string"})
                    .next("readJson2")
                    .next(function(aInput){
                        var dt = aInput.varData("testFS2.json", "object")
                        console.assert(dt["hello"] === "world")
                        aInput.outsB("testFS2.json", "deletePath").outs("Pass: testQMLStorage ", "testSuccess");
                    }, "testQMLStg", {vtype: "string"})
                }
            }
            MenuItem{
                text: "TestQMLStorage2"
                onClicked: Pipeline.run("writeJson2", "testFS2.json", "testQMLStg2", true, {"testFS2.json": {hello: "world"}})

                Component.onCompleted: {
                    Pipeline.find("writeJson2")
                    .next(function(aInput){
                        var tmp = function(){
                            var dt = aInput.varData("testFS2.json", "object")
                            console.assert(dt["hello"] === "world")
                            aInput.var("testFS2.json", {hello: "world2"})
                            dt = aInput.map("testFS2.json").call("readJson2").varData("testFS2.json", "object")
                            console.assert(dt["hello"] === "world")
                            aInput.outsB("testFS2.json", "deletePath").outs("Pass: testQMLStorage2 ", "testSuccess");
                        }
                        tmp()
                        gc()  //https://stackoverflow.com/questions/27315030/how-to-manage-lifetime-of-dynamically-allocated-qobject-returned-to-qml
                    }, "testQMLStg2", {vtype: "string"})
                }
            }
            MenuItem{
                text: "TestQMLStorage3"
                onClicked: {
                    Pipeline.input("testFS2.json", "testQMLStg3", true, {"testFS2.json": {hello: "world"}})
                    .call("writeJson2")
                    .call(function(aInput){
                        var dt = aInput.varData("testFS2.json", "object")
                        console.assert(dt["hello"] === "world")
                        aInput.var("testFS2.json", {hello: "world2"}).out()
                    })
                    .call("readJson2")
                    .call("deletePath")
                    .call(function(aInput){
                        var dt = aInput.varData("testFS2.json", "object")
                        console.assert(dt["hello"] === "world")
                        aInput.setData("Pass: testQMLStorage3 ").out()
                    })
                    .call("testSuccess")
                    .destroy()
                }
            }
            MenuItem{
                text: "TestQMLStreamProgram"
                onClicked: {
                    Pipeline.input(0, "TestQMLStreamProgram")
                    .call(function(aInput){
                        aInput.setData(aInput.data() + 1).out()
                    })
                    .call(function(aInput){
                        console.assert(aInput.data() === 1)
                        aInput.outs("world")
                    }, {vtype: "string"})
                    .call(function(aInput){
                        console.assert(aInput.data() === "world")
                        aInput.setData("Pass: test12_").out()
                    })
                    .call("testSuccess")
                    .destroy()
                }
            }
            MenuItem{
                text: "logTransaction"
                onClicked: Pipeline.run("logTransaction", 0, "", false)
            }
            MenuItem{
                text: "saveTransaction"
                onClicked: Pipeline.run("logTransaction", 1, "", false)
            }

            function test28(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data()["test28"] === "test28")
                    aInput.outs(aInput.data(), "test28_0")
                }, {name: "test28_", external: ""})
            }

            function test31_(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 3)
                    aInput.out()
                }, {name: "test31"})
                .nextF(function(aInput){
                    console.assert(aInput.data() == 3)
                    aInput.outs("Pass: test31", "testSuccessQML")
                })
            }

            function test31(){
                Pipeline2.run("test31", 3)
            }

            function test32_(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 4)
                    aInput.outs(5, "test32_")
                }, {name: "test32"})
                .nextF(function(aInput){
                    console.assert(aInput.data() == 5)
                    aInput.outs("Pass: test32", "testSuccessQML")
                }, "", {name: "test32_"})
            }

            function test32(){
                Pipeline2.run("test32", 4)
            }

            function test33_(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 66)
                    aInput.outs("test33", "test33_0")
                }, {name: "test33"})
                .next("test33_0")
                .next("testSuccessQML")

                Pipeline2.add(function(aInput){
                    aInput.out()
                }, {name: "test33_1"})
                .next("test33__")
                .next("testSuccessQML")

                Pipeline2.find("test33_0")
                .nextF(function(aInput){
                    aInput.out()
                }, "", {name: "test33__"})
                .next("testSuccessQML")

                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == "test33")
                    aInput.outs("Pass: test33", "testSuccessQML")
                    aInput.outs("Pass: test33_", "test33__")
                }, {name: "test33_0"})
            }

            function test33(){
                Pipeline2.run("test33", 66)
                Pipeline2.run("test33_1", "Pass: test33__")
            }

            function test34(){
                Pipeline2.find("test34__").removeNext("test_34")

                Pipeline2.add(function(aInput){
                    aInput.scope().cache("hello", "world")
                    aInput.out()
                }, {name: "test34"})
                .next("test34_")
                .next("test34__")
                .nextF(function(aInput){  //test34__next
                    console.assert(aInput.data() == 6)
                    console.assert(aInput.scope().data("hello") == null)
                    console.assert(aInput.scope().data("hello2") == "world")
                    aInput.outs("Pass: test34", "testSuccessQML")
                }, "", {name: "test_34"})

                Pipeline2.run("test34", 4)
            }

            function test35(){
                Pipeline2.find("test35").removeNext("test_35")

                Pipeline2.find("test35")
                .nextF(function(aInput){
                    console.assert(aInput.data() == "world")
                    aInput.outs("Pass: test35", "testSuccessQML")
                }, "", {name: "test_35"})

                Pipeline2.run("test35", "hello")
            }

            function test36(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 4)
                    console.assert(aInput.scope().data("hello") == "world")
                    aInput.setData(aInput.data() + 1).out()
                }, {name: "test36_", external: ""})

                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 5)
                    aInput.scope(true).cache("hello2", "world");
                    aInput.setData(aInput.data() + 1).out()
                }, {name: "test36__", external: ""})
            }

            function test37(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == "hello")
                    aInput.setData("world").out()
                }, {name: "test37", external: ""})
            }

            function test38_(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 66)
                    aInput.setData(77).out()
                }, {name: "test38", type: "Partial"})
                .nextFB(function(aInput){
                    console.assert(aInput.data() == 77)
                    aInput.outs("Pass: test38", "testSuccessQML")
                }, "test38")
                .nextF(function(aInput){
                    console.assert(aInput.data() == 77)
                    aInput.outs("Fail: test38", "testFailQML")
                }, "test38_")
            }

            function test38(){
                Pipeline2.run("test38", 66, "test38")
            }

            function test39_(){
                Pipeline2.find("test39")
                .nextFB(function(aInput){
                    console.assert(aInput.data() == 77.0)
                    aInput.outs("Pass: test39", "testSuccessQML")
                }, "test39", {name: "test39_"})
                .nextF(function(aInput){
                    console.assert(aInput.data() == 77.0)
                    aInput.outs("Fail: test39", "testFailQML")
                }, "test39_", {name: "test39__"})

            }

            function test39(){
                Pipeline2.run("test39", 66, "test39")
            }

            function test40(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 66)
                    aInput.setData(77).out()
                }, {name: "test40", external: "", type: "Partial"})
            }

            function test41_(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 66.0)
                    aInput.out()
                }, {name: "test41_0",
                    delegate: "test41",
                    type: "Delegate"})
                .next("testSuccessQML")

                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 56.0)
                    aInput.setData("Pass: test41").out()
                }, {name: "test41"})
            }

            function test41(){
                Pipeline2.run("test41_0", 66.0)
                Pipeline2.run("test41", 56.0)
            }

            function test42(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 56.0)
                    aInput.setData("Pass: test42").out()
                }, {name: "test42", external: ""})
            }

            function test43(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 66.0)
                    aInput.out()
                }, {name: "test43_0",
                    delegate: "test43",
                    type: "Delegate"})
                .nextB("testSuccess")
                .next("testSuccessQML")

                Pipeline2.run("test43_0", 66)
            }

            function test43__(){
                Pipeline2.run("test43", 56)
            }

            function test44(){
                Pipeline2.input(0, "test44")
                .asyncCallF(function(aInput){
                    aInput.setData(aInput.data() + 1).out()
                }).asyncCallF(function(aInput){
                    console.assert(aInput.data() == 1)
                    aInput.outs("world")
                }).asyncCallF(function(aInput){
                    console.assert(aInput.data() == "world")
                    aInput.setData("Pass: test44").out()
                }).asyncCall("testSuccessQML")
            }

            function test45(){
                Pipeline2.input(24, "test45")
                .asyncCall("test45")
                .asyncCall("testSuccessQML")
            }

            function test46(){
                Pipeline2.add(function(aInput){
                    console.assert(aInput.data() == 25.0)
                    aInput.setData("Pass: test46").out()
                }, {name: "test46", external: ""})
            }

            function test47(){
                Pipeline2.add(function(aInput){
                    let dt = aInput.data()
                    console.assert(dt == 1.0)
                    aInput.setData(dt + 1).out()
                }, {name: "test__47", before: "test_47", replace: true})

                Pipeline2.add(function(aInput){
                    let dt = aInput.data()
                    console.assert(dt == 2.0)
                    aInput.setData(dt + 1).out()
                }, {name: "test_47", before: "test47", replace: true})

                Pipeline2.add(function(aInput){
                    let dt = aInput.data()
                    console.assert(dt == 3.0)
                    aInput.setData(dt + 1).out()
                }, {name: "test47", replace: true})

                Pipeline2.add(function(aInput){
                    let dt = aInput.data()
                    console.assert(dt == 4.0)
                    aInput.setData(dt + 1).out()
                }, {name: "test47_", after: "test47", replace: true})

                Pipeline2.add(function(aInput){
                    let dt = aInput.data()
                    console.assert(dt == 5.0)
                    aInput.outs("Pass: test47", "testSuccessQML")
                }, {name: "test47__", after: "test47_", replace: true})

                Pipeline2.run("test47", 1)
            }

            Component.onCompleted: {
                Pipeline2.add(function(aInput){
                    test_pass++
                    console.log("Success: " + aInput.data() + "(" + test_pass + "/" + test_sum + ")")
                    aInput.out()
                }, {name: "testSuccessQML"})

                Pipeline2.add(function(aInput){
                    test_pass--
                    console.log("Fail: " + aInput.data() + "(" + test_pass + "/" + test_sum + ")")
                    aInput.out()
                }, {name: "testFailQML"})

                unit_test = {
                    test31: test31,
                    test32: test32,
                    test33: test33,
                    test34: test34,
                    test35: test35,
                    test38: test38,
                    test39: test39,
                    test40: test40,
                    test41: test41,
                    test42: test42,
                    test43: test43,
                    test43__: test43__,
                    test44: test44,
                    test45: test45,
                    test47: test47
                }
                test28()
                test31_()
                test32_()
                test33_()
                test36()
                test37()
                test38_()
                test39_()
                test40()
                test41_()
                test46()

                Pipeline2.add(function(aInput){
                    var dt = aInput.data()
                    for (var i in dt)
                        test_sum += dt[i]
                    for (var i in dt)
                        if (unit_test[i])
                            unit_test[i]()
                }, {name: "unitTestQML", external: "js"})

                Pipeline.add(function(aInput){
                    console.assert(aInput.data()["test8"] === "test8")
                    console.assert(aInput.varData("test8_var", "string") === "test8_var")
                    aInput.outs(aInput.data(), "test8_0", {}, false).var("test8_var", "test8_var_");
                }, {name: "test8_"})

                Pipeline.add(function(aInput){
                    console.assert(aInput.data()["56"] === "56")
                    aInput.outs("Pass: test4_")
                }, {name: "test4_1"}).nextL("testFail")

                Pipeline.add(function(aInput){
                    aInput.outs({"hello": "world"})
                    aInput.outs({"hello": "world2"})
                }, {name: "test7_"})
                .nextP(Pipeline.add(null, {type: "Buffer", count: 2}))
                .next(function(aInput){
                    var dt = aInput.data()
                    console.assert(dt[0]["hello"] === "world")
                    console.assert(dt[1]["hello"] === "world2")
                    aInput.outs("Pass: test7_")
                }, {}, {vtype: "array"})
                .nextL("testSuccess")
            }
        }

        Menu{
            title: "qsgShow"

            delegate: MenuItem {
                id: menuItem
                implicitWidth: 200
                implicitHeight: 40

                arrow: Canvas {
                    x: parent.width - width
                    implicitWidth: 40
                    implicitHeight: 40
                    visible: menuItem.subMenu
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.fillStyle = menuItem.highlighted ? "#ffffff" : "#21be2b"
                        ctx.moveTo(15, 15)
                        ctx.lineTo(width - 15, height / 2)
                        ctx.lineTo(15, height - 15)
                        ctx.closePath()
                        ctx.fill()
                    }
                }

                indicator: Item {
                    implicitWidth: 40
                    implicitHeight: 40
                    Rectangle {
                        width: 14
                        height: 14
                        anchors.centerIn: parent
                        visible: menuItem.checkable
                        border.color: "#21be2b"
                        radius: 3
                        Rectangle {
                            width: 8
                            height: 8
                            anchors.centerIn: parent
                            visible: menuItem.checked
                            color: "#21be2b"
                            radius: 2
                        }
                    }
                }

                contentItem: Text {
                    leftPadding: menuItem.indicator.width
                    rightPadding: menuItem.arrow.width
                    text: menuItem.text
                    font: menuItem.font
                    opacity: enabled ? 1.0 : 0.3
                    color: menuItem.highlighted ? "#ffffff" : "#21be2b"
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }

                background: Rectangle {
                    implicitWidth: 200
                    implicitHeight: 40
                    opacity: enabled ? 1 : 0.3
                    color: menuItem.highlighted ? "#21be2b" : "transparent"
                }
            }

            Menu{
                title: "updateModel"
                MenuItem {
                    text: "show"
                    onClicked: {
                        Pipeline.run("testQSGShow", view_cfg)
                    }
                }

                Action {
                    text: "face"
                    checkable: true
                    shortcut: "Ctrl+F"
                    onTriggered: {
                        view_cfg["face"] = 100 - view_cfg["face"]
                        Pipeline.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: "arrow"
                    checkable: true
                    shortcut: "Ctrl+A"
                    onTriggered: {
                        view_cfg["arrow"]["visible"] = !view_cfg["arrow"]["visible"]
                        Pipeline.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: "text"
                    checkable: true
                    shortcut: "Ctrl+T"
                    onTriggered: {
                        view_cfg["text"]["visible"] = !view_cfg["text"]["visible"]
                        Pipeline.run("testQSGShow", view_cfg)
                    }
                }
                Action {
                    text: "fps"
                    checkable: true
                    onTriggered: {
                        Pipeline.run("testFPS", {})
                    }
                }
            }

            Menu{
                title: "updateWholeAttr"

                MenuItem{
                    checkable: true
                    text: "wholeArrowVisible"
                    onClicked:{
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {key: ["arrow", "visible"], val: checked}, "wholeArrowVisible")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "wholeArrowPole"
                    onClicked:{
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {key: ["arrow", "pole"], val: checked}, "wholeArrowPole")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "wholeFaceOpacity"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {key: ["face"], val: checked ? 200 : 0}, "wholeFaceOpacity")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "wholeTextVisible"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {key: ["text", "visible"], val: checked}, "wholeTextVisible")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "wholeColor"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttrs_testbrd", [{key: ["color"], val: checked ? "yellow" : "green"}], "wholeColor")
                    }
                }

                MenuItem{
                    checkable: true
                    text: "wholeObjects"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttrs_testbrd", [{key: ["objects"], type: checked ? "add" : "del", tar: "shp_3", val: {
                                                                                                         type: "poly",
                                                                                                         points: [[500, 300, 700, 300, 700, 500, 500, 300]],
                                                                                                         color: "pink",
                                                                                                         caption: "new_obj",
                                                                                                         face: 200
                                                                                                     }}], "wholeObjects")
                    }
                }

            }
            Menu{
                title: "updateLocalAttr"

                MenuItem{
                    checkable: true
                    text: "polyArrowVisible"
                    onClicked:{
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["arrow", "visible"], val: checked}, "polyArrowVisible")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyArrowPole"
                    onClicked:{
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["arrow", "pole"], val: checked}, "polyArrowPole")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyFaceOpacity"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["face"], val: checked ? 200 : 0}, "polyFaceOpacity")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyTextVisible"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["text", "visible"], val: checked}, "polyTextVisible")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyColor"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["color"], val: checked ? "yellow" : "green"}, "polyColor")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyCaption"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["caption"], val: checked ? "poly_new" : "poly"}, "polyCaption")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyWidth"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["width"], val: checked ? 0 : 10}, "polyWidth")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyPoints"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["points"], val: checked ? [[50, 50, 200, 50, 200, 200, 50, 200, 50, 50], [80, 70, 120, 100, 120, 70, 80, 70]] : [[50, 50, 200, 200, 200, 50, 50, 50]]}, "polyPoints")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "polyStyle"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_0", key: ["style"], val: checked ? "dash" : "solid"}, "polyStyle")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "ellipseAngle"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["angle"], val: checked ? 90 : 20}, "ellipseAngle")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "ellipseCenter"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["center"], val: checked ? [600, 400] : [400, 400]}, "ellipseCenter")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "ellipseRadius"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["radius"], val: checked ? [200, 400] : [300, 200]}, "ellipseRadius")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "ellipseCCW"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "shp_1", key: ["ccw"], val: checked}, "ellipseCCW")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "imagePath"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "img_2", key: ["path"], val: checked ? "F:/3M/B4DT/DF Mark/V1-1.bmp" : "F:/3M/B4DT/DF Mark/V1-2.bmp"}, "imagePath")
                    }
                }
                MenuItem{
                    checkable: true
                    text: "imageRange"
                    onClicked: {
                        checked != checked
                        Pipeline.run("updateQSGAttr_testbrd", {obj: "img_2", key: ["range"], val: checked ? [0, 0, 600, 800] : [0, 0, 400, 300]}, "imageRange")
                    }
                }
            }

            Component.onCompleted: {
                Pipeline.find("QSGAttrUpdated_testbrd")
                .next(function(aInput){
                    console.log("qsgattr updated!")
                }, "", {vtype: "array"})
            }
        }

        Menu{
            title: "gui"
            Menu{
                title: "log"
                MenuItem{
                    text: "addLogRecord"
                    onClicked:{
                        Pipeline.run("addLogRecord", {type: "train", level: "info", msg: "train_info"})
                        Pipeline.run("addLogRecord", {type: "train", level: "warning", msg: "train_warning"})
                        Pipeline.run("addLogRecord", {type: "train", level: "error", msg: "train_error"})
                        Pipeline.run("addLogRecord", {type: "system", level: "info", msg: "system_info"})
                        Pipeline.run("addLogRecord", {type: "system", level: "warning", msg: "system_warning"})
                        Pipeline.run("addLogRecord", {type: "system", level: "error", msg: "system_error"})
                    }
                }
                MenuItem{
                    text: "showLogPanel"
                    onClicked:{
                        Pipeline.run("showLogPanel", {})
                    }
                }
            }
            Menu{
                title: "list"
                MenuItem{
                    text: "updateListView"
                    onClicked: {
                        Pipeline.run("_updateListView", {title: ["cat", "dog", "sheep", "rat"],
                                                          selects: [1, 3, 5],
                                                          data: [
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]}
                                                          ]})
                    }
                }
                MenuItem{
                    text: "modifyListView"
                    onClicked: {
                        Pipeline.run("_updateListView", {index: [2, 4, 5],
                                                          fontclr: "red",
                                                          data: [
                                                            {entry: [1, 3, 2, 3]},
                                                            {},
                                                            {entry: [2, 3, 2, 3]}
                                                          ]})
                    }
                }
                MenuItem{
                    text: "updatePageListView"
                    onClicked: {
                        Pipeline.run("pageList_updateListView", {title: ["idx", "dog", "sheep", "rat"],
                                                          selects: [1, 3, 11],
                                                          entrycount: 3,
                                                          pageindex: 2,
                                                          data: [
                                                            {entry: [0, 6, 2, 3]},
                                                            {entry: [1, 6, 2, 3]},
                                                            {entry: [2, 6, 2, 3]},
                                                            {entry: [3, 6, 2, 3]},
                                                            {entry: [4, 6, 2, 3]},
                                                            {entry: [5, 6, 2, 3]},
                                                            {entry: [6, 6, 2, 3]},
                                                            {entry: [7, 6, 2, 3]},
                                                            {entry: [8, 6, 2, 3]},
                                                            {entry: [9, 6, 2, 3]},
                                                            {entry: [10, 6, 2, 3]},
                                                            {entry: [11, 6, 2, 3]}
                                                          ]})
                    }
                }

                Component.onCompleted: {
                    Pipeline.find("_listViewSelected")
                    .next(function(aInput){
                        console.log(aInput.data())
                    }, "manual", {vtype: "array"})
                    Pipeline.find("pageList_listViewSelected")
                    .next(function(aInput){
                        console.log(aInput.data())
                    }, "manual", {vtype: "array"})
                }
            }

            Menu{
                title: "container"
                MenuItem{
                    text: "TWindow"
                    onClicked: {
                        twin.show()
                    }
                }
                MenuItem{
                    text: "swipe"
                    onClicked: swipe.show()
                }
                MenuItem{
                    text: "gridder"
                    onClicked: gridder.show()
                }
                MenuItem{
                    text: "flip"
                    onClicked: flip.show()
                }
                MenuItem{
                    text: "nest"
                    onClicked:
                        nest.show()
                }
                MenuItem{
                    text: "webwidget"
                    onClicked: Pipeline.run("openWebWindow", 0)
                }
                MenuItem{
                    text: "webqml"
                    onClicked: web.show()
                }
            }

            Menu{
                title: "dialog"

                Menu{
                    title: "file"
                    MenuItem{
                        text: "files"
                        onClicked: {
                            Pipeline.run("_selectFile", {folder: false, filter: ["Image files (*.jpg *.png *.jpeg *.bmp)"]}, "manual")
                        }
                    }
                    MenuItem{
                        text: "directory"
                        onClicked: {
                            Pipeline.run("_selectFile", {folder: true}, "manual2")
                        }
                    }
                    Component.onCompleted: {
                        Pipeline.find("_selectFile")
                        .next(function(aInput){
                            console.log(aInput.data())
                        }, "manual", {vtype: "array"})
                        Pipeline.find("_selectFile")
                        .next(function(aInput){
                            console.log(aInput.data())
                        }, "manual2", {vtype: "array"})
                    }
                }

                MenuItem{
                    text: "color"
                    onClicked:
                        Pipeline.run("_selectColor", {}, "manual2")
                    Component.onCompleted: {
                        Pipeline.find("_selectColor")
                        .next(function(aInput){
                            console.log(aInput.data())
                        }, "manual2", {vtype: "string"})
                    }
                }

                MenuItem{
                    text: "MsgDialog"
                    onClicked:
                        Pipeline.run("popMessage", {title: "hello4", text: "world"}, "manual")
                    Component.onCompleted: {
                        Pipeline.find("messagePoped")
                        .next(function(aInput){
                            console.log(aInput.data()["ok"])
                        }, "manual")
                    }
                }

            }

            Menu{
                title: "diagram"

                MenuItem{
                    text: "linechart"
                    onClicked: {
                        linechart.show()
                    }
                }

                MenuItem{
                    text: "histogram"
                    onClicked: {
                        histogram.show()
                    }
                }

                MenuItem{
                    text: "thistogram"
                    onClicked: {
                        thistogram.show()
                    }
                }
            }

            MenuItem{
                text: "matrix"
                onClicked: {
                    matrix.show()
                }
            }

            MenuItem{
                text: "status"
                onClicked:
                    Pipeline.run("_updateStatus", ["hello", "world"])
            }
            Menu{
                title: "navigation"
                MenuItem{
                    text: "2"
                    onClicked: Pipeline.run("_updateNavigation", ["first layer", "second layer"], "manual")
                }
                MenuItem{
                    text: "3"
                    onClicked: Pipeline.run("_updateNavigation", ["first layer", "second layer", "third layer"], "manual")
                }
                MenuItem{
                    text: "4"
                    onClicked: Pipeline.run("_updateNavigation", ["first layer", "second layer", "third layer", "forth layer"], "manual")
                }
                Component.onCompleted: {
                    Pipeline.find("_updateNavigation")
                    .next(function(aInput){
                        console.log(aInput.data())
                    }, "manual", {vtype: "array"})
                }
            }
            MenuItem{
                text: "search"
                onClicked:
                    Pipeline.run("_Searched", "", "manual")
            }

            MenuItem{
                text: "baseCtrl"
                onClicked: {
                    basectrl.show()
                }
            }
            MenuItem{
                text: "treeNodeView"
                onClicked: {
                    treeview.show()
                }
            }
            MenuItem{
                text: "treeView0"
                onClicked: {
                    treeview0.show()
                }
            }

            MenuItem{
                property string tag: "testProgress"
                property int cnt: 0
                property double hope
                text: "progress"
                onClicked: {
                    if (cnt % 10 == 0){
                        hope = 0.0
                        Pipeline.run("updateProgress", {title: "demo: ", sum: 10}, tag)
                        hope = 0.1
                        Pipeline.run("updateProgress", {}, tag)
                        ++cnt
                    }else if (cnt % 10 == 1){
                        hope = 0.2
                        Pipeline.run("updateProgress", {}, tag)
                        ++cnt
                    }else if (cnt % 10 == 2){
                        hope = 0.9
                        Pipeline.run("updateProgress", {step: 7}, tag)
                        cnt += 7
                    }else if (cnt % 10 == 9){
                        hope = 1.0
                        Pipeline.run("updateProgress", {}, tag)
                        ++cnt
                    }
                }
                Component.onCompleted: {
                    Pipeline.find("updateProgress")
                    .next(function(aInput){
                        console.assert(aInput.data() === hope)
                    }, tag, {vtype: "number"})
                }
            }
        }

        DynamicQML{
            name: "menu"
            onLoaded: function(aItem){
                mainmenu.addMenu(aItem)
            }
        }

        Component.onCompleted: {
            view_cfg = {
                face: 0,
                arrow: {
                    visible: false,
                    pole: true
                },
                text: {
                    visible: false,
                    location: "middle"
                }
            }
        }
    }

    contentData:
        Column{
            anchors.fill: parent
            Row{
                width: parent.width
                height: parent.height - 60
                Column{
                    property int del_size
                    width: parent.width * 0.3 + del_size
                    height: parent.height
                    Search {
                        text: "search"
                        width: parent.width
                        height: 30
                        prefix: "#"
                    }
                    Rectangle{
                        width: parent.width
                        height: (parent.height - 30) / 2
                        color: "white"
                        List{
                            anchors.fill: parent
                        }
                    }
                    Rectangle{
                        width: parent.width
                        height: (parent.height - 30) / 2
                        color: "white"
                        PageList{
                            name: "pageList"
                            anchors.fill: parent
                        }
                    }
                    Component.onCompleted: {
                        Pipeline.find("_Searched")
                        .next(function(aInput){
                            var dt = aInput.data()
                            console.assert(dt === "search")
                            console.log(dt + " is searched")
                        }, "manual", {vtype: "string"})
                    }
                }
                Sizable{

                }
                Column{
                    property int del_size
                    width: parent.width * 0.7 + del_size
                    height: parent.height
                    QSGBoard{
                        id: testbrd
                        name: "testbrd"
                        plugins: [{type: "transform"}]
                        width: parent.width
                        height: parent.height * 0.7
                        Component.onDestruction: {
                            beforeDestroy()
                        }
                    }

                    Log{
                        width: parent.width
                        height: parent.height * 0.3
                    }
                }
            }
            Navigation{
                width: parent.width
                height: 30
            }
            Status{
                width: parent.width
                height: 30
            }
        }

    TWindow{
        id: twin
        caption: "TWindow"
        content: Column{
            anchors.fill: parent
            Rectangle{
                width: parent.width
                height: parent.height
                color: "gray"
            }
        }
        titlebuttons: [{cap: "O", func: function(){console.log("hello")}},
                        {cap: "W", func: function(){console.log("world")}}]
        footbuttons: [{cap: "OK", func: function(){close()}},
                       {cap: "Cancel", func: function(){close()}}]
    }

    TWindow{
        id: basectrl
        caption: "baseCtrl"
        content: Rectangle{
            anchors.fill: parent
            color: "gray"
            Column{
                /*Repeater{
                    delegate: T{

                    }
                }*/
                id: clm
                leftPadding: 5
                Edit{
                    width: 180
                    caption.text: "attr1" + ":"
                    ratio: 0.4
                }
                Check{
                    width: 180
                    caption.text: "attribu2" + ":"
                    ratio: 0.4
                }
                Spin{
                    width: 180
                    caption.text: "attribute555" + ":"
                    ratio: 0.4
                }
                Combo{
                    width: 180
                    caption.text: "attribute3" + ":"
                    ratio: 0.4
                }
                ComboE{
                    width: 180
                    caption.text: "attributeeee6" + ":"
                    ratio: 0.4
                    combo.modellist: ['test1', 'test2', 'test3', 'test4']
                    combo.currentSelect: "test2"
                }
                Track{
                    width: 180
                    caption.text: "attri4" + ":"
                    ratio: 0.4
                    onIndexChanged: function(aIndex){
                        console.log("track index: " + aIndex)
                    }
                }
                Radio{
                    text: "attribute4"
                }
                AutoSize{

                }
            }
        }
        footbuttons: [{cap: "OK", func: function(){close()}}]
    }

    TWindow{
        id: gridder
        caption: "gridder"
        content: Gridder{
            id: gridder_cld

            name: "demo"
            size: [2, 2]
            com: Component{
                Rectangle{
                    property string name
                    width: parent.width / parent.columns
                    height: parent.height / parent.rows

                    color: "transparent"
                    border.color: "red"
                    Component.onCompleted: {
                        //console.log(name)
                    }
                }
            }

            padding: parent.width * 0.05
            width: parent.width * 0.9
            height: parent.height * 0.9
        }
        footbuttons: [
            {
                cap: "6",
                func: function(){
                    Pipeline.run(gridder_cld.name + "_updateViewCount", {size: 6})
                }
            },
            {
                cap: "10",
                func: function(){
                    gridder_cld.updateViewCount(10)
                }
            },
            {
                cap: "1",
                func: function(){
                    Pipeline.run(gridder_cld.name + "_updateViewCount", {size: 1})
                }
            },
            {
                cap: "5x5",
                func: function(){
                    Pipeline.run(gridder_cld.name + "_updateViewCount", {size: [5, 5]})
                }
            }

        ]
    }

    TWindow{
        id: swipe
        caption: "swipeview"
        content: TabView{
            anchors.fill: parent
            Tab{
                title: "Red"
                Rectangle{
                    color: "red"
                }
            }
            Tab{
                title: "Blue"
                Rectangle{
                    color: "blue"
                }
            }
            Tab{
                title: "Swipe"
                Swipe{

                }
            }
        }
    }

    TWindow{
        id: flip
        caption: "flipView"
        content: Flip{
            id: flipview
            anchors.fill: parent
            front: Rectangle{
                anchors.fill: parent
                color: "red"
                MouseArea{
                    anchors.fill: parent
                    onClicked: flipview.flipUp()
                }
            }
            back: Rectangle{
                anchors.fill: parent
                color: "blue"
                MouseArea{
                    anchors.fill: parent
                    onClicked: flipview.flipDown()
                }
            }
        }
    }

    TWindow{
        id: nest
        caption: "nestView"
        content: Nest{
            rows: 10
            columns: 10
            anchors.fill: parent
            size: [1, 2, 7, 8, 8, 2, 2, 8, 1, 10]
            Rectangle {
                color: "red"
            }
            Rectangle {
                color: "green"
            }
            Rectangle {
                color: "blue"
            }
            Rectangle {
                color: "yellow"
            }
            Rectangle {
                color: "black"
            }
        }
    }

    TWindow{
        id: matrix
        caption: "matrix"
        content: Matrix{
            id: mtx
            rowcap.text: "hello"
            colcap.text: "world"
            anchors.fill: parent
            content: [[1, 2], [3, 4], [5, 6]]
        }
        footbuttons: [
            {
                cap: "3x3",
                func: function(){
                    mtx.content = [[1, 2, 3], [4, 5, 6], [7, "8", 9]]
                    mtx.updateGUI(true)
                }
            },
            {
                cap: "2x2",
                func: function(){
                    mtx.content = [[1, 2], [3, 4]]
                    mtx.updateGUI(true)
                }
            },
            {
                cap: "5x4",
                func: function(){
                    Pipeline.run("_updateMatrix", {rowcap: "hello2",
                                                    colcap: "world2",
                                                    content: [[1, 2, 3, 4],
                                                              [5, 6, 7, 8],
                                                              [9, 10, 11, 12],
                                                              [13, 14, 15, 16],
                                                              [17, 18, 19, 20]]})
                }
            }

        ]
        Component.onCompleted: {
            Pipeline.find("_matrixSelected")
            .next(function(aInput){
                console.log("_matrixSelected: " + aInput.data())
            }, "manual", {vtype: "number"})
        }
    }

    TWindow{
        id: treeview
        property var sample: {
            "hello": "world",
            "he": [true, false],
            "hi": {
                "hi1": "hi2",
                "ge": {
                    "too": 3,
                    "heww": {
                        "ll": [3, 3, 4],
                        "dd": "dddd",
                        "ff": false
                    }
                }
            },
            "hi20": true,
            "hello2": "world",
            "he2": [0, {"kao": "gege"}, 1],
            "hi2": [
                {"hello": "world"},
                {"no": [true, false]}
            ],
            "hi22": 3
        }

        caption: "treeview"
        content: Rectangle{
            color: "gray"
            anchors.fill: parent
            TreeNodeView{
                anchors.fill: parent
            }
        }
        footbuttons: [
            {cap: "modify", func: function(){
                Pipeline.run("modifyTreeViewGUI", {key: ["hi2", "1", "no", "1"], val: true}, "modifyTreeView")}},
            {cap: "add", func: function(){
                Pipeline.run("modifyTreeViewGUI", {key: ["hi2", "1", "no", "2"], type: "add", val: 14}, "modifyTreeView")}},
            {cap: "delete", func: function(){
                Pipeline.run("modifyTreeViewGUI", {key: ["hi"], type: "del"}, "modifyTreeView")}},
            {cap: "load", func: function(){
                Pipeline.run("loadTreeView", {data: sample}, "testTreeView")}},
            {cap: "save", func: function(){
                Pipeline.run("saveTreeView", {}, "testTreeView")}},
            {cap: "style", func: function(){
                Pipeline.run("saveTreeView", {}, "styleTreeView")}}
            ]

        function sameObject(aTarget, aRef){
            for (var i in aTarget)
                if (typeof aRef[i] === "object"){
                    if (!sameObject(aTarget[i], aRef[i])){
                        //console.log(i + ";" + aTarget[i] + ";" + aRef[i])
                        return false
                    }
                }else if (aTarget[i] !== aRef[i]){
                    //console.log(i + ";" + aTarget[i] + ";" + aRef[i])
                    return false
                }
            return true
        }

        Component.onCompleted: {
            Pipeline.find("loadTreeView")
            .nextL("saveTreeView", "testTreeView")
            .next(function(aInput){
                console.assert(sameObject(aInput.data(), sample))
                aInput.outs("Pass: save/load TreeView", "testSuccess")
            })
            .next("testSuccess")

            Pipeline.find("treeViewGUIModified")
            .next(function(aInput){
                var dt = aInput.data()
                console.log("treeViewGUIModified;" + dt["key"] + ";" + dt["val"] + ";" + dt["type"])
            })

            Pipeline.find("saveTreeView")
            .nextL("styleTreeView", "styleTreeView")
            .nextL("loadTreeView")
            .nextL("saveTreeView")
            .next(function(aInput){
                aInput.setData({data: sample, style: aInput.data()}).out()
            })
            .nextL("loadTreeView")
            .next(function(aInput){
                aInput.outs({data: aInput.data()["style"], path: "style.json"})
            })
            .nextL("json2stg")
            .nextL("writeJson")
        }
    }

    TWindow{
        id: linechart
        caption: "lineChart"
        content: LineChart{
            anchors.fill: parent
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline.run("_updateLineChart", [20, 30, 100, 125, 30, 10, 12, 30, 50])
                }
            }
        ]
    }

    TWindow{
        id: histogram
        caption: "histogram"
        content: Histogram{
            anchors.fill: parent
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline.run("_updateHistogramGUI", {histogram: [40, 20, 15, 25, 14, 16, 13, 30]})
                }
            }

        ]
    }

    TWindow{
        id: web
        caption: "webview"
        content: WebEngineView {
            anchors.fill: parent
            id: webview
            //url: "file:/html/test.html"
            //url: "https://www.baidu.com"
            z: - 1
            Row{
                anchors.fill: parent
                spacing: width * 0.2
                z: 1
                Rectangle{
                    width: parent.width * 0.2
                    height: parent.height
                    color: "red"
                    z: 1
                }
                Rectangle{
                    width: parent.width * 0.2
                    height: parent.height
                    color: "blue"
                }
            }
            webChannel: WebChannel{
                id: webview_chn
               /* Component.onCompleted: {
                    var stm = Pipeline2.asyncCall("pipelineJSObject", 0)
                    webview_chn.registerObject("Pipeline", stm.data())
                    //webview_chn.registerObject("Pipeline", stm.scope().data("pipeline"))
                    webview.url = "file:/html/test.html"
                }*/
            }
        }
    }

    TWindow{
        id: treeview0
        content: TreeView0{
            anchors.fill: parent
            imagePath: {
                "image": '../../resource/image.png',
                "text": '../../resource/text.png',
                "shape": '../../resource/shape.png'
            }
            selectWay: 'background'
            selectColor: 'blue'
            openWay: 'all'
            onSetCurrentSelect: function (select) {
                console.log("treeview0 selected: " + select)
            }
            Component.onCompleted: {
                var tmp = [{
                               "type": "folder",
                               "name": "folder0",
                               "id": "folder0",
                               "children": [{
                                       "type": "page",
                                       "name": "page0",
                                       "id": "page0",
                                       "children": [{
                                               "id": "id0",
                                               "type": "image",
                                               "name": "image0",
                                               "position": [],
                                               "comment": "",
                                               "source": "" //url
                                           }, {
                                               "id": "id1",
                                               "type": "text",
                                               "name": "text0",
                                               "position": [],
                                               "comment": "",
                                               "relative_position": [],
                                               "content": "",
                                               "size": 16,
                                               "color": "green",
                                               "bold": ""
                                           }, {
                                               "id": "id2",
                                               "type": "shape",
                                               "name": "text0",
                                               "position": [],
                                               "comment": "",
                                               "relative_position": [],
                                               "direction": {
                                                   "color": "green",
                                                   "border": {
                                                       "type": "line",
                                                       "color": "red"
                                                   },
                                                   "radius": 30
                                               }
                                           }]
                                   }, {
                                       "type": "folder",
                                       "name": "folder0",
                                       "id": "folder1",
                                       "children": [{
                                               "id": "id3",
                                               "type": "text",
                                               "name": "text0_0",
                                               "range": [0, 0, 50, 50],
                                               "content": ""
                                           }]
                                   }]
                           }]
                buildDefaultTree(tmp, 'folder1')
            }
        }
    }

    TWindow{
        id: thistogram
        caption: "thistogram"
        content: Column{
            anchors.fill: parent
            spacing: 5
            THistogram{
                id: histo
                width: parent.width
                height: parent.height * 0.9
                //oneThreshold: true
            }
            Track{
                property var intervals: []
                property var histogramdata: []
                property double value: 0.1
                property int idx: 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.05
                width: parent.width * 0.8
                interval: 100
                caption.text: "Interval" + ":"
                ratio: 0.2

                function updateInterval(){
                    if (intervals.length > 1 && histogramdata.length > 1){
                        histo.drawHistoGram(histogramdata[idx])
                    }
                }

                onIndexChanged: function(aIndex){
                    idx = aIndex
                    updateInterval()
                }
                Component.onCompleted: {
                    Pipeline.add(function(aInput){
                        var dt = aInput.data()
                        intervals = []
                        histogramdata = []
                        interval = Object.keys(dt["histogram"]).length - 1
                        for (var i in dt["histogram"]){
                            intervals.push(i)
                            histogramdata.push(dt["histogram"][i])
                        }
                        updateInterval()
                        return {out: {}}
                    }, {name: "_updateTHistogramGUI"})
                }
            }

            Track{
                property double value: 0
                anchors.horizontalCenter: parent.horizontalCenter
                height: parent.height * 0.05
                width: parent.width * 0.8
                interval: 100
                caption.text: "IOU" + ":"
                ratio: 0.2

                onIndexChanged: function(aIndex){
                    value = aIndex / 100
                    //thresholdChanged(maxthreshold.x, minthreshold.x)
                }
            }
        }
        footbuttons: [
            {
                cap: "test",
                func: function(){
                    Pipeline.run("_updateTHistogramGUI", {histogram: {
                                      5: [10, 20, 15, 30, 25],
                                      10: [10, 20, 15, 30, 25, 20, 21, 23, 42, 12],
                                      20: [10, 20, 15, 30, 25, 20, 21, 23, 42, 12, 12, 10, 20, 42, 30, 15, 25, 20, 21, 23]
                                  }})
                }
            }

        ]
    }

    Progress{

    }
    MsgDialog{

    }
    File{

    }
    ColorSelect{

    }

}
