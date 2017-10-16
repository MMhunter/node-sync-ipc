/**
 * @file fuck.js.js
 *
 * Created by mmhunter on 10/10/2017.
 */


const ipc = require('./index').child;

console.log("hahaha");

//addon.connect();

//addon.write("hdhd");

//console.log("from fuck:" + addon.fuck());



for(let i = 0; i <10 ; i++){
    console.log("get "+i+" from parent:"+ipc.sendSync("test",i));
}