#include <node.h>
#include <uv.h>
#include <nan.h>



namespace demo {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Number;

int test = 0;

int64_t counter = 0;


uv_loop_t *loop = NULL;

uv_loop_t * serverLoop = new uv_loop_t;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void on_new_connection(uv_stream_t *q, int status);


void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
  buf->base = (char *) malloc(suggested_size);
  buf->len = suggested_size;
}

void remove_sock(int sig) {
    uv_fs_t req;
    uv_fs_unlink(loop, &req, "echo.sock", NULL);
    exit(0);
}

void wait_for_a_while(uv_idle_t* handle) {

    test ++ ;
    if(test > 10e6){
     uv_idle_stop(handle);
    }

}

void free_write_req(uv_write_t *req) {
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void echo_write(uv_write_t *req, int status) {
    if (status < 0) {
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    else{
        fprintf(stdout, "Write success\n");
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {

    if(buf->base[0] == ' '){
        free(buf->base);
        return;
    }

    if (nread > 0) {
        fprintf(stdout,"Read %ld: %s \n",nread,buf->base);
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write(&req->req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}


void Method(const FunctionCallbackInfo<Value>& args) {
    test = 10e6+1;
    if(loop == NULL){
        loop = new uv_loop_t;
        uv_loop_init(loop);
    }


    Isolate* isolate = args.GetIsolate();

    uv_idle_t idler;

    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, wait_for_a_while);

    printf("Idling...\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
    loop = NULL;

    args.GetReturnValue().Set(Number::New(isolate, test));
}

void SetMethod(const FunctionCallbackInfo<Value>& args) {

  Isolate* isolate = args.GetIsolate();


    if(loop == NULL){
        loop = new uv_loop_t;
        uv_loop_init(loop);
    }

    uv_idle_t idler;

    uv_idle_init(loop, &idler);
    uv_idle_start(&idler, wait_for_a_while);

    printf("Idling...\n");
    uv_run(loop, UV_RUN_DEFAULT);

    uv_loop_close(loop);
    free(loop);
    loop = NULL;

    args.GetReturnValue().Set(Number::New(isolate, test));
}

void createL(uv_work_t* req){

    uv_loop_t* loop = serverLoop;
    uv_loop_init(loop);
    uv_pipe_t* server  = new uv_pipe_t;
    uv_pipe_init(loop, server, 0);
    fprintf(stderr,"test");
    //signal(SIGINT, remove_sock);
    int r;
    if ((r = uv_pipe_bind(server, "echo.sock"))) {
        fprintf(stderr, "Bind error %s\n", uv_err_name(r));
        return;
    }
    if ((r = uv_listen((uv_stream_t*) server, 128, on_new_connection))) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return;
    }

    uv_run(loop,UV_RUN_DEFAULT);
    return;
}

void write_cb(uv_write_t* req, int status) {
   //uv_close((uv_handle_t *)req, NULL);
   if (status < 0) {
           fprintf(stderr, "Client Write error %s\n", uv_err_name(status));
   }
   else fprintf(stderr, "Client Write sucess %s\n", uv_err_name(status));
}


void on_client_connected(uv_connect_t* req, int status){

    if (status != 0) {
        // error!
        return;
    }

    fprintf(stdout,"client connected %d \n", status);

    uv_write_t write_req;


    char buffer[7] = "stasda";


    uv_buf_t buf = uv_buf_init(buffer, sizeof buffer);

    uv_write(&write_req, req->handle, &buf, 1, write_cb);

    uv_read_start((uv_stream_t*) req->handle, alloc_buffer, echo_read);

}

void connectL(uv_work_t* r){

    uv_loop_t* loop = new uv_loop_t;
    uv_loop_init(loop);

    uv_connect_t* req = new uv_connect_t;
    uv_pipe_t* client = new uv_pipe_t;
    uv_pipe_init(loop, client,0);

    uv_pipe_connect(req, client, "echo.sock", on_client_connected);

    uv_run(loop,UV_RUN_DEFAULT);
    return;
}


NAN_METHOD(listen) {

    //signal(SIGINT, remove_sock);
    uv_work_t *req = new uv_work_t;
    uv_queue_work(uv_default_loop(), req, createL, NULL);

    return;

}

NAN_METHOD(connect){

    uv_work_t *req = new uv_work_t;
    uv_queue_work(uv_default_loop(), req, connectL, NULL);

    return;


}


void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "hello", Method);
  NODE_SET_METHOD(exports, "fuck", SetMethod);
  Nan::SetMethod(exports, "listen", listen);
  Nan::SetMethod(exports, "connect", connect);
  //NODE_SET_METHOD(exports, "listen", );
}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)


void on_new_connection(uv_stream_t *server, int status) {

    if (status == -1) {
        // error!
        return;
    }
    fprintf(stderr,"connected %d \n", status);
    uv_pipe_t* client = new uv_pipe_t;
    uv_pipe_init(serverLoop,client,0);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

}  // namespace demo




