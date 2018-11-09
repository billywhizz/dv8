#include "openssl.h"

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

#define WHERE_INFO(ssl, w, flag, msg) { \
    if(w & flag) { \
      printf("\t"); \
      printf(msg); \
      printf(" - %s ", SSL_state_string(ssl)); \
      printf(" - %s ", SSL_state_string_long(ssl)); \
      printf("\n"); \
    }\
 } 

void dummy_ssl_info_callback(const SSL* ssl, int where, int ret) {
  if(ret == 0) {
    printf("dummy_ssl_info_callback, error occured.\n");
    return;
  }
  WHERE_INFO(ssl, where, SSL_CB_LOOP, "LOOP");
  WHERE_INFO(ssl, where, SSL_CB_EXIT, "EXIT");
  WHERE_INFO(ssl, where, SSL_CB_READ, "READ");
  WHERE_INFO(ssl, where, SSL_CB_WRITE, "WRITE");
  WHERE_INFO(ssl, where, SSL_CB_ALERT, "ALERT");
  WHERE_INFO(ssl, where, SSL_CB_HANDSHAKE_DONE, "HANDSHAKE DONE");
}

void dummy_ssl_msg_callback(
                            int writep
                            ,int version
                            ,int contentType
                            ,const void* buf
                            ,size_t len
                            ,SSL* ssl
                            ,void *arg
                            ) 
{
  printf("\tMessage callback with length: %zu\n", len);
}

	void SecureContext::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "SecureContext"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", SecureContext::Setup);
	
		DV8_SET_EXPORT(isolate, tpl, "SecureContext", exports);
	}

	void SecureContext::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			SecureContext* obj = new SecureContext();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void SecureContext::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		SecureContext* obj = ObjectWrap::Unwrap<SecureContext>(args.Holder());
		const SSL_METHOD* meth = TLSv1_2_server_method();
		//const SSL_METHOD* meth = SSLv3_method();
		int r = 0;
		SSL_CTX* ctx = SSL_CTX_new(meth);
		if (!ctx) {
			args.GetReturnValue().Set(Integer::New(isolate, 1));
			return;
		}
		//long options = SSL_OP_NO_SSLv2;
		long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
    	SSL_CTX_set_options(ctx, options);

		// Mitigate BEAST attacks
		SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
		SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);
		SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);
		// Save RAM by releasing read and write buffers when they're empty. (SSL3 and
		// TLS only.)  "Released" buffers are put onto a free-list in the context
		// or just freed (depending on the context's setting for freelist_max_len).
		SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
		// Allow SSL_write(..., n) to return r with 0 < r < n (i.e. report success
		// when just a single record has been written):
		SSL_CTX_set_mode(ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);

		// Read more data than was asked
		SSL_CTX_set_read_ahead(ctx, 1);
		// Respect MTU
		SSL_CTX_set_max_send_fragment(ctx, 1300);
		// Do not verify clients
		SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);
		// Turn off sessions
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);

		//SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_ENABLE_PARTIAL_WRITE | SSL_MODE_RELEASE_BUFFERS);
		SSL_CTX_set_info_callback(ctx, dummy_ssl_info_callback);
		SSL_CTX_set_msg_callback(ctx, dummy_ssl_msg_callback);

		const char *crtf = "./cert.pem";
		r = SSL_CTX_use_certificate_file(ctx, crtf, SSL_FILETYPE_PEM);
		if (r != 1) {
			args.GetReturnValue().Set(Integer::New(isolate, 4));
			return;
		}
		const char *key = "./key.pem";
		r = SSL_CTX_use_PrivateKey_file(ctx, key, SSL_FILETYPE_PEM);
		if (r != 1) {
			args.GetReturnValue().Set(Integer::New(isolate, 5));
			return;
		}
		r = SSL_CTX_check_private_key(ctx);
		if (r != 1) {
			args.GetReturnValue().Set(Integer::New(isolate, 6));
			return;
		}
		obj->ssl_context = ctx;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void SecureSocket::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "SecureSocket"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", SecureSocket::Setup);
	
		DV8_SET_EXPORT(isolate, tpl, "SecureSocket", exports);
	}

	void SecureSocket::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			SecureSocket* obj = new SecureSocket();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	uint32_t on_read_data(uint32_t nread, void* obj) {
		SecureSocket* secure = (SecureSocket*)obj;
		Socket* sock = secure->socket;
		dv8::socket::_context* context = sock->context;
		char* in = context->in.base;
		char* out = context->out.base;
		size_t outlen = context->out.len;
		int n = BIO_write(secure->input_bio, in, nread);
		int pending = 0;
		n = SSL_is_init_finished(secure->ssl);
		fprintf(stderr, "SSL_is_init_finished: %i\n", n);
		if (!n) {
			n = SSL_do_handshake(secure->ssl);
			fprintf(stderr, "SSL_do_handshake: %i\n", n);
			if (n == 0) {
				fprintf(stderr, "handshake failed\n");
			} else if (n == 1) {
				fprintf(stderr, "handshake ok\n");
				pending = BIO_ctrl_pending(secure->output_bio);
				fprintf(stderr, "pending output: %i\n", pending);
				if (pending > 0) {
					n = BIO_read(secure->output_bio, out, outlen);
					fprintf(stderr, "BIO_read_output: %i\n", n);
					uv_buf_t buf;
					buf.base = out;
					buf.len = n;
					int r = uv_try_write((uv_stream_t *)context->handle, &buf, 1);
					if (r == UV_EAGAIN || r == UV_ENOSYS) {
						fprintf(stderr, "try_write_again\n");
					} else if (r < 0) {
						fprintf(stderr, "try_write_fail\n");
					} else if (r < n) {
						fprintf(stderr, "try_write_partial\n");

					} else {
						fprintf(stderr, "try_write_ok\n");
					}
				}
				return 0;
			} else if (n < 0) {
				fprintf(stderr, "handshake error\n");
				n = SSL_get_error(secure->ssl, n);
				fprintf(stderr, "SSL_get_error: %i\n", n);
				if (n == SSL_ERROR_WANT_READ) {
					fprintf(stderr, "SSL_ERROR_WANT_READ\n");
					pending = BIO_ctrl_pending(secure->output_bio);
					fprintf(stderr, "pending output: %i\n", pending);
					if (pending > 0) {
						n = BIO_read(secure->output_bio, out, outlen);
						fprintf(stderr, "BIO_read_output: %i\n", n);
						uv_buf_t buf;
						buf.base = out;
						buf.len = n;
						int r = uv_try_write((uv_stream_t *)context->handle, &buf, 1);
						if (r == UV_EAGAIN || r == UV_ENOSYS) {
							fprintf(stderr, "try_write_again\n");
						} else if (r < 0) {
							fprintf(stderr, "try_write_fail\n");
						} else if (r < n) {
							fprintf(stderr, "try_write_partial\n");

						} else {
							fprintf(stderr, "try_write_ok\n");
						}
					}
				}
				else if (n == SSL_ERROR_WANT_WRITE) {
					fprintf(stderr, "SSL_ERROR_WANT_WRITE\n");
				}
				else if (n == SSL_ERROR_SSL) {
					fprintf(stderr, "SSL_ERROR_SSL\n");
					fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
				}
			}
		} else {
			fprintf(stderr, "handshake finished\n");
		}
		return 0;
	}

