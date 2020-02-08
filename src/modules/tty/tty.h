#ifndef DV8_TTY_H
#define DV8_TTY_H

#include <dv8.h>

namespace dv8
{

namespace tty
{
using v8::Array;
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


int on_tty_event(jsys_descriptor *client);

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

typedef struct
{
  uint8_t onClose;
  uint8_t onWrite;
  uint8_t onRead;
  uint8_t onError;
  uint8_t onDrain;
  uint8_t onEnd;
} callbacks_t;

void InitAll(Local<Object> exports);

class TTY : public dv8::ObjectWrap
{
public:
  static void Init(v8::Local<v8::Object> exports);
  jsys_descriptor* handle;
  unsigned int ttype;
  tty_stats stats;
  bool paused;
  callbacks_t callbacks;
  v8::Persistent<v8::Function> _onRead;
  v8::Persistent<v8::Function> _onEnd;
  v8::Persistent<v8::Function> _onDrain;
  v8::Persistent<v8::Function> _onClose;
  v8::Persistent<v8::Function> _onError;
  v8::Persistent<v8::Function> _onWrite;

protected:
		void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

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
  static void Stats(const v8::FunctionCallbackInfo<v8::Value> &args);

  static void onError(const v8::FunctionCallbackInfo<v8::Value> &args);   // when we have an error on the socket
  static void onRead(const v8::FunctionCallbackInfo<v8::Value> &args);    // when we receive bytes on the socket
  static void onWrite(const v8::FunctionCallbackInfo<v8::Value> &args);   // when write has been flushed to socket
  static void onClose(const v8::FunctionCallbackInfo<v8::Value> &args);   // when socket closes
  static void onEnd(const v8::FunctionCallbackInfo<v8::Value> &args);     // when a read socket gets EOF
  static void onDrain(const v8::FunctionCallbackInfo<v8::Value> &args);   // when socket write buffers have flushed
};

} // namespace tty
} // namespace dv8

#endif
