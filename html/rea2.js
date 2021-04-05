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
const generateUUID = function () {
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

////////////////////////////////////////////////////////////////////////////////////////////

var PipelineJS

if (typeof qt != "undefined"){
    new QWebChannel(qt.webChannelTransport, function(channel){
        context = channel.objects.context;
        PipelineJS = channel.objects.Pipeline;
        PipelineJS.executeJSPipe.connect(function(aName, aStream, aSync, aFromOutside){
            pipelines().execute(aName, new stream(aStream.data, aStream.tag, aStream.scope), aSync, aFromOutside)
        })
        PipelineJS.removeJSPipe.connect(function(aName){
            pipelines().remove(aName)
        })
        unitTest()
    })
}else
    alert("qt object not exists!")

////////////////////////////////////////////////////////////////////////////////////////////

class scopeCache {
    constructor(aData = {}){
        this.m_data = aData
    }
    cache(aName, aData){
        this.m_data[aName] = aData
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
    scope(){
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

    async asyncCall(aName){
        let ret
        let got_ret = false

        const monitor = pipelines().find(aName).nextF(aInput => {
            ret = new stream(aInput.data(), this.m_tag)
            got_ret = true

        }, this.m_tag)
        pipelines().execute(aName, this)

        function sleep(time) {
          return new Promise(resolve => setTimeout(resolve,time))
        }
        while(!got_ret)
            await sleep(5)
        pipelines().find(aName).removeNext(monitor.actName())
        pipelines().remove(monitor.actName(), true)

        return ret
    }
    async asyncCallF(aFunc, aParam = {}){
        const pip = pipelines().add(aFunc, aParam)
        const ret = await this.asyncCall(pip.actName())
        pipelines().remove(pip.actName(), true)
        return ret
    }
}

class pipe {

    constructor(aName){
        this.m_next = {}
        if (aName == null || aName == "")
            this.m_name = generateUUID()
        else
            this.m_name = aName
        pipelines().m_pipes[this.m_name] = this
    }
    
    resetTopo(){
        this.m_next = {}
    }

    actName() {
        return this.m_name
    }

    next(aName, aTag = ""){
        console.assert(!this.m_external)
        const tags = aTag.split(";")
        for (let i in tags)
            this.insertNext(aName, tags[i])
        return pipelines().find(aName)
    }

    nextB(aName, aTag = ""){
        this.next(aName, aTag)
        return this
    }

    nextF(aFunc, aTag = "", aParam = {}){
        return this.next(pipelines().add(aFunc, aParam).actName(), aTag)
    }

    nextFB(aFunc, aTag = "", aParam = {}){
        this.nextF(aFunc, aTag, aParam)
        return this
    }

    removeNext(aName){
        console.assert(!this.m_external)
        delete this.m_next[aName]
    }

    initialize(aFunc, aParam){
        this.m_func = aFunc
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
        const pip = pipelines().find(aName)
        if (pip)
            if (pip.m_external)
                PipelineJS.tryExecuteOutsidePipe(aName, {data: aStream.data(), tag: aStream.tag(), scope: aStream.scope()}, {}, false)
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
    
    constructor(aName){
        super(aName)
        this.m_next2 = []
    }

    insertNext(aName, aTag){
        this.m_next2.push([aName, aTag])
    }
}

class pipeFuture extends pipe{

    constructor(aName){
        super()
        this.m_act_name = aName
        this.m_next2 = []
        if (pipelines().find(aName + "_pipe_add", false)){
            const pip = new pipeFuture0(aName)
            pipelines().call(aName + "_pipe_add")
            for (let i in pip.m_next2)
                this.insertNext(pip.m_next2[i][0], pip.m_next2[i][1])
            this.m_external = pip.m_external
            pipelines().remove(aName)
        }

        pipelines().add(e => {
            const pip = pipelines().find(aName, false)
            for (let i in this.m_next2)
                pip.insertNext(this.m_next2[i][0], this.m_next2[i][1])
            pip.m_external = this.m_external
            pipelines().remove(this.m_name)
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
        PipelineJS.tryExecuteOutsidePipe(this.actName(), {data: aStream.data(), tag: aStream.tag(), scope: aStream.scope()}, sync, true)
    }
}

var pipeline_s = {}
function pipelines(aName = ""){
    if (!pipeline_s[aName])
        pipeline_s[aName] = new pipeline()
    return pipeline_s[aName]
}

class pipeline{

    constructor(){
        this.m_pipes = {}
    }

    add(aFunc, aParam){
        const nm = aParam["name"]
        let pip
        if (aParam["type"] == "Partial")
            pip = new pipePartial(nm)
        else if (aParam["type"] == "Delegate")
            pip = new pipeDelegate(nm)
        else
            pip = new pipe(nm)
        if (nm != ""){
            const ad = pip.actName() + "_pipe_add"
            pipelines().call(ad)
            pipelines().remove(ad)
        }
        pip.initialize(aFunc, aParam)
        return pip
    }

    remove(aName, aOutside = false){
        const pip = pipelines().m_pipes[aName]
        if (pip)
            delete pipelines().m_pipes[aName]
        if (aOutside)
            PipelineJS.removePipeOutside(aName)
    }

    find(aName, aNeedFuture = true){
        let pip = pipelines().m_pipes[aName]
        if (!pip && aNeedFuture)
            pip = new pipeFuture(aName)
        return pip
    }

    run(aName, aInput, aTag = "", aScope = null){
        pipelines().execute(aName, new stream(aInput, aTag, aScope))
    }

    call(aName, aInput){
        const pip = pipelines().m_pipes[aName]
        if (pip)
            pip.doEvent(new stream(aInput))
    }

    execute(aName, aStream, aSync, aFromOutside = false){
        const pip = this.find(aName, !aFromOutside)
        if (pip){            
            if (aSync){
                if (Object.keys(aSync).length > 0)
                    pip.resetTopo()
                if (aSync["next"])
                    for (let i in aSync["next"])
                        pip.insertNext(aSync["next"][i][0], aSync["next"][i][1])
            }
            pip.execute(aStream)    
        }
    }

    input(aInput, aTag = "", aScope = null){
        const tag = aTag == "" ? generateUUID() : aTag
        return new stream(aInput, tag, aScope)
    }
}

class pipePartial extends pipe{
    constructor(aName){
        super(aName)
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

    constructor(aName){
        super(aName)
        this.m_next2 = []
    }

    next(aNext, aTag = ""){
        pipelines().find(this.m_delegate).next(aNext, aTag)
    }
    removeNext(aName){
        pipelines().find(this.m_delegate).removeNext(aName)
    }
    insertNext(aName, aTag){
        this.m_next2.push([aName, aTag])
    }
    execute(aStream){
        this.doEvent(aStream)
    }
    initialize(aFunc, aParam){
        this.m_delegate = aParam["delegate"]
        const del = pipelines().find(this.m_delegate)
        for (let i in this.m_next2)
            del.insertNext(this.m_next2[i][0], this.m_next2[i][1])
        pipe.prototype.initialize.call(this, aFunc, aParam)
    }
}
