const ipc = require('bindings')('server');
const EventEmitter = require("events").EventEmitter;

let singleton = null;

class SyncIPCServer {

  constructor(handle) {
    if (singleton) {
      throw new Error('Multiple Sync Ipc Server is not currently supported')
    }
    this.running = false;
    this.emitter = new EventEmitter();
    this.stop  = () => {
      if(this.running){
        this.running = false;
        this.emitter.removeAllListeners();
        ipc.stop();
      }
      process.removeListener('exit', this.stop);
    };
    process.once('exit', this.stop);
  }

  start (handle){
    if(this.running) {
      throw new Error('Sync IPC Server has already stared');
      return;
    }
    this.running = true;
    ipc.startServer(handle);
    ipc.bindPipeListener(function(pid,result){
      let arr = JSON.parse(result);
      let event = arr.shift();
      arr.unshift(pid);
      this.emitter.emit(event, ...arr);
    });
  }

  onMessage(event, listener){
    this.emitter.on(event, function () {
      let args = Array.prototype.slice.call(arguments,0);
      args.unshift(res);
      listener.apply(null,args);
    })
  }

  dispose () {
    this.stop();
    process.removeListener('exit', this.stop);
    singleton = null;
    this.emitter.removeAllListeners();
    this.emitter = null;
  }

}

module.exports = {
  SyncIPCServer
};

process.on('SIGINT', () => {
  console.log('Received SIGINT.  Press Control-D to exit.');
  process.exit(0);
});
