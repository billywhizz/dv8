#ifndef DV8_Thread_H
	#define DV8_Thread_H
	
	#include <dv8.h>
	
	namespace dv8 {
	
	namespace thread {
typedef struct {
	void* data;
	void* object;
	size_t length;
} thread_handle;

	class Thread : public dv8::ObjectWrap {
	 public:
	  static void Init(v8::Local<v8::Object> exports);
	  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);
	uv_work_t* handle;
	v8::Persistent<v8::Function> onComplete;
	
	 private:
	
	  Thread() {
	  }
	
	  ~Thread() {
	  }
	
	  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	  static void Start(const v8::FunctionCallbackInfo<v8::Value>& args);

	  static v8::Persistent<v8::Function> constructor;
	
	};
	
	}
	}
	#endif
