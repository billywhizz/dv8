#include <socket.h>

namespace dv8
{
namespace socket
{
using v8::Local;
using v8::Object;
using v8::Value;

void InitAll(Local<Object> exports)
{
  Socket::Init(exports);
}
} // namespace socket
} // namespace dv8

extern "C"
{
  void *_register_socket()
  {
    return (void *)dv8::socket::InitAll;
  }
}
