const ipcServer = require('bindings')('server').SyncIpcServer;
const EventEmitter = require("events").EventEmitter;
const fs = require('fs');

const MAX_STRING_LENGTH = 2000;

function debug(...args) {
  console.log(...args);
}

class SyncIPCServer {

  constructor(pipeFile) {
    this.running = false;
    this.emitter = new EventEmitter();
    this._server = new ipcServer(pipeFile);
    this.pipeFile = pipeFile;
    process.on('exit', () =>{
      this.stop();
    });
    process.on('SIGINT', () => {
      this.stop();
    });
  }

  startListen() {
    if (require('os').platform() !== 'win32') {
      if (fs.existsSync(this.pipeFile)) {
        fs.unlink(this.pipeFile);
      }
    }
    this.running = true;
    this._server.listen((connection) => {
      let length = 0;
      let body = '';
      connection.onMessage((message) => {
        if (length == 0) {
          length = parseInt(message.split('#')[0], 10);
          body += message.substr(message.indexOf('#') + 1);
        } else {
          body += message;
        }
        if (body.length >= length) {
         const {event, args} = JSON.parse(body);
         body = '';
         length = 0;
         this.emitter.emit(event, connection, ...args);
        }
      });
    });
  }

  onMessage(event, listener){
    this.emitter.on(event, function (connection, ...args) {
      const res = function(result){
        if(result === undefined){
          result = null;
        }
        let string = JSON.stringify(result);
        string = string.length.toString() + "#" + string;
        let message = [];
        for (let i = 0 ; i < string.length; i += MAX_STRING_LENGTH) {
          message.push(string.substr(i, MAX_STRING_LENGTH));
        }
        connection.write(...message);
      };
      args.unshift(res);
      listener.apply(null,args);
    })
  }

  dispose () {
    this.stop();
    this.emitter = null;
  }

  stop() {
    if (this.running){
      this.running = false;
      this._server.stop();
    }
    this.emitter.removeAllListeners();
  }

}

module.exports = {
  SyncIPCServer
};
