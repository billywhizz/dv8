#ifndef DV8_TIMER_H
#define DV8_TIMER_H

#include <dv8.h>

namespace dv8
{

namespace timer
{

class Timer : public dv8::ObjectWrap
{
public:
  static void Init(v8::Local<v8::Object> exports);
  uv_timer_t *handle;
  v8::Persistent<v8::Function> onTimeout;

private:
  Timer()
  {
  }

  ~Timer()
  {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Start(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Stop(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Again(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void UnRef(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void OnTimeout(uv_timer_t *handle);
  static void OnClose(uv_handle_t *handle);
};

} // namespace timer
} // namespace dv8
#endif
