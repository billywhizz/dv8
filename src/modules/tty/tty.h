#ifndef DV8_TTY_H
#define DV8_TTY_H

#include <dv8.h>

namespace dv8
{

namespace tty
{

typedef struct
{
  uint32_t close;
  uint32_t error;
  struct
  {
    uint64_t written;
    uint32_t incomplete;
    uint32_t full;
    uint32_t drain;
    uint32_t maxQueue;
    uint32_t alloc;
    uint32_t free;
    uint32_t eagain;
  } out;
  struct
  {
    uint64_t read;
    uint32_t pause;
    uint32_t end;
    uint32_t data;
    uint32_t resume;
  } in;
} tty_stats;

class TTY : public dv8::ObjectWrap
{
public:
  static void Init(v8::Local<v8::Object> exports);
  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value> &args);
  uv_tty_t *handle;
  char *in;
  v8::Persistent<v8::Function> _onRead;
  v8::Persistent<v8::Function> _onEnd;
  v8::Persistent<v8::Function> _onDrain;
  v8::Persistent<v8::Function> _onClose;
  v8::Persistent<v8::Function> _onError;
  unsigned int fd;
  tty_stats stats;
  bool paused;
  bool closing;
  bool blocked;

private:
  TTY()
  {
  }

  ~TTY()
  {
  }

  static void New(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Write(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Close(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Setup(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Pause(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Resume(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void QueueSize(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Stats(const v8::FunctionCallbackInfo<v8::Value> &args);
  static void Error(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void OnClose(uv_handle_t *handle);
  static void OnRead(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
  static void OnWrite(uv_write_t *req, int status);

  static v8::Persistent<v8::Function> constructor;
};

} // namespace tty
} // namespace dv8
#endif
