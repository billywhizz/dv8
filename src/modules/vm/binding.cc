#include "vm.h"

namespace dv8
{
namespace vm
{
using v8::Local;
using v8::Object;

void InitAll(Local<Object> exports)
{
	VM::Init(exports);
}
} // namespace vm
} // namespace dv8

extern "C"
{
	void *_register_vm()
	{
		return (void *)dv8::vm::InitAll;
	}
}
