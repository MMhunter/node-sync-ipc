/**
 * @file child.js
 *
 * Created by mmhunter on 17/10/2017.
 */
var syncIpc = new (require("../../index").SyncIPCClient)(1002);

let sum = 0;

let count = Math.random() * 1000;

for (let i = 0; i < 10; i ++){
    sum += syncIpc.sendSync("next");
}

syncIpc.sendSync("result",sum);
