/**
 * @file parent.js
 *
 * Created by mmhunter on 17/10/2017.
 */
const path = require("path");
const assert = require("assert");
const SyncIPCServer = require("../../index").SyncIPCServer;
const childProcess = require('child_process');


describe("Server Client Mode Test",()=>{






  it("should get the right sum",(done)=>{

        const pipeFile = require("path").join(require('os').tmpdir(), 'temp1.sock');

        const server = new SyncIPCServer(pipeFile);

        server.startListen();

        require("child_process").fork(path.join(__dirname,"./child-cal-sum.js"),{env:{SYNC_PIPE:pipeFile}});

        var nextNumber = getNextNumber();

        var sum = 0;

        server.onMessage("next",function(res){
            res(nextNumber);
            sum += nextNumber;
            nextNumber = getNextNumber();
        });

        server.onMessage("result",function(res,result){
            res();
            try{
              assert.equal(sum,result);
              done();
            } finally {
              server.stop();
            }
        });
    });

    it("should get the right sum [using add child]",(done)=>{

      const pipeFile = require("path").join(require('os').tmpdir(), 'temp2.sock');

      const server = new SyncIPCServer(pipeFile);

        server.startListen();

        var child = require("child_process").fork(path.join(__dirname,"./child-cal-sum.js"),{env:{SYNC_PIPE:pipeFile}});

        var nextNumber = getNextNumber();

        var sum = 0;

        server.onMessage("next",function(res){
            res(nextNumber);
            sum += nextNumber;
            nextNumber = getNextNumber();
        });

        server.onMessage("result",function(res,result){
            res();
              try{
                assert.equal(sum,result);
                done();
              } finally {
                server.stop();
              }
        });
    });

    it("long arr should be same",(done)=>{

      const pipeFile = require("path").join(require('os').tmpdir(), 'temp3.sock');

      const server = new SyncIPCServer(pipeFile);

        var child = require("child_process").fork(path.join(__dirname,"./child-long-str.js"),{env:{SYNC_PIPE:pipeFile}});

        var r;

        server.startListen();

        server.onMessage("foo",function(res,bar){

            r = bar;
            setTimeout(function(){
                res(bar);
            },500);
        });

        server.onMessage("result",function(res,bar){
            res();
            assert.equal(r.length,bar.length);
            for(let i = 0; i < 5; i++){
                let index = Math.floor(Math.random() * r.length);
                assert.equal(r[index], bar[index]);
            }
            server.stop();
            done();
        });
    })

    it("multiple child share data",(done)=>{

      const pipeFile = require("path").join(require('os').tmpdir(), 'temp4.sock');

      const server = new SyncIPCServer(pipeFile);

        server.startListen();
        var nextNumber = getNextNumber();
        var sum = 0;
        var finishedCount = 0;
        var childSum = 0;
        for(let i = 0; i < 5; i++){

            var child = require("child_process").fork(path.join(__dirname,"./child-cal-sum-multiple.js"),{env:{SYNC_PIPE:pipeFile}});

        }

       server.onMessage("next",function(res){
         res(nextNumber);
         sum += nextNumber;
         nextNumber = getNextNumber();
       });

       server.onMessage("result",function(res,result){
         res();
         childSum += result;
         finishedCount ++;
         if(finishedCount === 5){
           assert.equal(sum,childSum);
           server.stop();
           done();
         }
       });

    })

});

function getNextNumber(){
    return Math.floor(Math.random()*1000);
}