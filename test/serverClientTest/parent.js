/**
 * @file parent.js
 *
 * Created by mmhunter on 17/10/2017.
 */
const path = require("path");
const assert = require("assert");
const SyncIPCServer = require('../../index').SyncIPCServer;

const server = new SyncIPCServer();

server.start(100);

server.onMessage('yes', (res, pid, v) => {
  res(v+1);
});