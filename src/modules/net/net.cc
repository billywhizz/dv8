#include "net.h"

namespace dv8 {

namespace net {

// Http 
	void setup_socket_context(jsys_stream_context* context, size_t buffer_size) {
		context->in = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_in");
		context->out = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_out");
		context->in->iov_base = context->loop->alloc(1, buffer_size, "context_in_base");
		context->in->iov_len = buffer_size;
		context->out->iov_base = context->loop->alloc(1, buffer_size, "context_out_base");
		context->out->iov_len = buffer_size;
	}

	void free_socket_context(jsys_stream_context* context) {
		jsys_loop* loop = context->loop;
		loop->free(context->in->iov_base, "context_in_base");
		loop->free(context->out->iov_base, "context_out_base");
		loop->free(context->in, "context_in");
		loop->free(context->out, "context_out");
		loop->free(context, "jsys_stream_context");
	}

	void free_http_context(jsys_stream_context* context) {
		jsys_loop* loop = context->loop;
		//loop->free(context->in->iov_base, "context_in_base");
		//loop->free(context->in, "context_in");
		loop->free(context->out, "context_out");
		loop->free(context, "jsys_stream_context");
	}

	int httpd_on_connect(jsys_descriptor *client) {
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
	  Http *socket = static_cast<Http*>(http_settings->data);
		context->in = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_in");
		context->out = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_out");
		const char* r200 = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
		int len = strlen(r200);
		context->current_buffer = 0;
		context->out->iov_base = (void*)r200;
		context->out->iov_len = len;
		context->in->iov_base = context->loop->alloc(1, http_settings->buffer_size, "context_in_base");
		context->in->iov_len = http_settings->buffer_size;
		return 0;
	}

	int httpd_on_headers(jsys_descriptor *client) {
		return 0;
	}

	int httpd_on_body(jsys_descriptor *client, char* chunk, size_t len) {
		return 0;
	}

	int httpd_on_request(jsys_descriptor *client) {
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
	  Http *socket = static_cast<Http*>(http_settings->data);
    if (socket->callbacks.onRequest == 1) {
			Isolate *isolate = Isolate::GetCurrent();
			v8::HandleScope handleScope(isolate);
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Object> global = ctx->Global();
      Local<Function> Callback = Local<Function>::New(isolate, socket->onRequest);
			jsys_http_server_context* http = (jsys_http_server_context*)context->data;
      uint64_t val = reinterpret_cast<uint64_t>(http);
      Local<Value> argv[1] = { BigInt::NewFromUnsigned(isolate, val) };
      Callback->Call(ctx, global, 1, argv);
		}
		int r = jsys_tcp_write(client, &context->out[0]);
		return 0;
	}

	int httpd_on_data(jsys_descriptor *client, size_t len) {
		return 0;
	}

	int httpd_on_end(jsys_descriptor *client) {
		free_http_context((jsys_stream_context*)client->data);
		return 0;
	}

// Socket 

	int on_socket_event(jsys_descriptor *client) {
		int r = 0;
		if (jsys_descriptor_is_error(client)) {
			jsys_stream_context* context = (jsys_stream_context*)client->data;
			context->settings->on_end(client);
			return jsys_descriptor_free(client);
		}
		if (jsys_descriptor_is_writable(client)) {
			jsys_stream_context* context = (jsys_stream_context*)client->data;
			r = context->settings->on_connect(client);
			if (r == -1) return jsys_descriptor_free(client);
			return r;
		}
		if (jsys_descriptor_is_readable(client)) {
			ssize_t bytes = 0;
			jsys_stream_context* context = (jsys_stream_context*)client->data;
			size_t len = context->in->iov_len;
			char* buf = static_cast<char*>(context->in->iov_base);
			int count = 32;
			int fd = client->fd;
			while ((bytes = read(fd, buf, len))) {
				if (bytes == -1) {
					if (errno == EAGAIN) {
						break;
					}
					perror("read");
					break;
				}
				r = context->settings->on_data(client, (size_t)bytes);
				if (r == -1) break;
				if (--count == 0) break;
			}
		}
		return r;
	}

	int on_client_connect(jsys_descriptor* client) {
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		int r = jsys_loop_mod_flags(client, EPOLLIN | EPOLLET);
		if (r != 0) return r;
		//setup_socket_context(context, 16384);
		Socket* sock = static_cast<Socket*>(context->settings->data);
		if (sock->callbacks.onConnect == 1) {
			Isolate *isolate = Isolate::GetCurrent();
			v8::HandleScope handleScope(isolate);
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Object> global = ctx->Global();
			Local<Value> argv[0] = { };
			Local<Function> callback = Local<Function>::New(isolate, sock->onConnect);
			callback->Call(ctx, global, 0, argv);
		}
		return 0;
	}

