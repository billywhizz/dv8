#include <socket.h>

namespace dv8
{

namespace socket
{
using dv8::builtins::Buffer;
using dv8::builtins::Environment;

void InitAll(Local<Object> exports)
{
  Socket::Init(exports);
}

_context *context_init(uv_stream_t *handle, Socket *s)
{
  _context* ctx = s->context;
  ctx->handle = handle;
  ctx->closing = false;
  ctx->blocked = false;
  ctx->paused = false;
  ctx->stats.error = 0;
  ctx->stats.close = 0;
  ctx->stats.in.read = 0;
  ctx->stats.in.pause = 0;
  ctx->stats.in.data = 0;
  ctx->stats.in.resume = 0;
  ctx->stats.in.end = 0;
  ctx->stats.out.written = 0;
  ctx->stats.out.incomplete = 0;
  ctx->stats.out.full = 0;
  ctx->stats.out.drain = 0;
  ctx->stats.out.maxQueue = 0;
  ctx->stats.out.alloc = 0;
  ctx->stats.out.free = 0;
  ctx->stats.out.eagain = 0;
  handle->data = ctx;
  ctx->data = s;
  return ctx;
}

void context_free(uv_handle_t *handle)
{
  _context *context = (_context *)handle->data;
  free(context);
  free(handle);
}

void after_write(uv_write_t *req, int status)
{
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  write_req_t *wr = (write_req_t *)req;
  uv_stream_t *stream = (uv_stream_t *)req->handle;
  _context *ctx = (_context*)stream->data;
  Socket *socket = (Socket *)ctx->data;
  if (socket->callbacks.onWrite == 1)
  {
    Local<Value> argv[2] = {Integer::New(isolate, wr->buf.len), Integer::New(isolate, status)};
    Local<Function> onWrite = Local<Function>::New(isolate, socket->_onWrite);
    Local<Context> ctx = isolate->GetCurrentContext();
    onWrite->Call(ctx, ctx->Global(), 2, argv);
  }
  if (status < 0)
  {
    ctx->stats.error++;
    free(wr->buf.base);
    free(wr);
    ctx->stats.out.free++;
    if (socket->callbacks.onError == 1)
    {
      Local<Value> argv[1] = {Number::New(isolate, status)};
      Local<Function> Callback = Local<Function>::New(isolate, socket->_onError);
      Local<Context> ctx = isolate->GetCurrentContext();
      Callback->Call(ctx, ctx->Global(), 1, argv);
    }
    return;
  }
  size_t queueSize = stream->write_queue_size;
  if (queueSize > ctx->stats.out.maxQueue)
  {
    ctx->stats.out.maxQueue = queueSize;
  }
  if (queueSize == 0)
  {
    // emit a drain event
    if (ctx->blocked)
    {
      if (socket->callbacks.onDrain == 1)
      {
        Local<Value> argv[0] = {};
        Local<Function> Callback = Local<Function>::New(isolate, socket->_onDrain);
        Local<Context> ctx = isolate->GetCurrentContext();
        Callback->Call(ctx, ctx->Global(), 0, argv);
      }
      ctx->stats.out.drain++;
      ctx->blocked = false;
    }
    if (ctx->closing)
    {
      uv_close((uv_handle_t *)ctx->handle, on_close);
      ctx->closing = false;
    }
  }
  ctx->stats.out.written += wr->buf.len;
  free(wr->buf.base);
  free(wr);
  ctx->stats.out.free++;
}

void on_close(uv_handle_t *peer)
{
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  _context *ctx = (_context *)peer->data;
  Socket *s = (Socket *)ctx->data;
  ctx->stats.close++;
  if (s->callbacks.onClose == 1)
  {
    Local<Value> argv[0] = {};
    Local<Function> onClose = Local<Function>::New(isolate, s->_onClose);
    Local<Context> context = isolate->GetCurrentContext();
    onClose->Call(context, context->Global(), 0, argv);
  }
  context_free(peer);
  if (s->first) {
    s->first->onClose(s->first);
  }
}

void on_close2(uv_handle_t *peer)
{
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  baton_t *baton = (baton_t *)peer->data;
  Socket *s = (Socket *)baton->object;
  if (s->callbacks.onClose == 1)
  {
    Local<Value> argv[0] = {};
    Local<Function> onClose = Local<Function>::New(isolate, s->_onClose);
    Local<Context> ctx = isolate->GetCurrentContext();
    onClose->Call(ctx, ctx->Global(), 0, argv);
  }
  free(peer);
}

void after_shutdown(uv_shutdown_t *req, int status)
{
  uv_handle_t *peer = (uv_handle_t *)req->handle;
  _context *ctx = (_context *)peer->data;
  size_t queueSize = ctx->handle->write_queue_size;
  if (queueSize > 0)
  {
    ctx->closing = true;
  }
  else
  {
    uv_close((uv_handle_t *)ctx->handle, on_close);
  }
  free(req);
}

void after_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf)
{
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  _context *ctx = (_context *)handle->data;
  Socket *s = (Socket *)ctx->data;
  v8::TryCatch try_catch(isolate);
  if (nread > 0) {
    ctx->stats.in.read += (uint64_t)nread;
    ctx->stats.in.data++;
    //TODO: allow returning a different length from onRead - if reader wantes to transform the buffer and the transformation has a different length
    if (s->first) {
      uint32_t r = s->first->onRead(nread, s->first);
    }
    if (s->callbacks.onRead == 1)
    {
      Local<Value> argv[1] = {Number::New(isolate, nread)};
      Local<Function> onRead = Local<Function>::New(isolate, s->_onRead);
      Local<Context> context = isolate->GetCurrentContext();
      onRead->Call(context, context->Global(), 1, argv);
    }
  }
  else if (nread == UV_EOF) {
    if (s->callbacks.onEnd == 1)
    {
      Local<Value> argv[0] = {};
      Local<Function> onEnd = Local<Function>::New(isolate, s->_onEnd);
      Local<Context> context = isolate->GetCurrentContext();
      onEnd->Call(context, context->Global(), 0, argv);
    }
    ctx->stats.in.end++;
    if (uv_is_closing((uv_handle_t *)handle) == 0) {
      //uv_close((uv_handle_t *)handle, on_close);
      ctx->closing = true;
    }
  }
  else if (nread < 0) {
    if (s->callbacks.onError == 1)
    {
      Local<Value> argv[2] = {Number::New(isolate, nread), String::NewFromUtf8(isolate, uv_strerror(nread), v8::NewStringType::kNormal).ToLocalChecked()};
      Local<Function> onError = Local<Function>::New(isolate, s->_onError);
      Local<Context> context = isolate->GetCurrentContext();
      onError->Call(context, context->Global(), 2, argv);
    }
    ctx->stats.in.end++;
    ctx->stats.error++;
    uv_close((uv_handle_t *)handle, on_close);
  } else {
      // nread = 0, we got an EAGAIN or EWOULDBLOCK
  }
  if (try_catch.HasCaught()) {
			dv8::ReportException(isolate, &try_catch);
  }
}

void alloc_chunk(uv_handle_t *handle, size_t size, uv_buf_t *buf)
{
  // we are safe to reuse the same buffer for each allocation
  // it will not be overwritten until after after_read completes
  _context *ctx = (_context *)handle->data;
  buf->base = ctx->in.base;
  buf->len = ctx->in.len;
}

void on_client_connection(uv_connect_t *client, int status)
{
  baton_t *baton = (baton_t *)client->data;
  cb callback = (cb)baton->callback;
  Socket *s = (Socket *)baton->object;
  _context *ctx = (_context *)client->handle->data;
  ctx->data = s;
  s->context = ctx;
  if (!uv_is_readable(client->handle) || !uv_is_writable(client->handle) || uv_is_closing((uv_handle_t *)client->handle))
  {
    fprintf(stderr, "Socket::on_client_connection bad client handle\n");
    return;
  }
  callback(ctx);
  status = uv_read_start(client->handle, alloc_chunk, after_read);
  assert(status == 0);
}

void on_connection(uv_stream_t *server, int status)
{
  baton_t *baton = (baton_t *)server->data;
  uv_stream_t *stream;
  cb callback = (cb)baton->callback;
  Socket *s = (Socket *)baton->object;
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  if (s->socktype == TCP)
  {
    stream = (uv_stream_t *)malloc(sizeof(uv_tcp_t));
    status = uv_tcp_init(env->loop, (uv_tcp_t *)stream);
    if (status != 0) {
      fprintf(stderr, "uv_tcp_init: %i, %s\n", status, uv_strerror(status));
    }
    status = uv_tcp_simultaneous_accepts((uv_tcp_t *)stream, 1);
    if (status != 0) {
      fprintf(stderr, "uv_tcp_simultaneous_accepts: %i, %s\n", status, uv_strerror(status));
    }
  }
  else
  {
    stream = (uv_stream_t *)malloc(sizeof(uv_pipe_t));
    status = uv_pipe_init(env->loop, (uv_pipe_t *)stream, 0);
    if (status != 0) {
      fprintf(stderr, "uv_pipe_init: %i, %s\n", status, uv_strerror(status));
    }
  }
  status = uv_accept(server, stream);
  if (status != 0) {
      fprintf(stderr, "uv_accept: %i, %s\n", status, uv_strerror(status));
  }
  if (s->callbacks.onConnect == 1) {
      int fd;
      status = uv_fileno((uv_handle_t*)stream, &fd);
      Local<Value> argv[1] = { Integer::New(isolate, fd) };
      Local<Function> foo = Local<Function>::New(isolate, s->_onConnect);
      v8::TryCatch try_catch(isolate);
      Local<Value> result = foo->Call(context, context->Global(), 1, argv).ToLocalChecked();
      if (try_catch.HasCaught()) {
        dv8::ReportException(isolate, &try_catch);
        return;
      }
      Local<Object> sock;
      MaybeLocal<Object> o = result->ToObject(context);
      if (!o.IsEmpty()) {
        bool ok = o.ToLocal(&sock);
        Socket *s = ObjectWrap::Unwrap<Socket>(sock);
        _context *ctx = context_init(stream, s);
        ctx->paused = true;
      }
  }
  // we should not start reading. let consumer call resume()
  //status = uv_read_start(stream, alloc_chunk, after_read);
  //if (status != 0) {
  //    fprintf(stderr, "uv_read_start: %i, %s\n", status, uv_strerror(status));
  //}
  //assert(status == 0);
}

void Socket::Init(Local<Object> exports)
{
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

  tpl->SetClassName(String::NewFromUtf8(isolate, "Socket").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listen", Listen);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "connect", Connect);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "bind", Bind);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", Close);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", Write);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "splice", Splice);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Setup);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setNoDelay", SetNoDelay);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", Pause);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", Resume);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setKeepAlive", SetKeepAlive);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "remoteAddress", RemoteAddress);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "error", Error);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "queueSize", QueueSize);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stats", Stats);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "unref", UnRef);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "ref", Ref);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "open", Open);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "create", Create);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "portNumber", PortNumber);

  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onConnect", onConnect);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onClose", onClose);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onDrain", onDrain);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onWrite", onWrite);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRead", onRead);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", onError);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onEnd", onEnd);

  DV8_SET_EXPORT(isolate, tpl, "Socket", exports);

  DV8_SET_CONSTANT(isolate, Integer::New(isolate, TCP), "TCP", tpl);
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, UNIX), "UNIX", tpl);
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, UNIX), "PIPE", tpl);
}

