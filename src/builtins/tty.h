#ifndef DV8_TTY_H
#define DV8_TTY_H

#include <dv8.h>

namespace dv8 {

namespace builtins {

class TTY : public dv8::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args);
  uv_tty_t* handle;
  uv_buf_t in;
  v8::Persistent<v8::Function> _onRead;
  v8::Persistent<v8::Function> _onEnd;
  v8::Persistent<v8::Function> _onDrain;
  v8::Persistent<v8::Function> _onClose;
  unsigned int fd;

 private:

  TTY() {
  }

  ~TTY() {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void WriteString(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void Pause(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Resume(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void QueueSize(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void OnClose(uv_handle_t* handle);
  static void OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t* buf);
  static void OnWrite(uv_write_t* req, int status);

  static v8::Persistent<v8::Function> constructor;

};

}
}
#endif
