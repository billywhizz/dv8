#ifndef DV8_COMMON_H
#define DV8_COMMON_H

#include <assert.h>
#include <v8.h>

namespace dv8
{

class ObjectWrap
{
  public:
    ObjectWrap()
    {
        refs_ = 0;
    }

    virtual ~ObjectWrap()
    {
        if (persistent().IsEmpty())
            return;
        assert(persistent().IsNearDeath());
        persistent().ClearWeak();
        persistent().Reset();
    }

    template <class T>
    static inline T *Unwrap(v8::Local<v8::Object> handle)
    {
        assert(!handle.IsEmpty());
        assert(handle->InternalFieldCount() > 0);
        void *ptr = handle->GetAlignedPointerFromInternalField(0);
        ObjectWrap *wrap = static_cast<ObjectWrap *>(ptr);
        return static_cast<T *>(wrap);
    }

    inline v8::Local<v8::Object> handle()
    {
        return handle(v8::Isolate::GetCurrent());
    }

    inline v8::Local<v8::Object> handle(v8::Isolate *isolate)
    {
        return v8::Local<v8::Object>::New(isolate, persistent());
    }

    inline v8::Persistent<v8::Object> &persistent()
    {
        return handle_;
    }

    inline void Wrap(v8::Local<v8::Object> handle)
    {
        assert(persistent().IsEmpty());
        assert(handle->InternalFieldCount() > 0);
        handle->SetAlignedPointerInInternalField(0, this);
        persistent().Reset(v8::Isolate::GetCurrent(), handle);
        MakeWeak();
    }


  protected:
    inline void MakeWeak(void)
    {
        persistent().SetWeak(this, WeakCallback, v8::WeakCallbackType::kParameter);
    }

    virtual void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {

    }

    virtual void Ref()
    {
        assert(!persistent().IsEmpty());
        persistent().ClearWeak();
        refs_++;
    }

    virtual void Unref()
    {
        assert(!persistent().IsEmpty());
        assert(!persistent().IsWeak());
        assert(refs_ > 0);
        if (--refs_ == 0)
            MakeWeak();
    }

    int refs_;

  private:
    static void WeakCallback(const v8::WeakCallbackInfo<ObjectWrap> &data)
    {
        ObjectWrap *wrap = data.GetParameter();
        assert(wrap->refs_ == 0);
        wrap->Destroy(data);
        wrap->handle_.Reset();
        delete wrap;
    }

    v8::Persistent<v8::Object> handle_;
};

} // namespace dv8
#endif
