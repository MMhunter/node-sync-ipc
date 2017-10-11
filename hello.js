const addon = require('./build/Release/server');

const child_process = require("child_process");



addon.listen();

let fuck = child_process.fork("./fuck.js");



