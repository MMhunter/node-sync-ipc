/**
 * @file child.js
 *
 * Created by mmhunter on 17/10/2017.
 */
const path = require("path");

var syncIpc = new (require("../../index").SyncIPCClient)(process.env.SYNC_PIPE);

let sum = 0;

let count = Math.random() * 10000;

for (let i = 0; i < count; i ++){
    sum += syncIpc.sendSync("next");
}

syncIpc.sendSync("result",sum);
