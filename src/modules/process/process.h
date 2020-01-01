#ifndef DV8_PROCESS_H
#define DV8_PROCESS_H

#include <dv8.h>
#include <time.h>
#include <modules/socket/socket.h>

namespace dv8 {

namespace process {
using v8::Array;
using v8::BigUint64Array;
using v8::Context;
using v8::Float64Array;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HeapSpaceStatistics;
using v8::HeapStatistics;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

static void on_exit(uv_process_t *req, int64_t exit_status, int term_signal);

void InitAll(Local<Object> exports);

class Process : public dv8::ObjectWrap {
public:
  static void Init(v8::Local<v8::Object> exports);
  v8::Persistent<v8::Function> onExit;

private:
  Process() {}

  ~Process() {}

  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void PID(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void MemoryUsage(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void HeapSpaceUsage(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void CPUUsage(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void HRTime(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Sleep(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Cwd(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void USleep(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void NanoSleep(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void RunMicroTasks(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Spawn(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void RunScript(const v8::FunctionCallbackInfo<v8::Value> &args);
};

} // namespace process
} // namespace dv8
#endif