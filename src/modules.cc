#include <modules.h>

namespace dv8 {

inline void initLibrary(Local<Object> exports, const char* module_name) {
  if (strcmp("tty", module_name) == 0) {
dv8::tty::InitAll(exports);
return;
}
if (strcmp("loop", module_name) == 0) {
dv8::loop::InitAll(exports);
return;
}
if (strcmp("timer", module_name) == 0) {
dv8::timer::InitAll(exports);
return;
}
if (strcmp("fs", module_name) == 0) {
dv8::fs::InitAll(exports);
return;
}
if (strcmp("net", module_name) == 0) {
dv8::net::InitAll(exports);
return;
}
if (strcmp("thread", module_name) == 0) {
dv8::thread::InitAll(exports);
return;
}
}

void LoadModule(const FunctionCallbackInfo<Value> &args) {
  v8::Isolate *isolate = args.GetIsolate();
  HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  String::Utf8Value str(args.GetIsolate(), args[0]);
  const char *module_name = *str;
  Local<Object> exports;
  bool ok = args[1]->ToObject(context).ToLocal(&exports);
  if (!ok) {
    args.GetReturnValue().Set(Null(isolate));
    return;
  }
  args.GetReturnValue().Set(exports);
  initLibrary(exports, module_name);
}

}