void Socket::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  Socket* sock = static_cast<Socket *>(wrap);
		#if TRACE
		fprintf(stderr, "Socket::Destroy\n");
		#endif
}

void Socket::QueueSize(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Socket *socket = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = socket->context;
  uv_stream_t *s = ctx->handle;
  size_t queueSize = s->write_queue_size;
  if (queueSize > ctx->stats.out.maxQueue)
  {
    ctx->stats.out.maxQueue = queueSize;
  }
  args.GetReturnValue().Set(Integer::New(isolate, queueSize));
}

void Socket::Stats(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Socket *socket = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = socket->context;
  Local<v8::BigUint64Array> array = args[0].As<v8::BigUint64Array>();
  Local<ArrayBuffer> ab = array->Buffer();
  uint64_t *fields = static_cast<uint64_t *>(ab->GetContents().Data());
  fields[0] = ctx->stats.close;
  fields[1] = ctx->stats.error;
  fields[2] = ctx->stats.in.read;
  fields[3] = ctx->stats.in.pause;
  fields[4] = ctx->stats.in.data;
  fields[5] = ctx->stats.in.resume;
  fields[6] = ctx->stats.in.end;
  fields[7] = 0;
  fields[8] = 0;
  fields[9] = 0;
  fields[10] = ctx->stats.out.written;
  fields[11] = ctx->stats.out.incomplete;
  fields[12] = ctx->stats.out.full;
  fields[13] = ctx->stats.out.drain;
  fields[14] = ctx->stats.out.maxQueue;
  fields[15] = ctx->stats.out.alloc;
  fields[16] = ctx->stats.out.free;
  fields[17] = ctx->stats.out.eagain;
  fields[18] = 0;
  fields[19] = 0;
}

