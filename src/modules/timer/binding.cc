#include <timer.h>

namespace dv8 {
namespace timer {
  using v8::Local;
  using v8::Object;

  void InitAll(Local<Object> exports) {
    Timer::Init(exports);
  }
}
}

extern "C" {
  void* _register_timer() {
    return (void*)dv8::timer::InitAll;
  }
}
