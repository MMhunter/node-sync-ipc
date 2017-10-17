# node-sync-ipc

`node-sync-ipc` is a tiny library for synchronous communication between parent and asynchronously forked child node processes. It can block the child process until the parent make a response. This use case is kind of rare but hope it can be some help if anyone needs.

# Install

````shell

npm install node-sync-ipc

````

# Usage

In parent process:



````javascript

// parent.js

var syncIpc = require("node-sync-ipc").parent();

// fork the child process just like child_process.fork()
var child = syncIpc.fork("./child.js");

child.onSync("foo",function(res,bar){

    bar = bar + " " +bar;
    // block the child process for one second
    // the first argument will be passed to child process as the result
    setTimeout(function(){
        res(bar);
    },1000)
});

````

In child process:

````javascript

var syncIpc = require("node-sync-ipc").child();

// will log "echo content echo content" in console

console.log(syncIpc.sendSync("foo","echo content"));

````

# Attention

This module is implemented with uv_pipe_t in libuv to create channel. On Unix based system, this module would create a nodePipe{parentPid}.sock in the home directory (or defined by the HOME environment variable). In most cases this file would be deleted when all child process exits or the parent process exits. But currently it is not guaranteed to be deleted if process was killed unexpectedly.


# Copyright

Copyright (c) 2017 Hang Ma. See [LICENSE](https://github.com/mmhunter/node-sync-ipc/blob/master/LICENSE) for details.


