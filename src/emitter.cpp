
#include "emitter.h"

namespace server{

    Nan::Persistent<v8::Function> Emitter::constructor;

    Emitter* pipEmitter = NULL;

    Emitter::Emitter() {
    }

    Emitter::~Emitter() {
    }


    void Emitter::Init(v8::Local<v8::Object> target) {
      // Prepare constructor template
        v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
        tpl->SetClassName(Nan::New<v8::String>("Emitter").ToLocalChecked());
        tpl->InstanceTemplate()->SetInternalFieldCount(1);

        SetPrototypeMethod(tpl, "call_emit", Emitter::CallEmit);

        constructor.Reset(tpl->GetFunction());

        //Emitter* obj = new Emitter();
        fprintf(stdout,"wtf emitter");
        Nan::Set(target, Nan::New("Emitter").ToLocalChecked(), tpl->GetFunction());
        //Nan::SetMethod(target, "emitter", Emitter.Get);
    }

    NAN_METHOD(Emitter::New) {
      if (info.IsConstructCall()) {
        if(pipEmitter == NULL){
            pipEmitter = new Emitter();
        }
        pipEmitter->Wrap(info.This());
        info.GetReturnValue().Set(info.This());
      } else {
        v8::Local<v8::Function> cons = Nan::New<v8::Function>(constructor);
        info.GetReturnValue().Set(Nan::NewInstance(cons).ToLocalChecked());
      }
    }

    NAN_METHOD(Emitter::CallEmit) {
      v8::Local<v8::Value> argv[1] = {
        Nan::New("event").ToLocalChecked(),  // event name
      };

      Nan::MakeCallback(info.This(), "emit", 1, argv);
      info.GetReturnValue().SetUndefined();
    }


//    NAN_METHOD(Emitter::Get) {
//        if(pipEmitter == NULL){
//            pipEmitter = new Emitter();
//        }
//        pipEmitter->Wrap(info.This());
//        info.GetReturnValue().Set(info.This());
//    }

}

