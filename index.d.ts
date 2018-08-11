export = NodeSyncIpc;
export as namespace NodeSyncIpc;

declare namespace NodeSyncIpc{

    export type MessageListener = (res:(returnValue?:any)=>void, ...args:any[]) => void

    export class SyncIPCServer {

        constructor(pipeFile: string);

        public startListen(): void;

        public onMessage(event: string, listener: MessageListener): void;

        public stop(): void;

        public dispose(): void;
    }

    export class SyncIPCClient {

        constructor(serverHandle?:number);

        sendSync(event:string,...args:any[]):any;

    }
}

declare module 'node-sync-ipc' {
  export = NodeSyncIpc;
}

