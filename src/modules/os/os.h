#ifndef DV8_OS_H
#define DV8_OS_H

#include <dv8.h>

namespace dv8 {

namespace os {
class OS : public dv8::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);
  v8::Persistent<v8::Function> _onSignal;

 private:

  OS() {
  }

  ~OS() {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void OnSignal(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void on_signal(uv_signal_t* handle, int signum);

  static v8::Persistent<v8::Function> constructor;

};

}
}
#endif