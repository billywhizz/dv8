#ifndef DV8_OS_H
#define DV8_OS_H

#include <dv8.h>

namespace dv8
{

namespace os
{
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


void InitAll(Local<Object> exports);

class OS : public dv8::ObjectWrap
{
public:
  static void Init(v8::Local<v8::Object> exports);
  v8::Persistent<v8::Function> _onSignal;

protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

private:
  OS()
  {
  }

  ~OS()
  {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void OnSignal(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void on_signal(uv_signal_t *handle, int signum);
};

} // namespace os
} // namespace dv8
#endif