void Socket::Error(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  int r = args[0]->IntegerValue(context).ToChecked();
  const char *error = uv_strerror(r);
  args.GetReturnValue().Set(String::NewFromUtf8(args.GetIsolate(), error, NewStringType::kNormal).ToLocalChecked());
}

void Socket::New(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.IsConstructCall()) {
    Local<Context> context = isolate->GetCurrentContext();
    Socket *obj = new Socket();
    obj->context = (_context *)calloc(1, sizeof(_context));
    int len = args.Length();
    if (len > 0) {
      obj->socktype = (socket_type)args[0]->Uint32Value(context).ToChecked();
    }
    obj->callbacks.onClose = 0;
    obj->callbacks.onWrite = 0;
    obj->callbacks.onRead = 0;
    obj->callbacks.onError = 0;
    obj->callbacks.onDrain = 0;
    obj->callbacks.onEnd = 0;
    obj->callbacks.onConnect = 0;
    obj->_stream = NULL;
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  }
}

void Socket::RemoteAddress(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  struct sockaddr_storage address;
  _context *ctx = s->context;
  int addrlen = sizeof(address);
  int r = uv_tcp_getpeername((uv_tcp_t *)ctx->handle, reinterpret_cast<sockaddr *>(&address), &addrlen);
  if (r)
  {
    return;
  }
  const sockaddr *addr = reinterpret_cast<const sockaddr *>(&address);
  char ip[INET_ADDRSTRLEN];
  const sockaddr_in *a4;
  a4 = reinterpret_cast<const sockaddr_in *>(addr);
  int len = sizeof ip;
  uv_inet_ntop(AF_INET, &a4->sin_addr, ip, len);
  len = strlen(ip);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, ip, v8::NewStringType::kNormal, len).ToLocalChecked());
  return;
}

