#ifndef DV8_ZLib_H
#define DV8_ZLib_H

#include <dv8.h>
#include "zconf.h"
#include "zlib.h"

namespace dv8 {

namespace libz {
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

#define Z_MIN_CHUNK 64
#define Z_MAX_CHUNK std::numeric_limits<double>::infinity()
#define Z_DEFAULT_CHUNK (16 * 1024)
#define Z_MIN_MEMLEVEL 1
#define Z_MAX_MEMLEVEL 9
#define Z_DEFAULT_MEMLEVEL 8
#define Z_MIN_LEVEL -1
#define Z_MAX_LEVEL 9
#define Z_DEFAULT_LEVEL Z_DEFAULT_COMPRESSION
#define Z_MIN_WINDOWBITS 8
#define Z_MAX_WINDOWBITS 15
#define Z_DEFAULT_WINDOWBITS 15

enum dv8_zlib_mode {
  NONE,
  DEFLATE,
  INFLATE,
  GZIP,
  GUNZIP,
  DEFLATERAW,
  INFLATERAW,
  UNZIP
};

void InitAll(Local<Object> exports);

class ZLib : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		unsigned int mode;
		Bytef* dictionary_;
		size_t dictionary_len_;

	private:
		z_stream strm_;

		ZLib() {
		}

		~ZLib() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void End(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
