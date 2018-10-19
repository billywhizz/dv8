#ifndef DV8_Filesystem_H
#define DV8_Filesystem_H

#include <dv8.h>

namespace dv8
{

namespace fs
{
class Filesystem : public dv8::ObjectWrap
{
  public:
	static void Init(v8::Local<v8::Object> exports);
	static void NewInstance(const v8::FunctionCallbackInfo<v8::Value> &args);

  private:
	Filesystem()
	{
	}

	~Filesystem()
	{
	}

	static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
	static void Unlink(const v8::FunctionCallbackInfo<v8::Value> &args);
	static v8::Persistent<v8::Function> constructor;
};

} // namespace fs
} // namespace dv8
#endif
