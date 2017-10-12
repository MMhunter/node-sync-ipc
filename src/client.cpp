#include <node.h>
#include <uv.h>
#include <nan.h>
#include <string.h>



namespace demo {

    using v8::FunctionCallbackInfo;
    using v8::Isolate;
    using v8::Local;
    using v8::Object;
    using v8::String;
    using v8::Value;
    using v8::Number;

    int test = 0;

    int connected = 1;

    uv_loop_t *loop = NULL;
    uv_pipe_t* client_handle;

    typedef struct {
        uv_write_t req;
        uv_buf_t buf;
    } write_req_t;


    const char* ToCString(const String::Utf8Value& value) {
          return *value ? *value : "<string conversion failed>";
    }

    void on_new_connection(uv_stream_t *q, int status);


    void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
      buf->base = (char *) malloc(suggested_size);
      buf->len = suggested_size;
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
        free_write_req(req);
    }

    void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {

        if(buf->base[0] == ' '){
            free(buf->base);
            return;
        }

        if (nread > 0) {
            fprintf(stdout,"Client Read %ld: %s \n",nread,buf->base);
//            write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));
//            req->buf = uv_buf_init(buf->base, nread);
//            uv_write(&req->req, client, &req->buf, 1, echo_write);
        }

        if (nread < 0) {
            if (nread != UV_EOF)
                fprintf(stderr, "Read error %s\n", uv_err_name(nread));
            uv_close((uv_handle_t*) client, NULL);
        }

        free(buf->base);
    }

    void write_cb(uv_write_t* req, int status) {
       //uv_close((uv_handle_t *)req, NULL);
       if (status != 0) {
               fprintf(stderr, "Client Write error %s\n", uv_err_name(status));
       }
       else fprintf(stderr, "Client Write success %s\n", uv_err_name(status));
       free_write_req(req);
    }


    void on_client_connected(uv_connect_t* req, int status){

        if (status != 0) {
            // error!
            fprintf(stdout,"client connection error %s\n", uv_err_name(status));
            return;
        }

        fprintf(stdout,"client connected %d \n", status);



        uv_read_start((uv_stream_t*) req->handle, alloc_buffer, echo_read);

        connected = 1;
    }


    void wait_for_a_connection(uv_idle_t* handle) {

        if (connected){
             uv_idle_stop(handle);
        }

    }

    void connectL(uv_work_t* r){

       loop = new uv_loop_t;
       uv_loop_init(loop);
       uv_connect_t* req = new uv_connect_t;
       client_handle = new uv_pipe_t;

       uv_pipe_init(loop, client_handle,0);

       uv_pipe_connect(req, client_handle, "echo.sock", on_client_connected);

       uv_run(loop,UV_RUN_DEFAULT);

       uv_loop_close(loop);
        return;
    }

    NAN_METHOD(connect){





        uv_work_t *req = new uv_work_t;

        uv_queue_work(uv_default_loop(),req,connectL,NULL);
//       uv_idle_t idler;
//
//       uv_idle_init(uv_default_loop(), &idler);
//       uv_idle_start(&idler, wait_for_a_while);
//
//       uv_run(loop,UV_RUN_DEFAULT);
//
//       uv_loop_close(loop);

       return;
    }

    NAN_METHOD(stop){

        fprintf(stderr, "wtf client \n");
        uv_shutdown_t* shutdown_req = new uv_shutdown_t;
        int r = uv_shutdown(shutdown_req, (uv_stream_t *) client_handle, NULL);
        if (r != 0) {
               fprintf(stderr, "shutdown err %s\n", uv_err_name(r));
        }
        uv_close((uv_handle_t *) client_handle,NULL);
        uv_loop_close(loop);
        //exit(0);
        return;

    }

    NAN_METHOD(write){
        fprintf(stdout,"attempt to write\n");
        if (info.Length() > 0) {
            if (info[0]->IsString()) {
              String::Utf8Value str(info[0]->ToString());


              char * buffer = new char[strlen(ToCString(str))];

              strcpy(buffer,ToCString(str));

              write_req_t *req = (write_req_t*) malloc(sizeof(write_req_t));

              req->buf = uv_buf_init(buffer, strlen(buffer));

              fprintf(stdout,"attempt to write :%s, size %lu",buffer, sizeof buffer);

              uv_write(&req->req, (uv_stream_t *)client_handle, &req->buf, 1, write_cb);

              return;
            }
        }
    }


    void init(Local<Object> exports) {
      Nan::SetMethod(exports, "stop", stop);
      Nan::SetMethod(exports, "connect", connect);
      Nan::SetMethod(exports, "write", write);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, init)



}  // namespace demo




