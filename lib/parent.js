const ipc = require('../build/Release/server');
const childProcess = require("child_process");
const EventEmitter = require("events").EventEmitter;

var children = {};

let stopped = false;

function stop(){
    if(!stopped){
        ipc.stop();
        stopped = true;
    }
}

module.exports = {

    fork:function(){

        let args = Array.prototype.slice.call(arguments,0);

        if(!args[2]){
            args[1] = [];
            args[2] = {};
        }
        if(!args[2].env){
            args[2].env = {}
        }
        args[2].env.PARENT_NODE_PID = process.pid;

        let child  = childProcess.fork.apply(childProcess,args);

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

    if(children[pid]){
        children[pid].__syncEvent.emit.apply(children[pid].__syncEvent,arr);
    }

});



process.on('exit',function(){
    console.log("parent exit");
    stop();
});

process.on('SIGINT', () => {
    console.log('Received SIGINT.  Press Control-D to exit.');
    stop();
});

