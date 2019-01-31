#ifndef DV8_SOCKET_H
#define DV8_SOCKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <map>
#include <queue>

#include <dv8.h>

namespace dv8
{

namespace socket
{
enum socket_type
{
  TCP = 0,
  UNIX
};

// write request struct
typedef struct
{
  uv_write_t req; // libu write handle
  uv_buf_t buf;   // buffer reference
} write_req_t;

// object for passing socket server structure to libuv
typedef struct
{
  void *object;
  void *callback;
} baton_t;

// struct of flags for JS callbacks set or not
typedef struct
{
  uint8_t onConnect;
  uint8_t onClose;
  uint8_t onWrite;
  uint8_t onRead;
  uint8_t onError;
  uint8_t onDrain;
  uint8_t onEnd;
} callbacks_t;
typedef struct _context _context;
typedef struct socket_plugin socket_plugin;

// typedefs for plugin  callbacks
typedef void (*on_plugin_close)(void* obj);
typedef uint32_t (*on_plugin_read_data)(uint32_t len, void* data);
typedef int (*on_data)(_context *, const char *at, size_t len);
typedef int (*cb)(_context *);

// socket plugin struct
struct socket_plugin
{
  void* data;
  on_plugin_read_data onRead;
  on_plugin_close onClose;
  socket_plugin* next;
};

// context operations
void context_init(uv_stream_t *handle, _context *ctx);
void context_free(uv_handle_t *handle);

// socket callback signatures
static void on_connection(uv_stream_t *server, int status);
static void after_read(uv_stream_t *handle, ssize_t nread, const uv_buf_t *buf);
static void after_write(uv_write_t *req, int status);
static void on_close(uv_handle_t *peer);
static void after_shutdown(uv_shutdown_t *req, int status);
static void alloc_chunk(uv_handle_t *handle, size_t size, uv_buf_t *buf);

// socket stats
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
} socket_stats;

// socket context
struct _context
{
  uint32_t readBufferLength;  // size of read buffer
  uint32_t writeBufferLength; // size of write buffer
  uint32_t index;             // position in buffer
  uv_buf_t in;                // buffer for reading from socket
  uv_buf_t out;               // buffer for writing to socket
  uv_stream_t *handle;        // stream handle
  void *data;                 // associated object
  uint8_t lastByte;
  bool closing;
  bool blocked;
  bool paused;
  socket_stats stats;
};

class Socket : public dv8::ObjectWrap
{
public:
  // initialisation
  static void Init(v8::Local<v8::Object> exports);

  // persistent pointers to JS callbacks
  v8::Persistent<v8::Function> _onConnect;
  v8::Persistent<v8::Function> _onClose;
  v8::Persistent<v8::Function> _onDrain;
  v8::Persistent<v8::Function> _onWrite;
  v8::Persistent<v8::Function> _onRead;
  v8::Persistent<v8::Function> _onError;
  v8::Persistent<v8::Function> _onEnd;

  socket_type socktype = TCP; // 0 = tcp socket, 1 = Unix Domain Socket/Named Pipe
  callbacks_t callbacks;
  uv_stream_t* _stream;
  _context* context;
  dv8::socket::socket_plugin* first = 0;
  dv8::socket::socket_plugin* last = 0;
  bool isServer = true;
  Socket()
  {
  }

protected:
  void Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data);

private:

  ~Socket()
  {
  }

  //Socket Contructor
  static void New(const v8::FunctionCallbackInfo<v8::Value> &args); // Socket constuctor

  //Socket Instance Methods
  static void Setup(const v8::FunctionCallbackInfo<v8::Value> &args);     // configure socket buffers/options
  static void Bind(const v8::FunctionCallbackInfo<v8::Value> &args);      // bind a socket to a port/path
  static void Connect(const v8::FunctionCallbackInfo<v8::Value> &args);   // connect to port/path/handle
  static void Listen(const v8::FunctionCallbackInfo<v8::Value> &args);    // listen to port/path/handle
  static void Close(const v8::FunctionCallbackInfo<v8::Value> &args);     // close a socket
  static void Write(const v8::FunctionCallbackInfo<v8::Value> &args);     // write from out buffer to the socket
  static void Splice(const v8::FunctionCallbackInfo<v8::Value> &args);     // write from out buffer to the socket
  static void Pause(const v8::FunctionCallbackInfo<v8::Value> &args);     // pause the socket
  static void Resume(const v8::FunctionCallbackInfo<v8::Value> &args);    // resume the socket
  static void Error(const v8::FunctionCallbackInfo<v8::Value> &args);     // get the uv error string for an error code
  static void QueueSize(const v8::FunctionCallbackInfo<v8::Value> &args); // size in bytes of the write queue
  static void Stats(const v8::FunctionCallbackInfo<v8::Value> &args);     // get the stats for the socket
  static void UnRef(const v8::FunctionCallbackInfo<v8::Value> &args);     // get the stats for the socket
  static void Open(const v8::FunctionCallbackInfo<v8::Value> &args);     // get the stats for the socket
  static void PortNumber(const v8::FunctionCallbackInfo<v8::Value> &args);     // get the port number

  // TCP only methods
  static void RemoteAddress(const v8::FunctionCallbackInfo<v8::Value> &args); // remote ip4 address as string
  static void SetNoDelay(const v8::FunctionCallbackInfo<v8::Value> &args);    // disable nagle on tcp socket
  static void SetKeepAlive(const v8::FunctionCallbackInfo<v8::Value> &args);  // turn tcp keepalive on or off

  // JS Callbacks
  //static void onConnect(v8::Local<v8::String> property, const v8::PropertyCallbackInfo<v8::Value>& info);
  static void onConnect(const v8::FunctionCallbackInfo<v8::Value> &args); // when we get a new socket connection
  static void onError(const v8::FunctionCallbackInfo<v8::Value> &args);   // when we have an error on the socket
  static void onRead(const v8::FunctionCallbackInfo<v8::Value> &args);    // when we receive bytes on the socket
  static void onWrite(const v8::FunctionCallbackInfo<v8::Value> &args);   // when write has been flushed to socket
  static void onClose(const v8::FunctionCallbackInfo<v8::Value> &args);   // when socket closes
  static void onEnd(const v8::FunctionCallbackInfo<v8::Value> &args);     // when a read socket gets EOF
  static void onDrain(const v8::FunctionCallbackInfo<v8::Value> &args);   // when socket write buffers have flushed
};

} // namespace socket
} // namespace dv8
#endif