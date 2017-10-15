/**
 * @file child.js
 *
 * Created by mmhunter on 15/10/2017.
 */

const ipc = require('../build/Release/client.node');

let client = {

    sendSync:function(eventName){

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

        return JSON.parse(ipc.send.call(ipc,JSON.stringify(args)));

    }

};

ipc.setPid(process.pid);


module.exports = client;
