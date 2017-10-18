import * as child_process from "child_process";

export = NodeSyncIpc;
export as namespace NodeSyncIpc;

declare namespace NodeSyncIpc{

    interface NodeSyncIpcParent{

        fork(modulePath:string,args?:any[],options?:Object):NodeSyncIpcChildProcess;

    }

    interface NodeSyncIpcChildProcess extends child_process.ChildProcess{

        onSync(event:string,listener:(res:(returnValue?)=>void,...args:any[])=>void);

    }


    interface NodeSyncIpcChild{

        sendSync(event:string,...args:any[]);

    }

    export function parent():NodeSyncIpcParent;

    export function child():NodeSyncIpcChild;


}

