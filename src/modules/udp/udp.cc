#include "udp.h"

namespace dv8 {

namespace udp {
	using dv8::builtins::Buffer;
	using dv8::builtins::Environment;

	void InitAll(Local<Object> exports) {
		UDP::Init(exports);
	}
/*
	int on_udp_event(jsys_descriptor *client) {
		int r = 0;
		if (jsys_descriptor_is_readable(client)) {
			ssize_t bytes = 0;
			UDP *obj = (UDP *)client->data;
			char* buf = static_cast<char*>(obj->in.iov_base);
			size_t len = obj->in.iov_len;
			int fd = client->fd;
			Isolate *isolate = Isolate::GetCurrent();
			v8::HandleScope handleScope(isolate);
			Local<Function> onMessage = Local<Function>::New(isolate, obj->onMessage);
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Value> argv[1] = { Number::New(isolate, 0) };
			while ((bytes = read(fd, buf, len))) {
				if (bytes == -1) {
					if (errno == EAGAIN) {
						break;
					}
					perror("read");
					break;
				}
				argv[0] = Number::New(isolate, bytes);
				// todo: try/catch
				onMessage->Call(ctx, ctx->Global(), 1, argv);
			}
		}
		return r;
	}
*/
	int on_udp_event(jsys_descriptor *client) {
		int r = 0;
		if (jsys_descriptor_is_readable(client)) {
			ssize_t bytes = 0;
			UDP *obj = (UDP *)client->data;
			int fd = client->fd;
			Isolate *isolate = Isolate::GetCurrent();
			v8::HandleScope handleScope(isolate);
			Local<Function> onMessage = Local<Function>::New(isolate, obj->onMessage);
			Local<Context> ctx = isolate->GetCurrentContext();
			Local<Value> argv[3] = { v8::Null(isolate), v8::Null(isolate), v8::Null(isolate) };
			char ip[INET_ADDRSTRLEN];
			int iplen = sizeof ip;
			struct sockaddr_storage peer;
			struct msghdr h;
			memset(&h, 0, sizeof(h));
			memset(&peer, 0, sizeof(peer));
			h.msg_name = &peer;
			h.msg_namelen = sizeof(peer);
			h.msg_iov = &obj->in;
			h.msg_iovlen = 1;

			const sockaddr_in *a4 = reinterpret_cast<const sockaddr_in *>(&peer);

			while ((bytes = recvmsg(fd, &h, 0))) {
				if (bytes == -1) {
					if (errno == EAGAIN || errno == EWOULDBLOCK) {
						break;
					}
					if (errno == EINTR) continue;
					perror("read");
					break;
				}
				inet_ntop(AF_INET, &a4->sin_addr, ip, iplen);
				argv[0] = Number::New(isolate, bytes);
				argv[1] = String::NewFromUtf8(isolate, ip, v8::NewStringType::kNormal, iplen).ToLocalChecked();
				argv[2] = Number::New(isolate, ntohs(a4->sin_port));
				// todo: try/catch
				onMessage->Call(ctx, ctx->Global(), 3, argv);
			}
		}
		return r;
	}

	void UDP::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "UDP").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", UDP::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "bind", UDP::Bind);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "send", UDP::Send);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", UDP::Start);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "stop", UDP::Stop);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", UDP::Close);

		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "getPeerName", UDP::GetPeerName);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "getSockName", UDP::GetSockName);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setBroadcast", UDP::SetBroadcast);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onMessage", UDP::OnMessage);

		DV8_SET_EXPORT(isolate, tpl, "UDP", exports);
	}

	void UDP::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			UDP* obj = new UDP();
			Local<Context> context = isolate->GetCurrentContext();
			Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
			jsys_descriptor* handle = jsys_descriptor_create(env->loop);
			obj->handle = handle;
			handle->data = obj;
			handle->type = JSYS_UDP;
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void UDP::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "UDP::Destroy\n");
		#endif
	}

	void UDP::GetPeerName(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
/*
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
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, ip, v8::NewStringType::kNormal, len).ToLocalChecked());
		return;
*/
	}

	void UDP::GetSockName(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
/*
		struct sockaddr_storage address;
		int addrlen = sizeof(address);
		int r = uv_udp_getsockname(obj->handle, reinterpret_cast<sockaddr *>(&address), &addrlen);
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
		char pair[128];
		snprintf(pair, 128, "%s:%u", ip, a4->sin_port);
		args.GetReturnValue().Set(String::NewFromUtf8(isolate, pair, v8::NewStringType::kNormal, strlen(pair)).ToLocalChecked());
		return;
*/
	}

	void UDP::SetBroadcast(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
    const int on = args[0]->IntegerValue(context).ToChecked();
		args.GetReturnValue().Set(Integer::New(isolate, jsys_socket_option(obj->handle, SOL_SOCKET, SO_BROADCAST, 1)));
	}

	void UDP::Send(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
    String::Utf8Value str(args.GetIsolate(), args[1]);
    const unsigned int port = args[2]->IntegerValue(context).ToChecked();
    char *ip_address = *str;
		int r = jsys_udp_send_len(obj->handle, &obj->out, ip_address, port, len);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void UDP::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());

		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
		obj->in.iov_base = (char*)b->_data;
		obj->in.iov_len = b->_length;
		b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->out.iov_base = (char*)b->_data;
		obj->out.iov_len = b->_length;

		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::Bind(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
    String::Utf8Value str(args.GetIsolate(), args[0]);
		obj->handle->fd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    char *ip_address = *str;
    const unsigned int port = args[1]->IntegerValue(context).ToChecked();
		int r = jsys_udp_bind_reuse(obj->handle, port, ip_address);
		obj->handle->callback = on_udp_event;
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void UDP::Start(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		int r = 0;
		if (env->loop->descriptors[obj->handle->fd] == NULL) {
			r = jsys_loop_add(env->loop, obj->handle);
		}
		if (r == 0) {
			r = jsys_tcp_resume(obj->handle);
		}
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void UDP::Stop(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		int r = jsys_tcp_pause(obj->handle);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void UDP::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		UDP* obj = ObjectWrap::Unwrap<UDP>(args.Holder());
		int r = jsys_descriptor_free(obj->handle);
		args.GetReturnValue().Set(Integer::New(isolate, r));
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

}
}	

extern "C" {
	void* _register_udp() {
		return (void*)dv8::udp::InitAll;
	}
}
