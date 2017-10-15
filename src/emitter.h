
#include <nan.h>
#include <node.h>
#include <node_object_wrap.h>


namespace server{

    class Emitter : public node::ObjectWrap {

        public:
            Emitter();
            ~Emitter();
            static void Init(v8::Local<v8::Object> target);

            static NAN_METHOD(Get);

        private:

            static NAN_METHOD(New);
            static NAN_METHOD(CallEmit);
            static Nan::Persistent<v8::Function> constructor;
            static Emitter* pipeEmitter;

    };



}