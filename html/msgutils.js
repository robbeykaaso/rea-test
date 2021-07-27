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

// 控件控制函数
function onBtnSendMsg()
{
    pipelines().run("unitTest")
    //var cmd = document.getElementById("待发送消息").value;
}

async function onBtnShowImage()
{
    let dt = await pipelines().input("", "test24").asyncCallS("unitTestImage")
    let img = new Image(400, 300)

    const cvs = document.getElementById("cvs")
    const ctx = cvs.getContext("2d")
    img.onload = function(){
        ctx.drawImage(img, 0, 0, 400, 300)
    }
    img.src = dt.data()
}
