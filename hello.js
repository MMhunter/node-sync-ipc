// const addon = require('./build/Release/server');
//
// const child_process = require("child_process");

//
//
//
// let i = 0;
// setInterval(()=>{
//     console.log(i);
//     i++;
// },1000);
//
// let fuck = child_process.fork("./fuck.js");
//
// setInterval(()=>{
//     addon.write(fuck.pid,"\"shabi\"");
// },2000);
//
// fuck.on("exit",()=>{
//     console.log("fuck exit");
// });
// process.on('exit',function(){
//     addon.stop();
// });
//
//
// addon.bindPipeListener(function(event,result){
//
//     console.log("event: "+event);
//     console.log(result);
// })
// // emitter.call_emit();
//
// console.log(process.pid);

const test = require("./lib/parent");

let sub = test.fork("./fuck.js");

let i = 0;

let r = {};

sub.onSync("test",function(res){

    r[i] = "";
    for(let j = 0; j < i ; j ++){
        r[i] += "1";
    }
    i++;

    res(i);
});

sub.on("exit",()=>{
    console.log("fuck exit");
});

setTimeout(()=>{
    process.exit(0);
},3000);