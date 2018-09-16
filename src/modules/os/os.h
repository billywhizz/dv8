#include <dv8.h>
#include <v8.h>

namespace dv8 {

namespace os {
class OS : public dv8::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);

 private:

  OS() {
  }

  ~OS() {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Sleep(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;

};

}
}