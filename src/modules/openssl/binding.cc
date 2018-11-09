#include "openssl.h"

namespace dv8 {
namespace openssl {
	using v8::Local;
	using v8::Object;

	void InitAll(Local<Object> exports) {
		SecureContext::Init(exports);
		SecureSocket::Init(exports);
		SSL_library_init();
		BIO* bio_err = BIO_new_fp(stderr, BIO_NOCLOSE);
		SSL_load_error_strings();
		ERR_load_BIO_strings();
		OpenSSL_add_all_algorithms();
		ERR_load_crypto_strings();
	}
}
}

extern "C" {
	void* _register_openssl() {
		return (void*)dv8::openssl::InitAll;
	}
}
