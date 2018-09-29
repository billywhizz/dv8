#include <tty.h>

namespace dv8 {
namespace tty {
  using v8::Local;
  using v8::Object;
  using v8::Value;

  void InitAll(Local<Object> exports) {
    TTY::Init(exports);
  }
}
}

extern "C" {
  void* _register_tty() {
    return (void*)dv8::tty::InitAll;
  }
}
