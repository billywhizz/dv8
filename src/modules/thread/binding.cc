#include "thread.h"

namespace dv8
{
namespace thread
{
using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports)
{
	Thread::Init(exports);
}
} // namespace thread
} // namespace dv8

extern "C"
{
	void *_register_thread()
	{
		return (void *)dv8::thread::InitAll;
	}
}
