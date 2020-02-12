#include "tty.h"
#include <string.h>

namespace dv8
{

namespace tty
{
using dv8::builtins::Buffer;
using dv8::builtins::Environment;

void InitAll(Local<Object> exports)
{
  TTY::Init(exports);
}

int on_tty_event(jsys_descriptor *handle) {
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  jsys_stream_context* context = (jsys_stream_context*)handle->data;
  TTY *t = static_cast<TTY*>(context->data);
  char* buf = static_cast<char*>(context->in->iov_base);
  size_t len = context->in->iov_len;
  Local<Context> ctx = isolate->GetCurrentContext();
  Local<Object> global = ctx->Global();
  if (jsys_descriptor_is_writable(handle)) {
    if (t->callbacks.onDrain == 1) {
      Local<Function> Callback = Local<Function>::New(isolate, t->_onDrain);
      Local<Value> argv[] = {};
      Callback->Call(ctx, global, 0, argv);
    }
  }
  if (jsys_descriptor_is_readable(handle)) {
    ssize_t bytes = 0;
    Local<Value> argv[1] = {Number::New(isolate, bytes)};
    Local<Function> Callback = Local<Function>::New(isolate, t->_onRead);
    int count = 32;
    int fd = handle->fd;
    while ((bytes = read(fd, buf, len))) {
      if (bytes == -1) {
        if (errno == EAGAIN) {
          break;
        }
        perror("read");
        break;
      }
      if (t->callbacks.onRead == 1) {
        argv[0] = Number::New(isolate, bytes);
        Callback->Call(ctx, global, 1, argv);
      }
      if (--count == 0) break;
    }
  }
  if (jsys_descriptor_is_error(handle)) {
    if (t->callbacks.onEnd == 1) {
        Local<Value> argv[] = {};
        Local<Function> Callback = Local<Function>::New(isolate, t->_onEnd);
        Callback->Call(ctx, global, 0, argv);
    } else {
      jsys_descriptor_free(handle);
    }
  }
  return 0;
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
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stats", TTY::Stats);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onClose", TTY::onClose);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onDrain", TTY::onDrain);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onWrite", TTY::onWrite);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRead", TTY::onRead);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", TTY::onError);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onEnd", TTY::onEnd);

    DV8_SET_EXPORT(isolate, tpl, "TTY", exports);

    DV8_SET_CONSTANT(isolate, Integer::New(isolate, STDIN_FILENO), "STDIN", tpl);
    DV8_SET_CONSTANT(isolate, Integer::New(isolate, STDOUT_FILENO), "STDOUT", tpl);
    DV8_SET_CONSTANT(isolate, Integer::New(isolate, STDERR_FILENO), "STDERR", tpl);
}

void TTY::New(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
        Local<Context> context = isolate->GetCurrentContext();
        Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
        TTY *obj = new TTY();
        obj->handle = jsys_descriptor_create(env->loop);
        obj->handle->type = JSYS_TTY;
        obj->paused = true;
        obj->ttype = STDIN_FILENO;
        int argc = args.Length();
        if (argc > 0) {
            obj->ttype = args[0]->Uint32Value(context).ToChecked();
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
        obj->Wrap(args.This());
        obj->Ref();
        args.GetReturnValue().Set(args.This());
    }
}

void TTY::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
    #if TRACE
    fprintf(stderr, "TTY::Destroy\n");
    #endif
}

void TTY::Setup(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    Local<Context> ctx = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(ctx->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    v8::HandleScope handleScope(isolate);
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
    jsys_stream_context* context = jsys_stream_context_create(env->loop, 1);
    context->offset = 0;
    jsys_tty_init(t->handle, t->ttype);
    t->handle->data = context;
    t->handle->callback = on_tty_event;
    context->data = t;
    context->in = (struct iovec*)calloc(1, sizeof(struct iovec));
    context->out = (struct iovec*)calloc(1, sizeof(struct iovec));
    context->in->iov_base = b->_data;
    context->in->iov_len = b->_length;
    context->out->iov_base = b->_data;
    context->out->iov_len = b->_length;
    int r = jsys_loop_add(env->loop, t->handle);
    if (r == -1) {
      args.GetReturnValue().Set(Integer::New(isolate, r));
      return;
    }
    r = jsys_tcp_pause(t->handle);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::Write(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    Local<Context> ctx = isolate->GetCurrentContext();
    uint32_t length = args[0]->Uint32Value(ctx).ToChecked();
    jsys_stream_context* context = (jsys_stream_context*)t->handle->data;
    int r = jsys_tcp_write_len(t->handle, context->out, length);
    args.GetReturnValue().Set(Integer::New(isolate, r));
}

void TTY::Close(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    t->Unref();
    jsys_descriptor_free(t->handle);
}

void TTY::Pause(const FunctionCallbackInfo<Value> &args)
{
    Isolate *isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    TTY *t = ObjectWrap::Unwrap<TTY>(args.Holder());
    t->stats.in.pause++;
    int r = jsys_tcp_pause(t->handle);
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
        int r = jsys_tcp_resume(t->handle);
        t->paused = false;
        args.GetReturnValue().Set(Integer::New(isolate, r));
        return;
    }
    args.GetReturnValue().Set(Integer::New(isolate, 0));
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
