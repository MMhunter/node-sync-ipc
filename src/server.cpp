#include <node.h>
#include <uv.h>
#include <nan.h>
#include <vector>

#define DEBUG 0

namespace server {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Number;

typedef struct {
    uv_pipe_t* client_handle;
    int pid;
} client;

int64_t counter = 0;

uv_loop_t * loop_s = NULL;

uv_pipe_t * server_handle = NULL;

std::vector<client *> clients;

Nan::Callback pipe_callback;


void add_client(int pid, uv_pipe_t* client_handle){

    client * c = new client;

    c->client_handle = client_handle;
    c->pid = pid;

    clients.push_back(c);

}

 const char* ToCString(const String::Utf8Value& value) {
          return *value ? *value : "<string conversion failed>";
    }

void delete_client(uv_pipe_t* ch){


     for(std::vector<client *>::iterator it=clients.begin(); it!=clients.end(); ){
        if((*it)->client_handle == ch)
        {
            it = clients.erase(it);
        }
        else
        {
            ++it;
        }
     }
}

client* get_client(int pid){
    client * c = NULL;

    int count = clients.size();
    for (int i = 0; i < count;i++)
    {
        if(clients.at(i)->pid == pid){
            return clients.at(i);
        }
    }
    return c;
}

client* get_client(uv_pipe_t* ch){


     for(std::vector<client *>::iterator it=clients.begin(); it!=clients.end(); ){
        if((*it)->client_handle == ch)
        {
            return *it;
        }
        it++;
     }

     return NULL;
}


typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void on_new_connection(uv_stream_t *q, int status);

void stop_server();


void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *) malloc(suggested_size);
  buf->len = suggested_size;
}

void stop_loop(uv_handle_t * handle){

    if(DEBUG) fprintf(stderr,"stop loop");
//    uv_stop(loop_s);
    exit(0);
}

void walk_cb(uv_handle_t* handle, void* arg){
    if(DEBUG) fprintf(stdout,"alive handle type %d\n", ((uv_pipe_t *) handle)->type);
    uv_close(handle,stop_loop);

}

void on_remove_sock(uv_fs_t * req){

    uv_close((uv_handle_t *) server_handle,NULL);
    //delete pipeEmitter;
    for(std::vector<client *>::iterator it=clients.begin(); it!=clients.end(); ){
        uv_close((uv_handle_t *) (*it)->client_handle, NULL);
        it = clients.erase(it);
    }

    exit(0);
}



