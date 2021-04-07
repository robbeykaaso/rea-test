// 向qt发送消息
function sendMessage(msg)
{
    if(typeof context == 'undefined')
    {
        alert("context对象获取失败！");
    }
    else
    {
        context.onMsg(msg);
    }
}

function recvMessage(msg)
{
    alert("接收到Qt发送的消息：" + msg);
}

function test1(){
    //test1 -> test1_next -> testSuccessJS
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 3)
        aInput.out()
    }, {name: "test1"})
    .nextF(function(aInput){  //test1_next
        console.assert(aInput.data() == 3)
        aInput.outs("Pass: test1", "testSuccessJS")
    })
    .next("testSuccess")

    pipelines().run("test1", 3)
}

function test2(){
    //test2 -> test2_ -> testSuccessJS
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 4)
        aInput.outs(5, "test2_")
    }, {name: "test2"})
    .nextF(function(aInput){
        console.assert(aInput.data() == 5)
        aInput.outs("Pass: test2", "testSuccessJS")
    }, "", {name: "test2_"})

    pipelines().run("test2", 4)
}

function test3(){
    //test3 -> test3_0 -> test3__ -> testSuccessJS
    //                 -> testSuccessJS
    //test3_1 -> test3__
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 66)
        aInput.outs("test3", "test3_0")
    }, {name: "test3"})
    .next("test3_0")
    .next("testSuccessJS")

    pipelines().add(function(aInput){
        aInput.out()
    }, {name: "test3_1"})
    .next("test3__")
    .next("testSuccessJS")

    pipelines().find("test3_0")
    .nextF(function(aInput){
        aInput.out()
    }, "", {name: "test3__"})
    .next("testSuccessJS")

    pipelines().add(function(aInput){
        console.assert(aInput.data() == "test3")
        aInput.outs("Pass: test3", "testSuccessJS")
        aInput.outs("Pass: test3_", "test3__")
    }, {name: "test3_0"})

    pipelines().run("test3", 66)
    pipelines().run("test3_1", "Pass: test3__")
}

function test4(){
    //test4 -> test4_(ex) -> test4__(ex) -> test_4 -> testSuccessJS
    pipelines().find("test4__").removeNext("test_4")

    pipelines().add(function(aInput){
        aInput.scope().cache("hello", "world")
        aInput.out()
    }, {name: "test4"})
    .next("test4_")
    .next("test4__")
    .nextF(function(aInput){  //test4__next
        console.assert(aInput.data() == 6)
        console.assert(aInput.scope().data("hello") == null)
        console.assert(aInput.scope().data("hello2") == "world")
        aInput.outs("Pass: test4", "testSuccessJS")
    }, "", {name: "test_4"})

    pipelines().run("test4", 4)
}

function test5(){
    //test5(ex) -> test_5 -> testSuccessJS
    pipelines().find("test5").removeNext("test_5")

    pipelines().find("test5")
    .nextF(function(aInput){
        console.assert(aInput.data() == "world")
        aInput.outs("Pass: test5", "testSuccessJS")
    }, "", {name: "test_5"})

    pipelines().run("test5", "hello")
}

function test6(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 4)
        console.assert(aInput.scope().data("hello") == "world")
        aInput.setData(aInput.data() + 1).out()
    }, {name: "test6_", external: true})

    pipelines().add(function(aInput){
        console.assert(aInput.data() == 5)
        aInput.scope(true).cache("hello2", "world");
        aInput.setData(aInput.data() + 1).out()
    }, {name: "test6__", external: true})

    return "test6"
}

function test7(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == "hello")
        aInput.setData("world").out()
    }, {name: "test7", external: true})

    return "test7"
}

function test8(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == "hello")
        aInput.outs("Pass: test8", "testSuccessJS")
    }, {name: "test8", external: true})

    return "test8"
}

function test9(){
    pipelines().run("test9", "hello")
}

function test11(){
    return "test11"
}

function test12(){
    return "test12"
}

function test13(){
    return "test13"
}

function test14(){
    return "test14"
}

function test15(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 66)
        aInput.setData(77).out()
    }, {name: "test15", type: "Partial"})
    .nextFB(function(aInput){
        console.assert(aInput.data() == 77)
        aInput.outs("Pass: test15", "testSuccessJS")
    }, "test15")
    .nextF(function(aInput){
        console.assert(aInput.data() == 77)
        aInput.outs("Fail: test15", "testFailJS")
    }, "test15_")

    pipelines().run("test15", 66, "test15")
}

function test16(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 66)
        aInput.setData(77).out()
    }, {name: "test16", external: true, type: "Partial"})
    return "test16"
}

function test17(){
    pipelines().find("test17").removeNext("test17_")
    pipelines().find("test17").removeNext("test17__")

    pipelines().find("test17")
    .nextFB(function(aInput){
        console.assert(aInput.data() == 77.0)
        aInput.outs("Pass: test17", "testSuccessJS")
    }, "test17", {name: "test17_"})
    .nextF(function(aInput){
        console.assert(aInput.data() == 77.0)
        aInput.outs("Fail: test17", "testFailJS")
    }, "test17_", {name: "test17__"})

    pipelines().run("test17", 66, "test17")
}

