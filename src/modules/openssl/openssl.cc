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

	void SecureContext::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "SecureContext"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", SecureContext::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "finish", SecureContext::Finish);
	
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

	void SecureContext::Finish(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureContext *s = ObjectWrap::Unwrap<SecureContext>(args.Holder());
		v8::HandleScope handleScope(isolate);
		SSL_CTX_free(s->ssl_context);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void SecureContext::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		SecureContext* obj = ObjectWrap::Unwrap<SecureContext>(args.Holder());
		const SSL_METHOD* meth = TLSv1_2_server_method();
		int r = 0;
		SSL_CTX* ctx = SSL_CTX_new(meth);
		if (!ctx) {
			args.GetReturnValue().Set(Integer::New(isolate, 1));
			return;
		}
		obj->ssl_context = ctx;
		int sockType = SERVER_SOCKET;
		int argc = args.Length();
		if (argc > 2) {
			sockType = args[2]->Int32Value(context).ToChecked();
		}
		if (sockType == SERVER_SOCKET) {
			long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;
			SSL_CTX_set_options(ctx, options);
			// Mitigate BEAST attacks
			SSL_CTX_set_options(ctx, SSL_OP_CIPHER_SERVER_PREFERENCE);
			SSL_CTX_set_options(ctx, SSL_OP_NO_COMPRESSION);
			SSL_CTX_set_options(ctx, SSL_OP_SINGLE_DH_USE);
			// Save RAM by releasing read and write buffers when they're empty. (SSL3 and
			// TLS only.)  "Released" buffers are put onto a free-list in the context
			// or just freed (depending on the context's setting for freelist_max_len).
			//SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
			SSL_CTX_set_mode(ctx, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);
			SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);
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
			SSL_CTX_set_info_callback(ctx, ssl_server_info_callback);
			SSL_CTX_set_msg_callback(ctx, ssl_msg_callback);
			String::Utf8Value certPath(args.GetIsolate(), args[0]);
			const char *crtf = *certPath;
			String::Utf8Value keyPath(args.GetIsolate(), args[1]);
			const char *key = *keyPath;
			r = SSL_CTX_use_certificate_file(ctx, crtf, SSL_FILETYPE_PEM);
			if (r != 1) {
				args.GetReturnValue().Set(Integer::New(isolate, 4));
				return;
			}
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
			args.GetReturnValue().Set(Integer::New(isolate, 0));
		} else {
			fprintf(stderr, "Client Secure Socket Not Implemented\n");
			args.GetReturnValue().Set(Integer::New(isolate, -1));
		}
	}

	void SecureSocket::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "SecureSocket"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", SecureSocket::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", SecureSocket::Write);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "finish", SecureSocket::Finish);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRead", SecureSocket::OnRead);
	
		DV8_SET_EXPORT(isolate, tpl, "SecureSocket", exports);
	}

	void SecureSocket::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			SecureSocket* obj = new SecureSocket();
			obj->Wrap(args.This());
			obj->callbacks.onRead = 0;
			obj->callbacks.onWrite = 0;
			obj->callbacks.onError = 0;
			args.GetReturnValue().Set(args.This());
		}
	}

	void cycleOut(SecureSocket* secure, uv_stream_t* stream, char* buf, size_t len) {
		int pending = BIO_ctrl_pending(secure->output_bio);
		fprintf(stderr, "pending output: %i\n", pending);
		if (pending > 0) {
			int n = BIO_read(secure->output_bio, buf, len);
			fprintf(stderr, "BIO_read_output: %i\n", n);
			uv_buf_t uvb;
			uvb.base = buf;
			uvb.len = n;
			int r = uv_try_write(stream, &uvb, 1);
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

	void cycleIn(SecureSocket* secure, uv_stream_t* stream, char* buf, size_t len) {
		int pending = BIO_ctrl_pending(secure->input_bio);
		fprintf(stderr, "pending input: %i\n", pending);	
		if (pending > 0) {
			int n = SSL_read(secure->ssl, buf, len);
			fprintf(stderr, "SSL_read: %i\n", n);
			if (secure->callbacks.onRead == 1) {
				Isolate *isolate = Isolate::GetCurrent();
				v8::HandleScope handleScope(isolate);
				Local<Value> argv[1] = {Number::New(isolate, n)};
				Local<Function> onRead = Local<Function>::New(isolate, secure->_onRead);
				onRead->Call(isolate->GetCurrentContext()->Global(), 1, argv);
			}

		}
	}

	uint32_t on_read_data(uint32_t nread, void* obj) {
		SecureSocket* secure = (SecureSocket*)obj;
		Socket* sock = secure->socket;
		dv8::socket::_context* context = sock->context;
		char* in = context->in.base;
		char* out = context->out.base;
		size_t outlen = context->out.len;
		size_t inlen = context->in.len;
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
				cycleOut(secure, (uv_stream_t *)context->handle, out, outlen);
				return 0;
			} else if (n < 0) {
				fprintf(stderr, "handshake error\n");
				n = SSL_get_error(secure->ssl, n);
				fprintf(stderr, "SSL_get_error: %i\n", n);
				if (n == SSL_ERROR_WANT_READ) {
					fprintf(stderr, "SSL_ERROR_WANT_READ\n");
					cycleOut(secure, (uv_stream_t *)context->handle, out, outlen);
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
			cycleIn(secure, (uv_stream_t *)context->handle, in, inlen);
		}
		return 0;
	}

	void SecureSocket::Write(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		SecureSocket *secure = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		int argc = args.Length();
		uint32_t off = 0;
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		dv8::socket::_context* ctx = secure->socket->context;
		char* out = ctx->out.base;
		if (argc > 1) {
			off = args[1]->Int32Value(context).ToChecked();
			out += off;
		}
		int n = SSL_write(secure->ssl, out, len);
		fprintf(stderr, "SSL_write: %i\n", n);
		while (n < 0) {
			int r = SSL_get_error(secure->ssl, n);
			fprintf(stderr, "SSL_get_error: %i\n", r);
			if (r == SSL_ERROR_WANT_READ) {
				fprintf(stderr, "SSL_ERROR_WANT_READ\n");
				cycleIn(secure, (uv_stream_t *)ctx->handle, ctx->in.base, ctx->in.len);
				n = SSL_write(secure->ssl, out, len);
			}
			else if (n == SSL_ERROR_SSL) {
				args.GetReturnValue().Set(Integer::New(isolate, -1));
				return;
			}
			else {
				args.GetReturnValue().Set(Integer::New(isolate, -1));
				return;
			}
		}
		cycleOut(secure, (uv_stream_t *)ctx->handle, ctx->out.base, ctx->out.len);
		args.GetReturnValue().Set(Integer::New(isolate, n));
	}

	void SecureSocket::Finish(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureSocket *s = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		SSL_free(s->ssl);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void SecureSocket::OnRead(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureSocket *s = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		if (args[0]->IsFunction())
		{
			Local<Function> onRead = Local<Function>::Cast(args[0]);
			s->_onRead.Reset(isolate, onRead);
			s->callbacks.onRead = 1;
		}
	}

	void SecureSocket::Setup(const FunctionCallbackInfo<Value> &args) {
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
		BIO_set_mem_eof_return(output_bio, -1);
		SSL_set_bio(ssl, input_bio, output_bio);
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
