#include <builtins/buffer.h>

const int8_t unbase64_table[256] =
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -2, -1, -1, -2, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -2, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, 62, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 63,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
  };


static const int8_t unhex_table[256] =
  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
     0,  1,  2,  3,  4,  5,  6,  7,  8,  9, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 10, 11, 12, 13, 14, 15, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
  };

static inline unsigned unhex(uint8_t x) {
  return unhex_table[x];
}

static size_t hex_decode(char* buf,
                         size_t len,
                         const char* src,
                         const size_t srcLen) {
  size_t i;
  for (i = 0; i < len && i * 2 + 1 < srcLen; ++i) {
    unsigned a = unhex(src[i * 2 + 0]);
    unsigned b = unhex(src[i * 2 + 1]);
    if (!~a || !~b)
      return i;
    buf[i] = (a << 4) | b;
  }

  return i;
}

static size_t hex_encode(const char* src, size_t slen, char* dst, size_t dlen) {
  // We know how much we'll write, just make sure that there's space.
  dlen = slen * 2;
  for (uint32_t i = 0, k = 0; k < dlen; i += 1, k += 2) {
    static const char hex[] = "0123456789abcdef";
    uint8_t val = static_cast<uint8_t>(src[i]);
    dst[k + 0] = hex[val >> 4];
    dst[k + 1] = hex[val & 15];
  }

  return dlen;
}

inline static int8_t unbase64(uint8_t x) {
  return unbase64_table[x];
}

static inline constexpr size_t base64_encoded_size(size_t size) {
  return ((size + 2 - ((size + 2) % 3)) / 3 * 4);
}

// Doesn't check for padding at the end.  Can be 1-2 bytes over.
static inline size_t base64_decoded_size_fast(size_t size) {
  size_t remainder = size % 4;

  size = (size / 4) * 3;
  if (remainder) {
    if (size == 0 && remainder == 1) {
      // special case: 1-byte input cannot be decoded
      size = 0;
    } else {
      // non-padded input, add 1 or 2 extra bytes
      size += 1 + (remainder == 3);
    }
  }

  return size;
}


size_t base64_decoded_size(const char* src, size_t size) {
  if (size == 0)
    return 0;

  if (src[size - 1] == '=')
    size--;
  if (size > 0 && src[size - 1] == '=')
    size--;

  return base64_decoded_size_fast(size);
}

bool base64_decode_group_slow(char* dst, const size_t dstlen,
                              const char* src, const size_t srclen,
                              size_t* const i, size_t* const k) {
  uint8_t hi;
  uint8_t lo;
#define V(expr)                                                               \
  for (;;) {                                                                  \
    const uint8_t c = src[*i];                                                \
    lo = unbase64(c);                                                         \
    *i += 1;                                                                  \
    if (lo < 64)                                                              \
      break;  /* Legal character. */                                          \
    if (c == '=' || *i >= srclen)                                             \
      return false;  /* Stop decoding. */                                     \
  }                                                                           \
  expr;                                                                       \
  if (*i >= srclen)                                                           \
    return false;                                                             \
  if (*k >= dstlen)                                                           \
    return false;                                                             \
  hi = lo;
  V(/* Nothing. */);
  V(dst[(*k)++] = ((hi & 0x3F) << 2) | ((lo & 0x30) >> 4));
  V(dst[(*k)++] = ((hi & 0x0F) << 4) | ((lo & 0x3C) >> 2));
  V(dst[(*k)++] = ((hi & 0x03) << 6) | ((lo & 0x3F) >> 0));
#undef V
  return true;  // Continue decoding.
}

size_t base64_decode_fast(char* dst, const size_t dstlen,
                          const char* src, const size_t srclen,
                          const size_t decoded_size) {
  const size_t available = dstlen < decoded_size ? dstlen : decoded_size;
  const size_t max_k = available / 3 * 3;
  size_t max_i = srclen / 4 * 4;
  size_t i = 0;
  size_t k = 0;
  while (i < max_i && k < max_k) {
    const uint32_t v =
        unbase64(src[i + 0]) << 24 |
        unbase64(src[i + 1]) << 16 |
        unbase64(src[i + 2]) << 8 |
        unbase64(src[i + 3]);
    // If MSB is set, input contains whitespace or is not valid base64.
    if (v & 0x80808080) {
      if (!base64_decode_group_slow(dst, dstlen, src, srclen, &i, &k))
        return k;
      max_i = i + (srclen - i) / 4 * 4;  // Align max_i again.
    } else {
      dst[k + 0] = ((v >> 22) & 0xFC) | ((v >> 20) & 0x03);
      dst[k + 1] = ((v >> 12) & 0xF0) | ((v >> 10) & 0x0F);
      dst[k + 2] = ((v >>  2) & 0xC0) | ((v >>  0) & 0x3F);
      i += 4;
      k += 3;
    }
  }
  if (i < srclen && k < dstlen) {
    base64_decode_group_slow(dst, dstlen, src, srclen, &i, &k);
  }
  return k;
}

