#include <socket.h>

namespace dv8 {

namespace socket {
  using v8::Context;
  using v8::Function;
  using v8::FunctionCallbackInfo;
  using v8::FunctionTemplate;
  using v8::Isolate;
  using v8::Local;
  using v8::Number;
  using v8::Integer;
  using v8::Object;
  using v8::Persistent;
  using v8::String;
  using v8::Value;
  using v8::Array;
  using dv8::builtins::Buffer;
  using dv8::builtins::Environment;

  Persistent<Function> Socket::constructor;
  static int contextid = 0; // incrementing counter for context ids
  static _context* contextMap[MAX_CONTEXTS];

  _context* context_init(uv_stream_t* handle, Socket* s, uint8_t cmode) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    _context* ctx;
    if(contexts.empty()) {
      ctx = (_context*)calloc(1, sizeof(_context));
      ctx->fd = contextid++;
      contextMap[ctx->fd] = ctx;
    }
    else {
      ctx = contexts.front();
      contexts.pop();
    }
    ctx->proxy = 0;
    ctx->cmode = cmode;
    ctx->handle = handle;
    handle->data = ctx;
    return ctx;
  }

  void context_free(uv_handle_t* handle) {
    _context* context = (_context*)handle->data;
    contexts.push(context);
    free(handle);
  }

