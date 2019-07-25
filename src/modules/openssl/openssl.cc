#include "openssl.h"

namespace dv8 {

namespace openssl {
using dv8::builtins::Environment;
using dv8::builtins::Buffer;
using dv8::socket::Socket;
using dv8::socket::socket_plugin;

	void InitAll(v8::Local<v8::Object> exports) {
			static std::atomic<uint64_t> inits{0};
			inits++;
			int loads = inits.load();
			//fprintf(stderr, "inited: %i\n", loads);
			Hash::Init(exports);
			Hmac::Init(exports);
			SecureContext::Init(exports);
			SecureSocket::Init(exports);
			if (loads == 1) {
				SSL_library_init();
				BIO* bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
				SSL_load_error_strings();
				ERR_load_BIO_strings();
				OpenSSL_add_all_algorithms();
				ERR_load_crypto_strings();
			}
	}

	void Hmac::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hmac"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Hmac::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "create", Hmac::Create);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "update", Hmac::Update);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "digest", Hmac::Digest);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "free", Hmac::Free);
	
		DV8_SET_EXPORT(isolate, tpl, "Hmac", exports);
	}

	void Hmac::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		Isolate *isolate = data.GetIsolate();
		v8::HandleScope handleScope(isolate);
		ObjectWrap *wrap = data.GetParameter();
		Hmac* obj = static_cast<Hmac *>(wrap);
		HMAC_CTX_free(obj->context);
		free(obj->in);
		free(obj->out);
		free(obj->context);
	}

	void Hmac::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Hmac* obj = new Hmac();
			obj->Wrap(args.This());
			obj->context = HMAC_CTX_new();
			obj->in = (uv_buf_t*)calloc(1, sizeof(uv_buf_t));
			obj->out = (uv_buf_t*)calloc(1, sizeof(uv_buf_t));
			args.GetReturnValue().Set(args.This());
		}
	}

	void Hmac::Setup(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		String::Utf8Value hash_type(isolate, args[0]);
		const EVP_MD* md = EVP_get_digestbyname(*hash_type);
		HMAC_CTX_reset(obj->context);
		String::Utf8Value key(isolate, args[1]);
		HMAC_Init_ex(obj->context, *key, strlen(*key), md, nullptr);
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
		obj->in->base = (char*)b->_data;
		obj->in->len = b->_length;
		b = ObjectWrap::Unwrap<Buffer>(args[3].As<v8::Object>());
		obj->out->base = (char*)b->_data;
		obj->out->len = b->_length;
		//EVP_cleanup();
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hmac::Create(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		HMAC_Init_ex(obj->context, nullptr, 0, nullptr, nullptr);
		int argc = args.Length();
		if (argc > 0) {
			Local<Context> context = isolate->GetCurrentContext();
			uint32_t len = args[0]->Uint32Value(context).ToChecked();
			char* data = obj->in->base;
			int r = HMAC_Update(obj->context, reinterpret_cast<const unsigned char*>(data), len);
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hmac::Update(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		char* data = obj->in->base;
		int r = HMAC_Update(obj->context, reinterpret_cast<const unsigned char*>(data), len);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void Hmac::Free(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		v8::HandleScope handleScope(isolate);
		HMAC_CTX_free(obj->context);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hmac::Digest(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Hmac *obj = ObjectWrap::Unwrap<Hmac>(args.Holder());
		v8::HandleScope handleScope(isolate);
		unsigned int md_len = 0;
		unsigned char* data = (unsigned char*)obj->out->base;
    HMAC_Final(obj->context, data, &md_len);
		args.GetReturnValue().Set(Integer::New(isolate, md_len));
	}

	void Hash::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "Hash"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", Hash::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "create", Hash::Create);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "update", Hash::Update);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "digest", Hash::Digest);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "free", Hash::Free);
	
		DV8_SET_EXPORT(isolate, tpl, "Hash", exports);
	}

	void Hash::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		Isolate *isolate = data.GetIsolate();
		v8::HandleScope handleScope(isolate);
		ObjectWrap *wrap = data.GetParameter();
		Hash* obj = static_cast<Hash *>(wrap);
		EVP_MD_CTX_free(obj->context);
		free(obj->in);
		free(obj->out);
		free(obj->context);
		//EVP_cleanup();
	}

	void Hash::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			Hash* obj = new Hash();
			obj->Wrap(args.This());
			obj->context = EVP_MD_CTX_new();
			obj->in = (uv_buf_t*)calloc(1, sizeof(uv_buf_t));
			obj->out = (uv_buf_t*)calloc(1, sizeof(uv_buf_t));
			args.GetReturnValue().Set(args.This());
		}
	}

	void Hash::Setup(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		String::Utf8Value hash_type(isolate, args[0]);
		obj->md = EVP_get_digestbyname(*hash_type);
		EVP_MD_CTX_init(obj->context);
		EVP_DigestInit_ex(obj->context, obj->md, nullptr);
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->in->base = (char*)b->_data;
		obj->in->len = b->_length;
		b = ObjectWrap::Unwrap<Buffer>(args[2].As<v8::Object>());
		obj->out->base = (char*)b->_data;
		obj->out->len = b->_length;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hash::Create(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		EVP_DigestInit_ex(obj->context, obj->md, nullptr);
		int argc = args.Length();
		if (argc > 0) {
			Local<Context> context = isolate->GetCurrentContext();
			uint32_t len = args[0]->Uint32Value(context).ToChecked();
			char* data = obj->in->base;
			int r = EVP_DigestUpdate(obj->context, reinterpret_cast<const unsigned char*>(data), len);
			args.GetReturnValue().Set(Integer::New(isolate, r));
			return;
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hash::Free(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		v8::HandleScope handleScope(isolate);
		EVP_MD_CTX_free(obj->context);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void Hash::Update(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		v8::HandleScope handleScope(isolate);
		Local<Context> context = isolate->GetCurrentContext();
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		char* data = obj->in->base;
		int r = EVP_DigestUpdate(obj->context, reinterpret_cast<const unsigned char*>(data), len);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void Hash::Digest(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Hash *obj = ObjectWrap::Unwrap<Hash>(args.Holder());
		v8::HandleScope handleScope(isolate);
		unsigned int md_len = 0;
		unsigned char* data = (unsigned char*)obj->out->base;
    EVP_DigestFinal_ex(obj->context, data, &md_len);
		args.GetReturnValue().Set(Integer::New(isolate, md_len));
	}


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
	
	int VerifyCallback(int preverify_ok, X509_STORE_CTX* ctx) {
		return SSL_TLSEXT_ERR_OK;
	}

	int TLSExtStatusCallback(SSL* s, void* arg) {
		return SSL_TLSEXT_ERR_OK;
	}

	int SelectSNIContextCallback(SSL* ssl, int* ad, void* arg) {
		SecureSocket* sock = static_cast<SecureSocket*>(SSL_get_app_data(ssl));
		const char* servername = SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
		if (sock->callbacks.onHost == 0) {
			return SSL_TLSEXT_ERR_OK;
		}
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		MaybeLocal<Value> ret;
		Local<Function> onHost = Local<Function>::New(isolate, sock->_onHost);
		if (servername) {
			Local<Value> argv[1] = { String::NewFromUtf8(isolate, servername, v8::String::kNormalString) };
			ret = onHost->Call(isolate->GetCurrentContext()->Global(), 1, argv);
		} else {
			Local<Value> argv[0] = {};
			ret = onHost->Call(isolate->GetCurrentContext()->Global(), 0, argv);
		}
		if (ret.IsEmpty()) {
			return SSL_TLSEXT_ERR_OK;
		}
		Local<Value> ctx;
		if (!ret.ToLocal(&ctx)) {
			// TODO: this doesn't seem to stop the session being established
			return SSL_TLSEXT_ERR_NOACK;
		}
		if (ctx->IsNullOrUndefined()) {
			return SSL_TLSEXT_ERR_OK;
		}
		if (ctx->IsFalse()) {
			return SSL_TLSEXT_ERR_NOACK;
		}
		SecureContext *secureContext = ObjectWrap::Unwrap<SecureContext>(ctx.As<v8::Object>());
		sock->context = secureContext;
		SSL_set_SSL_CTX(ssl, secureContext->ssl_context);
		return SSL_TLSEXT_ERR_OK;
	}

	void SecureContext::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		SecureContext* obj = ObjectWrap::Unwrap<SecureContext>(args.Holder());
		int sockType = SERVER_SOCKET;
		int argc = args.Length();
		if (argc > 0) {
			sockType = args[0]->Int32Value(context).ToChecked();
		}
		if (sockType == SERVER_SOCKET) {
			const SSL_METHOD* meth = TLS_server_method();
			int r = 0;
			SSL_CTX* ctx = SSL_CTX_new(meth);
			if (!ctx) {
				args.GetReturnValue().Set(Integer::New(isolate, -1));
				return;
			}
			obj->ssl_context = ctx;
			SSL_CTX_set_app_data(ctx, obj);
			String::Utf8Value certPath(args.GetIsolate(), args[1]);
			String::Utf8Value keyPath(args.GetIsolate(), args[2]);
			String::Utf8Value chainPath(args.GetIsolate(), args[3]);
			const char *key = *keyPath;
			const char *crtf = *certPath;
			const char *chainf = *chainPath;
			long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_COMPRESSION | SSL_OP_SINGLE_DH_USE;
			long mode = SSL_MODE_RELEASE_BUFFERS | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE;
			SSL_CTX_set_options(ctx, options);
			SSL_CTX_set_mode(ctx, mode);
			SSL_CTX_set_read_ahead(ctx, 1);
			SSL_CTX_set_max_send_fragment(ctx, 1300);
			SSL_CTX_set_tlsext_servername_callback(ctx, dv8::openssl::SelectSNIContextCallback);
			SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);
			//SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
			//SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, dv8::openssl::VerifyCallback);
			//SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, dv8::openssl::VerifyCallback);
			//SSL_CTX_set_tlsext_status_cb(ctx, dv8::openssl::TLSExtStatusCallback);
			//SSL_CTX_set_tlsext_status_arg(ctx, nullptr);
			//SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
			r = SSL_CTX_use_certificate_file(ctx, crtf, SSL_FILETYPE_PEM);
			if (r != 1) {
				args.GetReturnValue().Set(Integer::New(isolate, 4));
				return;
			}
			r = SSL_CTX_use_certificate_chain_file(ctx, chainf);
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
			return;
		}
		const SSL_METHOD* meth = TLS_client_method();
		int r = 0;
		SSL_CTX* ctx = SSL_CTX_new(meth);
		if (!ctx) {
			args.GetReturnValue().Set(Integer::New(isolate, -1));
			return;
		}
		obj->ssl_context = ctx;
		SSL_CTX_set_app_data(ctx, obj);
		long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_CIPHER_SERVER_PREFERENCE | SSL_OP_NO_COMPRESSION | SSL_OP_SINGLE_DH_USE;
		long mode = SSL_MODE_RELEASE_BUFFERS | SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER | SSL_MODE_AUTO_RETRY | SSL_MODE_ENABLE_PARTIAL_WRITE;
		SSL_CTX_set_options(ctx, options);
		SSL_CTX_set_mode(ctx, mode);
		SSL_CTX_set_read_ahead(ctx, 1);
		SSL_CTX_set_max_send_fragment(ctx, 1300);
		SSL_CTX_set_tlsext_servername_callback(ctx, dv8::openssl::SelectSNIContextCallback);
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_BOTH);
		SSL_CTX_set_cipher_list(ctx, "ALL:!ADH:!LOW:!EXP:!MD5:@STRENGTH");
		//SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, dv8::openssl::VerifyCallback);
		//TODO: this will trust all certs
		SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, dv8::openssl::VerifyCallback);
		//SSL_CTX_set_tlsext_status_cb(ctx, dv8::openssl::TLSExtStatusCallback);
		//SSL_CTX_set_tlsext_status_arg(ctx, nullptr);
		//SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_OFF);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void SecureSocket::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "SecureSocket"));
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", SecureSocket::Setup);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", SecureSocket::Write);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "finish", SecureSocket::Finish);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "start", SecureSocket::Start);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onRead", SecureSocket::OnRead);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onError", SecureSocket::OnError);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onWrite", SecureSocket::OnWrite);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onHost", SecureSocket::OnHost);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onSecure", SecureSocket::OnSecure);
	
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
		if (pending > 0) {
			int n = BIO_read(secure->output_bio, buf, len);
			uv_buf_t uvb;
			uvb.base = buf;
			uvb.len = n;
			int r = uv_try_write(stream, &uvb, 1);
			//TODO
			if (r == UV_EAGAIN || r == UV_ENOSYS) {
				fprintf(stderr, "try_write_again\n");
			} else if (r < 0) {
				fprintf(stderr, "try_write_fail\n");
			} else if (r < n) {
				fprintf(stderr, "try_write_partial\n");
			}
		}
	}

	void cycleIn(SecureSocket* secure, uv_stream_t* stream, char* buf, size_t len) {
		int pending = BIO_ctrl_pending(secure->input_bio);
		if (pending > 0) {
			int n = SSL_read(secure->ssl, buf, len);
			if (n <= 0) {
				n = SSL_get_error(secure->ssl, n);
				//fprintf(stderr, "SSL_read: %i\n", n);
				return;
			}
			if (n > 0) {
				if (secure->callbacks.onRead == 1) {
					Isolate *isolate = Isolate::GetCurrent();
					v8::HandleScope handleScope(isolate);
					Local<Value> argv[1] = {Number::New(isolate, n)};
					Local<Function> onRead = Local<Function>::New(isolate, secure->_onRead);
					onRead->Call(isolate->GetCurrentContext()->Global(), 1, argv);
				}
				socket_plugin* plugin = (socket_plugin*)secure->plugin;
				if (plugin->next) {
					uint32_t r = plugin->next->onRead(n, plugin->next);
				}
			}
		}
	}

	void on_plugin_close(void* obj) {
		//fprintf(stderr, "openssl.on_close\n");
		socket_plugin* plugin = (socket_plugin*)obj;
		SecureSocket* secure = (SecureSocket*)plugin->data;
		Socket* sock = (Socket*)secure->socket;
		dv8::socket::_context* context = sock->context;
		char* in = context->in.base;
		char* out = context->out.base;
		size_t outlen = context->out.len;
		size_t inlen = context->in.len;
		if (SSL_is_init_finished(secure->ssl)) {
			//fprintf(stderr, "openssl.flush\n");
			// TODO: check return codes etc.
			cycleIn(secure, (uv_stream_t *)context->handle, in, inlen);
			cycleOut(secure, (uv_stream_t *)context->handle, out, outlen);
		}
		if (plugin->next) {
			plugin->next->onClose(plugin->next);
		}
		SSL_free(secure->ssl);
	}

	uint32_t on_plugin_read_data(uint32_t nread, void* obj) {
		socket_plugin* plugin = (socket_plugin*)obj;
		SecureSocket* secure = (SecureSocket*)plugin->data;
		Socket* sock = (Socket*)secure->socket;
		dv8::socket::_context* context = sock->context;
		char* in = context->in.base;
		char* out = context->out.base;
		size_t outlen = context->out.len;
		size_t inlen = context->in.len;
		int n = BIO_write(secure->input_bio, in, nread);
		int pending = 0;
		if (SSL_is_init_finished(secure->ssl)) {
			// TODO: check return codes etc.
			cycleOut(secure, (uv_stream_t *)context->handle, out, outlen);
			cycleIn(secure, (uv_stream_t *)context->handle, in, inlen);
			return 0;
		}
		return start_ssl(secure);
	}

	void SecureSocket::Write(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		SecureSocket *secure = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		Socket* sock = (Socket*)secure->socket;
		v8::HandleScope handleScope(isolate);
		int argc = args.Length();
		uint32_t off = 0;
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		dv8::socket::_context* ctx = sock->context;
		char* out = ctx->out.base;
		if (argc > 1) {
			off = args[1]->Int32Value(context).ToChecked();
			out += off;
		}
		int n = SSL_write(secure->ssl, out, len);
		while (n < 0) {
			int r = SSL_get_error(secure->ssl, n);
			if (r == SSL_ERROR_WANT_READ) {
				cycleIn(secure, (uv_stream_t *)ctx->handle, ctx->in.base, ctx->in.len);
				n = SSL_write(secure->ssl, out, len);
				cycleOut(secure, (uv_stream_t *)ctx->handle, ctx->out.base, ctx->out.len);
			}
			else if (n == SSL_ERROR_SSL) {
				// TODO: lastError() method that returns last error so client can check when they get a bad status
				fprintf(stderr, "SSL_ERROR_SSL\n");
				fprintf(stderr, "%s\n", ERR_error_string(ERR_get_error(), NULL));
				args.GetReturnValue().Set(Integer::New(isolate, -1));
				return;
			}
			else {
				//TODO: handle other return codes
				fprintf(stderr, "Unknown SSL Error: %i\n", n);
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
		if (args[0]->IsFunction()) {
			Local<Function> onRead = Local<Function>::Cast(args[0]);
			s->_onRead.Reset(isolate, onRead);
			s->callbacks.onRead = 1;
		}
	}

	void SecureSocket::OnWrite(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureSocket *s = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		if (args[0]->IsFunction()) {
			Local<Function> onWrite = Local<Function>::Cast(args[0]);
			s->_onWrite.Reset(isolate, onWrite);
			s->callbacks.onWrite = 1;
		}
	}

	void SecureSocket::OnHost(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureSocket *s = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		if (args[0]->IsFunction()) {
			Local<Function> onHost = Local<Function>::Cast(args[0]);
			s->_onHost.Reset(isolate, onHost);
			s->callbacks.onHost = 1;
		}
	}

	void SecureSocket::OnSecure(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureSocket *s = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		if (args[0]->IsFunction()) {
			Local<Function> onSecure = Local<Function>::Cast(args[0]);
			s->_onSecure.Reset(isolate, onSecure);
			s->callbacks.onSecure = 1;
		}
	}

	void SecureSocket::OnError(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		SecureSocket *s = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		v8::HandleScope handleScope(isolate);
		if (args[0]->IsFunction())
		{
			Local<Function> onError = Local<Function>::Cast(args[0]);
			s->_onError.Reset(isolate, onError);
			s->callbacks.onError = 1;
		}
	}

	int start_ssl(SecureSocket* secure) {
		Isolate *isolate = Isolate::GetCurrent();
		v8::HandleScope handleScope(isolate);
		Socket* sock = (Socket*)secure->socket;
		dv8::socket::_context* context = sock->context;
		char* in = context->in.base;
		char* out = context->out.base;
		size_t outlen = context->out.len;
		size_t inlen = context->in.len;
		int n = SSL_do_handshake(secure->ssl);
		if (n == 0) {
			if (secure->callbacks.onError == 1) {
				Local<Value> argv[2] = {Number::New(isolate, n), String::NewFromUtf8(isolate, ERR_error_string(ERR_get_error(), NULL), v8::String::kNormalString)};
				Local<Function> onError = Local<Function>::New(isolate, secure->_onError);
				onError->Call(isolate->GetCurrentContext()->Global(), 2, argv);
			}
			return n;
		}
		if (n == 1) {
			if (secure->callbacks.onSecure == 1) {
				Local<Value> argv[0] = {};
				Local<Function> onSecure = Local<Function>::New(isolate, secure->_onSecure);
				onSecure->Call(isolate->GetCurrentContext()->Global(), 0, argv);
			}
			cycleOut(secure, (uv_stream_t *)context->handle, out, outlen);
			return 0;
		}
		n = SSL_get_error(secure->ssl, n);
		if (n == SSL_ERROR_WANT_READ) {
			cycleOut(secure, (uv_stream_t *)context->handle, out, outlen);
			return 0;
		}
		if (n == SSL_ERROR_SSL) {
			if (secure->callbacks.onError == 1) {
				Local<Value> argv[2] = {Number::New(isolate, n), String::NewFromUtf8(isolate, ERR_error_string(ERR_get_error(), NULL), v8::String::kNormalString)};
				Local<Function> onError = Local<Function>::New(isolate, secure->_onError);
				onError->Call(isolate->GetCurrentContext()->Global(), 2, argv);
			}
			return n;
		}
		if (secure->callbacks.onError == 1) {
			Local<Value> argv[2] = {Number::New(isolate, n), String::NewFromUtf8(isolate, ERR_error_string(ERR_get_error(), NULL), v8::String::kNormalString)};
			Local<Function> onError = Local<Function>::New(isolate, secure->_onError);
			onError->Call(isolate->GetCurrentContext()->Global(), 2, argv);
		}
		return n;
	}

	void SecureSocket::Start(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		v8::HandleScope handleScope(isolate);
		SecureSocket *secure = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		int r = start_ssl(secure);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void SecureSocket::Setup(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(32));
		v8::HandleScope handleScope(isolate);
		SecureSocket* secure = ObjectWrap::Unwrap<SecureSocket>(args.Holder());
		SecureContext *secureContext = ObjectWrap::Unwrap<SecureContext>(args[0].As<v8::Object>());
		Socket* sock = ObjectWrap::Unwrap<Socket>(args[1].As<v8::Object>());
		SSL_CTX* ctx = secureContext->ssl_context;
		SSL *ssl  = SSL_new(ctx);
		if (!ssl) {
			args.GetReturnValue().Set(Integer::New(isolate, -1));
			return;
		}
		BIO *output_bio = BIO_new(BIO_s_mem());
		BIO *input_bio = BIO_new(BIO_s_mem());
		secure->input_bio = input_bio;
		secure->output_bio = output_bio;
		BIO_set_mem_eof_return(input_bio, -1);		
		BIO_set_mem_eof_return(output_bio, -1);
		SSL_set_bio(ssl, input_bio, output_bio);
		if (sock->isServer) {
			SSL_set_accept_state(ssl);
		} else {
			SSL_set_connect_state(ssl);
		}
		SSL_set_app_data(ssl, secure);
		secure->context = secureContext;
		secure->ssl = ssl;
		secure->socket = sock;
		dv8::socket::socket_plugin* plugin = (dv8::socket::socket_plugin*)calloc(1, sizeof(dv8::socket::socket_plugin));
		plugin->data = secure;
		plugin->onRead = &on_plugin_read_data;
		plugin->onClose = &on_plugin_close;
		if (!sock->first) {
			sock->last = sock->first = plugin;
		} else {
			sock->last->next = plugin;
			sock->last = plugin;
		}
		plugin->next = 0;
		secure->plugin = plugin;
		if (sock->callbacks.onRead == 1) {
			Local<Function> onRead = Local<Function>::New(isolate, sock->_onRead);
			secure->_onRead.Reset(isolate, onRead);
			secure->callbacks.onRead = 1;
			sock->callbacks.onRead = 0;
			sock->_onRead.Reset();
		}
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

}
}	

extern "C" {
	void* _register_openssl() {
		return (void*)dv8::openssl::InitAll;
	}
}
