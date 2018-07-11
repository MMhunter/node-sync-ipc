/**
 * @file child.js
 *
 * Created by mmhunter on 17/10/2017.
 */
var SyncIPCClient = require("../../index").SyncIPCClient;

const client = new SyncIPCClient(100);

const r = client.sendSync('yes',1000);

console.log(r);
