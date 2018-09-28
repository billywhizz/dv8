#include <tty.h>
#include <string.h>

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
using dv8::write_req_t;

Persistent<Function> TTY::constructor;

void TTY::Init(Local<Object> exports)
{
    Isolate *isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "TTY"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "writeString", TTY::WriteString);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", TTY::Write);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", TTY::Close);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", TTY::Setup);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", TTY::Pause);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", TTY::Resume);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "queueSize", TTY::QueueSize);

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
}

void TTY::OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t* buf)
{
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    TTY* t = (TTY*)handle->data;
    v8::TryCatch try_catch(isolate);
    Local<Function> Callback;
    if (nread > 0) {
        //Local<Value> argv[1] = { String::NewFromUtf8(isolate, buf->base, v8::String::kNormalString, nread) };
        Local<Value> argv[1] = { Number::New(isolate, nread) };
        Local<Function> Callback = Local<Function>::New(isolate, t->_onRead);
        Callback->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    } else if (nread == UV_EOF) {
        Local<Value> argv[] = { };
        Local<Function> Callback = Local<Function>::New(isolate, t->_onEnd);
        Callback->Call(isolate->GetCurrentContext()->Global(), 0, argv);
    } else if (nread < 0) {
        Local<Value> argv[1] = { Number::New(isolate, nread) };
        Local<Function> Callback = Local<Function>::New(isolate, t->_onEnd);
        Callback->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    }
    if (try_catch.HasCaught()) {
        DecorateErrorStack(isolate, try_catch);
    }
}

static void alloc_chunk(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    TTY* t = (TTY*)handle->data;
    buf->base = t->in.base;
    if (size > t->in.len) {
        buf->len = t->in.len;
        return;
    }
    buf->len = size;
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
            if (len > 1) {
                if(args[1]->IsFunction()) {
                    Local<Function> onRead = Local<Function>::Cast(args[1]);
                    obj->_onRead.Reset(isolate, onRead);
                }
            }
            if (len > 2) {
                if(args[2]->IsFunction()) {
                    Local<Function> onEnd = Local<Function>::Cast(args[2]);
                    obj->_onEnd.Reset(isolate, onEnd);
                }
            }
            if (len > 3) {
                if(args[3]->IsFunction()) {
                    Local<Function> onClose = Local<Function>::Cast(args[3]);
                    obj->_onClose.Reset(isolate, onClose);
                }
            }
        } else {
            uv_tty_init(uv_default_loop(), obj->handle, fd, 0);
            if (len > 1) {
                if(args[1]->IsFunction()) {
                    Local<Function> onClose = Local<Function>::Cast(args[1]);
                    obj->_onClose.Reset(isolate, onClose);
                }
            }
            if (len > 2) {
                if(args[2]->IsFunction()) {
                    Local<Function> onDrain = Local<Function>::Cast(args[2]);
                    obj->_onDrain.Reset(isolate, onDrain);
                }
            }
        }
        obj->fd = fd;
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

void TTY::WriteString(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    uv_buf_t buf;
    String::Utf8Value str(args.GetIsolate(), args[0]);
    buf.base = *str;
    buf.len = str.length();
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    int r = uv_try_write((uv_stream_t*)t->handle, &buf, 1);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::OnWrite(uv_write_t* req, int status) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    write_req_t* wr = (write_req_t*) req;
    if (status < 0) {
        free(wr->buf.base);
        free(wr);
        TTY* t = (TTY*)wr->req.data;
        Local<Value> argv[1] = { Number::New(isolate, status) };
        Local<Function> Callback = Local<Function>::New(isolate, t->_onDrain);
        Callback->Call(isolate->GetCurrentContext()->Global(), 0, argv);
        return;
    }
    uv_stream_t* s = (uv_stream_t*)req->handle;
    size_t queueSize = s->write_queue_size;
    if (queueSize == 0) {
        // emit a drain event
        TTY* t = (TTY*)wr->req.data;
        Local<Value> argv[0] = { };
        Local<Function> Callback = Local<Function>::New(isolate, t->_onDrain);
        Callback->Call(isolate->GetCurrentContext()->Global(), 0, argv);
    }
    free(wr->buf.base);
    free(wr);
}

void TTY::Write(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    Local<Context> context = isolate->GetCurrentContext();
    uint32_t length = args[0]->Uint32Value(context).ToChecked();
    uv_buf_t buf;
    buf.base = t->in.base;
    buf.len = length;
    int r = uv_try_write((uv_stream_t*)t->handle, &buf, 1);
    if (r == UV_EAGAIN || r == UV_ENOSYS) {
        // no data could be sent, queue the async write
        write_req_t *wr;
        wr = (write_req_t *)malloc(sizeof *wr);
        wr->req.data = t;
        char* wrb = (char*)calloc(length, 1);
        memcpy(wrb, t->in.base, length);
        wr->buf = uv_buf_init(wrb, length);
        r = uv_write(&wr->req, (uv_stream_t*)t->handle, &wr->buf, 1, OnWrite);
    } else if (r < 0) {
        //fprintf(stderr, "write.ERROR: %i\n", r);
    } else if ((uint32_t)r < length) {
        write_req_t *wr;
        wr = (write_req_t *)malloc(sizeof *wr);
        wr->req.data = t;
        char* wrb = (char*)calloc(length - r, 1);
        char* base = t->in.base + r;
        memcpy(wrb, base, length - r);
        wr->buf = uv_buf_init(wrb, length - r);
        int status = uv_write(&wr->req, (uv_stream_t*)t->handle, &wr->buf, 1, OnWrite);
        if (status != 0) {
            r = status;
        }
    }
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::OnClose(uv_handle_t* handle) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    TTY* t = (TTY*)handle->data;
    free(handle);
    v8::TryCatch try_catch(isolate);
    Local<Value> argv[0] = { };
    Local<Function> Callback = Local<Function>::New(isolate, t->_onClose);
    Callback->Call(isolate->GetCurrentContext()->Global(), 0, argv);
}

void TTY::Close(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    uv_close((uv_handle_t*)t->handle, OnClose);
}

void TTY::Pause(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    int r = uv_read_stop((uv_stream_t*)t->handle);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::Resume(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    int r = uv_read_start((uv_stream_t*)t->handle, alloc_chunk, OnRead);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::QueueSize(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY* t = ObjectWrap::Unwrap<TTY>(args.Holder());
    uv_stream_t* s = (uv_stream_t*)t->handle;
    size_t queueSize = s->write_queue_size;
    args.GetReturnValue().Set(Integer::New(isolate, queueSize));
}

} // namespace builtins
} // namespace dv8