/**
 * @file fuck.js.js
 *
 * Created by mmhunter on 10/10/2017.
 */


const ipc = require('./index').child();

console.log("hahaha");

//addon.connect();

//addon.write("hdhd");

//console.log("from fuck:" + addon.fuck());



// setInterval(()=>{
//     console.log(ipc.sendSync("test"));
// },1);

while (true){
    console.log(ipc.sendSync("test"));
}