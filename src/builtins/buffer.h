#ifndef DV8_BUFFER_H
#define DV8_BUFFER_H

#include <dv8.h>

namespace dv8
{

namespace builtins
{

v8::MaybeLocal<v8::Object> NewBuffer(v8::Isolate* isolate, char* data, size_t length);

enum encoding
{
  ASCII = 0,
  UTF8 = 1,
  HEX = 2,
  BASE64 = 3,
  BINARYSTRING = 4,
  UCS2 = 5
};

class Buffer : public dv8::ObjectWrap
{
public:
  static void Init(v8::Local<v8::Object> exports);
  char *_data;
  bool _free;
  size_t _length;
  Buffer(char* data, size_t length) {
    this->_data = data;
    this->_length = length;
  }

protected:
  void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

private:
  Buffer()
  {
  }

  ~Buffer()
  {
  }
  
  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Alloc(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void AllocShared(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Load(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void LoadShared(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Free(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Read(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Write(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Copy(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Size(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Decode(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Encode(const v8::FunctionCallbackInfo<v8::Value> &args);

};

} // namespace builtins
} // namespace dv8
#endif
