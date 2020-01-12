#include <tty.h>
#include <string.h>

namespace dv8
{

namespace tty
{
using dv8::write_req_t;
using dv8::builtins::Buffer;
using dv8::builtins::Environment;

void InitAll(Local<Object> exports)
{
  TTY::Init(exports);
}

void on_close(uv_handle_t *handle)
{
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    TTY *t = (TTY *)handle->data;
    t->stats.close++;
    if (t->callbacks.onClose == 1) {
        Local<Value> argv[0] = {};
        Local<Function> Callback = Local<Function>::New(isolate, t->_onClose);
        Local<Context> ctx = isolate->GetCurrentContext();
        Callback->Call(ctx, ctx->Global(), 0, argv);
    }
    //free(handle);
}

void after_write(uv_write_t *req, int status)
{
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    write_req_t *wr = (write_req_t *)req;
    TTY *t = (TTY *)wr->req.data;
    if (t->callbacks.onWrite == 1)
    {
        Local<Value> argv[2] = {Integer::New(isolate, wr->buf.len), Integer::New(isolate, status)};
        Local<Function> onWrite = Local<Function>::New(isolate, t->_onWrite);
        Local<Context> ctx = isolate->GetCurrentContext();
        onWrite->Call(ctx, ctx->Global(), 2, argv);
    }
    if (status < 0)
    {
        t->stats.error++;
        free(wr->buf.base);
        free(wr);
        t->stats.out.free++;
        if (t->callbacks.onError) {
            Local<Value> argv[2] = {Number::New(isolate, status), String::NewFromUtf8(isolate, uv_strerror(status), v8::NewStringType::kNormal).ToLocalChecked()};
            Local<Function> Callback = Local<Function>::New(isolate, t->_onError);
            Local<Context> ctx = isolate->GetCurrentContext();
            Callback->Call(ctx, ctx->Global(), 2, argv);
        }
        return;
    }
    uv_stream_t *s = (uv_stream_t *)req->handle;
    size_t queueSize = s->write_queue_size;
    if (queueSize > t->stats.out.maxQueue)
    {
        t->stats.out.maxQueue = queueSize;
    }
    if (queueSize == 0)
    {
        // emit a drain event
        if (t->blocked)
        {
            if (t->callbacks.onDrain == 1) {
                Local<Value> argv[0] = {};
                Local<Function> Callback = Local<Function>::New(isolate, t->_onDrain);
                Local<Context> ctx = isolate->GetCurrentContext();
                Callback->Call(ctx, ctx->Global(), 0, argv);
            }
            t->stats.out.drain++;
            t->blocked = false;
        }
        if (t->closing)
        {
            uv_close((uv_handle_t *)t->handle, on_close);
            t->closing = false;
        }
    }
    t->stats.out.written += wr->buf.len;
    free(wr->buf.base);
    free(wr);
    t->stats.out.free++;
}

void after_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
    Isolate *isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    TTY *t = (TTY *)handle->data;
    v8::TryCatch try_catch(isolate);
    // each time we read from the stream uv will do the following inside the same function
    // call alloc_chunk and ask for a 64k buffer
    // read data into this buffer, handling EINTR
    // call this callback
    // so we are safe to reuse same buffer here.
    // in JS land in onRead we will have to copy the buffer if we want to use it beyond the scope of the callback
    if (nread > 0) {
        // we read some bytes
        if (t->callbacks.onRead == 1) {
            Local<Value> argv[1] = {Number::New(isolate, nread)};
            Local<Function> Callback = Local<Function>::New(isolate, t->_onRead);
            Local<Context> ctx = isolate->GetCurrentContext();
            Callback->Call(ctx, ctx->Global(), 1, argv);
        }
        t->stats.in.read += (uint64_t)nread;
        t->stats.in.data++;
    }
    else if (nread == UV_EOF) {
        // other end terminated or sent EOF
        if (t->callbacks.onEnd == 1) {
            Local<Value> argv[] = {};
            Local<Function> Callback = Local<Function>::New(isolate, t->_onEnd);
            Local<Context> ctx = isolate->GetCurrentContext();
            Callback->Call(ctx, ctx->Global(), 0, argv);
        }
        t->stats.in.end++;
        //uv_close((uv_handle_t*)handle, on_close);
    }
    else if (nread < 0) {
        // we got a system error
        //TODO: change to onerror? same as socket?
        if (t->callbacks.onError == 1) {
            Local<Value> argv[2] = {Number::New(isolate, nread), String::NewFromUtf8(isolate, uv_strerror(nread), v8::NewStringType::kNormal).ToLocalChecked()};
            Local<Function> Callback = Local<Function>::New(isolate, t->_onError);
            Local<Context> ctx = isolate->GetCurrentContext();
            Callback->Call(ctx, ctx->Global(), 2, argv);
        }
        t->stats.error++;
        uv_close((uv_handle_t*)handle, on_close);
    } else {
        // nread = 0, we got an EAGAIN or EWOULDBLOCK
    }
    if (try_catch.HasCaught()) {
        dv8::ReportException(isolate, &try_catch);
    }
}

static void alloc_chunk(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
    // we are safe to reuse the same buffer for each allocation
    // it will not be overwritten until after OnRead completes
    TTY *t = (TTY *)handle->data;
    buf->base = t->in;
    buf->len = size;
}

void TTY::Init(Local<Object> exports)
{
    Isolate *isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "TTY").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", TTY::Close);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", TTY::Write);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", TTY::Setup);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", TTY::Pause);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", TTY::Resume);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "queueSize", TTY::QueueSize);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stats", TTY::Stats);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onClose", TTY::onClose);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onDrain", TTY::onDrain);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onWrite", TTY::onWrite);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRead", TTY::onRead);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", TTY::onError);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onEnd", TTY::onEnd);

    DV8_SET_EXPORT(isolate, tpl, "TTY", exports);

    DV8_SET_CONSTANT(isolate, Integer::New(isolate, 0), "UV_TTY_MODE_NORMAL", tpl);
    DV8_SET_CONSTANT(isolate, Integer::New(isolate, 1), "UV_TTY_MODE_RAW", tpl);
    DV8_SET_CONSTANT(isolate, Integer::New(isolate, 2), "UV_TTY_MODE_IO", tpl);

    DV8_SET_CONSTANT(isolate, Integer::New(isolate, UV_EOF), "UV_EOF", tpl);
    DV8_SET_CONSTANT(isolate, Integer::New(isolate, UV_EAGAIN), "UV_EAGAIN", tpl);
}

