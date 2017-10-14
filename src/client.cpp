#include <node.h>
#include <uv.h>
#include <nan.h>
#include <string.h>
#ifdef _WIN32
char* pipename = "\\\\.\\pipe\\nodePipe";
#else
char* pipename = "nodePipe";
#endif


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

    char testValue[100] = "1231";

    uv_pipe_t* client_handle;

    uv_loop_t * ipc_loop = NULL;

    typedef struct {
        uv_write_t req;
        uv_buf_t buf;
    } write_req_t;


    char* subString (const char* input, int offset, int len, char* dest)
    {
      int input_len = strlen (input);

      if (offset + len > input_len)
      {
         return NULL;
      }

      strncpy (dest, input + offset, len);

      dest[len+1] = 0;

      return dest;
    }

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
        if(test > 100){
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


        fprintf(stdout,"Client Read start \n");

        if(buf->base[0] == ' '){
            free(buf->base);
            return;
        }

        if (nread > 0) {
            fprintf(stdout,"Client Read %ld: %s \n",nread,buf->base);
            if(buf->base[0] == 'T' && buf->base[1] == ':'){
                subString(buf->base,2,strlen(buf->base)-2,testValue);
                uv_read_stop(client);
//                uv_close((uv_handle_t*) client, NULL);
            }

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
       //free_write_req(req);
    }


    void on_client_connected(uv_connect_t* req, int status){

        if (status != 0) {
            // error!
            fprintf(stdout,"client connection error %s\n", uv_err_name(status));
            return;
        }

        fprintf(stdout,"client connected %d \n", status);

        uv_read_start((uv_stream_t*) req->handle, alloc_buffer, echo_read);

//        write_req_t *wreq = (write_req_t*) malloc(sizeof(write_req_t));
//
//        wreq->buf = uv_buf_init("test\n", 5);
//
//        uv_write(&wreq->req, (uv_stream_t *)client_handle, &wreq->buf, 1, write_cb);

        connected = 1;
    }


    void wait_for_value(uv_idle_t* handle) {

        if (testValue != NULL){
             uv_idle_stop(handle);
        }

    }

//    void connectL(uv_work_t* r){
//
//
//
//       uv_connect_t* req = new uv_connect_t;
//
//       client_handle = new uv_pipe_t;
//
//       uv_pipe_init(uv_default_loop(), client_handle,0);
//
//       uv_pipe_connect(req, client_handle, pipename, on_client_connected);
//
////       uv_run(loop,UV_RUN_DEFAULT);
////
////       uv_loop_close(loop);
//
//       return;
//    }

    void connect(){

            ipc_loop = new uv_loop_t;

            uv_loop_init(ipc_loop);
    //        uv_queue_work(uv_default_loop(),req,connectL,NULL);
           uv_idle_t idler;

           uv_idle_init(ipc_loop, &idler);
           uv_idle_start(&idler, wait_for_a_while);

           uv_connect_t* req = new uv_connect_t;

           client_handle = new uv_pipe_t;

           uv_pipe_init(ipc_loop, client_handle,0);

           uv_pipe_connect(req, client_handle, pipename, on_client_connected);

           uv_run(ipc_loop,UV_RUN_DEFAULT);

           fprintf(stdout,"connect loop successfully closed\n");

           uv_loop_close(ipc_loop);

           //delete ipc_loop;

           //ipc_loop = NULL;
        }

    NAN_METHOD(connect){

       connect();

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

              req->buf = uv_buf_init(buffer, strlen(buffer)+1);

              fprintf(stdout,"attempt to write :%s, size %lu \n",buffer, sizeof buffer);

              uv_write(&req->req, (uv_stream_t *)client_handle, &req->buf, 1, write_cb);

              return;
            }
        }
    }

    NAN_METHOD(getValue){

        fprintf(stdout,"attempt to getValue\n");

        connect();

        info.GetReturnValue().Set(Nan::New<v8::String>(testValue).ToLocalChecked());
    }


    void init(Local<Object> exports) {
      Nan::SetMethod(exports, "stop", stop);
      Nan::SetMethod(exports, "getValue", getValue);
      //Nan::SetMethod(exports, "connect", connect);
      Nan::SetMethod(exports, "write", write);
    }

    NODE_MODULE(NODE_GYP_MODULE_NAME, init)



}  // namespace demo




