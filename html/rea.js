//#region utils

var scripts = [];
var modules = {};

function getModuleExport(aName, aObj){
    return modules[aName] && modules[aName][aObj];
}

function setModuleExport(aName, aObj, aTarget){
    if (!modules[aName])
        modules[aName] = {};
    if (modules[aName][aObj])
        alert('error: ' + aObj + ' is existed! Please rename');
    else
        modules[aName][aObj] = aTarget;
}

function extend(child, parent){
    var Super = function(){};
    Super.prototype = parent.prototype;
    child.prototype = new Super();
//    child.prototype.constructor = child;
}

//copy from https://blog.csdn.net/zhaiwenyuan/article/details/72781609
function getURLParam(url) { //获取url中"?"符后的字串
   var theRequest = new Object();
   if (url.indexOf("?") != -1) {
       var str = url.substr(1);
       strs = str.split("&");
       for (var i = 0; i < strs.length; i++) {
           theRequest[strs[i].split("=")[0]] = decodeURIComponent(strs[i].split("=")[1]);
       }
   }
   return theRequest;
}

function windowWidth(){
    return window.innerWidth
        || document.documentElement.clientWidth
        || document.body.clientWidth;
}

function windowHeight(){
    return window.innerHeight
        || document.documentElement.clientHeight
        || document.body.clientHeight;
}

function dynamicLoadCss(url, aCallback) {  //copy from: https://www.cnblogs.com/morang/p/dynamicloadjscssiframe.html
    //console.log(url);
    let args = Array.prototype.slice.apply(arguments);
    args.splice(0, 2);
    let head = document.getElementsByTagName('head')[0];
    let link = document.createElement('link');
    link.type='text/css';
    link.rel = 'stylesheet';
    link.onload = function(){
        if (aCallback)
            aCallback(...args);
    };

    link.href = url;
    head.appendChild(link);
    return link;
}

function installScript(path, aCallback){  //ref from: https://www.cnblogs.com/croso/p/5294251.html
    let args = Array.prototype.slice.apply(arguments);
    args.splice(0, 2);

    if (scripts.indexOf(path) >= 0){
        scripts.push(path);
        if (aCallback)
            aCallback(...args);
        return;
    }

    let head= document.getElementsByTagName('head')[0];
    let script= document.createElement('script');
    script.type= 'text/javascript';
    script.onload = function() {
// if (!this.readyState || this.readyState === "loaded" || this.readyState === "complete" ) {

    // Handle memory leak in IE
       // script.onload = script.onreadystatechange = null;
        if (aCallback)
            aCallback(...args);
    };

    script.src= path;
    head.appendChild(script);
    return script;
}

//https://blog.csdn.net/qq_39425958/article/details/87642137
function generateUUID() {
    var s = [];
    var hexDigits = "0123456789abcdef";
    for (var i = 0; i < 36; i++) {
      s[i] = hexDigits.substr(Math.floor(Math.random() * 0x10), 1);
    }
    s[14] = "4"; // bits 12-15 of the time_hi_and_version field to 0010
    s[19] = hexDigits.substr((s[19] & 0x3) | 0x8, 1); // bits 6-7 of the clock_seq_hi_and_reserved to 01
    s[8] = s[13] = s[18] = s[23] = "-";
   
    var uuid = s.join("");
    return uuid
}

//#endregion

//#region linker;rea

var PipelineJS
var inited = false
var candidates_functions = []
function initPipelineJS(){
    return new Promise(function(resolve, reject){
        if (typeof qt != "undefined"){
            new QWebChannel(qt.webChannelTransport, function(channel){
                PipelineJS = channel.objects.Pipeline;
                if (!PipelineJS)
                    alert("no PipelineJS")
                PipelineJS.executeJSPipe.connect(function(aName, aData, aTag, aScope, aSync, aFromOutside){
                    const len = Object.keys(aScope).length
                    let sp = {}
                    for (let i = 0; i < len; i += 2)
                        sp[aScope[i]] = aScope[i + 1]
                    pipelines().execute(aName, new stream(aData, aTag, new scopeCache(sp)), aSync, aFromOutside)
                })
                PipelineJS.removeJSPipe.connect(function(aName){
                    pipelines().remove(aName)
                })
                inited = true

                for (let i in candidates_functions)
                    candidates_functions[i]()
                candidates_functions = []

                resolve()
            })
        }else{
            inited = true
            alert("qt object not exists!")
            reject()
        }
    })
}

function rea(aFunc){
    if (!inited){
        if (!candidates_functions.length)
            initPipelineJS()
        candidates_functions.push(aFunc)
    }else{
        aFunc()
    }
}

