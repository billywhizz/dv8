#ifndef DV8_COMMON_H
#define DV8_COMMON_H

#include <assert.h>
#include <v8.h>

namespace dv8 {

static void DecorateErrorStack(v8::Isolate* isolate, const v8::TryCatch &try_catch) {
  v8::Local<v8::Value> exception = try_catch.Exception();
  if (!exception->IsObject())
    return;
  v8::Local<v8::Object> err_obj = exception.As<v8::Object>();
  v8::Local<v8::Value> stack = err_obj->Get(v8::String::NewFromUtf8(isolate, "stack", v8::NewStringType::kNormal).ToLocalChecked());
  v8::String::Utf8Value strStack(isolate, stack);
  fprintf(stderr, "UncaughtException:\n%s\n", *strStack);
}
class ObjectWrap {
  public:
    ObjectWrap() {
        refs_ = 0;
    }

    virtual ~ObjectWrap() {
        if (persistent().IsEmpty()) return;
        assert(persistent().IsNearDeath());
        persistent().ClearWeak();
        persistent().Reset();
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
        persistent().Reset(v8::Isolate::GetCurrent(), handle);
        MakeWeak();
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

} // namespace dv8
#endif
