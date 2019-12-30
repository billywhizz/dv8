#include "picoHttpParser.h"

namespace dv8 {

namespace picoHttpParser {
using dv8::builtins::Environment;
using dv8::builtins::Buffer;
using dv8::socket::Socket;
using dv8::socket::socket_plugin;

	void InitAll(Local<Object> exports) {
		PicoHTTPParser::Init(exports);
	}

	void PicoHTTPParser::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "PicoHTTPParser").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "reset", PicoHTTPParser::Reset);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", PicoHTTPParser::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onHeaders", PicoHTTPParser::onHeaders);
	
		DV8_SET_EXPORT(isolate, tpl, "PicoHTTPParser", exports);
	}

	void PicoHTTPParser::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			PicoHTTPParser* obj = new PicoHTTPParser();
			obj->context = (_context *)calloc(sizeof(_context), 1);
    	obj->callbacks.onHeaders = 0;
    	obj->callbacks.onRequest = 0;
    	obj->callbacks.onBody = 0;
    	obj->callbacks.onError = 0;
    	obj->callbacks.onResponse = 0;
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void on_plugin_close(void* obj) {
		socket_plugin* plugin = (socket_plugin*)obj;
		PicoHTTPParser* parser = (PicoHTTPParser*)plugin->data;
		dv8::socket::socket_plugin* parserPlugin = (dv8::socket::socket_plugin*)parser->plugin;
		if (parserPlugin->next) {
			parserPlugin->next->onClose(parserPlugin->next);
		}
	}

	uint32_t on_plugin_read_data(uint32_t nread, void* obj) {
		if (nread <= 0) return 0;
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		socket_plugin* plugin = (socket_plugin*)obj;
		PicoHTTPParser* parser = (PicoHTTPParser*)plugin->data;
		const char* haystack = parser->context->base;
    const char* needle = "\r\n\r\n";
		int nlen = 4;
		int hlen = nread;
		__m128i needle16 = _mm_loadu_si128((const __m128i *)needle);
		int off = 0;
		const char* buf = haystack;
		int r = 0;
		__m128i haystack16;
		while (off < hlen) {
			haystack16 = _mm_loadu_si128((const __m128i *)buf);
			r = _mm_cmpestri(needle16, nlen, haystack16, 16, _SIDD_CMP_EQUAL_ORDERED);
			if (r < (16 - nlen)) {
				if (parser->callbacks.onHeaders == 1) {
					Local<Value> argv[0] = {};
					Local<Function> onHeaders = Local<Function>::New(isolate, parser->_onHeaders);
					Local<Context> context = isolate->GetCurrentContext();
					onHeaders->Call(context, context->Global(), 0, argv);
				}
			}
			off += 16 - nlen;
			buf += 16 - nlen;
		}
		dv8::socket::socket_plugin* parserPlugin = (dv8::socket::socket_plugin*)parser->plugin;
		if (parserPlugin->next) {
			uint32_t r = parserPlugin->next->onRead(nread, parserPlugin->next);
			parserPlugin->next->onClose(parserPlugin->next);
		}
		return 0;
	}

	void PicoHTTPParser::Setup(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		PicoHTTPParser *obj = ObjectWrap::Unwrap<PicoHTTPParser>(args.Holder());
		v8::HandleScope handleScope(isolate);
		int argc = args.Length();
		Buffer *buf = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
		obj->context->base = buf->_data;
		buf = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->context->buf = buf->_data;
		obj->context->workBufferLength = buf->_length;
	}

	void PicoHTTPParser::onHeaders(const v8::FunctionCallbackInfo<v8::Value> &args) {
		Isolate *isolate = args.GetIsolate();
		PicoHTTPParser *parser = ObjectWrap::Unwrap<PicoHTTPParser>(args.Holder());
		v8::HandleScope handleScope(isolate);
		if (args[0]->IsFunction()) {
			Local<Function> onHeaders = Local<Function>::Cast(args[0]);
			parser->_onHeaders.Reset(isolate, onHeaders);
			parser->callbacks.onHeaders = 1;
		}
	}

	void PicoHTTPParser::Reset(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		PicoHTTPParser *obj = ObjectWrap::Unwrap<PicoHTTPParser>(args.Holder());
		v8::HandleScope handleScope(isolate);
		int argc = args.Length();
		if (argc > 0) {
			Socket* sock = ObjectWrap::Unwrap<Socket>(args[0].As<v8::Object>());
			socket_plugin* plugin = (socket_plugin*)calloc(1, sizeof(socket_plugin));
			plugin->data = obj;
			plugin->onRead = &on_plugin_read_data;
			plugin->onClose = &on_plugin_close;
			if (!sock->first) {
				sock->last = sock->first = plugin;
			} else {
				sock->last->next = plugin;
				sock->last = plugin;
			}
			plugin->next = 0;
			obj->plugin = plugin;
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
}
}	

extern "C" {
	void* _register_picoHttpParser() {
		return (void*)dv8::picoHttpParser::InitAll;
	}
}
