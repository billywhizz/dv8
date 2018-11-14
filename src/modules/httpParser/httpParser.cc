#include "httpParser.h"

namespace dv8
{

namespace httpParser
{
int message_begin_cb(http_parser *parser) {
	_context *context = (_context *)parser->data;
	context->request->urllength = 0;
	context->request->headerLength = 0;
	context->request->lastel = NONE;
	context->soff = STRING_OFFSET;
	return 0;
}

int url_cb(http_parser *parser, const char *buf, size_t len) {
	_context *context = (_context *)parser->data;
	uint8_t *work = (uint8_t *)context->buf + context->soff;
	memcpy(work, buf, len);
	context->soff += len;
	context->request->urllength += len;
	return 0;
}

int header_field_cb(http_parser *parser, const char *buf, size_t len) {
	_context *context = (_context *)parser->data;
	uint8_t *work = (uint8_t *)context->buf + context->soff;
	if (context->request->lastel == VALUE) {
		memcpy(work, "\r\n", 2);
		work += 2;
		context->request->headerLength += 2;
		context->soff += 2;
	}
	context->request->headerLength += len;
	memcpy(work, buf, len);
	context->soff += len;
	context->request->lastel = FIELD;
	return 0;
}

int header_value_cb(http_parser *parser, const char *buf, size_t len) {
	_context *context = (_context *)parser->data;
	uint8_t *work = (uint8_t *)context->buf + context->soff;
	if (context->request->lastel != VALUE) {
		memcpy(work, ": ", 2);
		work += 2;
		context->request->headerLength += 2;
		context->soff += 2;
	}
	memcpy(work, buf, len);
	context->request->headerLength += len;
	context->soff += len;
	context->request->lastel = VALUE;
	return 0;
}

int body_cb(http_parser *parser, const char *buf, size_t len) {
	if (len > 0) {
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		_context *context = (_context *)parser->data;
		HTTPParser *obj = (HTTPParser *)context->data;
		if (obj->callbacks.onBody == 1) {
			uint8_t *work = (uint8_t *)context->buf;
			memcpy(work, buf, len);
			Local<Value> argv[1] = {Integer::New(isolate, len)};
			Local<Function> onBody = Local<Function>::New(isolate, obj->_onBody);
			onBody->Call(isolate->GetCurrentContext()->Global(), 1, argv);
		}
	}
	return 0;
}

int headers_complete_cb(http_parser *parser) {
	Isolate *isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	_context *context = (_context *)parser->data;
	HTTPParser *obj = (HTTPParser *)context->data;
	uint8_t *work = (uint8_t *)context->buf;
	work[0] = parser->http_major;
	work[1] = parser->http_minor;
	if (context->parser_mode == 0) {
		work[2] = 0xff & (http_method)parser->method;
		work[3] = parser->upgrade;
	} else {
		work[2] = 0xff & (parser->status_code >> 8);
		work[3] = 0xff & parser->status_code;
	}
	work[4] = http_should_keep_alive(parser);
	work[5] = 0xff & (context->request->urllength >> 8);
	work[6] = 0xff & context->request->urllength;
	work[7] = 0xff & (context->request->headerLength >> 8);
	work[8] = 0xff & context->request->headerLength;
	if (obj->callbacks.onHeaders == 1) {
		Local<Value> argv[0] = {};
		Local<Function> onHeaders = Local<Function>::New(isolate, obj->_onHeaders);
		onHeaders->Call(isolate->GetCurrentContext()->Global(), 0, argv);
	}
	return 0;
}

int message_complete_cb(http_parser *parser) {
	Isolate *isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	_context *context = (_context *)parser->data;
	HTTPParser *obj = (HTTPParser *)context->data;
	if (context->parser_mode == 0 && obj->callbacks.onRequest == 1) {
		Local<Value> argv[0] = {};
		Local<Function> onRequest = Local<Function>::New(isolate, obj->_onRequest);
		onRequest->Call(isolate->GetCurrentContext()->Global(), 0, argv);
	} else if (context->parser_mode == 1 && obj->callbacks.onResponse == 1) {
		Local<Value> argv[0] = {};
		Local<Function> onResponse = Local<Function>::New(isolate, obj->_onResponse);
		onResponse->Call(isolate->GetCurrentContext()->Global(), 0, argv);
	}
	return 0;
}

void HTTPParser::Init(Local<Object> exports) {
	Isolate *isolate = exports->GetIsolate();
	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "HTTPParser"));
	tpl->InstanceTemplate()->SetInternalFieldCount(1);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", HTTPParser::Setup);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "reset", HTTPParser::Reset);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "execute", HTTPParser::Execute);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onHeaders", HTTPParser::onHeaders);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onBody", HTTPParser::onBody);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRequest", HTTPParser::onRequest);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onResponse", HTTPParser::onResponse);
	DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", HTTPParser::onError);
	DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, 0), "REQUEST", exports);
	DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, 1), "RESPONSE", exports);
	DV8_SET_EXPORT(isolate, tpl, "HTTPParser", exports);
}