//#endregion

//#region rea-js

class scopeCache {
    constructor(aData = {}){
        this.m_data = aData
    }
    cache(aName, aData){
        this.m_data[aName] = aData
        return this
    }
    data(aName){
        return this.m_data[aName]
    }
}

class stream {
    constructor(aInput, aTag = "", aScope = null){
        this.m_data = aInput
        this.m_tag = aTag
        this.m_scope = aScope
    }
    setData(aData){
        this.m_data = aData
        return this
    }
    scope(aNew = false){
        if (!this.m_scope || aNew)
            this.m_scope = new scopeCache()
        return this.m_scope
    }
    data(){
        return this.m_data
    }
    tag(){
        return this.m_tag
    }
    out(aTag = ""){
        if (!this.m_outs)
            this.m_outs = []
        if (aTag != "")
            this.m_tag = aTag
        return this
    }
    outs(aOut, aNext = "", aTag = ""){
        if (!this.m_outs)
            this.m_outs = []
        this.m_outs.push([aNext, new stream(aOut, aTag == "" ? this.m_tag : aTag, this.m_scope)])
    }

    async asyncCallS(){
        let stm = this
        for (const i in arguments)
            if (typeof arguments[i] == "string")
                stm = await stm.asyncCall(arguments[i])
            else
                stm = await stm.asyncCallF(arguments[i][0], arguments[i][1])
    }

    async asyncCall(aName, aPipeline = pipelines()){
        let ret
        let got_ret = false

        const monitor = aPipeline.find(aName).nextF(aInput => {
            ret = new stream(aInput.data(), this.m_tag)
            got_ret = true

        }, this.m_tag)
        aPipeline.execute(aName, this)

        function sleep(time) {
          return new Promise(resolve => setTimeout(resolve,time))
        }
        while(!got_ret)
            await sleep(5)
        aPipeline.find(aName).removeNext(monitor.actName())
        aPipeline.remove(monitor.actName(), true)

        return ret
    }
    async asyncCallF(aFunc, aParam = {}, aPipeline = pipelines()){
        const pip = aPipeline.add(aFunc, aParam)
        const ret = await this.asyncCall(pip.actName())
        aPipeline.remove(pip.actName(), true)
        return ret
    }
}

class pipe {

    constructor(aParent, aName){
        this.m_parent = aParent
        this.m_external = aParent.name()
        this.m_next = {}
        if (aName == null || aName == "")
            this.m_name = generateUUID()
        else
            this.m_name = aName
        this.m_parent.m_pipes[this.m_name] = this
    }
    
    resetTopo(){
        this.m_next = {}
    }

    actName() {
        return this.m_name
    }

    next(aName, aTag = ""){
        const tags = aTag.split(";")
        for (let i in tags)
            this.insertNext(aName, tags[i])
        return this.m_parent.find(aName)
    }

    nextB(aName, aTag = ""){
        this.next(aName, aTag)
        return this
    }

    nextF(aFunc, aTag = "", aParam = {}){
        return this.next(this.m_parent.add(aFunc, aParam).actName(), aTag)
    }

    nextFB(aFunc, aTag = "", aParam = {}){
        this.nextF(aFunc, aTag, aParam)
        return this
    }

    removeNext(aName){
        delete this.m_next[aName]
    }

    initialize(aFunc, aParam){
        this.m_func = aFunc
        if (aParam["external"] != null)
            this.m_external = aParam["external"]
        return this
    }

    execute(aStream){
        this.doEvent(aStream)
        this.doNextEvent(this.m_next, aStream)
    }

    insertNext(aName, aTag){
        this.m_next[aName] = aTag
    }

    doEvent(aStream){
        this.m_func(aStream)
    }

    tryExecutePipe(aName, aStream){
        const pip = this.m_parent.find(aName)
        if (pip)
            if (pip.m_external != this.m_parent.name())
                PipelineJS.tryExecuteOutsidePipe(aName, aStream.data(), aStream.tag(), aStream.scope().m_data, {}, pip.m_external)
            else
                pip.execute(aStream)
    }

    doNextEvent(aNexts, aStream){
        const outs = aStream.m_outs
        aStream.m_outs = null
        if (outs)
            if (outs.length){
                for (let i in outs){
                    if (outs[i][0] == "")
                        for (let j in aNexts)
                            this.tryExecutePipe(j, outs[i][1])
                    else
                        this.tryExecutePipe(outs[i][0], outs[i][1])
                }
            }else
                for (let i in aNexts)
                    this.tryExecutePipe(i, aStream)
    }
}

class pipeFuture0 extends pipe{
    
