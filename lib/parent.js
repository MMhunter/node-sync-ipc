const ipc = require('../build/Debug/server');
const childProcess = require("child_process");
const EventEmitter = require("events").EventEmitter;

var children = {};

let running = false;

function stop(){
    if(running){
        running = false;
        ipc.stop();
    }
}

function startServer(){
    running = true;
    ipc.startServer();
}

function stopServerIfNoChildLeft(){
    if(!running) return;
    let shouldStop = true;
    for(let key in children){
        if(children[key]){
            shouldStop = false;
        }
    }
    if(shouldStop){
        stop();
    }
}

module.exports = {

    fork:function(){

        if(!running){
            startServer();
        }

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
            stopServerIfNoChildLeft();
        });

        return child;
    }

};


ipc.bindPipeListener(function(pid,result){

    let arr = JSON.parse(result);

    if(children[pid]){
        children[pid].__syncEvent.emit.apply(children[pid].__syncEvent,arr);
    }

});


process.on('exit',function(){
    stop();
});

process.on('SIGINT', () => {
    console.log('Received SIGINT.  Press Control-D to exit.');
    process.exit(0);
});

