#include "dns.h"

namespace dv8
{
namespace dns
{
using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports)
{
	DNS::Init(exports);
}
} // namespace dns
} // namespace dv8

extern "C"
{
	void *_register_dns()
	{
		return (void *)dv8::dns::InitAll;
	}
}