function test18(){
    return "test18"
}

function test19(){
    pipelines().remove("test19")

    pipelines().add(function(aInput){
        console.assert(aInput.data() == 66.0)
        aInput.out()
    }, {name: "test19_0",
        delegate: "test19",
        type: "Delegate"})
    .next("testSuccessJS")

    pipelines().add(function(aInput){
        console.assert(aInput.data() == 56.0)
        aInput.setData("Pass: test19").out()
    }, {name: "test19"})

    pipelines().run("test19_0", 66.0)
    pipelines().run("test19", 56.0)
}

function test20(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 56.0)
        aInput.setData("Pass: test20").out()
    }, {name: "test20", external: true})

    return "test20"
}

function test20_(){

}

function test21(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 66.0)
        aInput.out()
    }, {name: "test21_0",
        delegate: "test21",
        type: "Delegate"})
    .nextB("testSuccess")
    .next("testSuccessJS")

    pipelines().run("test21_0", 66)
    pipelines().run("test21", 56)
}

function test21_(){
    return "test21"
}

function test22(){
    return "test22"
}

async function test23(){
    await pipelines().input(0, "test23").asyncCallS(
        [function(aInput){
            aInput.setData(aInput.data() + 1).out()
        }],
        [function(aInput){
            console.assert(aInput.data() == 1)
            aInput.outs("world")
        }],
        [function(aInput){
            console.assert(aInput.data() == "world")
            aInput.setData("Pass: test23").out()
        }],
        "testSuccessJS"
    )
}

async function test24(){
    await pipelines().input(24, "test24").asyncCallS("test24", "testSuccessJS")
}

function test25(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 25.0)
        aInput.setData("Pass: test25").out()
    }, {name: "test25", external: true})

    return "test25"
}

function test26(){
    return "test26"
}

function test27(){
    return "test27"
}

function test28(){
    return "test28"
}

function test29(){
    pipelines().add(function(aInput){
        context = aInput.data()
        console.assert(aInput.scope().data("ctx") == context)
        sendMessage("lala")
    }, {name: "test29", external: true})

    return "test29"
}

// 控件控制函数
function onBtnSendMsg()
{
    pipelines().run("unitTest")
    //var cmd = document.getElementById("待发送消息").value;
}

function unitTest(){
    let test_sum = 0
    let test_pass = 0

    pipelines().add(function(aInput){
        test_pass++
        console.log("Success: " + aInput.data() + "(" + test_pass + "/" + test_sum + ")")
    }, {name: "testSuccessJS"})

    pipelines().add(function(aInput){
        test_pass--
        console.log("Fail: " + aInput.data() + "(" + test_pass + "/" + test_sum + ")")
    }, {name: "testFailJS"})

    pipelines().add(function(aInput){
        let test = [
            [test1, 1], //test js anonymous next
            [test2, 1], //test js specific next
            [test3, 3], //test js pipe future
            [test4, 1], //test pipe mixture: js->js.future(c++)->js.future(c++)->js ; scopeCache
            [test5, 1], //test pipe mixture: js.future(c++)->js
            [test9, 1], //test pipe mixture: js.future(c++)
            [test15, 1], //test js pipe partial
            [test17, 1], //test pipe mixture partial: js.future(c++)->js
            [test19, 1], //test js pipe delegate and pipe param
            [test20_, 1], //test pipe mixture delegate: c++->c++.future(js)->js, c++
            [test21, 1],  //test pipe mixture delegate: js->js.future(c++)->c++, js
            [test23, 1], //test js asyncCall
            [test24, 1] //test pipe mixture: js.asyncCall.c++
        ]
        for (let i in test)
            test_sum += test[i][1]
        for (let i in test)
            test[i][0]()

        aInput.setData({
            [test6()]: 1, //test pipe mixture: c++->c++.future(js)->c++.future(js)->c++ ; scopeCache
            [test7()]: 1, //test pipe mixture: c++.future(js)->c++
            [test8()]: 1, //test pipe mixture: c++.future(js)
            [test11()]: 1, //test c++ anonymous next
            [test12()]: 1, //test c++ specific next
            [test13()]: 3, //test c++ pipe future
            [test14()]: 1, //test c++ pipe future
            [test16()]: 1, //test pipe mixture partial: c++.future(js)->c++
            [test18()]: 1, //test c++ pipe delegate and pipe param
            [test20()]: 1, //test pipe mixture delegate: c++->c++.future(js)->js, c++
            [test21_()]: 1,  //test pipe mixture delegate: js->js.future(c++)->c++, js
            [test22()]: 1, //test c++ asyncCall
            [test25()]: 1, //test pipe mixture: c++.asyncCall.js
            [test26()]: 1, //test c++ aop and keep topo
            [test27()]: 1, //test c++ functor
            [test28()]: 1, //test pipe qml
            [test29()]: 1  //test rea-js arbitrary type
                    }).out()
    }, {name: "unitTest"})
    .next("unitTestC++")
}