void TTY::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
        Local<Context> context = isolate->GetCurrentContext();
        Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));

        TTY *obj = new TTY();
        obj->handle = (uv_tty_t *)calloc(1, sizeof(uv_tty_t));
        obj->handle->data = obj;
        obj->closing = false;
        obj->paused = true;
        obj->blocked = false;
        unsigned int ttype = 0; // stdin
        int argc = args.Length();
        if (argc > 0) {
            ttype = args[0]->Uint32Value(context).ToChecked();
        }
        obj->stats.close = 0;
        obj->stats.error = 0;
        obj->stats.in.read = 0;
        obj->stats.in.pause = 0;
        obj->stats.in.data = 0;
        obj->stats.in.resume = 0;
        obj->stats.in.end = 0;
        obj->stats.out.written = 0;
        obj->stats.out.incomplete = 0;
        obj->stats.out.full = 0;
        obj->stats.out.drain = 0;
        obj->stats.out.maxQueue = 0;
        obj->stats.out.alloc = 0;
        obj->stats.out.free = 0;
        obj->stats.out.eagain = 0;

        obj->callbacks.onClose = 0;
        obj->callbacks.onWrite = 0;
        obj->callbacks.onRead = 0;
        obj->callbacks.onError = 0;
        obj->callbacks.onDrain = 0;
        obj->callbacks.onEnd = 0;

        if (ttype == 0) {
            uv_tty_init(env->loop, obj->handle, ttype, 1);
        } else {
            uv_tty_init(env->loop, obj->handle, ttype, 0);
        }
        uv_tty_set_mode(obj->handle, UV_TTY_MODE_NORMAL);
        obj->handle->data = obj;
        obj->Wrap(args.This());
        args.GetReturnValue().Set(args.This());
    }
}

