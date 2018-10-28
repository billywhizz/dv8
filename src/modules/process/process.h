#ifndef DV8_PROCESS_H
#define DV8_PROCESS_H

#include <dv8.h>

namespace dv8
{

namespace process
{
class Process : public dv8::ObjectWrap
{
public:
  static void Init(v8::Local<v8::Object> exports);
  //static void NewInstance(const v8::FunctionCallbackInfo<v8::Value> &args);

private:
  Process()
  {
  }

  ~Process()
  {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void PID(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void MemoryUsage(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void HeapSpaceUsage(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void CPUUsage(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void HRTime(const v8::FunctionCallbackInfo<v8::Value> &args);

  //static v8::Persistent<v8::Function> constructor;
};

} // namespace process
} // namespace dv8
#endif