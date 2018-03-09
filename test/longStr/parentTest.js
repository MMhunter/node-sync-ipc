/**
 * @file parent.js
 *
 * Created by mmhunter on 17/10/2017.
 */
var syncIpc = require("../../index").parent();
// var assert = require("assert");
//
// // fork the child process just like child_process.fork()
// describe("long str test",()=>{
//
//     it("long arr should be same",()=>{
//
//     })
//
//
// })

var child = syncIpc.fork("./child.js");

var r;

child.onSync("foo",function(res,bar){

    r = bar;
    // block the child process for one second
    // the first argument will be passed to child process as the result
    setTimeout(function(){
        res(bar);
    },1000);
});


child.onSync("result",function(res,bar){
    res(bar);
    //assert.equal(1,2);
    //done();
    // block the child process for one second
    // the first argument will be passed to child process as the result
});