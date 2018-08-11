# node-sync-ipc

[![Build Status](https://travis-ci.org/MMhunter/node-sync-ipc.svg?branch=master)](https://travis-ci.org/MMhunter/node-sync-ipc)

`node-sync-ipc` is a tiny library making it possible for node processes to send synchronous message to other processes. It can block the client side process until the server make a response.

# Install

````shell

npm install node-sync-ipc

````

# Usage
## Server And Client Mode

Server side:

````javascript

// server.js

const SyncIPCServer = require("node-sync-ipc").SyncIPCServer;

// pipe File
// on Unix based systems, pipe file should be a sock file path
// on Windows, pipe should be named pipes
const pipeFile = path.join(require('os').tmpDir(), 'tmp.sock');

const server = new SyncIPCServer(pipeFile);

server.startListen();

server.onMessage("foo",function(res,bar){
    bar = bar + " " +bar;
    // block the child process for one second
    setTimeout(function(){
        // the first argument will be passed to child process as the result
        res(bar);
    },1000)
});

//stop server when not needed
//server.stop()

````

Client Side:

````javascript

// client.js

const SyncIPCClient = require("node-sync-ipc").SyncIPCClient;

// handle number
const serverHandle = path.join(require('os').tmpDir(), 'tmp.sock');

const client = new SyncIPCClient(serverHandle);

// will log "echo content echo content" in console

console.log(client.sendSync("foo","echo content"));

````



# Attention

## Data should be serializable

Data to be transferred will be serialized and deserialized in the format of JSON. Error will be thrown if data is not serializable by `JSON.stringify`.

Also class information will be lost during the communication.

## Electron && NW.js

This module has c++ add-ons, so you have to rebuild it to use it in Electron.

# Copyright

Copyright (c) 2018 Hang Ma. See [LICENSE](https://github.com/mmhunter/node-sync-ipc/blob/master/LICENSE) for details.