void HTTPParser::New(const FunctionCallbackInfo<Value> &args) {
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	if (args.IsConstructCall()) {
		HTTPParser *obj = new HTTPParser();
		obj->Wrap(args.This());
		obj->context = (_context *)calloc(sizeof(_context), 1);
		obj->context->parser = (http_parser *)calloc(sizeof(http_parser), 1);
		obj->context->request = (_request *)calloc(sizeof(_request), 1);
		obj->context->parser->data = obj->context;
		obj->context->data = obj;
    	obj->callbacks.onHeaders = 0;
    	obj->callbacks.onRequest = 0;
    	obj->callbacks.onBody = 0;
    	obj->callbacks.onError = 0;
    	obj->callbacks.onResponse = 0;
		args.GetReturnValue().Set(args.This());
	}
}

void HTTPParser::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  HTTPParser* obj = static_cast<HTTPParser *>(wrap);
}

void HTTPParser::Setup(const FunctionCallbackInfo<Value> &args) {
	Isolate *isolate = args.GetIsolate();
	HTTPParser *obj = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
	v8::HandleScope handleScope(isolate);
	int argc = args.Length();
	Buffer *buf = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
	obj->context->base = buf->_data;
	buf = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
	obj->context->buf = buf->_data;
	obj->context->workBufferLength = buf->_length;
}

void on_plugin_close(void* obj) {
	fprintf(stderr, "parser.on_close\n");
	socket_plugin* plugin = (socket_plugin*)obj;
	HTTPParser* parser = (HTTPParser*)plugin->data;
	if (parser->plugin->next) {
		parser->plugin->next->onClose(parser->plugin->next);
	}
}

uint32_t on_plugin_read_data(uint32_t nread, void* obj) {
	socket_plugin* plugin = (socket_plugin*)obj;
	HTTPParser* parser = (HTTPParser*)plugin->data;
	ssize_t np = http_parser_execute(parser->context->parser, &settings, parser->context->base, nread);
	if (np != nread && parser->context->parser->http_errno == HPE_PAUSED) {
		uint8_t *lastByte = (uint8_t *)(parser->context->base + np);
		parser->context->lastByte = *lastByte;
	}
	if (parser->plugin->next) {
		uint32_t r = parser->plugin->next->onRead(nread, parser->plugin->next);
	}
	return 0;
}

void HTTPParser::Reset(const FunctionCallbackInfo<Value> &args) {
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HTTPParser *obj = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
	v8::HandleScope handleScope(isolate);
	int argc = args.Length();
	uint32_t mode = args[0]->Uint32Value(context).ToChecked();
	http_parser_init(obj->context->parser, (http_parser_type)mode);
	obj->context->parser_mode = mode;
	if (argc > 1) {
		Socket* sock = ObjectWrap::Unwrap<Socket>(args[1].As<v8::Object>());
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

void HTTPParser::Execute(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	v8::HandleScope handleScope(isolate);
	HTTPParser *obj = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
	uint32_t nread = args[0]->Uint32Value(context).ToChecked();
	ssize_t np = http_parser_execute(obj->context->parser, &settings, obj->context->base, nread);
	if (np != nread) {
		if (obj->context->parser->http_errno == HPE_PAUSED) {
			uint8_t *lastByte = (uint8_t *)(obj->context->base + np);
			obj->context->lastByte = *lastByte;
		} else {
			Local<Value> argv[2] = {Number::New(isolate, obj->context->parser->http_errno), String::NewFromUtf8(isolate, http_errno_description((http_errno)obj->context->parser->http_errno), v8::String::kNormalString)};
			Local<Function> onError = Local<Function>::New(isolate, obj->_onError);
			onError->Call(context->Global(), 2, argv);
		}
	}
	args.GetReturnValue().Set(Integer::New(isolate, 0));
}

void HTTPParser::onHeaders(const v8::FunctionCallbackInfo<v8::Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HTTPParser *parser = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
  if (args[0]->IsFunction()) {
    Local<Function> onHeaders = Local<Function>::Cast(args[0]);
    parser->_onHeaders.Reset(isolate, onHeaders);
    parser->callbacks.onHeaders = 1;
  }
}

void HTTPParser::onBody(const v8::FunctionCallbackInfo<v8::Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HTTPParser *parser = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
  if (args[0]->IsFunction()) {
    Local<Function> onBody = Local<Function>::Cast(args[0]);
    parser->_onBody.Reset(isolate, onBody);
    parser->callbacks.onBody = 1;
  }
}

void HTTPParser::onRequest(const v8::FunctionCallbackInfo<v8::Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HTTPParser *parser = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
  if (args[0]->IsFunction()) {
    Local<Function> onRequest = Local<Function>::Cast(args[0]);
    parser->_onRequest.Reset(isolate, onRequest);
    parser->callbacks.onRequest = 1;
  }
}

void HTTPParser::onResponse(const v8::FunctionCallbackInfo<v8::Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HTTPParser *parser = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
  if (args[0]->IsFunction()) {
    Local<Function> onResponse = Local<Function>::Cast(args[0]);
    parser->_onResponse.Reset(isolate, onResponse);
    parser->callbacks.onResponse = 1;
  }
}

void HTTPParser::onError(const v8::FunctionCallbackInfo<v8::Value> &args) {
  Isolate *isolate = args.GetIsolate();
  HTTPParser *parser = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
  if (args[0]->IsFunction()) {
    Local<Function> onError = Local<Function>::Cast(args[0]);
    parser->_onError.Reset(isolate, onError);
    parser->callbacks.onError = 1;
  }
}

} // namespace httpParser
} // namespace dv8
