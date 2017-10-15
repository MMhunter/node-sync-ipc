const ipc = require('../build/Release/server');
const childProcess = require("child_process");
const EventEmitter = require("events").EventEmitter;

var children = {};

module.exports = {

    fork:function(){
        let child  = childProcess.fork.apply(childProcess,arguments);

        child.__syncEvent = new EventEmitter();

        child.onSync = function(event,listener){

            child.__syncEvent.addListener(event,function(){

                let res = function(result){
                    if(result === undefined){
                        result = null;
                    }
                    ipc.write(child.pid,JSON.stringify(result));
                };

                let args = Array.prototype.slice.call(arguments,0);

                args.unshift(res);

                listener.apply(null,args);

            })
        };

        children[child.pid] = child;

        child.on("exit",function(){
            children[child.pid] = null;
        });

        return child;
    }

};


ipc.bindPipeListener(function(pid,result){

    let arr = JSON.parse(result);

    console.log(arr);

    let eventName = arr[0];

    if(children[pid]){
        children[pid].__syncEvent.emit.apply(children[pid].__syncEvent,arr);
    }

});

process.on('exit',function(){
    console.log("parent exit");
    ipc.stop();
});

process.on('SIGINT', () => {
    console.log('Received SIGINT.  Press Control-D to exit.');
    ipc.stop();
});

