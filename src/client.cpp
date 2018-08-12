#include <node.h>
#include <uv.h>
#include <nan.h>
#include <map>
#include "basic.h"
#include "utils.cpp"


namespace client {

  using v8::FunctionCallbackInfo;
  using v8::Isolate;
  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::Value;
  using v8::Number;

  /* message buffer received from server */
  char * returnValue;

  const char * pipeFile;

  /* the buffer's total length to be read from server */
  int readFullLen;

  int parts;

  char** writings;

  /* the client-side uv_pipe handle */
  uv_pipe_t* client_handle;

  /* uv_loop created inside default loop*/
  uv_loop_t * ipc_loop = NULL;

  const char* ToCString(const String::Utf8Value& value) {
  		return *value ? *value : "<string conversion failed>";
  }

  void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *) malloc(suggested_size);
    buf->len = suggested_size;
  }




  void extractFirstInteger(char* origin, int * firstInt, char ** rest){

      int count = strlen(origin);
      int i = 0;
      while(origin[i] != '#' && origin[i]){
          i++;
          if(i >= count){
              break;
          }
      }

      if(i >= count){
          return;
      }

      char * pidSub = (char *) malloc(sizeof(char) * (i+1));
      memcpy( pidSub,origin, i );
      pidSub[i] = '\0';
      *firstInt = atoi(pidSub);
      free(pidSub);

      *rest = (char *) malloc(sizeof(char) * (strlen(origin)-i));
      memcpy(*rest,origin+i+1,strlen(origin)-i-1);
      (*rest)[strlen(origin)-i-1] = '\0';

  }

  void on_client_closed(uv_handle_t * client){

      free(client);
  }

  void on_read_value(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {

      if (nread > 0) {

          char * buffer = (char *) malloc(nread+1);
          memcpy(buffer,buf->base,nread);
          buffer[nread] = 0;

          if(DEBUG) fprintf(stdout,"Client Read %ld: %s \n",nread,buffer);

          char *body;

          if(returnValue == NULL){
              extractFirstInteger(buffer,&readFullLen,&body);
              returnValue = (char *) malloc(readFullLen+1);
              for(int i = 0; i <  readFullLen + 1; i++){
                  returnValue[i] = 0;
              }
              free(buffer);
          }
          else{
              body = buffer;
          }

          strcat(returnValue, body);

          free(body);

          if(DEBUG) fprintf(stdout,"client received %d / %d \n", strlen(returnValue), readFullLen);

          if(strlen(returnValue) == readFullLen){
              if(DEBUG) fprintf(stdout,"Client Read Finished %ld: %s \n",nread,returnValue);

              uv_read_stop(client);
              uv_close((uv_handle_t *) client,on_client_closed);
          }
      }

      if (nread < 0) {
          if (nread != UV_EOF)
              if(DEBUG) fprintf(stderr, "Read error %s\n", uv_err_name(nread));
          uv_close((uv_handle_t*) client, on_client_closed);
      }

      free(buf->base);
  }

  void on_client_connected(uv_connect_t* req, int status){

      if (status != 0) {
          // error!
          if(DEBUG) fprintf(stdout,"client connection error %s\n", uv_err_name(status));

      }

      else{

          if(DEBUG) fprintf(stdout,"client connected %d \n", status);

          returnValue = NULL;

          uv_read_start((uv_stream_t*) req->handle, alloc_buffer, on_read_value);

          debug("start send data, data has %d parts\n", parts);

          utils::writeData((uv_stream_t*) client_handle, writings, parts);

      }


      free(req);

  }

  /* connect to server and send message.
    	Here we create a new uv_loop inside default loop to block the default loop. */
  void connect_and_send(){

      ipc_loop = (uv_loop_t *) malloc(sizeof(uv_loop_t));

      uv_loop_init(ipc_loop);

      uv_connect_t* req = (uv_connect_t *) malloc(sizeof(uv_connect_t));

      client_handle = (uv_pipe_t *) malloc(sizeof(uv_pipe_t));

      uv_pipe_init(ipc_loop, client_handle,0);

      if(DEBUG) fprintf(stdout,"connecting to server %s\n", pipeFile);

      uv_pipe_connect(req, client_handle, pipeFile, on_client_connected);

      /* the next statement will block default loop until client_handle stops. */
      uv_run(ipc_loop,UV_RUN_DEFAULT);

      if(DEBUG) fprintf(stdout,"connect loop successfully closed\n");

      uv_loop_close(ipc_loop);

      free(ipc_loop);

  }

  /* Send sync method */
  NAN_METHOD(send){

      if (info.Length() > 1) {

          if (info[0]->IsString()) {

             String::Utf8Value str(info[0]->ToString());

             pipeFile = ToCString(str);

             if (info[1]->IsString()) {

                   String::Utf8Value str(info[0]->ToString());

                   const char* s = ToCString(str);

                   debug("get data to send, data has %d parts\n", info.Length() - 1);
                   char** strings = new char *[info.Length()-1];
                   for (int i = 0; i < info.Length() - 1; i ++) {
                     v8::String::Utf8Value str(info[i+1]->ToString());
                     char * value = strdup(ToCString(str));
                     strings[i] = value;
                   }

                   writings = strings;

                   parts = info.Length() - 1;

                   connect_and_send();

                   delete[] strings;

                   if(returnValue != NULL){
                     info.GetReturnValue().Set(Nan::New<v8::String>(returnValue).ToLocalChecked());
                     free(returnValue);

                     returnValue = NULL;

                     if(DEBUG) fprintf(stdout, "send Sync result success \n");
                     return;
                   }
                   else{
                     Nan::ThrowError("Failed To Send Sync");
                   }

             }

          }

      }
      info.GetReturnValue().Set(Nan::Null());
  }

  NAN_MODULE_INIT(init) {

    Nan::SetMethod(target, "sendSync", send);

  }

  NODE_MODULE(NODE_GYP_MODULE_NAME, init)
}