size_t base64_decode(char* dst, const size_t dstlen,
                     const char* src, const size_t srclen) {
  const size_t decoded_size = base64_decoded_size(src, srclen);
  return base64_decode_fast(dst, dstlen, src, srclen, decoded_size);
}

static size_t base64_encode(const char* src,
                            size_t slen,
                            char* dst,
                            size_t dlen) {
  // We know how much we'll write, just make sure that there's space.
  dlen = base64_encoded_size(slen);

  unsigned a;
  unsigned b;
  unsigned c;
  unsigned i;
  unsigned k;
  unsigned n;

  static const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                              "abcdefghijklmnopqrstuvwxyz"
                              "0123456789+/";

  i = 0;
  k = 0;
  n = slen / 3 * 3;

  while (i < n) {
    a = src[i + 0] & 0xff;
    b = src[i + 1] & 0xff;
    c = src[i + 2] & 0xff;

    dst[k + 0] = table[a >> 2];
    dst[k + 1] = table[((a & 3) << 4) | (b >> 4)];
    dst[k + 2] = table[((b & 0x0f) << 2) | (c >> 6)];
    dst[k + 3] = table[c & 0x3f];

    i += 3;
    k += 4;
  }

  if (n != slen) {
    switch (slen - n) {
      case 1:
        a = src[i + 0] & 0xff;
        dst[k + 0] = table[a >> 2];
        dst[k + 1] = table[(a & 3) << 4];
        dst[k + 2] = '=';
        dst[k + 3] = '=';
        break;

      case 2:
        a = src[i + 0] & 0xff;
        b = src[i + 1] & 0xff;
        dst[k + 0] = table[a >> 2];
        dst[k + 1] = table[((a & 3) << 4) | (b >> 4)];
        dst[k + 2] = table[(b & 0x0f) << 2];
        dst[k + 3] = '=';
        break;
    }
  }

  return dlen;
}

namespace dv8
{

namespace builtins
{
using v8::Array;
using v8::ArrayBuffer;
using v8::ArrayBufferCreationMode;
using v8::Context;
using v8::Float64Array;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Uint8Array;
using v8::Value;
using v8::EscapableHandleScope;
using v8::WeakCallbackInfo;
using v8::SharedArrayBuffer;

void Buffer::Init(Local<Object> exports)
{
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

  tpl->SetClassName(String::NewFromUtf8(isolate, "Buffer").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "alloc", Buffer::Alloc);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "allocShared", Buffer::AllocShared);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "load", Buffer::Load);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "loadShared", Buffer::LoadShared);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "free", Buffer::Free);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "read", Buffer::Read);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", Buffer::Write);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "copy", Buffer::Copy);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "size", Buffer::Size);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "decode", Buffer::Decode);
  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "encode", Buffer::Encode);

  DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, ASCII), "ASCII", exports);
  DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UTF8), "UTF8", exports);
  DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, HEX), "HEX", exports);
  DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, BASE64), "BASE64", exports);
  DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, BINARYSTRING), "BINARYSTRING", exports);
  DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, UCS2), "UCS2", exports);

  DV8_SET_EXPORT(isolate, tpl, "Buffer", exports);
}

void Buffer::New(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.IsConstructCall()) {
    Buffer *obj = new Buffer();
    obj->Wrap(args.This());
    obj->_free = true;
    args.GetReturnValue().Set(args.This());
  }
}

// TODO: figure out how to do shared buffers across threads
void Buffer::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  Buffer* b = static_cast<Buffer *>(wrap);
  if (!b->_free) {
    free(b->_data);
    b->_free = true;
  }
  #if TRACE
  fprintf(stderr, "Buffer::Destroy\n");
  #endif
}

void Buffer::Alloc(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int argc = args.Length();
  if (argc > 0) {
    uint32_t length = args[0]->Uint32Value(context).ToChecked();
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    b->_length = 0;
    if (length > 0)
    {
      b->_data = (char *)calloc(length, 1);
      if (b->_data == nullptr)
      {
        fprintf(stderr, "oh dear\n");
        return;
      }
      Local<ArrayBuffer> ab = ArrayBuffer::New(isolate, b->_data, length, ArrayBufferCreationMode::kExternalized);
      b->_length = length;
      b->_free = false;
      args.GetReturnValue().Set(scope.Escape(ab));
    }
  } else {
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    Local<ArrayBuffer> ab = ArrayBuffer::New(isolate, b->_data, b->_length, ArrayBufferCreationMode::kExternalized);
    args.GetReturnValue().Set(scope.Escape(ab));
    b->_free = false;
  }
}

