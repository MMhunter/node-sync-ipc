/**
 * @file child.js
 *
 * Created by mmhunter on 17/10/2017.
 */
var syncIpc = require("../../index").child();

console.log("child ready");

let longStrArray = [];

for(let i = 0 ; i < 100; i ++){
    longStrArray.push("1231");
}

let echoed = syncIpc.sendSync("foo",longStrArray);

//console.error(echoed);

// syncIpc.sendSync("result",echoed);

console.log(1231);


