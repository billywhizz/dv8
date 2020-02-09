#include "jsyshttp.h"

namespace dv8 {

namespace jsyshttp {

	void setup_context(jsys_stream_context* context, size_t buffer_size) {
		context->in = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_in");
		context->out = (struct iovec*)context->loop->alloc(1, sizeof(struct iovec), "context_out");
		const char* r200 = "HTTP/1.1 200 OK\r\nContent-Length: 0\r\n\r\n";
		int len = strlen(r200);
		context->current_buffer = 0;
		context->out->iov_base = (void*)r200;
		context->out->iov_len = len;
		context->in->iov_base = context->loop->alloc(1, buffer_size, "context_in_base");
		context->in->iov_len = buffer_size;
	}

	void free_context(jsys_stream_context* context) {
		jsys_loop* loop = context->loop;
		loop->free(context->in->iov_base, "context_in_base");
		loop->free(context->in, "context_in");
		loop->free(context->out, "context_out");
	}

	int httpd_on_connect(jsys_descriptor *client) {
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
		setup_context(context, http_settings->buffer_size);
		return 0;
	}

	int httpd_on_headers(jsys_descriptor *client) {
		return 0;
	}

	int httpd_on_body(jsys_descriptor *client, char* chunk, size_t len) {
		return 0;
	}

	int httpd_on_request(jsys_descriptor *client) {
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		jsys_stream_context* context = (jsys_stream_context*)client->data;
		jsys_httpd_settings* http_settings = (jsys_httpd_settings *)context->settings->data;
	  JSYSHttp *socket = static_cast<JSYSHttp*>(http_settings->data);
    if (socket->callbacks.onRequest == 1) {
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Object> global = ctx->Global();
      Local<Function> Callback = Local<Function>::New(isolate, socket->onRequestCallback);
      Local<Value> argv[] = {};
      Callback->Call(ctx, global, 0, argv);
			context->current_buffer = 1;
    } else {
			context->current_buffer = 1;
		}
	#if TRACE
		jsys_http_server_context* http = (jsys_http_server_context*)context->data;
		fprintf(stderr, "httpd_on_request (%i), size: (%lu)\n", client->fd, http->header_size);
		fprintf(stderr, "  method   : %.*s\n", (int)http->method_len, http->method);
		fprintf(stderr, "  path     : %.*s\n", (int)http->path_len, http->path);
		fprintf(stderr, "  version  : 1.%i\n", http->minor_version);
		fprintf(stderr, "  body     : %lu\n", http->body_length);
		fprintf(stderr, "  bytes    : %lu\n", http->body_bytes);
		fprintf(stderr, "  headers  :\n");
		size_t i = 0;
		while (i < http->num_headers) {
			fprintf(stderr, "    %.*s : %.*s\n", (int)http->headers[i].name_len, http->headers[i].name, (int)http->headers[i].value_len, http->headers[i].value);
			i++;
		}
	#endif
		return 0;
	}

	int httpd_on_data(jsys_descriptor *client, size_t len) {
		return 0;
	}

	int httpd_on_end(jsys_descriptor *client) {
		free_context((jsys_stream_context*)client->data);
		return 0;
	}

	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports)
	{
		JSYSHttp::Init(exports);
	}

	void JSYSHttp::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
		tpl->SetClassName(String::NewFromUtf8(isolate, "JSYSHttp").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "listen", JSYSHttp::Listen);
	  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRequest", JSYSHttp::onRequest);

		DV8_SET_EXPORT(isolate, tpl, "Socket", exports);
	}

	void JSYSHttp::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		if (args.IsConstructCall()) {
			JSYSHttp* obj = new JSYSHttp();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void JSYSHttp::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "JSYSHttp::Destroy\n");
		#endif
	}

	void JSYSHttp::onRequest(const v8::FunctionCallbackInfo<v8::Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		JSYSHttp *obj = ObjectWrap::Unwrap<JSYSHttp>(args.Holder());
		if (args[0]->IsFunction())
		{
			Local<Function> onRequest = Local<Function>::Cast(args[0]);
			obj->onRequestCallback.Reset(isolate, onRequest);
			obj->callbacks.onRequest = 1;
		}
		args.GetReturnValue().Set(args.Holder());
	}

	void JSYSHttp::Listen(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		JSYSHttp* obj = ObjectWrap::Unwrap<JSYSHttp>(args.Holder());

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
	void* _register_jsyshttp() {
		return (void*)dv8::jsyshttp::InitAll;
	}
}