void TTY::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  TTY* tty = static_cast<TTY *>(wrap);
    #if TRACE
    fprintf(stderr, "TTY::Destroy\n");
    #endif
}

void TTY::Setup(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    v8::HandleScope handleScope(isolate);
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
    t->in = b->_data;
    int argc = args.Length();
    if (argc > 1) {
        uint32_t mode = args[1]->Uint32Value(context).ToChecked();
        uv_tty_set_mode(t->handle, mode);
    }
}

void TTY::Write(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    Local<Context> context = isolate->GetCurrentContext();
    uint32_t length = args[0]->Uint32Value(context).ToChecked();
    uv_buf_t buf;
    buf.base = t->in;
    buf.len = length;
    // we are safe to reuse same buffer for writing also
    // if try_write succeeds that means the buffer has gone to the kernel and
    // we are free to overwrite it
    // if try_write fails without error, we will copy the bytes and queue the write
    int r = uv_try_write((uv_stream_t *)t->handle, &buf, 1);
    if (r == UV_EAGAIN || r == UV_ENOSYS)
    {
        // no data could be sent, queue the async write and copy the buffer
        write_req_t *wr;
        wr = (write_req_t *)malloc(sizeof *wr);
        wr->req.data = t;
        char *wrb = (char *)calloc(length, 1);
        memcpy(wrb, t->in, length);
        t->stats.out.alloc++;
        t->stats.out.eagain++;
        wr->buf = uv_buf_init(wrb, length);
        int status = uv_write(&wr->req, (uv_stream_t *)t->handle, &wr->buf, 1, after_write);
        r = 0;
        if (status != 0) {
            r = status;
        }
        t->blocked = true;
    }
    else if (r < 0)
    {
        t->stats.error++;
        if (t->callbacks.onError == 1) {
            Local<Value> argv[2] = {Number::New(isolate, r), String::NewFromUtf8(isolate, uv_strerror(r), v8::NewStringType::kNormal).ToLocalChecked()};
            Local<Function> Callback = Local<Function>::New(isolate, t->_onError);
            Local<Context> ctx = isolate->GetCurrentContext();
            Callback->Call(ctx, ctx->Global(), 2, argv);
        }
    }
    else if (r == 0) {
        fprintf(stderr, "this should not happen\n");
        abort();
    }
    else if ((uint32_t)r < length)
    {
        // data partially sent, queue the async write for remainder
        t->stats.out.incomplete++;
        t->stats.out.written += r;
        write_req_t *wr;
        wr = (write_req_t *)malloc(sizeof *wr);
        wr->req.data = t;
        char *wrb = (char *)calloc(length - r, 1);
        char *base = t->in + r;
        memcpy(wrb, base, length - r);
        t->stats.out.alloc++;
        wr->buf = uv_buf_init(wrb, length - r);
        int status = uv_write(&wr->req, (uv_stream_t *)t->handle, &wr->buf, 1, after_write);
        if (t->callbacks.onWrite == 1) {
            Local<Value> argv[2] = {Integer::New(isolate, r), Integer::New(isolate, status)};
            Local<Function> onWrite = Local<Function>::New(isolate, t->_onWrite);
            Local<Context> ctx = isolate->GetCurrentContext();
            onWrite->Call(ctx, ctx->Global(), 2, argv);
        }
        if (status != 0)
        {
            r = status;
        }
        t->blocked = true;
    }
    else
    {
        t->stats.out.full++;
        t->stats.out.written += (uint64_t)r;
        if (t->callbacks.onWrite == 1) {
            Local<Value> argv[2] = {Integer::New(isolate, r), Integer::New(isolate, 0)};
            Local<Function> onWrite = Local<Function>::New(isolate, t->_onWrite);
            Local<Context> ctx = isolate->GetCurrentContext();
            onWrite->Call(ctx, ctx->Global(), 2, argv);
        }
    }
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::Close(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    uv_stream_t *s = (uv_stream_t *)t->handle;
    size_t queueSize = s->write_queue_size;
    if (queueSize > 0) {
        t->closing = true;
    }
    else {
        if (!uv_is_closing((uv_handle_t *)t->handle)) {
            uv_close((uv_handle_t *)t->handle, on_close);
        }
        t->closing = false;
    }
}

void TTY::Pause(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    t->stats.in.pause++;
    int r = uv_read_stop((uv_stream_t *)t->handle);
    t->paused = true;
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::Resume(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    if (t->paused)
    {
        t->stats.in.resume++;
        int r = uv_read_start((uv_stream_t *)t->handle, alloc_chunk, after_read);
        t->paused = false;
        args.GetReturnValue().Set(Integer::New(isolate, r));
        return;
    }
    args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void TTY::QueueSize(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    uv_stream_t *s = (uv_stream_t *)t->handle;
    size_t queueSize = s->write_queue_size;
    if (queueSize > t->stats.out.maxQueue)
    {
        t->stats.out.maxQueue = queueSize;
    }
    args.GetReturnValue().Set(Integer::New(isolate, queueSize));
}

void TTY::Stats(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    Local<v8::BigUint64Array> array = args[0].As<v8::BigUint64Array>();
    Local<ArrayBuffer> ab = array->Buffer();
    uint64_t *fields = static_cast<uint64_t *>(ab->GetContents().Data());
    fields[0] = t->stats.close;
    fields[1] = t->stats.error;
    fields[2] = t->stats.in.read;
    fields[3] = t->stats.in.pause;
    fields[4] = t->stats.in.data;
    fields[5] = t->stats.in.resume;
    fields[6] = t->stats.in.end;
    fields[7] = 0;
    fields[8] = 0;
    fields[9] = 0;
    fields[10] = t->stats.out.written;
    fields[11] = t->stats.out.incomplete;
    fields[12] = t->stats.out.full;
    fields[13] = t->stats.out.drain;
    fields[14] = t->stats.out.maxQueue;
    fields[15] = t->stats.out.alloc;
    fields[16] = t->stats.out.free;
    fields[17] = t->stats.out.eagain;
    fields[18] = 0;
    fields[19] = 0;
}

void TTY::onClose(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onClose = Local<Function>::Cast(args[0]);
    t->_onClose.Reset(isolate, onClose);
    t->callbacks.onClose = 1;
  }
}

void TTY::onDrain(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onDrain = Local<Function>::Cast(args[0]);
    t->_onDrain.Reset(isolate, onDrain);
    t->callbacks.onDrain = 1;
  }
}

void TTY::onEnd(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onEnd = Local<Function>::Cast(args[0]);
    t->_onEnd.Reset(isolate, onEnd);
    t->callbacks.onEnd = 1;
  }
}

void TTY::onWrite(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onWrite = Local<Function>::Cast(args[0]);
    t->_onWrite.Reset(isolate, onWrite);
    t->callbacks.onWrite = 1;
  }
}

void TTY::onRead(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onRead = Local<Function>::Cast(args[0]);
    t->_onRead.Reset(isolate, onRead);
    t->callbacks.onRead = 1;
  }
}

void TTY::onError(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onError = Local<Function>::Cast(args[0]);
    t->_onError.Reset(isolate, onError);
    t->callbacks.onError = 1;
  }
}


} // namespace tty
} // namespace dv8

extern "C" {
	void* _register_tty() {
		return (void*)dv8::tty::InitAll;
	}
}
