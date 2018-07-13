/**
 * @file child.js
 *
 * Created by mmhunter on 17/10/2017.
 */
var syncIpc = new (require("../../index").SyncIPCClient)(100);

let sum = 0;

let count = Math.random() * 10000;

for (let i = 0; i < count; i ++){
    sum += syncIpc.sendSync("next");
}

syncIpc.sendSync("result",sum);
