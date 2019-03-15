#include "libz.h"

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
	using dv8::builtins::Environment;
	using dv8::builtins::Buffer;

  static void* AllocForZlib(void* data, uInt items, uInt size) {
    ZLib* ctx = static_cast<ZLib*>(data);
    size_t real_size = items * size;
    char* memory = (char*)calloc(1, real_size);
    *reinterpret_cast<size_t*>(memory) = real_size;
    return memory + sizeof(size_t);
  }

  static void FreeForZlib(void* data, void* pointer) {
    ZLib* ctx = static_cast<ZLib*>(data);
    char* real_pointer = static_cast<char*>(pointer) - sizeof(size_t);
    size_t real_size = *reinterpret_cast<size_t*>(real_pointer);
    free(real_pointer);
  }

	void ZLib::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "ZLib"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", ZLib::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", ZLib::Write);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "end", ZLib::End);

    DV8_SET_EXPORT_CONSTANT(isolate, String::NewFromUtf8(isolate, ZLIB_VERSION), "ZLIB_VERSION", exports);
		// mode
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, NONE), "ZLIB_MODE_NONE", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, DEFLATE), "ZLIB_MODE_DEFLATE", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, INFLATE), "ZLIB_MODE_INFLATE", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, GZIP), "ZLIB_MODE_GZIP", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, GUNZIP), "ZLIB_MODE_GUNZIP", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, DEFLATERAW), "ZLIB_MODE_DEFLATERAW", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, INFLATERAW), "ZLIB_MODE_INFLATERAW", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UNZIP), "ZLIB_MODE_UNZIP", exports);
	
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_NO_FLUSH), "Z_NO_FLUSH", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_PARTIAL_FLUSH), "Z_PARTIAL_FLUSH", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_SYNC_FLUSH), "Z_SYNC_FLUSH", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_FULL_FLUSH), "Z_FULL_FLUSH", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_FINISH), "Z_FINISH", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_BLOCK), "Z_BLOCK", exports);

    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_DEFAULT_COMPRESSION), "Z_DEFAULT_COMPRESSION", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_BEST_COMPRESSION), "Z_BEST_COMPRESSION", exports);

    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_BUF_ERROR), "Z_BUF_ERROR", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_OK), "Z_OK", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_STREAM_END), "Z_STREAM_END", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_NEED_DICT), "Z_NEED_DICT", exports);

    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_MAX_WINDOWBITS), "Z_MAX_WINDOWBITS", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_MIN_WINDOWBITS), "Z_MIN_WINDOWBITS", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_MIN_MEMLEVEL), "Z_MIN_MEMLEVEL", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_MAX_MEMLEVEL), "Z_MAX_WINDOWBITS", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_FILTERED), "Z_FILTERED", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_RLE), "Z_RLE", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_HUFFMAN_ONLY), "Z_HUFFMAN_ONLY", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_FIXED), "Z_FIXED", exports);
    DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, Z_DEFAULT_STRATEGY), "Z_DEFAULT_STRATEGY", exports);

		DV8_SET_EXPORT(isolate, tpl, "ZLib", exports);
	}

	void ZLib::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Local<Context> context = isolate->GetCurrentContext();
			ZLib* obj = new ZLib();
			obj->Wrap(args.This());
			obj->mode = NONE;
			int argc = args.Length();
			if (argc > 0) {
					obj->mode = args[0]->Uint32Value(context).ToChecked();
			}
			//obj->strm_.zalloc = AllocForZlib;
			//obj->strm_.zfree = FreeForZlib;
			obj->strm_.zalloc = Z_NULL;
			obj->strm_.zfree = Z_NULL;
			obj->strm_.opaque = static_cast<void*>(obj);
			args.GetReturnValue().Set(args.This());
		}
	}

	void ZLib::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		ZLib* obj = ObjectWrap::Unwrap<ZLib>(args.Holder());
    Buffer *in = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
    Buffer *out = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		unsigned int compression = Z_DEFAULT_COMPRESSION;
		int windowbits = Z_DEFAULT_WINDOWBITS;
		unsigned int err = 0;
		if (obj->mode == GZIP) {
			int argc = args.Length();
			if (argc > 2) compression = args[2]->Uint32Value(context).ToChecked();
			err = deflateInit2(&obj->strm_, compression, Z_DEFLATED, windowbits, Z_DEFAULT_MEMLEVEL, Z_DEFAULT_STRATEGY);
			//err = deflateInit(&obj->strm_, compression);
		} else if (obj->mode == GUNZIP) {
			err = inflateInit(&obj->strm_);
		}
    obj->strm_.next_out = reinterpret_cast<Bytef *>(out->_data);
    obj->strm_.next_in = reinterpret_cast<Bytef *>(in->_data);
    obj->strm_.avail_in = in->_length;
    obj->strm_.avail_out = out->_length;
/*
		gz_header header {0};
		header.name = const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>('hello'));
		header.comment = Z_NULL;
		header.extra = Z_NULL;
    deflateSetHeader(&obj->strm_, &header);
*/
		args.GetReturnValue().Set(Integer::New(isolate, err));
	}
	
	void ZLib::Write(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		ZLib* obj = ObjectWrap::Unwrap<ZLib>(args.Holder());
		unsigned int flush = Z_NO_FLUSH;
		unsigned int len = 0;
		int argc = args.Length();
		if (argc > 0) len = args[0]->Uint32Value(context).ToChecked();
		if (argc > 1) flush = args[1]->Uint32Value(context).ToChecked();
		if (argc > 2) {
			obj->strm_.avail_out = args[2]->Uint32Value(context).ToChecked();
		}
		obj->strm_.avail_in = len;
		int err = 0;
		switch (obj->mode) {
			case GZIP:
        err = deflate(&obj->strm_, flush);
				switch (err) {
					case Z_OK:
					case Z_BUF_ERROR:
						if (obj->strm_.avail_out != 0 && flush == Z_FINISH) {
							fprintf(stderr, "unexpected end of file\n");
							err = -2;
						}
						break;
					case Z_STREAM_END:
						break;
					case Z_NEED_DICT:
						if (obj->dictionary_ == nullptr) {
							fprintf(stderr, "Missing dictionary\n");
						} else {
							fprintf(stderr, "Bad dictionary\n");
						}
						err = -3;
						break;
					default:
						fprintf(stderr, "Zlib error\n");
						err = -4;
						break;
				}
				break;
			case GUNZIP:
        err = inflate(&obj->strm_, flush);
				switch (err) {
					case Z_OK:
					case Z_BUF_ERROR:
						if (obj->strm_.avail_out != 0 && flush == Z_FINISH) {
							fprintf(stderr, "unexpected end of file\n");
							err = -2;
						}
						break;
					case Z_STREAM_END:
						break;
					case Z_NEED_DICT:
						if (obj->dictionary_ == nullptr) {
							fprintf(stderr, "Missing dictionary\n");
						} else {
							fprintf(stderr, "Bad dictionary\n");
						}
						err = -3;
						break;
					default:
						fprintf(stderr, "Zlib error\n");
						err = -4;
						break;
				}
				break;
			default:
				err = -1;
				break;
		}
		if (err < 0) {
			args.GetReturnValue().Set(Integer::New(isolate, err));
			return;
		}
		args.GetReturnValue().Set(Integer::New(isolate, obj->strm_.avail_out));
	}

	void ZLib::End(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		ZLib* obj = ObjectWrap::Unwrap<ZLib>(args.Holder());
		deflateEnd(&obj->strm_);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
}
}	
