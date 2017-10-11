/**
 * @file fuck.js.js
 *
 * Created by mmhunter on 10/10/2017.
 */


const addon = require('./build/Release/client');

addon.connect();

//console.log("from fuck:" + addon.fuck());

setTimeout(()=>{
    process.kill(process.pid);
},2000);