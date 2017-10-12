/**
 * @file fuck.js.js
 *
 * Created by mmhunter on 10/10/2017.
 */


const addon = require('./build/Release/client');

console.log("hahaha");

addon.connect();

//console.log("from fuck:" + addon.fuck());



// setTimeout(()=>{
//     process.exit(0);
// },20000);
let i = 0;
setInterval(()=>{
    addon.write("haha");
    console.log("fuck"+i);
    i++;

    console.log("fuck 2"+i);
},1000);

process.on('exit',function(){
    addon.stop();
});