/*
	uint32_t on_read_data(uint32_t nread, void* b) {
		char* data = (char*)b;
		fprintf(stderr, "on_read_data: %ui\n", nread);
		char buf[16384];
		SecureSocket* sock = (SecureSocket*)data;
		int status = 0;
		int n = 0;
		int z = 0;
		int pending = BIO_ctrl_pending(sock->output_bio);
		fprintf(stderr, "pending: %i\n", pending);
		while (nread > 0) {
			n = BIO_write(sock->input_bio, data, nread);
			fprintf(stderr, "BIO_write_ssl: %i\n", n);
			if (n <= 0) return -1;
			data += n;
			nread -= n;
			if (!SSL_is_init_finished(sock->ssl)) {
				n = SSL_do_handshake(sock->ssl);
				fprintf(stderr, "SSL_do_handshake: %i\n", n);
				status = SSL_get_error(sock->ssl, n);
				fprintf(stderr, "SSL_get_error: %i\n", status);
				//n = SSL_accept(sock->ssl);
				if (status == SSL_ERROR_WANT_READ) {
					fprintf(stderr, "SSL_ERROR_WANT_READ\n");
					int pending = BIO_ctrl_pending(sock->output_bio);
					fprintf(stderr, "pending: %i\n", pending);
					do {
						n = BIO_read(sock->output_bio, buf, sizeof(buf));
						fprintf(stderr, "BIO_read_app: %i\n", n);
						if (n <= 0) {
							z = BIO_should_retry(sock->output_bio);
							fprintf(stderr, "BIO_should_retry: %i\n", z);
							if (!z) return -1;
						}
					} while (n>0);
				}
				if (status == SSL_ERROR_WANT_WRITE) {
					fprintf(stderr, "SSL_ERROR_WANT_WRITE\n");
				}
				if (status == SSL_ERROR_SSL) {
					fprintf(stderr, "SSL_ERROR_SSL\n");
					fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
				}
				if (status == SSLSTATUS_FAIL) {
					fprintf(stderr, "SSLSTATUS_FAIL\n");
					return -1;
				}
			}
			do {
				n = SSL_read(sock->ssl, buf, sizeof(buf));
				fprintf(stderr, "read plain bytes: %i\n", n);
			} while (n > 0);
			status = SSL_get_error(sock->ssl, n);
			fprintf(stderr, "SSL_get_error: %i\n", status);
			//n = SSL_accept(sock->ssl);
			if (status == SSL_ERROR_WANT_READ) {
				fprintf(stderr, "SSL_ERROR_WANT_READ\n");
				do {
					n = BIO_read(sock->output_bio, buf, sizeof(buf));
					fprintf(stderr, "BIO_read_app: %i\n", n);
					if (n <= 0) {
						z = BIO_should_retry(sock->output_bio);
						fprintf(stderr, "BIO_should_retry: %i\n", z);
						if (!z) return -1;
					}
				} while (n>0);
			}
			if (status == SSL_ERROR_WANT_WRITE) {
				fprintf(stderr, "SSL_ERROR_WANT_WRITE\n");
			}
			if (status == SSL_ERROR_SSL) {
				fprintf(stderr, "SSL_ERROR_SSL\n");
				fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
			}
			if (status == SSLSTATUS_FAIL) {
				fprintf(stderr, "SSLSTATUS_FAIL\n");
				return -1;
			}
		}
		fprintf(stderr, "done\n");
		return 0;
	}
*/
	void SecureSocket::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		SecureSocket* obj = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		SecureContext *secureContext = ObjectWrap::Unwrap<SecureContext>(args[0].As<v8::Object>());
		Socket* sock = ObjectWrap::Unwrap<Socket>(args[1].As<v8::Object>());
		SSL_CTX* ctx = secureContext->ssl_context;
		SSL *ssl  = SSL_new(ctx);
		if (!ssl) {
			args.GetReturnValue().Set(Integer::New(isolate, 2));
			return;
		}
		BIO *output_bio = BIO_new(BIO_s_mem());
		BIO *input_bio = BIO_new(BIO_s_mem());
		obj->input_bio = input_bio;
		obj->output_bio = output_bio;
		BIO_set_mem_eof_return(input_bio, -1);		
		BIO_set_mem_eof_return(output_bio, -1);		//////////
		SSL_set_bio(ssl, input_bio, output_bio);
		//int fd = 0;
		//uv_fileno((uv_handle_t *)sock->_stream, &fd);
		//SSL_set_fd(ssl, fd);
		SSL_set_accept_state(ssl);
		sock->callbacks.onPluginRead = 1;
		sock->onPluginRead = &on_read_data;
		obj->socket = sock;
		obj->context = secureContext;
		obj->ssl = ssl;
		sock->pluginData = obj;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}


}
}	