void remove_sock(int sig) {

//    if (r != 0) {
//        if(DEBUG) fprintf(stderr, "remove sock error %s\n", uv_err_name(r));
//    }
//    uv_close((uv_handle_t *) server_handle,stop_loop);
    if(uv_loop_alive(loop_s)){
        uv_close((uv_handle_t *) server_handle,stop_loop);
    }
    else{
        exit(0);
    }

    //uv_close((uv_handle_t *) server_handle,stop_loop);
}

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void echo_write(uv_write_t *req, int status) {
    if (status < 0) {
        if(DEBUG) fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void getPidAndMessage(char* raw, int * pid, char ** message){


    int count = strlen(raw);
    int i = 0;
    while(raw[i] != '#' && raw[i]){
        i++;
        if(i >= count){
            break;
        }
    }

    if(i >= count){
        return;
    }

    char * pidSub = new char[i];
    memcpy( pidSub,raw, i );
    pidSub[i] = '\0';
    *pid = atoi(pidSub);



    *message = new char[strlen(raw)-i-1];
    memcpy(*message,raw+i+1,strlen(raw)-i-1);
    (*message)[strlen(raw)-i-1] = '\0';

}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {





    if (nread > 0) {
//        if(buf->base[0] == ' '){
//            free(buf->base);
//            return;
//        }
        if(DEBUG) fprintf(stdout,"Server Read %ld: %s \n",nread,buf->base);
//        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
//        req->buf = uv_buf_init(buf->base, nread);
//        uv_write(&req->req, client, &req->buf, 1, echo_write);
        int pid;
        char * message;

        getPidAndMessage(buf->base,&pid,&message);
        if(get_client((uv_pipe_t *) client) == NULL){
            add_client(pid ,(uv_pipe_t *) client);
            if(DEBUG) fprintf(stdout,"clients count %d\n", clients.size());
        }
        Nan::HandleScope scope;
        v8::Local<v8::Value> argv[2];

        argv[0] = Nan::New<v8::Number>(pid);

        argv[1] = Nan::New(message).ToLocalChecked();

        pipe_callback.Call(2, argv);
        return;
    }

    if (nread < 0) {
        if(DEBUG) fprintf(stderr, "Server Read error %s\n", uv_err_name(nread));
        if (nread != UV_EOF)
            if(DEBUG) fprintf(stderr, "Server Read error %s\n", uv_err_name(nread));
        delete_client((uv_pipe_t *) client);
        uv_close((uv_handle_t*) client, NULL);
        if(DEBUG) fprintf(stdout,"clients count after delete %d\n", clients.size());
    }

    free(buf->base);
}

int getpid(){
    return GetCurrentProcessId();
}


void createL(){

    loop_s = uv_default_loop();
    uv_loop_init(loop_s);
    server_handle  = new uv_pipe_t;
    uv_pipe_init(loop_s, server_handle,0);

//    #ifdef _WIN32
//    #else
//    signal(SIGINT, remove_sock);
//    #endif


//    struct sockaddr_in bind_addr;
//    uv_ip4_addr("0.0.0.0", 7000, &bind_addr);
//    uv_pipe_bind(server_handle, (const struct sockaddr *)&bind_addr, 0);
    int r;

    char * pipename;

    int pid = getpid();

    int pid_digits = 0;

    int temp = pid;

    while(temp>0){
       pid_digits += 1;
       temp /= 10;
    }
    #ifdef _WIN32
    const char* pipename_preset = "\\\\.\\pipe\\nodePipe";
    #else
    const char* pipename_preset = "nodePipe";
    #endif

    pipename = new char[strlen(pipename_preset)+pid_digits];
    sprintf(pipename,"%s%d",pipename_preset,pid);

    if ((r = uv_pipe_bind(server_handle, pipename))) {
        if(DEBUG) fprintf(stderr, "Bind error %s\n", uv_err_name(r));
        return;
    }
    if ((r = uv_listen((uv_stream_t*) server_handle, 128, on_new_connection))) {
        if(DEBUG) fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return;
    }
}

void write_cb(uv_write_t* req, int status) {
   //uv_close((uv_handle_t *)req, NULL);
   if (status < 0) {
           if(DEBUG) fprintf(stderr, "Client Write error %s\n", uv_err_name(status));
   }
}




void stop_server(){

    if(DEBUG) fprintf(stderr, "wtf server1 \n");
    //uv_read_stop((uv_stream_t *) server_handle);
    //uv_close((uv_handle_t *) server_handle, stop_loop);
//    if(DEBUG) fprintf(stderr, "delete file \n");
//    uv_fs_t req;
//    uv_loop_t* loop = new uv_loop_t;
//    uv_loop_init(loop);
//    int r = uv_fs_unlink(loop, &req, "echo.sock", on_remove_sock);
//    uv_run(loop,UV_RUN_DEFAULT);
//    uv_loop_close(loop);

    on_remove_sock(NULL);
}

NAN_METHOD(stop){
    //exit(0);
    //server_handle = NULL;
    stop_server();
    return;

}

NAN_METHOD(write){

    if (info.Length() > 1) {
        if (info[1]->IsString() && info[0]->IsNumber()) {
          String::Utf8Value str(info[1]->ToString());


          char * buffer = strdup(ToCString(str));

          write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));

          req->buf = uv_buf_init(buffer, strlen(buffer)+1);

          int pid = (int) Local<Number>::Cast(info[0])->NumberValue();

          client* c = get_client(pid);

          if(c != NULL){
            uv_write(&req->req,(uv_stream_t *) c->client_handle , &req->buf, 1, write_cb);
          }

          return;
        }
    }
}


NAN_METHOD(bindPipeListener){

    if (!info[0]->IsFunction()) {
        Nan::ThrowTypeError("First argument must be a function");
        return;
    }

    pipe_callback.Reset(info[0].As<v8::Function>());


}


NAN_MODULE_INIT(init) {
//  NODE_SET_METHOD(exports, "hello", Method);
//  NODE_SET_METHOD(exports, "fuck", SetMethod);
    Nan::SetMethod(target, "stop", stop);
    Nan::SetMethod(target, "write", write);
    Nan::SetMethod(target, "bindPipeListener", bindPipeListener);
    createL();
    //Emitter::Init(target);

}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)


void on_new_connection(uv_stream_t *server, int status) {

    if (status == -1) {
        // error!
        return;
    }
    if(DEBUG) fprintf(stderr,"connected %d \n", status);
    uv_pipe_t* client = new uv_pipe_t;
    uv_pipe_init(loop_s,client,0);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        if(DEBUG) fprintf(stderr,"accepted \n");
//client_handle_default = (uv_stream_t*) client;
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        if(DEBUG) fprintf(stderr,"read start error \n");
        uv_close((uv_handle_t*) client, NULL);
    }
}

}  // namespace demo



