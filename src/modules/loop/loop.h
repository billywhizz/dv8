#ifndef DV8_EventLoop_H
#define DV8_EventLoop_H

#include <dv8.h>

namespace dv8 {

namespace loop {

typedef struct
{
  uint8_t onIdle;
  uint8_t onCheck;
  uint8_t onPrepare;
} callbacks_t;

static void on_idle(uv_idle_t* handle);
static void on_check(uv_check_t* handle);
static void on_prepare(uv_prepare_t* handle);

class EventLoop : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		callbacks_t callbacks;
		uv_idle_t *idle_handle;
		uv_check_t *check_handle;
		uv_prepare_t *prepare_handle;
		v8::Persistent<v8::Function> onIdle;
		v8::Persistent<v8::Function> onCheck;
		v8::Persistent<v8::Function> onPrepare;

	protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

	private:

		EventLoop() {
		}

		~EventLoop() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Stop(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Run(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void IsAlive(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnIdle(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnCheck(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnPrepare(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
