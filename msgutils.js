var context;
var Pipeline;
// 初始化

function init()
{
    if (typeof qt != 'undefined')
    {
        new QWebChannel(qt.webChannelTransport, function(channel){
            context = channel.objects.context;
            Pipeline = channel.objects.Pipeline;
            Pipeline.trig.connect(function(aInput){
                for (var i in aInput)
                    console.log(i + ";" + aInput[i])
                Pipeline.trigged(function(){
                    console.log(i + ";" + aInput[i])
                })
            })
        });
    }
    else
    {
        alert("qt对象获取失败！");
    }
}
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
    var cmd = document.getElementById("待发送消息").value;
    sendMessage(cmd);
}

function recvMessage(msg)
{
    alert("接收到Qt发送的消息：" + msg);
}

init();
