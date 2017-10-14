const addon = require('./build/Release/server');

const child_process = require("child_process");


//
// addon.listen();






setTimeout(()=>{
   addon.unlock();
},2000);

let i = 0;
setInterval(()=>{
    console.log(i);
    i++;
},1000);

setTimeout(()=>{
    let fuck = child_process.fork("./fuck.js");
},200);

process.on('exit',function(){
    addon.stop();
});