    constructor(aParent, aName){
        super(aParent, aName)
        this.m_next2 = []
    }

    insertNext(aName, aTag){
        this.m_next2.push([aName, aTag])
    }
}

class pipeFuture extends pipe{

    constructor(aParent, aName){
        super(aParent)
        this.m_act_name = aName
        this.m_next2 = []
        if (this.m_parent.find(aName + "_pipe_add", false)){
            const pip = new pipeFuture0(this.m_parent, aName)
            this.m_parent.call(aName + "_pipe_add")
            for (let i in pip.m_next2)
                this.insertNext(pip.m_next2[i][0], pip.m_next2[i][1])
            this.m_external = pip.m_external
            this.m_parent.remove(aName)
        }

        this.m_parent.add(e => {
            const pip = this.m_parent.find(aName, false)
            for (let i in this.m_next2)
                pip.insertNext(this.m_next2[i][0], this.m_next2[i][1])
            pip.m_external = this.m_external
            this.m_parent.remove(this.m_name)
        }, {name: aName + "_pipe_add"})
    }

    resetTopo(){
        this.m_next2 = []
    }

    actName(){
        return this.m_act_name
    }

    removeNext(aName){
        for (let i = this.m_next2.length - 1; i >= 0; --i){
            if (this.m_next2[i][0] == aName)
                delete this.m_next2[i]
        }
    }

    insertNext(aName, aTag){
        this.m_next2.push([aName, aTag])
    }

    execute(aStream){
        let sync = {}
        if (this.m_next2.length)
            sync["next"] = this.m_next2
        PipelineJS.tryExecuteOutsidePipe(this.actName(), aStream.data(), aStream.tag(), aStream.scope().m_data, sync, "any")
    }
}

var pipeline_s = {}
function pipelines(aName = "js"){
    if (!pipeline_s[aName])
        pipeline_s[aName] = new pipeline(aName)
    return pipeline_s[aName]
}

class pipeline{

    constructor(aName){
        this.m_name = aName
        this.m_pipes = {}
    }

    name(){
        return this.m_name
    }

    add(aFunc, aParam){
        const nm = aParam["name"]
        let pip
        if (aParam["type"] == "Partial")
            pip = new pipePartial(this, nm)
        else if (aParam["type"] == "Delegate")
            pip = new pipeDelegate(this, nm)
        else
            pip = new pipe(this, nm)
        if (nm != ""){
            const ad = pip.actName() + "_pipe_add"
            this.call(ad)
            this.remove(ad)
        }
        pip.initialize(aFunc, aParam)
        return pip
    }

    remove(aName, aOutside = false){
        const pip = this.m_pipes[aName]
        if (pip)
            delete this.m_pipes[aName]
        if (aOutside)
            PipelineJS.removePipeOutside(aName)
    }

    find(aName, aNeedFuture = true){
        let pip = this.m_pipes[aName]
        if (!pip && aNeedFuture)
            pip = new pipeFuture(this, aName)
        return pip
    }

    run(aName, aInput, aTag = "", aScope = null){
        this.execute(aName, new stream(aInput, aTag, aScope))
    }

    call(aName, aInput){
        const pip = this.m_pipes[aName]
        if (pip)
            pip.doEvent(new stream(aInput))
    }

    execute(aName, aStream, aSync, aFromOutside = false){
        let pip = this.find(aName, !aFromOutside)
        if (!pip)
            return
        pip = this.find(aName)
        if (aSync){
            if (Object.keys(aSync).length > 0)
                pip.resetTopo()
            if (aSync["next"])
                for (let i in aSync["next"])
                    pip.insertNext(aSync["next"][i][0], aSync["next"][i][1])
        }
        pip.execute(aStream)
    }

    input(aInput, aTag = "", aScope = null){
        const tag = aTag == "" ? generateUUID() : aTag
        return new stream(aInput, tag, aScope)
    }
}

class pipePartial extends pipe{
    constructor(aParent, aName){
        super(aParent, aName)
        this.m_next2 = {}
    }

    insertNext(aName, aTag){
        if (!this.m_next2[aTag])
            this.m_next2[aTag] = {}
        this.m_next2[aTag][aName] = aTag
    }

    removeNext(aName){
        for (let i in this.m_next2)
            delete this.m_next2[i][aName]
    }

    execute(aStream){
        this.doEvent(aStream)
        this.doNextEvent(this.m_next2[aStream.tag()], aStream)
    }
}

class pipeDelegate extends pipe{

    constructor(aParent, aName){
        super(aParent, aName)
        this.m_next2 = []
    }

