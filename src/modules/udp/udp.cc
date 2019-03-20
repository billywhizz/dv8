#include "udp.h"

namespace dv8 {

namespace udp {
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

	void after_send(uv_udp_send_t* req, int status) {
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		UDP *obj = (UDP *)req->handle->data;
		Local<Value> argv[1] = { Number::New(isolate, status) };
		Local<Function> onSend = Local<Function>::New(isolate, obj->onSend);
		onSend->Call(isolate->GetCurrentContext()->Global(), 1, argv);
		free(req);
	}

	void after_read(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr* addr, unsigned flags) {
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		UDP *obj = (UDP *)handle->data;
		v8::TryCatch try_catch(isolate);
		if (nread > 0) {
			char ip[INET_ADDRSTRLEN];
			int len = sizeof ip;
			const sockaddr_in *a4 = reinterpret_cast<const sockaddr_in *>(addr);
			uv_inet_ntop(AF_INET, &a4->sin_addr, ip, len);
			len = strlen(ip);
      Local<Value> argv[3] = { Number::New(isolate, nread), String::NewFromUtf8(isolate, ip, v8::String::kNormalString, len), Number::New(isolate, ntohs(a4->sin_port)) };
      Local<Function> onMessage = Local<Function>::New(isolate, obj->onMessage);
      onMessage->Call(isolate->GetCurrentContext()->Global(), 3, argv);
		}
	}

	void on_close(uv_handle_t *handle) {
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		UDP *obj = (UDP *)handle->data;
		Local<Value> argv[0] = {};
		Local<Function> onMessage = Local<Function>::New(isolate, obj->onClose);
		onMessage->Call(isolate->GetCurrentContext()->Global(), 0, argv);
		free(handle);
	}

	void alloc_chunk(uv_handle_t *handle, size_t size, uv_buf_t *buf) {
		UDP *obj = (UDP *)handle->data;
		buf->base = obj->in.base;
		buf->len = obj->in.len;
	}

	void UDP::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "UDP"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "open", UDP::Open);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", UDP::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "bind", UDP::Bind);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "send", UDP::Send);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", UDP::Start);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", UDP::Stop);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", UDP::Close);

		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "getPeerName", UDP::GetPeerName);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onMessage", UDP::OnMessage);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", UDP::OnError);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onClose", UDP::OnClose);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onSend", UDP::OnSend);

		DV8_SET_EXPORT(isolate, tpl, "UDP", exports);
	}

	void UDP::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			UDP* obj = new UDP();
			Local<Context> context = isolate->GetCurrentContext();
			Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(32));
			obj->handle = (uv_udp_t *)calloc(1, sizeof(uv_udp_t));
			int r = uv_udp_init(env->loop, obj->handle);
			obj->handle->data = obj;
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void UDP::Open(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::GetPeerName(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		struct sockaddr_storage address;
		int addrlen = sizeof(address);
		int r = uv_udp_getpeername(obj->handle, reinterpret_cast<sockaddr *>(&address), &addrlen);
		if (r)
		{
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		const sockaddr *addr = reinterpret_cast<const sockaddr *>(&address);
		char ip[INET_ADDRSTRLEN];
		const sockaddr_in *a4;
		a4 = reinterpret_cast<const sockaddr_in *>(addr);
		int len = sizeof ip;
		uv_inet_ntop(AF_INET, &a4->sin_addr, ip, len);
		len = strlen(ip);
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, ip, v8::String::kNormalString, len));
		return;
	}

	void UDP::Send(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
    String::Utf8Value str(args.GetIsolate(), args[1]);
    const unsigned int port = args[2]->IntegerValue(context).ToChecked();
    const char *ip_address = *str;
    struct sockaddr_in addr;
    uv_ip4_addr(ip_address, port, &addr);
		uv_udp_send_t* message = (uv_udp_send_t*)calloc(1, sizeof(uv_udp_send_t));
		uv_buf_t buf;
		buf.base = obj->out.base;
		buf.len = len;
		int r = uv_udp_send(message, obj->handle, &buf, 1, (const struct sockaddr *)&addr, after_send);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void UDP::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());

		Buffer* b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
		obj->in = uv_buf_init((char *)b->_data, b->_length);
		b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->out = uv_buf_init((char *)b->_data, b->_length);

		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::Bind(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
    String::Utf8Value str(args.GetIsolate(), args[0]);
    const unsigned int port = args[1]->IntegerValue(context).ToChecked();
    const char *ip_address = *str;
    struct sockaddr_in addr;
    uv_ip4_addr(ip_address, port, &addr);
    int r = uv_udp_bind(obj->handle, (const struct sockaddr *)&addr, UV_UDP_REUSEADDR);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void UDP::Start(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
    int r = uv_udp_recv_start(obj->handle, alloc_chunk, after_read);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::Stop(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
    int r = uv_udp_recv_stop(obj->handle);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
    uv_handle_t *handle = (uv_handle_t *)obj->handle;
    uv_close(handle, on_close);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::OnClose(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		if (args[0]->IsFunction()) {
			Local<Function> onClose = Local<Function>::Cast(args[0]);
			obj->onClose.Reset(isolate, onClose);
		}
	}

	void UDP::OnError(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		if (args[0]->IsFunction()) {
			Local<Function> onError = Local<Function>::Cast(args[0]);
			obj->onError.Reset(isolate, onError);
		}
	}

	void UDP::OnMessage(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		if (args[0]->IsFunction()) {
			Local<Function> onMessage = Local<Function>::Cast(args[0]);
			obj->onMessage.Reset(isolate, onMessage);
		}
	}

	void UDP::OnSend(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		if (args[0]->IsFunction()) {
			Local<Function> onSend = Local<Function>::Cast(args[0]);
			obj->onSend.Reset(isolate, onSend);
		}
	}

}
}	