void Socket::Close(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (s->_stream) {
    uv_close((uv_handle_t *)s->_stream, on_close2);
    args.GetReturnValue().Set(Integer::New(isolate, 0));
    return;
  }
  int do_shutdown = 1;
  int argc = args.Length();
  Local<Context> context = isolate->GetCurrentContext();
  _context *ctx = s->context;
  if (argc > 0) {
    do_shutdown = args[0]->Int32Value(context).ToChecked();
  }
  if (do_shutdown) {
    uv_shutdown_t *req;
    req = (uv_shutdown_t *)malloc(sizeof *req);
    int r = uv_shutdown(req, ctx->handle, after_shutdown);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }
  else {
    size_t queueSize = ctx->handle->write_queue_size;
    if (queueSize > 0) {
      ctx->closing = true;
      args.GetReturnValue().Set(Integer::New(isolate, 1));
    }
    else {
      uv_close((uv_handle_t *)ctx->handle, on_close);
      args.GetReturnValue().Set(Integer::New(isolate, 0));
    }
  }
}

int onNewConnection(_context *ctx)
{
  Socket *obj = (Socket *)ctx->data;
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  if (obj->callbacks.onConnect == 1)
  {
    Local<Value> argv[0] = {};
    Local<Function> foo = Local<Function>::New(isolate, obj->_onConnect);
    v8::TryCatch try_catch(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    foo->Call(context, context->Global(), 0, argv);
    if (try_catch.HasCaught()) {
      dv8::ReportException(isolate, &try_catch);
    }
  }
  return 0;
}

void Socket::Setup(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  v8::HandleScope handleScope(isolate);
  _context* ctx = s->context;
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
  size_t len = b->_length;
  ctx->in.base = (char *)b->_data;
  ctx->in.len = len;
  //ctx->in = uv_buf_init((char *)b->_data, len);
  //ctx->readBufferLength = len;
  b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
  len = b->_length;
  ctx->out.base = (char *)b->_data;
  ctx->out.len = len;
  //ctx->out = uv_buf_init((char *)b->_data, len);
  //ctx->writeBufferLength = len;
  args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void Socket::SetKeepAlive(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Local<Context> context = isolate->GetCurrentContext();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = s->context;
  int enable = static_cast<int>(args[0]->BooleanValue(isolate));
  unsigned int delay = args[1]->Uint32Value(context).ToChecked();
  int r = uv_tcp_keepalive((uv_tcp_t *)ctx->handle, enable, delay);
  args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Socket::UnRef(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = s->context;
  uv_unref((uv_handle_t*)ctx->handle);
}

void Socket::Ref(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = s->context;
  uv_ref((uv_handle_t*)ctx->handle);
}

void Socket::SetNoDelay(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = s->context;
  int enable = static_cast<int>(args[0]->BooleanValue(isolate));
  int r = uv_tcp_nodelay((uv_tcp_t *)ctx->handle, enable);
  args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Socket::Pause(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = s->context;
  ctx->paused = true;
  ctx->stats.in.pause++;
  int r = uv_read_stop(ctx->handle);
  args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Socket::Resume(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  _context *ctx = s->context;
  if (ctx->paused)
  {
    ctx->stats.in.resume++;
    int r = uv_read_start(ctx->handle, alloc_chunk, after_read);
    args.GetReturnValue().Set(Integer::New(isolate, r));
    ctx->paused = false;
    return;
  }
  args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void Socket::Write(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *socket = ObjectWrap::Unwrap<Socket>(args.Holder());
  Local<Context> context = isolate->GetCurrentContext();
  int argc = args.Length();
  uint32_t off = 0;
  uint32_t len = args[0]->Uint32Value(context).ToChecked();
  if (argc > 1) {
    off = args[1]->Int32Value(context).ToChecked();
  }
  _context *ctx = socket->context;
  char *src = ctx->out.base + off;
  uv_buf_t buf;
  int r = 0;
  buf.base = src;
  buf.len = len;
  if (!ctx->closing) {
    r = uv_try_write((uv_stream_t *)ctx->handle, &buf, 1);
    if (r == UV_EAGAIN || r == UV_ENOSYS)
    {
      write_req_t *wr;
      wr = (write_req_t *)malloc(sizeof *wr);
      char *wrb = (char *)calloc(len, 1);
      memcpy(wrb, src, len);
      ctx->stats.out.alloc++;
      ctx->stats.out.eagain++;
      wr->buf = uv_buf_init(wrb, len);
      int status = uv_write(&wr->req, ctx->handle, &wr->buf, 1, after_write);
      r = 0;
      if (status != 0) {
        r = status;
      }
      ctx->blocked = true;
    }
    else if (r < 0)
    {
      ctx->stats.error++;
      if (socket->callbacks.onError == 1)
      {
        Local<Value> argv[1] = {Number::New(isolate, r)};
        Local<Function> Callback = Local<Function>::New(isolate, socket->_onError);
        Local<Context> context = isolate->GetCurrentContext();
        Callback->Call(context, context->Global(), 1, argv);
      }
    }
    else if ((uint32_t)r < len)
    {
      ctx->stats.out.incomplete++;
      ctx->stats.out.written += r;
      write_req_t *wr;
      wr = (write_req_t *)malloc(sizeof *wr);
      char *wrb = (char *)calloc(len - r, 1);
      char *base = src + r;
      memcpy(wrb, base, len - r);
      ctx->stats.out.alloc++;
      wr->buf = uv_buf_init(wrb, len - r);
      int status = uv_write(&wr->req, ctx->handle, &wr->buf, 1, after_write);
      ctx->blocked = true;
      if (socket->callbacks.onWrite == 1)
      {
        Local<Value> argv[2] = {Integer::New(isolate, r), Integer::New(isolate, status)};
        Local<Function> onWrite = Local<Function>::New(isolate, socket->_onWrite);
        Local<Context> context = isolate->GetCurrentContext();
        onWrite->Call(context, context->Global(), 2, argv);
      }
      if (status != 0)
      {
        r = status;
      }
    } else {
      ctx->stats.out.full++;
      ctx->stats.out.written += (uint64_t)r;
      if (socket->callbacks.onWrite == 1)
      {
        Local<Value> argv[2] = {Integer::New(isolate, r), Integer::New(isolate, 0)};
        Local<Function> onWrite = Local<Function>::New(isolate, socket->_onWrite);
        Local<Context> context = isolate->GetCurrentContext();
        onWrite->Call(context, context->Global(), 2, argv);
      }
    }
  } else {
    r = -1;
  }
  args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Socket::Splice(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *socket = ObjectWrap::Unwrap<Socket>(args.Holder());
  Local<Context> context = isolate->GetCurrentContext();
  int argc = args.Length();
  uint32_t off = 0;
  uint32_t len = args[0]->Uint32Value(context).ToChecked();
  if (argc > 1) {
    off = args[1]->Int32Value(context).ToChecked();
  }
  _context *ctx = socket->context;
  char *src = ctx->out.base + off;

  int fd;
  uv_fileno((uv_handle_t *)ctx->handle, &fd);
  fprintf(stderr, "fd: %i\n", fd);
  struct iovec iov { src, len };
  int r = vmsplice(fd, &iov, 1, SPLICE_F_NONBLOCK);
  fprintf(stderr, "vmsplice: %i\n", r);
  fprintf(stderr, "errno: %i\n", errno);
/*
  uv_buf_t buf;
  buf.base = src;
  buf.len = len;
  r = uv_try_write((uv_stream_t *)ctx->handle, &buf, 1);
  if (r == UV_EAGAIN || r == UV_ENOSYS)
  {
    write_req_t *wr;
    wr = (write_req_t *)malloc(sizeof *wr);
    char *wrb = (char *)calloc(len, 1);
    memcpy(wrb, src, len);
    ctx->stats.out.alloc++;
    ctx->stats.out.eagain++;
    wr->buf = uv_buf_init(wrb, len);
    int status = uv_write(&wr->req, ctx->handle, &wr->buf, 1, after_write);
    r = 0;
    if (status != 0) {
      r = status;
    }
    ctx->blocked = true;
  }
  else if (r < 0)
  {
    ctx->stats.error++;
    if (socket->callbacks.onError == 1)
    {
      Local<Value> argv[1] = {Number::New(isolate, r)};
      Local<Function> Callback = Local<Function>::New(isolate, socket->_onError);
      Callback->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    }
  }
  else if ((uint32_t)r < len)
  {
    ctx->stats.out.incomplete++;
    ctx->stats.out.written += r;
    write_req_t *wr;
    wr = (write_req_t *)malloc(sizeof *wr);
    char *wrb = (char *)calloc(len - r, 1);
    char *base = src + r;
    memcpy(wrb, base, len - r);
    ctx->stats.out.alloc++;
    wr->buf = uv_buf_init(wrb, len - r);
    int status = uv_write(&wr->req, ctx->handle, &wr->buf, 1, after_write);
    ctx->blocked = true;
    if (socket->callbacks.onWrite == 1)
    {
      Local<Value> argv[2] = {Integer::New(isolate, r), Integer::New(isolate, status)};
      Local<Function> onWrite = Local<Function>::New(isolate, socket->_onWrite);
      onWrite->Call(isolate->GetCurrentContext()->Global(), 2, argv);
    }
    if (status != 0)
    {
      r = status;
    }
  } else {
    ctx->stats.out.full++;
    ctx->stats.out.written += (uint64_t)r;
    if (socket->callbacks.onWrite == 1)
    {
      Local<Value> argv[2] = {Integer::New(isolate, r), Integer::New(isolate, 0)};
      Local<Function> onWrite = Local<Function>::New(isolate, socket->_onWrite);
      onWrite->Call(isolate->GetCurrentContext()->Global(), 2, argv);
    }
  }
*/
  args.GetReturnValue().Set(Integer::New(isolate, r));
}

void Socket::Connect(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  s->isServer = false;
  if (s->socktype == TCP)
  {
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *ip_address = *str;
    const unsigned int port = args[1]->IntegerValue(context).ToChecked();
    struct sockaddr_in address;
    int r = uv_ip4_addr(ip_address, port, &address);
    uv_tcp_t *sock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    sock->data = s;
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = s;
    sock->data = baton;
    r = uv_tcp_init(env->loop, sock);
    if (r)
    {
      args.GetReturnValue().Set(Integer::New(isolate, r));
      return;
    }
    uv_connect_t *cn_wrap = (uv_connect_t *)malloc(sizeof(uv_connect_t));
    cn_wrap->data = baton;
    r = uv_tcp_connect(cn_wrap, sock, (const struct sockaddr *)&address, on_client_connection);
    if (r)
    {
      free(cn_wrap);
      args.GetReturnValue().Set(Integer::New(isolate, r));
      return;
    }
    context_init((uv_stream_t *)sock, s);
    args.GetReturnValue().Set(Integer::New(isolate, 0));
  }
  else
  {
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *path = *str;
    uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
    sock->data = s;
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = s;
    sock->data = baton;
    int r = uv_pipe_init(env->loop, sock, 0);
    if (r)
    {
      args.GetReturnValue().Set(Integer::New(isolate, r));
      return;
    }
    uv_connect_t *cn_wrap = (uv_connect_t *)malloc(sizeof(uv_connect_t));
    cn_wrap->data = baton;
    uv_pipe_connect(cn_wrap, sock, path, on_client_connection);
    context_init((uv_stream_t *)sock, s);
    args.GetReturnValue().Set(Integer::New(isolate, 0));
  }
}

void Socket::onClose(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onClose = Local<Function>::Cast(args[0]);
    s->_onClose.Reset(isolate, onClose);
    s->callbacks.onClose = 1;
  }
}

void Socket::onDrain(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onDrain = Local<Function>::Cast(args[0]);
    s->_onDrain.Reset(isolate, onDrain);
    s->callbacks.onDrain = 1;
  }
}

void Socket::onEnd(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onEnd = Local<Function>::Cast(args[0]);
    s->_onEnd.Reset(isolate, onEnd);
    s->callbacks.onEnd = 1;
  }
}

void Socket::onWrite(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onWrite = Local<Function>::Cast(args[0]);
    s->_onWrite.Reset(isolate, onWrite);
    s->callbacks.onWrite = 1;
  }
}

void Socket::onRead(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onRead = Local<Function>::Cast(args[0]);
    s->_onRead.Reset(isolate, onRead);
    s->callbacks.onRead = 1;
  }
}

void Socket::onError(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onError = Local<Function>::Cast(args[0]);
    s->_onError.Reset(isolate, onError);
    s->callbacks.onError = 1;
  }
}

void Socket::onConnect(const v8::FunctionCallbackInfo<v8::Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (args[0]->IsFunction())
  {
    Local<Function> onConnect = Local<Function>::Cast(args[0]);
    s->_onConnect.Reset(isolate, onConnect);
    s->callbacks.onConnect = 1;
  }
}

void Socket::PortNumber(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  uv_tcp_t* sock = (uv_tcp_t*)s->_stream;
  struct sockaddr_storage address;
  int addrlen = sizeof(address);
  int status = uv_tcp_getsockname(sock, reinterpret_cast<sockaddr *>(&address), &addrlen);
  const sockaddr *addr2 = reinterpret_cast<const sockaddr *>(&address);
  const sockaddr_in *a4 = reinterpret_cast<const sockaddr_in *>(addr2);
  args.GetReturnValue().Set(Integer::New(isolate, ntohs(a4->sin_port)));
}

void Socket::Listen(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  s->isServer = true;
  if (args[0]->IsNumber())
  { // we have been passed a socket handle that has already been bound
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    int fd = args[0]->Int32Value(context).ToChecked();
    if (s->socktype == TCP)
    {
      uv_tcp_t *sock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
      sock->data = s;
      baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
      baton->callback = (void *)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      int status = uv_tcp_init(env->loop, sock);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_tcp_open(sock, fd);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_listen((uv_stream_t *)sock, SOMAXCONN, on_connection);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
    }
    else if (s->socktype == UNIX)
    {
      uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
      sock->data = s;
      baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
      baton->callback = (void *)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      int status = uv_pipe_init(env->loop, sock, 0);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_pipe_open(sock, fd);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_listen((uv_stream_t *)sock, SOMAXCONN, on_connection);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
    }
    else
    {
      args.GetReturnValue().Set(Integer::New(isolate, -1));
      return;
    }
  }
  else if (s->socktype == TCP)
  {
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    uv_tcp_t *sock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    sock->data = s;
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *ip_address = *str;
    const unsigned int port = args[1]->IntegerValue(context).ToChecked();
    struct sockaddr_in addr;
    uv_ip4_addr(ip_address, port, &addr);
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = sock->data;
    sock->data = baton;
    uv_thread_t t = uv_thread_self();
    int status = uv_tcp_init_ex(env->loop, sock, AF_INET);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    int fd;
    uv_fileno((uv_handle_t *)sock, &fd);
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    status = uv_tcp_bind(sock, (const struct sockaddr *)&addr, 0);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    status = uv_listen((uv_stream_t *)sock, SOMAXCONN, on_connection);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    s->_stream = (uv_stream_t *)sock;
  }
  else if (s->socktype == UNIX)
  { // use first argument as path to domain socket
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *path = *str;
    uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
    sock->data = s;
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = sock->data;
    sock->data = baton;
    int status = uv_pipe_init(env->loop, sock, 0);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
/*
    int fd;
    uv_fileno((uv_handle_t *)sock, &fd);
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SOCK_ZEROCOPY, &on, sizeof(on));
*/
    status = uv_pipe_bind(sock, path);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    status = uv_listen((uv_stream_t *)sock, SOMAXCONN, on_connection);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    s->_stream = (uv_stream_t *)sock;
  }
  else
  {
    args.GetReturnValue().Set(Integer::New(isolate, -1));
    return;
  }
  args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void Socket::Open(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  int argc = args.Length();
  if (argc == 0) {
    int fd[2];
    socketpair(AF_UNIX, SOCK_STREAM, 0, fd);
    uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
    sock->data = s;
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = sock->data;
    sock->data = baton;
    int status = uv_pipe_init(env->loop, sock, 0);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    status = uv_pipe_open(sock, fd[0]);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    _context *ctx = context_init((uv_stream_t*)sock, s);
    ctx->paused = true;
    onNewConnection(ctx);
    args.GetReturnValue().Set(Integer::New(isolate, fd[1]));
  } else {
    if (s->socktype == UNIX) {
      uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
      sock->data = s;
      baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
      baton->callback = (void *)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      int status = uv_pipe_init(env->loop, sock, 0);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      int fd = args[0]->Int32Value(context).ToChecked();
      status = uv_pipe_open(sock, fd);
      _context *ctx = context_init((uv_stream_t*)sock, s);
      ctx->paused = true;
      onNewConnection(ctx);
      args.GetReturnValue().Set(Integer::New(isolate, status));
    } else {
      uv_tcp_t *sock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
      sock->data = s;
      baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
      baton->callback = (void *)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      //int status = uv_tcp_init_ex(env->loop, sock, AF_INET);
      int status = uv_tcp_init(env->loop, sock);
      if (status)
      {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      int fd = args[0]->Int32Value(context).ToChecked();
      status = uv_tcp_open(sock, fd);
      _context *ctx = context_init((uv_stream_t*)sock, s);
      ctx->paused = true;
      onNewConnection(ctx);
      args.GetReturnValue().Set(Integer::New(isolate, status));
    }
  }
}

void Socket::Create(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
  int status = uv_pipe_init(env->loop, sock, 0);
  if (status)
  {
    args.GetReturnValue().Set(Integer::New(isolate, status));
    return;
  }
  _context *ctx = context_init((uv_stream_t*)sock, s);
  ctx->paused = true;
  //s->_stream = (uv_stream_t *)sock;
  args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void Socket::Bind(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  Socket *s = ObjectWrap::Unwrap<Socket>(args.Holder());
  if (s->socktype == TCP)
  {
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    const unsigned int port = args[1]->IntegerValue(context).ToChecked();
    uv_tcp_t *sock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
    sock->data = s;
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *ip_address = *str;
    struct sockaddr_in addr;
    uv_ip4_addr(ip_address, port, &addr);
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = sock->data;
    sock->data = baton;
    int status = uv_tcp_init_ex(env->loop, sock, AF_INET);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    int fd;
    uv_fileno((uv_handle_t *)sock, &fd);
    int on = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
    status = uv_tcp_bind(sock, (const struct sockaddr *)&addr, 0);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
  }
  else if (s->socktype == UNIX)
  { // it is a domain socket
    Local<Context> context = isolate->GetCurrentContext();
    Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *path = *str;
    uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
    sock->data = s;
    baton_t *baton = (baton_t *)malloc(sizeof(baton_t));
    baton->callback = (void *)onNewConnection;
    baton->object = sock->data;
    sock->data = baton;
    int status = uv_pipe_init(env->loop, sock, 0);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
    status = uv_pipe_bind(sock, path);
    if (status)
    {
      args.GetReturnValue().Set(Integer::New(isolate, status));
      return;
    }
  }
  else
  {
    args.GetReturnValue().Set(Integer::New(isolate, -1));
    return;
  }
  args.GetReturnValue().Set(Integer::New(isolate, 0));
}
} // namespace socket
} // namespace dv8

extern "C" {
	void* _register_socket() {
		return (void*)dv8::socket::InitAll;
	}
}
