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
// 控件控制函数
function onBtnSendMsg()
{
    pipelines().run("unitTest")
    var cmd = document.getElementById("待发送消息").value;
    sendMessage(cmd);
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
        aInput.out()
    }, {name: "test4"})
    .next("test4_")
    .next("test4__")
    .nextF(function(aInput){  //test4__next
        console.assert(aInput.data() == 6)
        aInput.outs("Pass: test4", "testSuccessJS")
    }, "", {name: "test_4", external: true, vtype: "number"})

    pipelines().run("test4", 4)
}

function test5(){
    //test5(ex) -> test_5 -> testSuccessJS
    pipelines().find("test5").removeNext("test_5")

    pipelines().find("test5")
    .nextF(function(aInput){
        console.assert(aInput.data() == "world")
        aInput.outs("Pass: test5", "testSuccessJS")
    }, "", {name: "test_5", external: true, vtype: "string"})

    pipelines().run("test5", "hello")
}

function test6(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 4)
        aInput.setData(aInput.data() + 1).out()
    }, {name: "test6_", external: true, vtype: "number"})

    pipelines().add(function(aInput){
        console.assert(aInput.data() == 5)
        aInput.setData(aInput.data() + 1).out()
    }, {name: "test6__", external: true, vtype: "number"})
}

function test7(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == "hello")
        aInput.setData("world").out()
    }, {name: "test7", external: true, vtype: "string"})
}

function test8(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == "hello")
        aInput.outs("Pass: test8", "testSuccessJS")
    }, {name: "test8", external: true, vtype: "string"})
}

function test9(){
    pipelines().run("test9", "hello")
}

function test11(){

}

function test12(){

}

function test13(){

}

function unitTest(){
    pipelines().add(function(aInput){
        console.log("Success: " + aInput.data())
    }, {name: "testSuccessJS"})

    pipelines().add(function(aInput){
        test1() //test js anonymous next
        test2() //test js specific next
        test3() //test js pipe future
        test4() //test pipe mixture: js->js.future(c++)->js.future(c++)->js(external)->js
        test5() //test pipe mixture: js.future(c++)->js(external)->js
        test6() //test pipe mixture: c++->c++.future(js)->c++.future(js)->c++(external)->c++
        test7() //test pipe mixture: c++.future(js)->c++(external)->c++
        test8() //test pipe mixture: c++.future(js)->js
        test9() //test pipe mixture: js.future(c++)->c++
        test11() //test c++ anonymous next
        test12() //test c++ specific next
        test13() //test c++ pipe future
    }, {name: "unitTest"})
}

unitTest();
