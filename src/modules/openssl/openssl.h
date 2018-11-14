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

using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Integer;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Array;
using dv8::builtins::Environment;
using dv8::socket::Socket;
using dv8::socket::socket_plugin;

typedef struct
{
  uint8_t onWrite;
  uint8_t onRead;
  uint8_t onError;
  uint8_t onHost;
  uint8_t onSecure;
} callbacks_t;

enum socket_type
{
  SERVER_SOCKET = 0,
  CLIENT_SOCKET
};

static uint32_t on_plugin_read_data(uint32_t nread, void* data);
static void on_plugin_close(void* obj);

extern int VerifyCallback(int preverify_ok, X509_STORE_CTX* ctx);
static int TLSExtStatusCallback(SSL* s, void* arg);
static int SelectSNIContextCallback(SSL* s, int* ad, void* arg);

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
    v8::Persistent<v8::Function> _onHost;
    v8::Persistent<v8::Function> _onSecure;
		SecureContext* context;
		dv8::socket::Socket* socket;
		SSL* ssl;
		BIO *output_bio;
		BIO *input_bio;
    callbacks_t callbacks;
		dv8::socket::socket_plugin* plugin;

	private:

		SecureSocket() {
		}

		~SecureSocket() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Write(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Finish(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Start(const v8::FunctionCallbackInfo<v8::Value>& args);

		static void OnRead(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnWrite(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnError(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnHost(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void OnSecure(const v8::FunctionCallbackInfo<v8::Value>& args);

};

static int start_ssl(SecureSocket* secure);

}
}
#endif
