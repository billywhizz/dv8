#ifndef DV8_H
#define DV8_H

#include <assert.h>
#include <libplatform/libplatform.h>
#include <v8.h>
#include <unistd.h>
#include <uv.h>

namespace dv8 {

using v8::ArrayBuffer;
using v8::Context;
using v8::Object;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Message;
using v8::NewStringType;
using v8::ObjectTemplate;
using v8::Script;
using v8::ScriptOrigin;
using v8::String;
using v8::TryCatch;
using v8::V8;
using v8::Value;

typedef void* (*register_plugin)();

const char *ToCString(const String::Utf8Value &value);
void ReportException(Isolate *isolate, TryCatch *try_catch);
bool ExecuteString(Isolate *isolate, Local<String> source, Local<Value> name, bool report_exceptions);
MaybeLocal<String> ReadFile(Isolate *isolate, const char *name);
Local<Context> CreateContext(Isolate *isolate);
// Global Functions
void Print(const FunctionCallbackInfo<Value> &args);
void Version(const FunctionCallbackInfo<Value> &args);
void LoadModule(const FunctionCallbackInfo<Value>& args);

class ObjectWrap {
  public:
    ObjectWrap() {
        refs_ = 0;
    }

    virtual ~ObjectWrap() {
        if (persistent().IsEmpty()) return;
        assert(persistent().IsNearDeath());
        //persistent().ClearWeak();
        //persistent().Reset();
    }

    template <class T> static inline T *Unwrap(v8::Local<v8::Object> handle) {
        assert(!handle.IsEmpty());
        assert(handle->InternalFieldCount() > 0);
        void *ptr = handle->GetAlignedPointerFromInternalField(0);
        ObjectWrap *wrap = static_cast<ObjectWrap *>(ptr);
        return static_cast<T *>(wrap);
    }

    inline v8::Local<v8::Object> handle() {
        return handle(v8::Isolate::GetCurrent());
    }

    inline v8::Local<v8::Object> handle(v8::Isolate *isolate) {
        return v8::Local<v8::Object>::New(isolate, persistent());
    }

    inline v8::Persistent<v8::Object> &persistent() {
        return handle_;
    }

  protected:
    inline void Wrap(v8::Local<v8::Object> handle) {
        assert(persistent().IsEmpty());
        assert(handle->InternalFieldCount() > 0);
        handle->SetAlignedPointerInInternalField(0, this);
        //persistent().Reset(v8::Isolate::GetCurrent(), handle);
        //MakeWeak();
    }

    inline void MakeWeak(void) {
        persistent().SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);
    }

    virtual void Ref() {
        assert(!persistent().IsEmpty());
        persistent().ClearWeak();
        refs_++;
    }

    virtual void Unref() {
        assert(!persistent().IsEmpty());
        assert(!persistent().IsWeak());
        assert(refs_ > 0);
        if (--refs_ == 0) MakeWeak();
    }

    int refs_;

  private:
    static void WeakCallback(const v8::WeakCallbackInfo<ObjectWrap> &data) {
        ObjectWrap *wrap = data.GetParameter();
        assert(wrap->refs_ == 0);
        wrap->handle_.Reset();
        delete wrap;
    }

    v8::Persistent<v8::Object> handle_;
};

inline void DV8_SET_METHOD(v8::Isolate *isolate, v8::Local<v8::Template> recv, const char *name, v8::FunctionCallback callback) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback);
    fprintf(stderr, "Function: %s\n", name);
    v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
    t->SetClassName(fn_name);
    recv->Set(fn_name, t);
}

inline void DV8_SET_PROTOTYPE_METHOD(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> recv, const char *name, v8::FunctionCallback callback) {
    v8::HandleScope handle_scope(isolate);
    v8::Local<v8::Signature> s = v8::Signature::New(isolate, recv);
    v8::Local<v8::FunctionTemplate> t = v8::FunctionTemplate::New(isolate, callback, v8::Local<v8::Value>(), s);
    fprintf(stderr, "  Method: %s\n", name);
    v8::Local<v8::String> fn_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
    t->SetClassName(fn_name);
    recv->PrototypeTemplate()->Set(fn_name, t);
}

inline void DV8_SET_EXPORT(v8::Isolate *isolate, v8::Local<v8::FunctionTemplate> recv, const char *name, v8::Local<v8::Object> exports) {
    v8::Local<v8::String> export_name = v8::String::NewFromUtf8(isolate, name, v8::NewStringType::kInternalized).ToLocalChecked();
    fprintf(stderr, "Export: %s\n", name);
    exports->Set(export_name, recv->GetFunction());
}

} // namespace dv8
#endif