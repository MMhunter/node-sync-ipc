#include <node.h>
#include <uv.h>
#include <nan.h>
#include <map>
#include "basic.h"
#include "utils.cpp"

namespace server {
  using std::map;

  const char* ToCString(const v8::String::Utf8Value& value) {
    return *value ? *value : "<string conversion failed>";
  }

  void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = (char *) malloc(suggested_size);
    buf->len = suggested_size;
  }

  class Connection : public Nan::ObjectWrap {
    public:

      static NAN_MODULE_INIT(Init) {
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("Connection").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        Nan::SetPrototypeMethod(tpl, "write", Write);
        Nan::SetPrototypeMethod(tpl, "onMessage", onMessageBind);

        constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
      }

      static map<uv_pipe_t*, Connection *> connections;

      Connection() {
      }

      void setHandle(uv_pipe_t* _handle) {
        handle = _handle;
        Connection::connections[handle] = this;
      }

      void onReceivedData(char * buffer) {
        if(!onMessage->IsEmpty()) {
          Nan::HandleScope scope;
          v8::Local<v8::Value> argv[1];
          argv[0] = Nan::New(buffer).ToLocalChecked();
          debug("on Message %s\n",buffer);
          onMessage->Call(1, argv);
          free(buffer);
        }
      }

      static v8::Local<v8::Object> newInstance(uv_pipe_t* handle){
        v8::Local<v8::Function> cons = Nan::New(constructor());
        v8::Local<v8::Value> argv[1] = {};
        v8::Local<v8::Object> instance = Nan::NewInstance(cons, 0, argv).ToLocalChecked();
        Connection* obj = Nan::ObjectWrap::Unwrap<Connection>(instance);
        obj->setHandle(handle);
        obj->instance = instance;
        return instance;
      }

      uv_pipe_t* handle = NULL;

      static void onClientClosed(uv_handle_t * handle){
         Connection* connection = getConnection((uv_pipe_t *) handle);
         if (connection != NULL) {
            connections.erase(connection->handle);
            free(handle);
            handle = NULL;
         }
         debug("connection closed");
      }

      static Connection* getConnection(uv_pipe_t * handle) {
         map<uv_pipe_t*, Connection *>::iterator iter;
         iter = connections.find(handle);
         if(iter != connections.end()) {
            return iter->second;
         } else {
            return NULL;
         }
      }

      static void onReadData(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf) {

          Connection* connection = getConnection((uv_pipe_t *) handle);

          if (connection == NULL) {
             return;
          }

          if (nread > 0) {
              char * buffer = (char *) malloc(nread+1);
              memcpy(buffer,buf->base,nread);
              buffer[nread] = 0;
              debug("Server Read Length %ld \n",nread);
              connection->onReceivedData(buffer);
          }
          else if (nread < 0) {
              debug("Server Read error %s\n", uv_err_name(nread));
              if (nread != UV_EOF)
                  error( "Server Read error %s\n", uv_err_name(nread));
              uv_close((uv_handle_t*) handle, onClientClosed);
          }

          free(buf->base);
      }

    private:

      ~Connection() {
        if(handle != NULL) {
          free(handle);
        }
      }

      v8::Local<v8::Object> instance;

      Nan::Callback* onMessage = new Nan::Callback();

      static NAN_METHOD(New) {
        if (info.IsConstructCall()) {
          Connection *obj = new Connection();
          obj->Wrap(info.This());
          info.GetReturnValue().Set(info.This());
        }
      }

      static NAN_METHOD(Write) {
        Connection* obj = Nan::ObjectWrap::Unwrap<Connection>(info.Holder());
        char** strings = new char*[info.Length()];
        for (int i = 0; i < info.Length(); i ++) {
          v8::String::Utf8Value str(info[i]->ToString());
          char * value = strdup(ToCString(str));
          strings[i] = value;
        }
        utils::writeData((uv_stream_t *) obj->handle, strings, info.Length());
        delete[] strings;
      }

      static NAN_METHOD(onMessageBind) {
        Connection* obj = Nan::ObjectWrap::Unwrap<Connection>(info.Holder());
        if (!info[0]->IsFunction()) {
            Nan::ThrowTypeError("First argument must be a function");
            return;
        }
        obj->onMessage->Reset(info[0].As<v8::Function>());
      }

      static inline Nan::Persistent<v8::Function> & constructor() {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
      }
  };

  class SyncIpcServer : public Nan::ObjectWrap {

    public:
      static NAN_MODULE_INIT(Init) {
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New("SyncIpcServer").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        Nan::SetPrototypeMethod(tpl, "getPipeFile", GetPipeFile);
        Nan::SetPrototypeMethod(tpl, "listen", Listen);
        Nan::SetPrototypeMethod(tpl, "stop", Stop);

        constructor().Reset(Nan::GetFunction(tpl).ToLocalChecked());
        Nan::Set(target, Nan::New("SyncIpcServer").ToLocalChecked(),
        Nan::GetFunction(tpl).ToLocalChecked());
      }

      SyncIpcServer(char* pipeFile) {
          _pipeFile = pipeFile;
      }

     private:

      ~SyncIpcServer() {
        if (serverHandle != NULL) {
          stopServer();
        }
      }

      char * _pipeFile;

      /* The server-side uv_pipe handle */
      uv_pipe_t * serverHandle = NULL;

      /* Callback function to be executed when messsage received from clients */
      Nan::Callback* onConnectionCallback = new Nan::Callback();

      static map<uv_pipe_t*, SyncIpcServer *> servers;

      static SyncIpcServer* getServer(uv_pipe_t * handle) {
         map<uv_pipe_t*, SyncIpcServer *>::iterator iter;
         iter = servers.find(handle);
         if(iter != servers.end()) {
            return iter->second;
         } else {
            return NULL;
         }
      }
      /* create server-side uv_pipe handle and start listening */
      int createServer(){

          serverHandle = (uv_pipe_t *) malloc(sizeof(uv_pipe_t));
          uv_pipe_init(uv_default_loop(), serverHandle, 0);

          int r = 0;

          debug("creating sync server with pipe file %s\n", _pipeFile);

          if ((r = uv_pipe_bind(serverHandle, _pipeFile))) {
              error("Bind error %s\n", uv_err_name(r));
              return r;
          }

          servers[serverHandle] = this;

          if ((r = uv_listen((uv_stream_t*) serverHandle, 128, onNewConnection))) {
              error("Listen error %s\n", uv_err_name(r));
              return r;
          }

          return r;
      }
      
      static void onNewConnection(uv_stream_t *handle, int status) {
           Nan::HandleScope scope;

           SyncIpcServer * server = getServer((uv_pipe_t*) handle);
      
          if (status == -1) {
            // error!
            if (DEBUG) fprintf(stderr, "connected error %d \n", status);
            return;
          }
          
          debug("connected %d \n", status);
          
          uv_pipe_t* client = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
          
          uv_pipe_init(uv_default_loop(), client, 0);

          v8::Local<v8::Object> connection = Connection::newInstance(client);

          debug("connection instance created \n");
          if(!server->onConnectionCallback->IsEmpty()) {
            v8::Local<v8::Value> argv[1] = {connection};
            server->onConnectionCallback->Call(1, argv);
          }

          debug("on connection callback invoked \n");

          if (uv_accept(handle, (uv_stream_t*)client) == 0) {
            debug("connection accepted \n");
            uv_read_start((uv_stream_t*)client, alloc_buffer, Connection::onReadData);
          }
          else {
            if (DEBUG) fprintf(stderr, "read start error \n");
            uv_close((uv_handle_t*)client, Connection::onClientClosed);
          }
      }

      /*callback when server is closed */
      void onServerStopped(uv_handle_t * server){
          free(server);
      }

      void stopServer(){

          debug("stopping server \n");

          if(serverHandle != NULL && uv_is_active((uv_handle_t *) serverHandle)){

              debug("current connected clients length is %d",uv_pipe_pending_count(serverHandle));

              #ifdef _WIN32
              #else
              // delete sock file on unix based systems.
              uv_fs_t* req = (uv_fs_t *) malloc(sizeof(uv_fs_t));
              int r = uv_fs_unlink(uv_default_loop(), req, _pipeFile, (uv_fs_cb) free);
              if(r != 0){
                  error("delete sock error %s\n", uv_err_name(r));
              }
              #endif
              uv_close((uv_handle_t *) serverHandle, (uv_close_cb) free);
          }
          serverHandle = NULL;
      }

      static NAN_METHOD(New) {
        if (info.IsConstructCall()) {
          v8::String::Utf8Value str(info[0]->ToString());
          char * value = strdup(ToCString(str));
          SyncIpcServer *obj = new SyncIpcServer(value);
          obj->Wrap(info.This());
          info.GetReturnValue().Set(info.This());
        } else {
          const int argc = 1;
          v8::Local<v8::Value> argv[argc] = {info[0]};
          v8::Local<v8::Function> cons = Nan::New(constructor());
          info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
        }
      }

      // return the pipeFile
      static NAN_METHOD(GetPipeFile) {
        SyncIpcServer* obj = Nan::ObjectWrap::Unwrap<SyncIpcServer>(info.Holder());
        info.GetReturnValue().Set(Nan::New<v8::String>(obj->_pipeFile).ToLocalChecked());
      }

      // start listen
      static NAN_METHOD(Listen) {
        SyncIpcServer* obj = Nan::ObjectWrap::Unwrap<SyncIpcServer>(info.Holder());
        if (!info[0]->IsFunction()) {
            Nan::ThrowTypeError("First argument must be a function");
            return;
        }

        obj->onConnectionCallback->Reset(info[0].As<v8::Function>());
        obj->createServer();

      }

      static NAN_METHOD(Stop) {
        SyncIpcServer* obj = Nan::ObjectWrap::Unwrap<SyncIpcServer>(info.Holder());
        obj->stopServer();
      }

      static inline Nan::Persistent<v8::Function> & constructor() {
        static Nan::Persistent<v8::Function> my_constructor;
        return my_constructor;
      }


  };

  map<uv_pipe_t*, Connection *> Connection::connections;
  map<uv_pipe_t*, SyncIpcServer *> SyncIpcServer::servers;
}

NAN_MODULE_INIT(Init) {
  server::SyncIpcServer::Init(target);
  server::Connection::Init(target);
}

NODE_MODULE(objectwrapper, Init)
