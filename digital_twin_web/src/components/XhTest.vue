<template>
  <div class="hello">
    <div @click = "CustomEvent()"  class = "XhWebArea"><span>自定义事件</span></div>
    <div @click = "HiddenActor()"  class = "XhWebArea"><span>隐藏对象  </span></div>
    <div @click = "VisibleActor()" class = "XhWebArea"><span>显示对象  </span></div>
    <div @click = "SpawnActor()"   class = "XhWebArea"><span>生成对象  </span></div>
    <div @click = "DestroyActor()" class = "XhWebArea"><span>销毁对象  </span></div>
    <div @click = "FlyTo()"        class = "XhWebArea"><span>飞行      </span></div>
</div>
</template>

<script setup>
import { onMounted, onBeforeUnmount} from "vue";
import {init,uninit,ue} from "@/js"
//var ue = null;
onMounted(() => {
  init()
})
onBeforeUnmount(() => {
  uninit()
})
function CustomEvent()
{
  ue.CustomEvent({name:"你好",age:"26"});
  console.log("CustomEvent")
}
function HiddenActor()
{
  ue.HiddenActor("StaticMeshActor","HiddenTest",true);
  console.log("HiddenActor")
}
function VisibleActor()
{
  ue.HiddenActor("StaticMeshActor","HiddenTest",false);
  console.log("VisibleActor")
}
function SpawnActor()
{
  ue.SpawnActor(
    "SpawnTest",
    {
      L:[20,410,0],
      R:[0,0,0],
      S:[1,1,1]
    },
    {
      addr:"湖北省武汉市",
      time:"2024-06-01"
    }
  );
  console.log("SpawnActor")
}
function DestroyActor()
{
  ue.DestroyActor("SpawnTest");
  console.log("DestroyActor")
}
function FlyTo()
{
  ue.FlyTo(
    {L:[7.494722,121.877868,0],R:[0,-2,-2.697803],Len:1000,T:1},
    function(){
      console.log("flyto的回调");
      ue.send("flyto的回调")
    }
  );
  console.log("FlyTo")
}

</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.hello
{
  margin-left: 20px;
  margin-right: 20px;
  display: flex;
  flex-direction: column;
  flex: auto;
  align-items: start;
  justify-content: center;
}
.hello div
{
  margin: 10px;
  padding: 10px 20px;
  color: white;
  background-color: rgb(0, 157, 255);
  cursor: pointer;
}
</style>
