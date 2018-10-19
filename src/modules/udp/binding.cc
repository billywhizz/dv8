#include "udp.h"

namespace dv8
{
namespace udp
{
using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports)
{
	UDP::Init(exports);
}
} // namespace udp
} // namespace dv8

extern "C"
{
	void *_register_udp()
	{
		return (void *)dv8::udp::InitAll;
	}
}
