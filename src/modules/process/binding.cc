#include "process.h"

namespace dv8
{
namespace process
{
using v8::Local;
using v8::Object;
using v8::Value;

void InitAll(Local<Object> exports)
{
  Process::Init(exports);
}
} // namespace process
} // namespace dv8

extern "C"
{
  void *_register_process()
  {
    return (void *)dv8::process::InitAll;
  }
}
