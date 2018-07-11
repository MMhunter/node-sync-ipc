const ipc = require('bindings')('client.node');

class SyncIPCClient {

    constructor(serverHandle) {
        this.serverHanldle = serverHandle|| 0;
    }

    sendSync(eventName, ...other) {
        if(typeof eventName !== 'string' || eventName.length === 0){
            throw new Error("Event name must be a non empty string! ");
        }

        let args = Array.prototype.slice.call(arguments);

        args = args.map(function(a){
            if(a === undefined){
              a = null;
            }
            return a;
          });

        ipc.setServerPid(this.serverHanldle);

        let result = ipc.sendSync(JSON.stringify(args));

        return JSON.parse(result);
    }

}

module.exports = {
    SyncIPCClient
};
