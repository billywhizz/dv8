#include "httpParser.h"

namespace dv8
{

namespace httpParser
{
using dv8::builtins::Buffer;
using dv8::builtins::Environment;
using v8::Array;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

int message_begin_cb(http_parser *p)
{
	_context *ctx = (_context *)p->data;
	ctx->request->headerCount = 0;
	ctx->request->urllength = 0;
	uint8_t *rr = (uint8_t *)ctx->buf + 8;
	rr[0] = 0;
	rr[1] = 0;
	rr[2] = 0;
	rr[3] = 0;
	ctx->request->lastel = NONE;
	ctx->index = 12;
	ctx->request->fieldlen = 0;
	return 0;
}

int url_cb(http_parser *p, const char *buf, size_t len)
{
	_context *ctx = (_context *)p->data;
	if (ctx->index + len > ctx->workBufferLength)
	{
		return -1;
	}
	uint8_t *rr = (uint8_t *)ctx->buf + ctx->index;
	memcpy(rr, buf, len);
	ctx->index += len;
	ctx->request->urllength += len;
	ctx->request->fieldlenp = ctx->index;
	return 0;
}

int header_field_cb(http_parser *p, const char *buf, size_t len)
{
	_context *ctx = (_context *)p->data;
	if (ctx->request->lastel != FIELD)
	{
		// new field
		if (ctx->index + len + 2 > ctx->workBufferLength)
		{
			return -1;
		}
		//TODO: check for max headers
		ctx->request->headerCount++;
		uint8_t *rr = (uint8_t *)ctx->buf + ctx->index;
		ctx->request->fieldlen = len;
		ctx->request->fieldlenp = ctx->index;
		rr[0] = 0xff & (len >> 8);
		rr[1] = 0xff & len;
		rr = (uint8_t *)ctx->buf + ctx->index + 2;
		//TODO: buffer overrun - check for end of headers buffer
		ctx->request->key = (char *)rr;
		memcpy(rr, buf, len);
		ctx->index += (len + 2);
	}
	else
	{
		// existing field
		if (ctx->index + len > ctx->workBufferLength)
		{
			return -1;
		}
		ctx->request->fieldlen += len;
		uint8_t *rr = (uint8_t *)ctx->buf + ctx->request->fieldlenp;
		rr[0] = 0xff & (ctx->request->fieldlen >> 8);
		rr[1] = 0xff & ctx->request->fieldlen;
		rr = (uint8_t *)ctx->buf + ctx->index;
		//TODO: buffer overrun - check for end of headers buffer
		memcpy(rr, buf, len);
		ctx->index += len;
	}
	ctx->request->lastel = FIELD;
	return 0;
}

int header_value_cb(http_parser *p, const char *buf, size_t len)
{
	_context *ctx = (_context *)p->data;
	if (ctx->request->lastel != VALUE)
	{
		// new field
		if (ctx->index + len + 2 > ctx->workBufferLength)
		{
			return -1;
		}
		uint8_t *rr = (uint8_t *)ctx->buf + ctx->index;
		ctx->request->fieldlen = len;
		ctx->request->fieldlenp = ctx->index;
		rr[0] = 0xff & (len >> 8);
		rr[1] = 0xff & len;
		rr = (uint8_t *)ctx->buf + ctx->index + 2;
		ctx->request->val = (char *)rr;
		//TODO: buffer overrun - check for end of headers buffer
		memcpy(rr, buf, len);
		ctx->index += (len + 2);
	}
	else
	{
		// existing field
		if (ctx->index + len > ctx->workBufferLength)
		{
			return -1;
		}
		ctx->request->fieldlen += len;
		uint8_t *rr = (uint8_t *)ctx->buf + ctx->request->fieldlenp;
		rr[0] = 0xff & (ctx->request->fieldlen >> 8);
		rr[1] = 0xff & ctx->request->fieldlen;
		rr = (uint8_t *)ctx->buf + ctx->index;
		//TODO: buffer overrun - check for end of headers buffer
		memcpy(rr, buf, len);
		ctx->index += len;
	}
	ctx->request->lastel = VALUE;
	return 0;
}

int body_cb(http_parser *p, const char *buf, size_t len)
{
	if (len > 0)
	{
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		_context *ctx = (_context *)p->data;
		if (len > ctx->workBufferLength)
		{
			return -1;
		}
		HTTPParser *parser = (HTTPParser *)ctx->data;
		if (parser->callbacks.onBody == 1) {
			uint8_t *rr = (uint8_t *)ctx->buf;
			//TODO: buffer overrun - check for end of headers buffer
			memcpy(rr, buf, len);
			Local<Value> argv[1] = {Integer::New(isolate, len)};
			Local<Function> onBody = Local<Function>::New(isolate, parser->_onBody);
			onBody->Call(isolate->GetCurrentContext()->Global(), 1, argv);
		}
	}
	return 0;
}

int headers_complete_cb(http_parser *p)
{
	Isolate *isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	_context *ctx = (_context *)p->data;
	HTTPParser *parser = (HTTPParser *)ctx->data;
	uint8_t *rr = (uint8_t *)ctx->buf;
	rr[0] = p->http_major;
	rr[1] = p->http_minor;
	rr[2] = ctx->request->headerCount;
	if (ctx->parser_mode == 0)
	{
		rr[3] = 0xff & (http_method)p->method;
		rr[4] = p->upgrade;
	}
	else
	{
		rr[3] = 0xff & (p->status_code >> 8);
		rr[4] = 0xff & p->status_code;
	}
	rr[5] = http_should_keep_alive(p);
	rr[8] = 0xff & (ctx->request->urllength >> 24);
	rr[9] = 0xff & (ctx->request->urllength >> 16);
	rr[10] = 0xff & (ctx->request->urllength >> 8);
	rr[11] = 0xff & ctx->request->urllength;
	if (parser->callbacks.onHeaders == 1) {
		Local<Value> argv[1] = {Integer::New(isolate, ctx->index)};
		Local<Function> onHeaders = Local<Function>::New(isolate, parser->_onHeaders);
		onHeaders->Call(isolate->GetCurrentContext()->Global(), 1, argv);
	}
	return 0;
}

int message_complete_cb(http_parser *p)
{
	Isolate *isolate = Isolate::GetCurrent();
	v8::HandleScope handleScope(isolate);
	_context *ctx = (_context *)p->data;
	HTTPParser *parser = (HTTPParser *)ctx->data;
	if (ctx->parser_mode == 0 && parser->callbacks.onRequest == 1)
	{
		Local<Value> argv[0] = {};
		Local<Function> onRequest = Local<Function>::New(isolate, parser->_onRequest);
		onRequest->Call(isolate->GetCurrentContext()->Global(), 0, argv);
	} else if (ctx->parser_mode == 1 && parser->callbacks.onResponse == 1) {
		Local<Value> argv[0] = {};
		Local<Function> onResponse = Local<Function>::New(isolate, parser->_onResponse);
		onResponse->Call(isolate->GetCurrentContext()->Global(), 0, argv);
	}
	return 0;
}

void HTTPParser::Init(Local<Object> exports)
{
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

void HTTPParser::New(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	HandleScope handle_scope(isolate);
	if (args.IsConstructCall())
	{
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

void HTTPParser::Setup(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HTTPParser *obj = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
	v8::HandleScope handleScope(isolate);
	uint32_t mode = args[0]->Uint32Value(context).ToChecked();
	http_parser_init(obj->context->parser, (http_parser_type)mode);

	Buffer *b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
	obj->context->base = b->_data;
	obj->context->parser_mode = mode;

	b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
	obj->context->buf = b->_data;
	obj->context->workBufferLength = b->_length;
}

void HTTPParser::Reset(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	HTTPParser *obj = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
	v8::HandleScope handleScope(isolate);
	uint32_t mode = args[0]->Uint32Value(context).ToChecked();
	http_parser_init(obj->context->parser, (http_parser_type)mode);
}

void HTTPParser::Execute(const FunctionCallbackInfo<Value> &args)
{
	Isolate *isolate = args.GetIsolate();
	Local<Context> context = isolate->GetCurrentContext();
	v8::HandleScope handleScope(isolate);
	HTTPParser *obj = ObjectWrap::Unwrap<HTTPParser>(args.Holder());
	uint32_t nread = args[0]->Uint32Value(context).ToChecked();
	ssize_t np = http_parser_execute(obj->context->parser, &settings, obj->context->base, nread);
	if (np != nread)
	{
		if (obj->context->parser->http_errno == HPE_PAUSED)
		{
			uint8_t *lastByte = (uint8_t *)(obj->context->base + np);
			obj->context->lastByte = *lastByte;
		}
		else
		{
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
