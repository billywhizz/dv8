#ifndef DV8_PicoHTTPParser_H
#define DV8_PicoHTTPParser_H

#include <dv8.h>
#include <socket.h>
#include <x86intrin.h>

namespace dv8 {

namespace picoHttpParser {

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
using dv8::builtins::Environment;
using dv8::builtins::Buffer;
using dv8::socket::Socket;
using dv8::socket::socket_plugin;

struct _context {
  char* buf; // work buffer
  char* base; // read buffer
  uint32_t workBufferLength; // size of read buffer
};

static uint32_t on_plugin_read_data(uint32_t nread, void* data);
static void on_plugin_close(void* obj);

typedef struct
{
  uint8_t onHeaders;
  uint8_t onRequest;
  uint8_t onResponse;
  uint8_t onError;
  uint8_t onBody;
} callbacks_t;


class PicoHTTPParser : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		dv8::socket::socket_plugin* plugin;
		_context* context;
		v8::Persistent<v8::Function> _onHeaders;
		callbacks_t callbacks;
    
	private:

		PicoHTTPParser() {
		}

		~PicoHTTPParser() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Reset(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void onHeaders(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
