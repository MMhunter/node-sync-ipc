const ipc = require('bindings')('client.node');

const MAX_STRING_LENGTH = 1000;

class SyncIPCClient {

    constructor(serverHandle) {
        this.serverHanldle = serverHandle|| 0;
    }

    sendSync(event, ...args) {
        if(typeof event !== 'string' || event.length === 0){
            throw new Error("Event name must be a non empty string! ");
        }

        args = args.map(function(a){
            if(a === undefined){
              a = null;
            }
            return a;
          });

        let string = JSON.stringify({
          event,
          args
        });

        string = string.length.toString() + "#" + string;

        let message = [];
        for (let i = 0 ; i < string.length; i += MAX_STRING_LENGTH) {
            message.push(string.substr(i, MAX_STRING_LENGTH));
        }
        let result = ipc.sendSync(this.serverHanldle, ...message);

        return JSON.parse(result);
    }

}

module.exports = {
    SyncIPCClient
};