	int on_client_data(jsys_descriptor *client, size_t bytes) {
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		Socket* sock = static_cast<Socket*>(context->settings->data);
		if (sock->callbacks.onData == 1) {
			Isolate *isolate = Isolate::GetCurrent();
			v8::HandleScope handleScope(isolate);
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Object> global = ctx->Global();
			Local<Value> argv[1] = { Number::New(isolate, bytes) };
			Local<Function> callback = Local<Function>::New(isolate, sock->onData);
			callback->Call(ctx, global, 1, argv);
		}
		return 0;
	}

	int on_client_end(jsys_descriptor* client) {
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		Socket* sock = static_cast<Socket*>(context->settings->data);
		if (sock->callbacks.onEnd == 1) {
			Isolate *isolate = Isolate::GetCurrent();
			v8::HandleScope handleScope(isolate);
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Object> global = ctx->Global();
			Local<Value> argv[0] = { };
			Local<Function> callback = Local<Function>::New(isolate, sock->onEnd);
			callback->Call(ctx, global, 0, argv);
		}
		//free_socket_context((jsys_stream_context*)client->data);
		return 0;
	}

	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		Http::Init(exports);
		Socket::Init(exports);
	}

	void Http::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, Http::New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "Http").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listen", Http::Listen);
	  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRequest", Http::OnRequest);

		DV8_SET_EXPORT(isolate, tpl, "Http", exports);
	}

	void Socket::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, Socket::New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "Socket").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);

		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "open", Socket::Open);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pair", Socket::Pair);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "bind", Socket::Bind);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listen", Socket::Listen);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "connect", Socket::Connect);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", Socket::Write);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "pause", Socket::Pause);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "resume", Socket::Resume);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", Socket::Close);

		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Socket::Setup);

	  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onConnect", Socket::OnConnect);
	  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onData", Socket::OnData);
	  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onEnd", Socket::OnEnd);

		DV8_SET_EXPORT(isolate, tpl, "Socket", exports);
	}

	void Http::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			Http* obj = new Http();
			obj->Wrap(args.This());
			obj->callbacks.onRequest = 0;
			args.GetReturnValue().Set(args.This());
		}
	}

	void Socket::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			Socket* obj = new Socket();
			obj->Wrap(args.This());
			obj->handle = nullptr;
			obj->callbacks.onConnect = 0;
			obj->callbacks.onData = 0;
			obj->callbacks.onEnd = 0;
			args.GetReturnValue().Set(args.This());
		}
	}

	void Http::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Http::Destroy\n");
		#endif
	}

	void Socket::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "Socket::Destroy\n");
		#endif
