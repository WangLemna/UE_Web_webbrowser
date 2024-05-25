var ue5 = function (url,TickTime = 30000) {
    let _this = this
    this.FlyToCallback = null
    this.socket = new WebSocket(url)
    this.TimeoutId = null
    this.SocketOpen = null
    this.send = function(str)
    {
        this.socket.send(str);
    }
    this.reader = new FileReader();
    this.reader.onload = function(e) {
        console.log('收到消息:', e.target.result);
        if(e.target.result == "HEARTBEAT") {
            console.log("检测到心跳")
            return
        } 
        let json = JSON.parse(e.target.result);
        // console.log(json.event)
        // console.log(json.option)
        if(json.event == "FlyToCallback") {
            _this.FlyToCallback()
        } else {
            _this[json.event](json.option)
        }
    }
    this.socket.onopen = () => {
        console.log('WebSocket 连接已建立');
        this.HtmlWidth(document.getElementsByTagName("html")[0].clientWidth)
        this.AddArea(getXhWebArea(document.getElementsByClassName("XhWebArea")))
        if(typeof _this.SocketOpen === 'function') {
            _this.SocketOpen();
        }
        this.TimeoutId = setInterval(() => {
            if(this.socket.readyState == 1)
            {
                this.socket.send("HEARTBEAT")
            }
        }, TickTime);
    }
    this.socket.onmessage = (event) => {
        console.log('收到消息:', event.data);
        this.reader.readAsText(event.data, "UTF-8");
    }
    this.socket.onerror = (error) => {
        console.log('错误:', error);
    // 处理收到的消息数据
    };
    this.socket.onclose = () => {
        clearInterval(_this.TimeoutId)
        console.log('WebSocket 连接已关闭');

    // 在连接关闭后，可以执行清理操作或重新连接等处理
    }
    this.SendJson = function (option){
        this.send(JSON.stringify(option))
    }
    this.CustomEvent = function (option){
        this.SendJson({event:"CustomEvent",option:option})
    }
    this.HiddenActor = function (type,tag,hidden){
        this.SendJson({
            event:"HiddenActor",
            option:{
                type:type,
                tag:tag,
                hidden:hidden
            }
        })
    }
    this.SpawnActor = function (type,trans,ExtraData){
        let L = trans.L ? trans.L : [100,145,852];
        let R = trans.R ? trans.R : [0,0,0];
        let S = trans.S ? trans.S : [1,1,1];
        this.SendJson({
            event:"SpawnActor",
            option:{
                type:type,
                transform:{
                    location:L,
                    rotation:R,
                    scale:S,
                },
                ExtraData:ExtraData
            }
        })
    }
    this.DestroyActor = function (type,tag){
        let _tag = tag ? tag : ""
        this.SendJson({
            event:"DestroyActor",
            option:{
                type:type,
                tag:_tag
            }
        })
    }
    this.FlyTo = function (option,callback){
        let L      = option.L   ? option.L   : [100,145,852];
        let R      = option.R   ? option.R   : [0,0,0];
        let length = option.Len ? option.Len : 0;
        let time   = option.T   ? option.T   : 1;
        let IsCallback = callback ? true : false;
        if(typeof callback === 'function') {
            _this.FlyToCallback = callback;
        }
        this.SendJson({
            event:"FlyTo",
            option:{
                transform:{
                    location:L,
                    rotation:R
                },
                ArmLength:length,
                time:time,
                bcallback:IsCallback
            }
        });
    }
    this.AddArea = function (AreaArray){
        this.SendJson({event:"AddArea",option:{array:AreaArray}})
    }
    this.RemoveArea = function (AreaArray){
        this.SendJson({event:"RemoveArea",option:{array:AreaArray}})
    }
    this.HtmlWidth = function (width){
        this.SendJson({event:"HtmlWidth",option:{width:width}})
    }
    this.GetWebSize =() => {
        this.HtmlWidth(document.getElementsByTagName("html")[0].clientWidth)
    }
}


export const getXhWebArea = (Doms) => {
    let ClickAreaDom = Doms ? Doms : [];
    let ClickArea = []
    for (let i = 0; i < ClickAreaDom.length; i++) {
        let top = getElementTop(ClickAreaDom[i])
        let left = getElementLeft(ClickAreaDom[i])
        let buttom = top + ClickAreaDom[i].clientHeight;
        let right = left + ClickAreaDom[i].clientWidth;
        ClickArea.push({min:[left,top],max:[right,buttom]})
        console.log(top,left,buttom,right)
    }
    return ClickArea
}
function getElementLeft(element){
    var actualLeft = element.offsetLeft;
    var current = element.offsetParent;
    while (current !== null){
        actualLeft += current.offsetLeft;
        current = current.offsetParent;
    }
    return actualLeft;
}
// 获取顶部
function getElementTop(element){
    var actualTop = element.offsetTop;
    var current = element.offsetParent;
        
    while (current !== null){
        actualTop += current.offsetTop;
        current = current.offsetParent;
    }
    return actualTop;
}
export default ue5;