    next(aNext, aTag = ""){
        this.m_parent.find(this.m_delegate).next(aNext, aTag)
    }
    removeNext(aName){
        this.m_parent.find(this.m_delegate).removeNext(aName)
    }
    insertNext(aName, aTag){
        this.m_next2.push([aName, aTag])
    }
    execute(aStream){
        this.doEvent(aStream)
    }
    initialize(aFunc, aParam){
        this.m_delegate = aParam["delegate"]
        const del = this.m_parent.find(this.m_delegate)
        for (let i in this.m_next2)
            del.insertNext(this.m_next2[i][0], this.m_next2[i][1])
        pipe.prototype.initialize.call(this, aFunc, aParam)
    }
}

//#endregion

//#region test

//#region test1;test2;test3;test4

function test1_(){
    //test1 -> test1_next -> testSuccessJS
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 3)
        aInput.out()
    }, {name: "test1"})
    .nextF(function(aInput){  //test1_next
        console.assert(aInput.data() == 3)
        aInput.outs("Pass: test1", "testSuccessJS")
    })
}

function test1(){
    pipelines().run("test1", 3)
}

function test2_(){
    //test2 -> test2_ -> testSuccessJS
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 4)
        aInput.outs(5, "test2_")
    }, {name: "test2"})
    .nextF(function(aInput){
        console.assert(aInput.data() == 5)
        aInput.outs("Pass: test2", "testSuccessJS")
    }, "", {name: "test2_"})
}

function test2(){
    pipelines().run("test2", 4)
}

function test3_(){
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
}

function test3(){
    pipelines().run("test3", 66)
    pipelines().run("test3_1", "Pass: test3__")
}

function test4(){
    //test4 -> test4_(ex) -> test4__(ex) -> test_4 -> testSuccessJS
    pipelines().find("test4_").removeNext("test4__")
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

//#endregion

//#region test5;test6;test7;test8

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
    }, {name: "test6_", external: ""})

    pipelines().add(function(aInput){
        console.assert(aInput.data() == 5)
        aInput.scope(true).cache("hello2", "world");
        aInput.setData(aInput.data() + 1).out()
    }, {name: "test6__", external: ""})

    return "test6"
}

function test7(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == "hello")
        aInput.setData("world").out()
    }, {name: "test7", external: ""})

    return "test7"
}

function test8(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == "hello")
        aInput.outs("Pass: test8", "testSuccessJS")
    }, {name: "test8", external: ""})

    return "test8"
}

function test8_(){

}

//#endregion

//#region test9;test11;test12;test13

function test9(){
    pipelines().run("test9", "hello")
    return "test9"
}

function test9_(){
    return "test9_"
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

//#endregion

//#region test14;test15;test16;test17

function test14(){
    return "test14"
}

function test15_(){
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
}

function test15(){
    pipelines().run("test15", 66, "test15")
}

function test16_(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 66)
        aInput.setData(77).out()
    }, {name: "test16", external: "", type: "Partial"})
}

function test16(){
    return "test16"
}

function test17_(){
    pipelines().find("test17")
    .nextFB(function(aInput){
        console.assert(aInput.data() == 77.0)
        aInput.outs("Pass: test17", "testSuccessJS")
    }, "test17", {name: "test17_"})
    .nextF(function(aInput){
        console.assert(aInput.data() == 77.0)
        aInput.outs("Fail: test17", "testFailJS")
    }, "test17_", {name: "test17__"})

}

function test17(){
    pipelines().run("test17", 66, "test17")
}

//#endregion

//#region test18;test19;test20

function test18(){
    return "test18"
}

function test19_(){
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
}

function test19(){
    pipelines().run("test19_0", 66.0)
    pipelines().run("test19", 56.0)
}

function test20(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 56.0)
        aInput.setData("Pass: test20").out()
    }, {name: "test20", external: ""})

    return "test20"
}

function test20_(){

}

//#endregion

//#region test21;test22;test23;test24

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

}

function test21_(){
    return "test21_"
}

