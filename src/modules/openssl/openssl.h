#ifndef DV8_OpenSSL_H
#define DV8_OpenSSL_H

#include <dv8.h>
#include <socket.h>
#include <openssl/err.h>
#include <openssl/dh.h>
#include <openssl/ssl.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

/* SSL debug */
#define SSL_WHERE_INFO(ssl, w, flag, msg) {                \
    if(w & flag) {                                         \
      printf("+ %s: ", name);                              \
      printf("%20.20s", msg);                              \
      printf(" - %30.30s ", SSL_state_string_long(ssl));   \
      printf(" - %5.10s ", SSL_state_string(ssl));         \
      printf("\n");                                        \
    }                                                      \
  } 

namespace dv8 {

namespace openssl {

static uint32_t on_read_data(uint32_t nread, void* data);
enum sslstatus { SSLSTATUS_OK, SSLSTATUS_WANT_IO, SSLSTATUS_FAIL};

static void krx_ssl_info_callback(const SSL* ssl, int where, int ret, const char* name) {
 
  if(ret == 0) {
    printf("-- krx_ssl_info_callback: error occured.\n");
    return;
  }
 
  SSL_WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_START, "HANDSHAKE START");
  SSL_WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}

static void krx_ssl_server_info_callback(const SSL* ssl, int where, int ret) {
  krx_ssl_info_callback(ssl, where, ret, "server");
}

static void krx_ssl_client_info_callback(const SSL* ssl, int where, int ret) {
  krx_ssl_info_callback(ssl, where, ret, "client");
}
 
static enum sslstatus get_sslstatus(SSL* ssl, int n)
{
  switch (SSL_get_error(ssl, n))
  {
    case SSL_ERROR_NONE:
      return SSLSTATUS_OK;
    case SSL_ERROR_WANT_WRITE:
    case SSL_ERROR_WANT_READ:
      return SSLSTATUS_WANT_IO;
    case SSL_ERROR_ZERO_RETURN:
    case SSL_ERROR_SYSCALL:
    default:
      return SSLSTATUS_FAIL;
  }
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

};

class SecureSocket : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);
		SecureContext* context;
		dv8::socket::Socket* socket;
		SSL* ssl;
		BIO *output_bio;
		BIO *input_bio;

	private:

		SecureSocket() {
		}

		~SecureSocket() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
