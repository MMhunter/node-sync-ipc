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

uv_loop_t * loop_s = NULL;

uv_pipe_t * server_handle = NULL;

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

    fprintf(stderr,"stop loop");
//    uv_stop(loop_s);
    exit(0);
}

void walk_cb(uv_handle_t* handle, void* arg){
    fprintf(stdout,"alive handle type %d\n", ((uv_pipe_t *) handle)->type);
    uv_close(handle,stop_loop);

}

void on_remove_sock(uv_fs_t * req){

    uv_close((uv_handle_t *) server_handle,NULL);
    exit(0);
    //uv_walk(loop_s,walk_cb,NULL);

}



void remove_sock(int sig) {

//    if (r != 0) {
//        fprintf(stderr, "remove sock error %s\n", uv_err_name(r));
//    }
    uv_close((uv_handle_t *) server_handle,stop_loop);

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
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {

    if(buf->base[0] == ' '){
        free(buf->base);
        return;
    }

    if (nread > 0) {
        fprintf(stdout,"Server Read %ld: %s \n",nread,buf->base);
        write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write(&req->req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        if (nread != UV_EOF)
            fprintf(stderr, "Server Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}


void createL(){

    loop_s = uv_default_loop();
    uv_loop_init(loop_s);
    server_handle  = new uv_pipe_t;
    uv_pipe_init(loop_s, server_handle,0);
    signal(SIGINT, remove_sock);
//    struct sockaddr_in bind_addr;
//    uv_ip4_addr("0.0.0.0", 7000, &bind_addr);
//    uv_pipe_bind(server_handle, (const struct sockaddr *)&bind_addr, 0);
    int r;
    if ((r = uv_pipe_bind(server_handle, "echo.sock"))) {
        fprintf(stderr, "Bind error %s\n", uv_err_name(r));
        return;
    }
    if ((r = uv_listen((uv_stream_t*) server_handle, 128, on_new_connection))) {
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return;
    }

    fprintf(stderr, "Bind success \n");

    //uv_run(loop_s,UV_RUN_DEFAULT);

//    fprintf(stderr,"stop loop\n"");
//    uv_stop(loop_s);
//
//    fprintf(stderr, "close loop \n");
//    r = uv_loop_close(loop_s);
//    if (r != 0) {
//        fprintf(stderr, "close loop err %s\n", uv_err_name(r));
//    }
    //uv_walk(loop_s,walk_cb,NULL);
}

void write_cb(uv_write_t* req, int status) {
   //uv_close((uv_handle_t *)req, NULL);
   if (status < 0) {
           fprintf(stderr, "Client Write error %s\n", uv_err_name(status));
   }
}

//NAN_METHOD(listen) {
//
//    //signal(SIGINT, remove_sock);
//    uv_work_t *req = new uv_work_t;
//    uv_queue_work(uv_default_loop(), req, createL, NULL);
//
//    return;
//
//}
//
//NAN_METHOD(connect){
//
//    uv_work_t *req = new uv_work_t;
//    uv_queue_work(uv_default_loop(), req, connectL, NULL);
//
//    return;
//
//
//}



void stop_server(){

    fprintf(stderr, "wtf server1 \n");
    //uv_read_stop((uv_stream_t *) server_handle);
    //uv_close((uv_handle_t *) server_handle, stop_loop);
//    fprintf(stderr, "delete file \n");
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

void init(Local<Object> exports) {
//  NODE_SET_METHOD(exports, "hello", Method);
//  NODE_SET_METHOD(exports, "fuck", SetMethod);
    Nan::SetMethod(exports, "stop", stop);
    createL();

}

NODE_MODULE(NODE_GYP_MODULE_NAME, init)


void on_new_connection(uv_stream_t *server, int status) {

    if (status == -1) {
        // error!
        return;
    }
    fprintf(stderr,"connected %d \n", status);
    uv_pipe_t* client = new uv_pipe_t;
    uv_pipe_init(loop_s,client,0);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        fprintf(stderr,"accepted \n");
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    }
    else {
        fprintf(stderr,"read start error \n");
        uv_close((uv_handle_t*) client, NULL);
    }
}

}  // namespace demo