function test21__(){
    pipelines().run("test21", 56)

    return "test21__"
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

//#endregion

//#region test25;test26;test27;test28

function test25_(){
    pipelines().add(function(aInput){
        console.assert(aInput.data() == 25.0)
        aInput.setData("Pass: test25").out()
    }, {name: "test25", external: ""})
}

function test25(){
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

//#endregion

//#region test29;test30;test31;test32

function test29(){
    pipelines().add(function(aInput){
        context = aInput.data()
        console.assert(aInput.scope().data("ctx") == context)
        sendMessage("lala")
    }, {name: "test29", external: ""})

    return "test29"
}

function test30(){
    return "test30"
}

function test31(){
    return "test31"
}

function test32(){
    return "test32"
}

//#endregion

//#region test33;test34;test35;test36
function test33(){
    return "test33"
}

function test34(){
    return "test34"
}

function test35(){
    return "test35"
}

function test36(){
    return "test36"
}

//#endregion

//#region test37;test38;test39; test40

function test37(){
    return "test37"
}

function test38(){
    return "test38"
}

function test39(){
    return "test39"
}

function test40(){
    return "test40"
}

//#endregion

//#region test41;test42;test43;test44

function test41(){
    return "test41"
}

function test42(){
    return "test42"
}

function test42_(){
    return "test42"
}

function test43(){
    return "test43"
}

function test43_(){
    return "test43_"
}

function test43__(){
    return "test43__"
}

function test44(){
    return "test44"
}

function test45(){
    return "test45"
}

//#endregion

//#region test46

function test46(){
    return "test46"
}

function test47(){
    return "test47"
}

//#endregion

rea(e=>{
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

    test1_()
    test2_()
    test3_()
    test15_()
    test16_()
    test17_()
    test19_()
    test25_()
    pipelines().add(function(aInput){
        let test = [
            [test1, 1], //test js anonymous next
            [test2, 1], //test js specific next
            [test3, 3], //test js pipe future
            [test4, 1], //test pipe mixture: js->js.future(c++)->js.future(c++)->js ; scopeCache
            [test5, 1], //test pipe mixture: js.future(c++)->js
            [test8_, 1],
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

        aInput.outs({
                        [test31()]: 1, //test qml anonymous next
                        [test32()]: 1, //test qml specific next
                        [test33()]: 3,  //test qml pipe future
                        [test34()]: 1,  //test pipe mixture: qml->qml.future(c++)->qml.future(c++)->qml; scopeCache
                        [test35()]: 1, //test pipe mixture: qml.future(c++)->qml,
                        [test38()]: 1, //test qml pipe partial
                        [test39()]: 1, //test pipe mixture partial: qml.future(c++)->qml
                        [test41()]: 1, //test qml pipe delegate and pipe param

                        [test42_()]: 1, //test pipe mixture delegate: c++->c++.future(qml)->qml, c++
                        [test43()]: 1,  //test pipe mixture delegate: qml->qml.future(c++)->c++, qml,
                        [test44()]: 1, //test qml asyncCall
                        [test45()]: 1, //test pipe mixture: qml.asyncCall.c++
                        [test47()]: 1, //test qml aop and keep topo
                    }, "unitTestQML")
        aInput.outs({
            [test6()]: 1, //test pipe mixture: c++->c++.future(js)->c++.future(js)->c++ ; scopeCache
            [test7()]: 1, //test pipe mixture: c++.future(js)->c++
            [test8()]: 0, //test pipe mixture: c++.future(js)
            [test9_()]: 1, //test pipe mixture: js.future(c++)
            [test11()]: 1, //test c++ anonymous next
            [test12()]: 1, //test c++ specific next
            [test13()]: 3, //test c++ pipe future
            [test14()]: 1, //test c++ pipe partial
            [test16()]: 1, //test pipe mixture partial: c++.future(js)->c++
            [test18()]: 1, //test c++ pipe delegate and pipe param

            [test20()]: 1, //test pipe mixture delegate: c++->c++.future(js)->js, c++

            [test21_()]: 1,  //test pipe mixture delegate: js->js.future(c++)->c++, js
            [test22()]: 1, //test c++ asyncCall
            [test25()]: 1, //test pipe mixture: c++.asyncCall.js
            [test26()]: 1, //test c++ aop and keep topo
            [test27()]: 1, //test c++ functor
            [test28()]: 1, //test pipe qml
            [test29()]: 1,  //test rea-js arbitrary type
            [test30()]: 1, //test c++ pipe parallel
            [test36()]: 1, //test pipe mixture: c++->c++.future(qml)->c++.future(qml)->c++ ; scopeCache
            [test37()]: 1, //test pipe mixture: c++.future(qml)->c++
            [test40()]: 1, //test pipe mixture partial: c++.future(qml)->c++

            [test42()]: 1, //test pipe mixture delegate: c++->c++.future(qml)->qml, c++
            [test43_()]: 1,  //test pipe mixture delegate: qml->qml.future(c++)->c++, qml
            [test46()]: 1, //test pipe mixture: c++.asyncCall.qml
                    }, "unitTestC++")
    }, {name: "unitTest"})
    .next("unitTestC++")
    .nextF(function(aInput){
        aInput.setData({
            [test9()]: 0,
            [test21__()]: 0,
            [test43__()]: 0
        }).out()
    })
    .next("unitTestQML")
})

//#endregion
