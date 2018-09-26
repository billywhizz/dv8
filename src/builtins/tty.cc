#include <tty.h>

namespace dv8
{

namespace builtins
{
using v8::Array;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

Persistent<Function> TTY::constructor;

void TTY::Init(Local<Object> exports)
{
    Isolate *isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "TTY"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", TTY::Write);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", TTY::Setup);

    constructor.Reset(isolate, tpl->GetFunction());
    DV8_SET_EXPORT(isolate, tpl, "TTY", exports);
}

void TTY::Setup(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    v8::HandleScope handleScope(isolate);
    Buffer* b = ObjectWrap::Unwrap<Buffer>(args[0]->ToObject());
    size_t len = b->_length;
    t->in = uv_buf_init((char*)b->_data, len);
    args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void TTY::OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t* buf)
{
    if (nread > 0) {
        TTY* t = (TTY*)handle->data;
        Isolate * isolate = Isolate::GetCurrent();
        v8::HandleScope handleScope(isolate);
        v8::TryCatch try_catch(isolate);
        Local<Value> argv[1] = { Number::New(isolate, nread) };
        //Local<Value> argv[2] = { String::NewFromUtf8(isolate, buf->base, v8::String::kNormalString, nread) };
        Local<Function> onData = Local<Function>::New(isolate, t->_onRead);
        onData->Call(isolate->GetCurrentContext()->Global(), 1, argv);
        if (try_catch.HasCaught()) {
            DecorateErrorStack(isolate, try_catch);
        }
    }
}

static void alloc_chunk(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    TTY* t = (TTY*)handle->data;
    buf->base = t->in.base;
    buf->len = t->in.len;
}

void TTY::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall())
    {
        Local<Context> context = isolate->GetCurrentContext();
        TTY *obj = new TTY();
        obj->handle = (uv_tty_t*)calloc(1, sizeof(uv_tty_t));
        obj->handle->data = obj;
        int len = args.Length();
        unsigned int fd = 0; // stdin
        if(len > 0) {
            fd = args[0]->Uint32Value(context).ToChecked();
        }
        if (fd == 0) {
            uv_tty_init(uv_default_loop(), obj->handle, fd, 1);
            if(args[1]->IsFunction()) {
                Local<Function> onRead = Local<Function>::Cast(args[1]);
                obj->_onRead.Reset(isolate, onRead);
            }
            int status = uv_read_start((uv_stream_t*)obj->handle, alloc_chunk, OnRead);
        } else {
            uv_tty_init(uv_default_loop(), obj->handle, fd, 0);
        }
        uv_tty_set_mode(obj->handle, UV_TTY_MODE_NORMAL);
        obj->handle->data = obj;
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
    else
    {
        Local<Function> cons = Local<Function>::New(isolate, constructor);
        Local<Context> context = isolate->GetCurrentContext();
        Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
        args.GetReturnValue().Set(instance);
    }
}

void TTY::NewInstance(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    const unsigned argc = 2;
    Local<Value> argv[argc] = {args[0], args[1]};
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
}

void TTY::Write(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    uv_buf_t buf;
    String::Utf8Value str(args.GetIsolate(), args[0]);
    buf.base = *str;
    buf.len = str.length();
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    int r = uv_try_write((uv_stream_t*)t->handle, &buf, 1);
    uv_tty_reset_mode();
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

} // namespace builtins
} // namespace dv8