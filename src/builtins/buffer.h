#ifndef DV8_BUFFER_H
#define DV8_BUFFER_H

#include <dv8.h>

namespace dv8 {

namespace builtins {
class Buffer : public dv8::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);
  char* _data;
  size_t _length;
  v8::Persistent<v8::ArrayBuffer> ab;

 private:

  Buffer() {
  }

  ~Buffer() {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Alloc(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Pull(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Push(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Length(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;

};

}
}
#endif
