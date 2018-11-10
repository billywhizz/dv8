#ifndef DV8_OpenSSL_H
#define DV8_OpenSSL_H

#include <dv8.h>
#include <socket.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

namespace dv8 {

namespace openssl {

typedef struct
{
  uint8_t onWrite;
  uint8_t onRead;
  uint8_t onError;
} callbacks_t;

enum socket_type
{
  SERVER_SOCKET = 0,
  CLIENT_SOCKET
};

/* SSL debug */
#define WHERE_INFO(ssl, w, flag, msg) {                \
    if(w & flag) {                                         \
      printf("+ %s: ", name);                              \
      printf("%20.20s", msg);                              \
      printf(" - %30.30s ", SSL_state_string_long(ssl));   \
      printf(" - %5.10s ", SSL_state_string(ssl));         \
      printf("\n");                                        \
    }                                                      \
  } 

static uint32_t on_read_data(uint32_t nread, void* data);

static void ssl_info_callback(const SSL* ssl, int where, int ret, const char* name) {
  if(ret == 0) {
    printf("ssl_info_callback, error occured.\n");
    return;
  }
  WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  WHERE_INFO(ssl, where, SSL_CB_EXIT, "EXIT");
  WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
  WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
  WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
  WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}

static void ssl_server_info_callback(const SSL* ssl, int where, int ret) {
  ssl_info_callback(ssl, where, ret, "server");
}

static void ssl_client_info_callback(const SSL* ssl, int where, int ret) {
  ssl_info_callback(ssl, where, ret, "client");
}
 
static void ssl_msg_callback(int writep, int version, int contentType, const void* buf, size_t len, SSL* ssl, void *arg) {
  fprintf(stderr, "\tMessage callback with length: %zu\n", len);
}

class SecureContext : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		SSL_CTX* ssl_context;
	private:

		SecureContext() {
		}

		~SecureContext() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Finish(const v8::FunctionCallbackInfo<v8::Value>& args);

};

class SecureSocket : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
    v8::Persistent<v8::Function> _onWrite;
    v8::Persistent<v8::Function> _onRead;
    v8::Persistent<v8::Function> _onError;
		SecureContext* context;
		dv8::socket::Socket* socket;
		SSL* ssl;
		BIO *output_bio;
		BIO *input_bio;
    callbacks_t callbacks;

	private:

		SecureSocket() {
		}

		~SecureSocket() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Finish(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnRead(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
