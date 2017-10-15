/**
 * @file fuck.js.js
 *
 * Created by mmhunter on 10/10/2017.
 */


const addon = require('./lib/child');

console.log("hahaha");

//addon.connect();

//addon.write("hdhd");

//console.log("from fuck:" + addon.fuck());

console.log(addon.sendSync("test","yes",1));
// setTimeout(()=>{
//     process.exit(0);
// },20000);
// let i = 0;
// setInterval(()=>{
//
//     console.log("fuck"+i);
//     i++;
//
// },1000);