/*
		Isolate *isolate = data.GetIsolate();
		v8::HandleScope handleScope(isolate);
		ObjectWrap *wrap = data.GetParameter();
		Socket* sock = static_cast<Socket *>(wrap);
		if (sock->handle != nullptr) {
			fprintf(stderr, "we have a socket\n");
		}
*/
	}

	void Http::OnRequest(const v8::FunctionCallbackInfo<v8::Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Http *obj = ObjectWrap::Unwrap<Http>(args.Holder());
		if (args[0]->IsFunction())
		{
			Local<Function> onRequest = Local<Function>::Cast(args[0]);
			obj->onRequest.Reset(isolate, onRequest);
			obj->callbacks.onRequest = 1;
		}
		args.GetReturnValue().Set(args.Holder());
	}

	void Socket::OnConnect(const v8::FunctionCallbackInfo<v8::Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Socket *obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		if (args[0]->IsFunction())
		{
			Local<Function> onConnect = Local<Function>::Cast(args[0]);
			obj->onConnect.Reset(isolate, onConnect);
			obj->callbacks.onConnect = 1;
		}
		args.GetReturnValue().Set(args.Holder());
	}

	void Socket::OnData(const v8::FunctionCallbackInfo<v8::Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Socket *obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		if (args[0]->IsFunction())
		{
			Local<Function> onData = Local<Function>::Cast(args[0]);
			obj->onData.Reset(isolate, onData);
			obj->callbacks.onData = 1;
		}
		args.GetReturnValue().Set(args.Holder());
	}

	void Socket::OnEnd(const v8::FunctionCallbackInfo<v8::Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Socket *obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		if (args[0]->IsFunction())
		{
			Local<Function> onEnd = Local<Function>::Cast(args[0]);
			obj->onEnd.Reset(isolate, onEnd);
			obj->callbacks.onEnd = 1;
		}
		args.GetReturnValue().Set(args.Holder());
	}

	void Socket::Pair(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> ctx = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(ctx->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		int argc = args.Length();
		int rc = 0;
		int fd = 0;
		if (argc == 0) {
			int fds[2];
			socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0, fds);
			fd = fds[0];
			rc = fds[1];
			obj->pairfd = rc;
		} else if (argc == 1) {
      fd = args[0]->Int32Value(ctx).ToChecked();
			rc = fd;
		}
		jsys_loop* loop = env->loop;
		jsys_descriptor* sock = jsys_descriptor_create(loop);
		sock->fd = fd;
		sock->type = JSYS_SOCKET;
		jsys_stream_settings* settings = (jsys_stream_settings*)calloc(1, sizeof(jsys_stream_settings));
		settings->on_connect = on_client_connect;
		settings->on_data = on_client_data;
		settings->on_end = on_client_end;
		settings->data = obj;
		settings->buffers = 1;
		jsys_stream_context* context = jsys_stream_context_create(loop, 1);
		context->settings = settings;
		context->offset = 0;
		context->data = settings;
		sock->data = context;
		sock->closing = 0;
		obj->handle = sock;
		sock->callback = on_socket_event;
  	jsys_loop_add_flags(loop, sock, EPOLLOUT);
		args.GetReturnValue().Set(Integer::New(isolate, rc));
	}

	void Socket::Open(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Socket::Bind(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Socket::Listen(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Socket::Connect(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Socket::Write(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> ctx = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(ctx->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		jsys_descriptor* sock = obj->handle;
		jsys_stream_context* context = (jsys_stream_context*)sock->data;
		int len = args[0]->Int32Value(ctx).ToChecked();
		int r = jsys_tcp_write_len(sock, context->out, len);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void Socket::Pause(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		jsys_descriptor* sock = obj->handle;
		int r = jsys_tcp_pause(sock);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void Socket::Resume(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		jsys_descriptor* sock = obj->handle;
		int r = jsys_tcp_resume(sock);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void Socket::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		jsys_descriptor_free(obj->handle);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Socket::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> ctx = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(ctx->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Socket* obj = ObjectWrap::Unwrap<Socket>(args.Holder());
		jsys_descriptor* sock = obj->handle;
		jsys_stream_context* context = (jsys_stream_context*)sock->data;
		int argc = args.Length();
		if (argc == 2) {
			Buffer *in = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
			Buffer *out = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
			context->in = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_in");
			context->out = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_out");
			context->in->iov_base = in->_data;
			context->in->iov_len = in->_length;
			context->out->iov_base = out->_data;
			context->out->iov_len = out->_length;
			args.GetReturnValue().Set(Integer::New(isolate, 0));
			return;
		}
		args.GetReturnValue().Set(Integer::New(isolate, -1));
	}

	void Http::Listen(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		Http* obj = ObjectWrap::Unwrap<Http>(args.Holder());

    String::Utf8Value str(args.GetIsolate(), args[0]);
    const char *ip_address = *str;
    const unsigned int port = args[1]->IntegerValue(context).ToChecked();

		jsys_httpd_settings* http_settings = jsys_http_create_httpd_settings(env->loop);
		http_settings->on_headers = httpd_on_headers;
		http_settings->on_request = httpd_on_request;
		http_settings->on_body = httpd_on_body;
		http_settings->on_connect = httpd_on_connect;
		http_settings->on_end = httpd_on_end;
		http_settings->on_data = httpd_on_data;
		http_settings->buffer_size = 16384;
		http_settings->max_headers = 16;
		http_settings->domain = AF_INET;
		http_settings->type = SOCK_STREAM;
		http_settings->buffers = 1;
		http_settings->data = obj;
		jsys_descriptor* server = jsys_http_create_server(env->loop, http_settings);
		struct sockaddr_in addr;
		int r = inet_aton(ip_address, &addr.sin_addr);
		if (r == -1) {
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		r = jsys_tcp_bind_reuse(server, port, addr.sin_addr.s_addr);
		if (r == -1) {
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		r = jsys_tcp_listen(server, SOMAXCONN);
		if (r == -1) {
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		r = jsys_loop_add(env->loop, server);
		if (r == -1) {
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

}
}	

extern "C" {
	void* _register_net() {
		return (void*)dv8::net::InitAll;
	}
}
