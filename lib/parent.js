const ipc = require('../build/Release/server');

setTimeout(()=>{
    ipc.unlock();
},10000);
