#include <modules.h>

namespace dv8 {

inline void initLibrary(Local<Object> exports, const char* module_name) {
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
if (strcmp("process", module_name) == 0) {
dv8::process::InitAll(exports);
return;
}
if (strcmp("tty", module_name) == 0) {
dv8::tty::InitAll(exports);
return;
}
if (strcmp("libz", module_name) == 0) {
dv8::libz::InitAll(exports);
return;
}
if (strcmp("os", module_name) == 0) {
dv8::os::InitAll(exports);
return;
}
if (strcmp("socket", module_name) == 0) {
dv8::socket::InitAll(exports);
return;
}
if (strcmp("thread", module_name) == 0) {
dv8::thread::InitAll(exports);
return;
}
if (strcmp("udp", module_name) == 0) {
dv8::udp::InitAll(exports);
return;
}
if (strcmp("mbedtls", module_name) == 0) {
dv8::mbedtls::InitAll(exports);
return;
}
if (strcmp("httpParser", module_name) == 0) {
dv8::httpParser::InitAll(exports);
return;
}
if (strcmp("openssl", module_name) == 0) {
dv8::openssl::InitAll(exports);
return;
}
}

void LoadModule(const FunctionCallbackInfo<Value> &args) {
  Isolate *isolate = args.GetIsolate();
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
