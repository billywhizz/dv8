#ifndef DV8_DNS_H
	#define DV8_DNS_H
	
	#include <dv8.h>
	
	namespace dv8 {
	
	namespace dns {
	class DNS : public dv8::ObjectWrap {
	 public:
	  static void Init(v8::Local<v8::Object> exports);
	  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);
	
	 private:
	
	  DNS() {
	  }
	
	  ~DNS() {
	  }
	
	  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
	  static void Hello(const v8::FunctionCallbackInfo<v8::Value>& args);
	  static v8::Persistent<v8::Function> constructor;
	
	};
	
	}
	}
	#endif
