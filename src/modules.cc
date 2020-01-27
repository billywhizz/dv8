#include <modules.h>

namespace dv8 {

inline void initLibrary(Local<Object> exports, const char* module_name) {
  if (strcmp("fs", module_name) == 0) {
dv8::fs::InitAll(exports);
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
if (strcmp("process", module_name) == 0) {
dv8::process::InitAll(exports);
return;
}
if (strcmp("tty", module_name) == 0) {
dv8::tty::InitAll(exports);
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
#if DV8STATIC
  initLibrary(exports, module_name);
#else
  const char *module_path = "/usr/local/lib/dv8/";
  char lib_name[1024];
  uv_lib_t lib;
  int success = 0;
  args.GetReturnValue().Set(exports);
  if (args.Length() > 2) {
    String::Utf8Value str(args.GetIsolate(), args[2]);
    module_path = *str;
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
    success = uv_dlopen(lib_name, &lib);
  } else {
    initLibrary(exports, module_name);
    snprintf(lib_name, 1024, "%s%s.so", module_path, module_name);
    success = uv_dlopen(NULL, &lib);
  }
  if (success != 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "uv_dlopen failed").ToLocalChecked()));
    return;
  }
  char register_name[128];
  snprintf(register_name, 128, "_register_%s", module_name);
  void *address;
  success = uv_dlsym(&lib, register_name, &address);
  if (success != 0) {
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(isolate, "uv_dlsym failed").ToLocalChecked()));
    return;
  }
  register_plugin _init = reinterpret_cast<register_plugin>(address);
  auto _register = reinterpret_cast<InitializerCallback>(_init());
  _register(exports);
#endif
}

}