void Buffer::AllocShared(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int argc = args.Length();
  if (argc > 0) {
    uint32_t length = args[0]->Uint32Value(context).ToChecked();
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    b->_length = 0;
    if (length > 0)
    {
      b->_data = (char *)calloc(length, 1);
      if (b->_data == nullptr)
      {
        fprintf(stderr, "oh dear\n");
        return;
      }
      Local<SharedArrayBuffer> ab = SharedArrayBuffer::New(isolate, b->_data, length, ArrayBufferCreationMode::kExternalized);
      b->_length = length;
      b->_free = false;
      isolate->AdjustAmountOfExternalAllocatedMemory(length);
      args.GetReturnValue().Set(scope.Escape(ab));
    }
  } else {
    Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
    Local<SharedArrayBuffer> ab = SharedArrayBuffer::New(isolate, b->_data, b->_length, ArrayBufferCreationMode::kExternalized);
    args.GetReturnValue().Set(scope.Escape(ab));
    b->_free = false;
  }
}

void Buffer::Load(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<ArrayBuffer> ab = args[0].As<ArrayBuffer>();
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  v8::ArrayBuffer::Contents foo = ab->Externalize();
  if (!b->_free) {
    isolate->AdjustAmountOfExternalAllocatedMemory((int64_t)b->_length * -1);
    free(b->_data);
  }
  b->_data = (char*)foo.Data();
  b->_length = foo.ByteLength();
  b->_free = false;
  args.GetReturnValue().Set(scope.Escape(ab));
}

void Buffer::LoadShared(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<SharedArrayBuffer> ab = args[0].As<SharedArrayBuffer>();
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  v8::SharedArrayBuffer::Contents foo = ab->Externalize();
  if (!b->_free) {
    isolate->AdjustAmountOfExternalAllocatedMemory((int64_t)b->_length * -1);
    free(b->_data);
  }
  b->_data = (char*)foo.Data();
  b->_length = foo.ByteLength();
  b->_free = false;
  args.GetReturnValue().Set(scope.Escape(ab));
}

void Buffer::Copy(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  uint32_t length = args[1]->Uint32Value(context).ToChecked();
  Buffer *source = ObjectWrap::Unwrap<Buffer>(args.Holder());
  Buffer *dest = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
  memcpy(dest->_data, source->_data, length);
}

void Buffer::Free(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  if (!b->_free) {
    isolate->AdjustAmountOfExternalAllocatedMemory(b->_length * -1);
    free(b->_data);
    b->_free = true;
  }
}

void Buffer::Read(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  int32_t off = args[0]->Int32Value(context).ToChecked();
  int32_t len = args[1]->Int32Value(context).ToChecked();
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  const char *data = b->_data + off;
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, data, v8::NewStringType::kNormal, len).ToLocalChecked());
}

void Buffer::Encode(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  size_t off = args[0]->Int32Value(context).ToChecked();
  size_t len = args[1]->Int32Value(context).ToChecked();
  int enc = BASE64;
  Buffer *source = ObjectWrap::Unwrap<Buffer>(args.Holder());
  Buffer *dest = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
  if (args.Length() > 3) {
    enc = args[3]->Int32Value(context).ToChecked();
  }
  const char *data = source->_data + off;
  if (enc == BASE64) {
    size_t dlen = base64_encoded_size(len);
    dest->_data = (char*)calloc(1, dlen);
    len = base64_encode(data, len, dest->_data, dlen);
    dest->_length = len;
    args.GetReturnValue().Set(Integer::New(isolate, len));
  }
}

void Buffer::Decode(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Buffer *dest = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
  size_t off = args[1]->Int32Value(context).ToChecked();
  int enc = BASE64;
  if (args.Length() > 2) {
    enc = args[2]->Int32Value(context).ToChecked();
  }
  Buffer *source = ObjectWrap::Unwrap<Buffer>(args.Holder());
  char *buf = source->_data + off;
  size_t buflen = source->_length - off;
  if (enc == BASE64) {
    const size_t decoded_size = base64_decoded_size(buf, buflen);
    dest->_length = decoded_size;
    dest->_data = (char*)calloc(1, decoded_size);
    size_t nbytes = base64_decode_fast(dest->_data, dest->_length, buf, buflen, decoded_size);
    args.GetReturnValue().Set(Integer::New(isolate, nbytes));
  }
}

void Buffer::Write(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Local<String> str = args[0].As<String>();
  int length = str->Length();
  int32_t off = args[1]->Int32Value(context).ToChecked();
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  char *data = b->_data + off;
  int written;
  str->WriteUtf8(isolate, data, length, &written, v8::String::HINT_MANY_WRITES_EXPECTED | v8::String::NO_NULL_TERMINATION);
  args.GetReturnValue().Set(Integer::New(isolate, written));
}

void Buffer::Size(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Buffer *b = ObjectWrap::Unwrap<Buffer>(args.Holder());
  args.GetReturnValue().Set(Integer::New(isolate, b->_length));
}

} // namespace builtins
} // namespace dv8