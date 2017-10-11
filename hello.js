const addon = require('./build/Release/addon');

const child_process = require("child_process");



addon.listen();



setTimeout(()=>{
    let fuck = child_process.fork("./fuck.js");
    //console.log(addon.hello());
},1000);

