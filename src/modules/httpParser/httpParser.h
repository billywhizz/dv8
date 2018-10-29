#ifndef DV8_HTTPParser_H
#define DV8_HTTPParser_H

#include <dv8.h>
#include <socket.h>
#include "http_parser.h"

namespace dv8 {

namespace httpParser {

enum header_element_type { NONE = 0, FIELD, VALUE };
static http_parser_settings settings;

int message_begin_cb(http_parser *p);
int url_cb(http_parser *p, const char *buf, size_t len);
int header_field_cb(http_parser *p, const char *buf, size_t len);
int header_value_cb(http_parser *p, const char *buf, size_t len);
int body_cb(http_parser *p, const char *buf, size_t len);
int headers_complete_cb(http_parser *p);
int message_complete_cb(http_parser *p);
static uint32_t on_read_data(uint32_t nread, void* data);

struct _request {
  uint8_t headerCount; // no. of http headers
  uint8_t maxHeaders; // maximum number of headers
  uint8_t setHeaders; // should we copy the headers to js structures or not
  uint16_t fieldlen; // length of current parser field
  uint32_t fieldlenp; // pointer in buffer to current chunk of parser field
  uint32_t urllength; // length of http url
  char *key; // holds current header key
  char *val; // holds current header value
  header_element_type lastel; // flag for http parser
};

struct _context {
  uint32_t workBufferLength; // size of read buffer
  uint32_t index; // position in buffer
  char* buf; // work buffer
  char* base; // read buffer
  http_parser *parser; // http parser instance
  _request* request; // request structure
  uint8_t lastByte;
  void* data;
  uint8_t parser_mode;
};

typedef struct
{
  uint8_t onHeaders;
  uint8_t onRequest;
  uint8_t onResponse;
  uint8_t onError;
  uint8_t onBody;
} callbacks_t;

class HTTPParser : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		char *buf;
		_context* context;
		callbacks_t callbacks;
		v8::Persistent<v8::Function> _onHeaders;
		v8::Persistent<v8::Function> _onBody;
		v8::Persistent<v8::Function> _onRequest;
		v8::Persistent<v8::Function> _onResponse;
		v8::Persistent<v8::Function> _onError;

	private:

		HTTPParser() {
			settings.on_message_begin = message_begin_cb;
			settings.on_header_field = header_field_cb;
			settings.on_header_value = header_value_cb;
			settings.on_url = url_cb;
			settings.on_body = body_cb;
			settings.on_headers_complete = headers_complete_cb;
			settings.on_message_complete = message_complete_cb;
		}

		~HTTPParser() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Reset(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Execute(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void onHeaders(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void onBody(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void onRequest(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void onResponse(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void onError(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