  void after_write(uv_write_t* req, int status) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    write_req_t* wr = (write_req_t*) req;
    _context* ctx = contextMap[wr->fd];
    Socket* s = (Socket*)ctx->data;
    if(s->callbacks.onWrite == 1) {
      Local<Value> argv[4] = {Integer::New(isolate, ctx->fd),
                              Integer::New(isolate, wr->req.write_index),
                              Integer::New(isolate, status), Integer::New(isolate, wr->buf.len)};
      Local<Function> onWrite = Local<Function>::New(isolate, s->_onWrite);
      onWrite->Call(isolate->GetCurrentContext()->Global(), 4, argv);
    }
    free(wr);
  }

  void after_write2(uv_write_t* req, int status) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    write_req_t* wr = (write_req_t*) req;
    _context* ctx = contextMap[wr->fd];
    Socket* s = (Socket*)ctx->data;
    if(s->callbacks.onWrite == 1) {
      Local<Value> argv[4] = {Integer::New(isolate, ctx->fd),
                              Integer::New(isolate, wr->req.write_index),
                              Integer::New(isolate, status), Integer::New(isolate, wr->buf.len)};
      Local<Function> onWrite = Local<Function>::New(isolate, s->_onWrite);
      onWrite->Call(isolate->GetCurrentContext()->Global(), 4, argv);
    }
    free(wr->buf.base);
    free(wr);
  }

  void on_close(uv_handle_t* peer) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    _context* ctx = (_context*)peer->data;
    Socket* s = (Socket*)ctx->data;
    if(ctx->proxy == 1) {
      //if(uv_is_closing((uv_handle_t*)ctx->proxyContext->handle) == 0) {
      if(ctx->proxyContext->handle->type == UV_NAMED_PIPE || ctx->proxyContext->handle->type == UV_TCP) {
        uv_shutdown_t *req;
        req = (uv_shutdown_t *)malloc(sizeof *req);
        uv_shutdown(req, ctx->proxyContext->handle, after_shutdown);
      }
    }
    if(s->callbacks.onClose == 1) {
      Local<Value> argv[1] = { Integer::New(isolate, ctx->fd) };
      Local<Function> onClose = Local<Function>::New(isolate, s->_onClose);
      onClose->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    }
    context_free(peer);
  }

  void on_close2(uv_handle_t* peer) {
    free(peer);
  }

  void after_shutdown(uv_shutdown_t* req, int status) {
    if(uv_is_closing((uv_handle_t*)req->handle) == 0) {
      uv_close((uv_handle_t*)req->handle, on_close);
      free(req);
    }
  }

  void after_read(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf) {
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    _context* ctx = (_context*)handle->data;
    Socket* s = (Socket*)ctx->data;
    if (nread == UV_EOF) {
      uv_close((uv_handle_t*)handle, on_close);
      return;
    }
    if (nread < 0) {
      if(s->callbacks.onError == 1) {
        Local<Value> argv[3] = { Integer::New(isolate, ctx->fd), Number::New(isolate, nread), String::NewFromUtf8(isolate, uv_strerror(nread), v8::String::kNormalString) };
        Local<Function> onError = Local<Function>::New(isolate, s->_onError);
        onError->Call(isolate->GetCurrentContext()->Global(), 3, argv);
      }
      uv_close((uv_handle_t*)handle, on_close);
      return;
    }
    if (nread == 0) {
      return;
    }
    ctx->lastByte = 0;
    if(ctx->proxy == 1) {
      write_req_t *wr;
      wr = (write_req_t *)malloc(sizeof *wr);
      char* wrb = (char*)calloc(nread, 1);
      memcpy(wrb, buf->base, nread);
      wr->buf = uv_buf_init(wrb, nread);
      wr->fd = ctx->proxyContext->fd;
      int r = uv_write(&wr->req, ctx->proxyContext->handle, &wr->buf, 1, after_write2);
      // TODO: handle bad return code here
    }
    if(s->callbacks.onData == 1) {
      Local<Value> argv[2] = { Integer::New(isolate, ctx->fd), Number::New(isolate, nread) };
      Local<Function> onData = Local<Function>::New(isolate, s->_onData);
      onData->Call(isolate->GetCurrentContext()->Global(), 2, argv);
    }
  }

  void echo_alloc(uv_handle_t* handle, size_t size, uv_buf_t* buf) {
    _context *ctx = (_context *)handle->data;
    Socket* s = (Socket*)ctx->data;
    buf->base = ctx->in.base;
    buf->len = ctx->readBufferLength;
  }

  void on_client_connection(uv_connect_t *client, int status) {
    baton_t* baton = (baton_t*)client->data;
    cb foo = (cb)baton->callback;
    Socket* s = (Socket*)baton->object;
    _context *ctx = (_context *)client->handle->data;
    ctx->data = s;
    if (!uv_is_readable(client->handle) || !uv_is_writable(client->handle) ||
        uv_is_closing((uv_handle_t *)client->handle)) {
      return;
    }
    foo(ctx);
    status = uv_read_start(client->handle, echo_alloc, after_read);
    assert(status == 0);
  }

  void on_connection(uv_stream_t* server, int status) {
    baton_t* baton = (baton_t*)server->data;
    uv_stream_t* stream;
    cb foo = (cb)baton->callback;
    Socket* s = (Socket*)baton->object;
    Isolate * isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
    if(s->socktype == TCP) {
      stream = (uv_stream_t*)malloc(sizeof(uv_tcp_t));
      status = uv_tcp_init(env->loop, (uv_tcp_t*)stream);
      status = uv_tcp_simultaneous_accepts((uv_tcp_t*) stream, 1);
    }
    else {
      stream = (uv_stream_t*)malloc(sizeof(uv_pipe_t));
      status = uv_pipe_init(env->loop, (uv_pipe_t*)stream, 0);
    }
    status = uv_accept(server, stream);
    _context* ctx = context_init(stream, s, 0);
    ctx->data = baton->object;
    foo(ctx);
    status = uv_read_start(stream, echo_alloc, after_read);
    assert(status == 0);
  }

  void Socket::Init(Local<Object> exports) {
    Isolate* isolate = exports->GetIsolate();
    Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

    tpl->SetClassName(String::NewFromUtf8(isolate, "Socket"));
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listen", Listen);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "connect", Connect);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "bind", Bind);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", Close);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pull", Pull);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "push", Push);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", Write);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "writeText", WriteText);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Setup);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setNoDelay", SetNoDelay);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", Pause);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", Resume);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setKeepAlive", SetKeepAlive);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "proxy", Proxy);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "remoteAddress", RemoteAddress);

    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onConnect", onConnect);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onClose", onClose);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onWrite", onWrite);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onData", onData);
    DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", onError);

    constructor.Reset(isolate, tpl->GetFunction());
    DV8_SET_EXPORT(isolate, tpl, "Socket", exports);
  }

  void Socket::New(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    HandleScope handle_scope(isolate);
    if (args.IsConstructCall()) {
      Local<Context> context = isolate->GetCurrentContext();
      Socket* obj = new Socket();
      int len = args.Length();
      if(len > 0) {
        obj->socktype = (socket_type)args[0]->Uint32Value(context).ToChecked();
      }
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } else {
      Local<Function> cons = Local<Function>::New(isolate, constructor);
      Local<Context> context = isolate->GetCurrentContext();
      Local<Object> instance = cons->NewInstance(context, 0, NULL).ToLocalChecked();
      args.GetReturnValue().Set(instance);
    }
  }

  void Socket::NewInstance(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    const unsigned argc = 2;
    Local<Value> argv[argc] = { args[0], args[1] };
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    Local<Context> context = isolate->GetCurrentContext();
    Local<Object> instance = cons->NewInstance(context, argc, argv).ToLocalChecked();
    args.GetReturnValue().Set(instance);
  }

  void Socket::RemoteAddress(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "", v8::String::kNormalString, 0));
    return;
    struct sockaddr_storage address;
    if(args.Length() > 0) {
      Local<Context> context = isolate->GetCurrentContext();
      int fd = args[0]->Int32Value(context).ToChecked();
      _context* ctx = contextMap[fd];
      int addrlen = sizeof(address);
      int r = uv_tcp_getpeername((uv_tcp_t *)ctx->handle, reinterpret_cast<sockaddr *>(&address), &addrlen);
      if (r) {
        return;
      }
      const sockaddr *addr = reinterpret_cast<const sockaddr *>(&address);
      char ip[INET6_ADDRSTRLEN];
      const sockaddr_in *a4;
      a4 = reinterpret_cast<const sockaddr_in*>(addr);
      int len = sizeof ip;
      uv_inet_ntop(AF_INET, &a4->sin_addr, ip, len);
      args.GetReturnValue().Set(String::NewFromUtf8(isolate, ip, v8::String::kNormalString, len));
      return;
    }
  }

  void Socket::Close(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handleScope(isolate);
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    int do_shutdown = 1;
    if(args.Length() > 0) {
      Local<Context> context = isolate->GetCurrentContext();
      int fd = args[0]->Int32Value(context).ToChecked();
      _context* ctx = contextMap[fd];
      if(args.Length() > 1) {
        do_shutdown = args[1]->Int32Value(context).ToChecked();
      }
      if(do_shutdown) {
        uv_shutdown_t *req;
        req = (uv_shutdown_t *)malloc(sizeof *req);
        int r = uv_shutdown(req, ctx->handle, after_shutdown);
        args.GetReturnValue().Set(Integer::New(isolate, r));
      } else {
        uv_close((uv_handle_t*)ctx->handle, on_close);
        args.GetReturnValue().Set(Integer::New(isolate, 0));
      }
    } else if(s->_stream) {
      uv_close((uv_handle_t*)s->_stream, on_close2);
      args.GetReturnValue().Set(Integer::New(isolate, 0));
    }
  }

  int onNewConnection(_context* ctx) {
    Socket* obj = (Socket*)ctx->data;
    Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate);
    if(obj->callbacks.onConnect == 1) {
      const unsigned int argc = 1;
      Local<Value> argv[argc] = { Integer::New(isolate, ctx->fd) };
      Local<Function> foo = Local<Function>::New(isolate, obj->_onConnect);
      foo->Call(isolate->GetCurrentContext()->Global(), 1, argv);
    }
    return 0;
  }

  void Socket::Setup(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    v8::HandleScope handleScope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    Buffer* b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
    size_t len = b->_length;
    ctx->in = uv_buf_init((char*)b->_data, len);
    ctx->readBufferLength = len;
    b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
    len = b->_length;
    ctx->out = uv_buf_init((char*)b->_data, len);
    ctx->readBufferLength = len;
    args.GetReturnValue().Set(Integer::New(isolate, 0));
  }

  void Socket::Pull(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    int32_t off = args[1]->Int32Value(context).ToChecked();
    int32_t len = args[2]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    char *data = ctx->in.base + off;
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, data, v8::String::kNormalString, len));
  }

  void Socket::Push(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    Local<String> str = args[1].As<String>();
    int length = str->Length();
    int off = args[2]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    char *src = ctx->out.base + off;
    int chars_written;
    int written = str->WriteUtf8(isolate, src, length, &chars_written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
    args.GetReturnValue().Set(Integer::New(isolate, written));
  }

  void Socket::WriteText(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    int off = args[1]->Int32Value(context).ToChecked();
    Local<String> str = args[2].As<String>();
    int len = str->Length();
    char *src = ctx->out.base + off;
    int chars_written;
    len = str->WriteUtf8(isolate, src, len, &chars_written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
    write_req_t *wr;
    wr = (write_req_t *)malloc(sizeof *wr);
    wr->buf = uv_buf_init(src, len);
    wr->fd = ctx->fd;
    if (ctx->handle == NULL) {
      args.GetReturnValue().Set(Integer::New(isolate, -1));
      return;
    }
    int r = uv_write(&wr->req, ctx->handle, &wr->buf, 1, after_write);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }

  void Socket::SetKeepAlive(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    int enable = static_cast<int>(args[1]->BooleanValue(context).ToChecked());
    unsigned int delay = args[2]->Uint32Value(context).ToChecked();
    int r = uv_tcp_keepalive((uv_tcp_t *)ctx->handle, enable, delay);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }

  void Socket::SetNoDelay(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    int enable = static_cast<int>(args[1]->BooleanValue(context).ToChecked());
    int r = uv_tcp_nodelay((uv_tcp_t *)ctx->handle, enable);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }

  void Socket::Pause(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    Socket* s = (Socket*)ctx->data;
    int r = uv_read_stop(ctx->handle);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }

  void Socket::Proxy(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    int bfd = args[1]->Int32Value(context).ToChecked();
    _context* server = contextMap[fd];
    _context* backend = contextMap[bfd];
    server->proxy = 1;
    backend->proxy = 1;
    server->proxyContext = backend;
    backend->proxyContext = server;
    args.GetReturnValue().Set(Integer::New(isolate, 0));
  }

  void Socket::Resume(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    Socket* s = (Socket*)ctx->data;
    int r = uv_read_start(ctx->handle, echo_alloc, after_read);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }

  void Socket::Write(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Local<Context> context = isolate->GetCurrentContext();
    int fd = args[0]->Int32Value(context).ToChecked();
    int off = args[1]->Int32Value(context).ToChecked();
    int len = args[2]->Int32Value(context).ToChecked();
    _context* ctx = contextMap[fd];
    write_req_t *wr;
    wr = (write_req_t *)malloc(sizeof *wr);
    char *src = ctx->out.base + off;
    wr->buf = uv_buf_init(src, len);
    wr->fd = ctx->fd;
    if (ctx->handle == NULL) {
      args.GetReturnValue().Set(Integer::New(isolate, -1));
      return;
    }
    int r = uv_write(&wr->req, ctx->handle, &wr->buf, 1, after_write);
    args.GetReturnValue().Set(Integer::New(isolate, r));
  }

  void Socket::Connect(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    Local<Context> context = isolate->GetCurrentContext();
    Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
    if (s->socktype == TCP) {
      String::Utf8Value str(args.GetIsolate(), args[0]);
      const char* ip_address = *str;
      const unsigned int port = args[1]->IntegerValue(context).ToChecked();
      struct sockaddr_in address;
      int r = uv_ip4_addr(ip_address, port, &address);
      uv_tcp_t *sock = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));
      sock->data = s;
      baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
      baton->callback = (void*)onNewConnection;
      baton->object = s;
      sock->data = baton;
      r = uv_tcp_init(env->loop, sock);
      if (r) {
        args.GetReturnValue().Set(Integer::New(isolate, r));
        return;
      }
      uv_connect_t *cn_wrap = (uv_connect_t *)malloc(sizeof(uv_connect_t));
      cn_wrap->data = baton;
      r = uv_tcp_connect(cn_wrap, sock, (const struct sockaddr*) &address, on_client_connection);
      if (r) {
        free(cn_wrap);
        args.GetReturnValue().Set(Integer::New(isolate, r));
        return;
      }
      context_init((uv_stream_t *)sock, s, 1);
      args.GetReturnValue().Set(Integer::New(isolate, 0));
      return;
    } else {
      String::Utf8Value str(args.GetIsolate(), args[0]);
      const char* path = *str;
      uv_pipe_t *sock = (uv_pipe_t *)malloc(sizeof(uv_pipe_t));
      sock->data = s;
      baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
      baton->callback = (void*)onNewConnection;
      baton->object = s;
      sock->data = baton;
      int r = uv_pipe_init(env->loop, sock, 0);
      if (r) {
        args.GetReturnValue().Set(Integer::New(isolate, r));
        return;
      }
      uv_connect_t *cn_wrap = (uv_connect_t *)malloc(sizeof(uv_connect_t));
      cn_wrap->data = baton;
      uv_pipe_connect(cn_wrap, sock, path, on_client_connection);
      context_init((uv_stream_t *)sock, s, 1);
      args.GetReturnValue().Set(Integer::New(isolate, 0));
      return;
    }
  }

  void Socket::onClose(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(args[0]->IsFunction()) {
      Local<Function> onClose = Local<Function>::Cast(args[0]);
      s->_onClose.Reset(isolate, onClose);
      s->callbacks.onClose = 1;
    }
  }

  void Socket::onWrite(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(args[0]->IsFunction()) {
      Local<Function> onWrite = Local<Function>::Cast(args[0]);
      s->_onWrite.Reset(isolate, onWrite);
      s->callbacks.onWrite = 1;
    }
  }

  void Socket::onData(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(args[0]->IsFunction()) {
      Local<Function> onData = Local<Function>::Cast(args[0]);
      s->_onData.Reset(isolate, onData);
      s->callbacks.onData = 1;
    }
  }

  void Socket::onError(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(args[0]->IsFunction()) {
      Local<Function> onError = Local<Function>::Cast(args[0]);
      s->_onError.Reset(isolate, onError);
      s->callbacks.onError = 1;
    }
  }

  void Socket::onConnect(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(args[0]->IsFunction()) {
      Local<Function> onConnect = Local<Function>::Cast(args[0]);
      s->_onConnect.Reset(isolate, onConnect);
      s->callbacks.onConnect = 1;
    }
  }

  void Socket::Listen(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(args[0]->IsNumber()) { // we have been passed a socket handle that has already been bound
      Local<Context> context = isolate->GetCurrentContext();
      Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
      int fd = args[0]->Int32Value(context).ToChecked();
      if(s->socktype == TCP) {
        uv_tcp_t* sock = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
        sock->data = s;
        baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
        baton->callback = (void*)onNewConnection;
        baton->object = sock->data;
        sock->data = baton;
        int status = uv_tcp_init(env->loop, sock);
        if (status) {
          args.GetReturnValue().Set(Integer::New(isolate, status));
          return;
        }
        status = uv_tcp_open(sock, fd);
        if (status) {
          args.GetReturnValue().Set(Integer::New(isolate, status));
          return;
        }
        status = uv_listen((uv_stream_t*)sock, 1024, on_connection);
        if (status) {
          args.GetReturnValue().Set(Integer::New(isolate, status));
          return;
        }
      }
      else if(s->socktype == UNIX) {
        uv_pipe_t* sock = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
        sock->data = s;
        baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
        baton->callback = (void*)onNewConnection;
        baton->object = sock->data;
        sock->data = baton;
        int status = uv_pipe_init(env->loop, sock, 0);
        if (status) {
          args.GetReturnValue().Set(Integer::New(isolate, status));
          return;
        }
        status = uv_pipe_open(sock, fd);
        if (status) {
          args.GetReturnValue().Set(Integer::New(isolate, status));
          return;
        }
        status = uv_listen((uv_stream_t*)sock, SOMAXCONN, on_connection);
        if (status) {
          args.GetReturnValue().Set(Integer::New(isolate, status));
          return;
        }
      }
      else {
        // error
      }
    }
    else if(s->socktype == TCP) { // we are getting a port so must be TCP
      Local<Context> context = isolate->GetCurrentContext();
      Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
      uv_tcp_t* sock = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
      sock->data = s;
      String::Utf8Value str(args.GetIsolate(), args[0]);
      const char* ip_address = *str;
      const unsigned int port = args[1]->IntegerValue(context).ToChecked();
      struct sockaddr_in addr;
      uv_ip4_addr(ip_address, port, &addr);
      baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
      baton->callback = (void*)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      uv_thread_t t = uv_thread_self();
      fprintf(stderr, "thread: %lu\n", t);
      int status = uv_tcp_init_ex(env->loop, sock, AF_INET);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      int fd;
      uv_fileno((uv_handle_t*)sock, &fd);
      int on = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
      status = uv_tcp_bind(sock, (const struct sockaddr*) &addr, 0);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_listen((uv_stream_t*)sock, SOMAXCONN, on_connection);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      s->_stream = (uv_stream_t*)sock;
    }
    else if(s->socktype == UNIX) { // use first argument as path to domain socket
      Local<Context> context = isolate->GetCurrentContext();
      Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
      String::Utf8Value str(args.GetIsolate(), args[0]);
      const char* path = *str;
      uv_pipe_t* sock = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
      sock->data = s;
      baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
      baton->callback = (void*)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      int status = uv_pipe_init(env->loop, sock, 0);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_pipe_bind(sock, path);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_listen((uv_stream_t*)sock, SOMAXCONN, on_connection);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      s->_stream = (uv_stream_t*)sock;
    }
    else {
      // error
      fprintf(stderr, "foo\n");
    }
    args.GetReturnValue().Set(Integer::New(isolate, 0));
  }

  void Socket::Bind(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    Socket* s = ObjectWrap::Unwrap<Socket>(args.Holder());
    if(s->socktype == TCP) {
      Local<Context> context = isolate->GetCurrentContext();
      Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
      const unsigned int port = args[1]->IntegerValue(context).ToChecked();
      uv_tcp_t* sock = (uv_tcp_t*)malloc(sizeof(uv_tcp_t));
      sock->data = s;
      String::Utf8Value str(args.GetIsolate(), args[0]);
      const char* ip_address = *str;
      struct sockaddr_in addr;
      uv_ip4_addr(ip_address, port, &addr);
      baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
      baton->callback = (void*)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      int status = uv_tcp_init_ex(env->loop, sock, AF_INET);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      int fd;
      uv_fileno((uv_handle_t*)sock, &fd);
      int on = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &on, sizeof(on));
      status = uv_tcp_bind(sock, (const struct sockaddr*) &addr, 0);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
    }
    else if(s->socktype == UNIX) { // it is a domain socket
      Local<Context> context = isolate->GetCurrentContext();
      Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
      String::Utf8Value str(args.GetIsolate(), args[0]);
      const char* path = *str;
      uv_pipe_t* sock = (uv_pipe_t*)malloc(sizeof(uv_pipe_t));
      sock->data = s;
      baton_t* baton = (baton_t*)malloc(sizeof(baton_t));
      baton->callback = (void*)onNewConnection;
      baton->object = sock->data;
      sock->data = baton;
      int status = uv_pipe_init(env->loop, sock, 0);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
      status = uv_pipe_bind(sock, path);
      if (status) {
        args.GetReturnValue().Set(Integer::New(isolate, status));
        return;
      }
    }
    else {
      // error
    }
    args.GetReturnValue().Set(Integer::New(isolate, 0));
  }
}
}