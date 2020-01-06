#ifndef DV8_Thread_H
#define DV8_Thread_H

#include <dv8.h>

namespace dv8
{

namespace thread
{

using v8::Array;
using v8::ArrayBufferCreationMode;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

typedef struct
{
	void *data;
	void *object;
	size_t length;
	size_t size;
	char* source;
	char* name;
	js_error error;
	uv_async_t *async;
	uv_thread_t tid;
} thread_handle;

void InitAll(Local<Object> exports);

void on_context_complete(thread_handle *th, int status);

class Thread : public dv8::ObjectWrap
{
  public:
	static void Init(v8::Local<v8::Object> exports);
	v8::Persistent<v8::Function> onComplete;
	thread_handle *handle;

protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

  private:
	Thread()
	{
	}

	~Thread()
	{
	}

	static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
	static void Start(const v8::FunctionCallbackInfo<v8::Value> &args);
	static void Stop(const v8::FunctionCallbackInfo<v8::Value> &args);
};

} // namespace thread
} // namespace dv8
#endif
