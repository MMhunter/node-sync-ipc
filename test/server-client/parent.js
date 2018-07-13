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

    const server = new SyncIPCServer();

    it("should get the right sum",(done)=>{

        server.start(100);

        require("child_process").fork(path.join(__dirname,"./child-cal-sum.js"));

        var nextNumber = getNextNumber();

        var sum = 0;

        server.onMessage("next",function(res){
            res(nextNumber);
            sum += nextNumber;
            nextNumber = getNextNumber();
        });

        server.onMessage("result",function(res,pid,result){
            res();
            assert.equal(sum,result);
            server.stop();
            done();
        });
    });

    it("should get the right sum [using add child]",(done)=>{

        server.start(100);

        var child = require("child_process").fork(path.join(__dirname,"./child-cal-sum.js"));

        var nextNumber = getNextNumber();

        var sum = 0;

        server.onMessage("next",function(res){
            res(nextNumber);
            sum += nextNumber;
            nextNumber = getNextNumber();
        });

        server.onMessage("result",function(res,pid,result){
            res();
            assert.equal(sum,result);
            server.stop();
            done();
        });
    });

    it("long arr should be same",(done)=>{

        var child = require("child_process").fork(path.join(__dirname,"./child-long-str.js"));

        var r;

        server.start(1001);

        server.onMessage("foo",function(res,pid,bar){

            r = bar;
            setTimeout(function(){
                res(bar);
            },500);
        });

        server.onMessage("result",function(res,pid,bar){
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
        server.start(1002);
        var nextNumber = getNextNumber();
        var sum = 0;
        var finishedCount = 0;
        var childSum = 0;
        for(let i = 0; i < 5; i++){

            var child = require("child_process").fork(path.join(__dirname,"./child-cal-sum-multiple.js"));

        }

       server.onMessage("next",function(res){
         res(nextNumber);
         sum += nextNumber;
         nextNumber = getNextNumber();
       });

       server.onMessage("result",function(res,pid,result){
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