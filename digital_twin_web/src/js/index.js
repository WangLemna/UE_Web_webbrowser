import ue5 from "@/js/ue5"
var ue = null;
var url = "ws://192.168.31.205:60001"

export const init = () => {
  ue = new ue5(url)
  add_ue5_event()
  setInterval(() => {
    if(!ue.socket) {
      ue = new ue5(url)
      add_ue5_event()
    } else if(ue.socket.readyState == 3) {
      ue = new ue5(url)
      add_ue5_event()
    }
  }, 3000)
  // ue.SocketOpen = () => {
  // }
}
function WebTest(data)
{
  console.log(data.name)
  console.log(data.age)
  console.log(data)
  ue.SendJson(data)
}
function add_ue5_event(){
  ue.WebTest = data => {
    WebTest(data)
  }
}
export const uninit = () => {
  if (ue) {
    ue.socket.close();
  }
}


export {ue}