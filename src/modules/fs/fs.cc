#include "fs.h"

namespace dv8 {

namespace fs {
using dv8::builtins::Environment;
using dv8::builtins::Buffer;

	void InitAll(Local<Object> exports) {
		FileSystem::Init(exports);
		File::Init(exports);
	}

	void File::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "File").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		// Sync Methods
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "setup", File::Setup); // done
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "open", File::Open); // done
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "close", File::Close); // done
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "read", File::Read); // done
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "write", File::Write); // done

		DV8_SET_EXPORT(isolate, tpl, "File", exports);
	}

	void File::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			File* obj = new File();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void File::Setup(const FunctionCallbackInfo<Value> &args)
	{
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		File *obj = ObjectWrap::Unwrap<File>(args.Holder());
		Buffer *b = ObjectWrap::Unwrap<Buffer>(args[0].As<v8::Object>());
		obj->in.base = (char*)b->_data;
		obj->in.len = b->_length;
		b = ObjectWrap::Unwrap<Buffer>(args[1].As<v8::Object>());
		obj->out.base = (char*)b->_data;
		obj->out.len = b->_length;
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void File::Open(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		File* obj = ObjectWrap::Unwrap<File>(args.Holder());
		String::Utf8Value filename(isolate, args[0]);
		int flags = O_RDONLY;
		int argc = args.Length();
		if (argc > 1) {
			flags = args[1]->Int32Value(context).ToChecked();
		}
		int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
		if (argc > 2) {
			mode = args[2]->Int32Value(context).ToChecked();
		}
		int fd = uv_fs_open(env->loop, &obj->req, *filename, flags, mode, NULL);
		//uv_ref((uv_handle_t*)obj->req);
		args.GetReturnValue().Set(Integer::New(isolate, fd));
	}

	void File::Read(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		File* obj = ObjectWrap::Unwrap<File>(args.Holder());
		int argc = args.Length();
		uint32_t off = 0;
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		if (argc > 1) {
			off = args[1]->Int32Value(context).ToChecked();
		}
		uv_fs_t req;
		int oldlen = obj->in.len;
		obj->in.len = len;
		int r = uv_fs_read(env->loop, &req, obj->req.result, &obj->in, 1, off, NULL);
		obj->in.len = oldlen;
		uv_fs_req_cleanup(&req);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void File::Write(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		File* obj = ObjectWrap::Unwrap<File>(args.Holder());
		int argc = args.Length();
		uint32_t off = 0;
		uint32_t len = args[0]->Uint32Value(context).ToChecked();
		if (argc > 1) {
			off = args[1]->Int32Value(context).ToChecked();
		}
		uv_fs_t req;
		int oldlen = obj->out.len;
		obj->out.len = len;
		int r = uv_fs_write(env->loop, &req, obj->req.result, &obj->out, 1, off, NULL);
		obj->out.len = oldlen;
		uv_fs_req_cleanup(&req);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

	void File::Close(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		File* obj = ObjectWrap::Unwrap<File>(args.Holder());
		String::Utf8Value filename(isolate, args[0]);
		uv_fs_t req;
		int r = uv_fs_close(env->loop, &req, obj->req.result, NULL);
		uv_fs_req_cleanup(&req);
		uv_fs_req_cleanup(&obj->req);
		args.GetReturnValue().Set(Integer::New(isolate, r));
	}

// TODO: setup with array of in and out buffers. can send multiple buffers in read/write calls

	void FileSystem::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "FileSystem").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		// Sync Methods
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "unlink", FileSystem::Unlink);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "mkdir", FileSystem::Mkdir);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "fstat", FileSystem::FStat);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "rename", FileSystem::Rename);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "fsync", FileSystem::FSync);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "ftruncate", FileSystem::FTruncate);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "copy", FileSystem::Copy);
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "sendfile", FileSystem::SendFile);
	
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_RDONLY), "O_RDONLY", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_WRONLY), "O_WRONLY", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_RDWR), "O_RDWR", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_APPEND), "O_APPEND", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_CREAT), "O_CREAT", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_TRUNC), "O_TRUNC", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, O_EXCL), "O_EXCL", exports);

		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IRUSR), "S_IRUSR", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IWUSR), "S_IWUSR", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IXUSR), "S_IXUSR", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IRGRP), "S_IRGRP", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IWGRP), "S_IWGRP", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IXGRP), "S_IXGRP", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IROTH), "S_IROTH", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IWOTH), "S_IWOTH", exports);
		DV8_SET_EXPORT_CONSTANT(isolate, Integer::New(isolate, S_IXOTH), "S_IXOTH", exports);

/*

https://linux.die.net/man/2/open

flags

access modes
O_RDONLY
O_WRONLY
O_RDWR

file creation flags
O_CLOEXEC
O_CREAT
O_DIRECTORY
O_EXCL
O_NOCTTY
O_NOFOLLOW
O_TRUNC
O_TTY_INIT

file status flags

O_APPEND
O_ASYNC
O_CLOEXEC
O_CREAT

mode - only when using O_CREAT
effective permissions are mode & ~umask

S_IRWXU
00700 user (file owner) has read, write and execute permission
S_IRUSR
00400 user has read permission
S_IWUSR
00200 user has write permission
S_IXUSR
00100 user has execute permission
S_IRWXG
00070 group has read, write and execute permission
S_IRGRP
00040 group has read permission
S_IWGRP
00020 group has write permission
S_IXGRP
00010 group has execute permission
S_IRWXO
00007 others have read, write and execute permission
S_IROTH
00004 others have read permission
S_IWOTH
00002 others have write permission
S_IXOTH
00001 others have execute permission

O_DIRECT
O_LARGEFILE
O_NOATIME
O_NONBLOCK
O_NDELAY
O_SYNC

errors

EACCES
EDQUOT
EEXIST
EFAULT
EFBIG
EINTR
EISDIR
ELOOP
EMFILE
ENAMETOOLONG
ENFILE
ENODEV
ENOENT
ENOMEM
ENOSPC
ENOTDIR
ENXIO
EOVERFLOW
EPERM
EROFS
ETXTBSY
EWOULDBLOCK

*/

		DV8_SET_EXPORT(isolate, tpl, "FileSystem", exports);
	}

	void FileSystem::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			FileSystem* obj = new FileSystem();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void FileSystem::Unlink(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Mkdir(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FStat(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Rename(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FSync(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FTruncate(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Copy(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::SendFile(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		FileSystem* obj = ObjectWrap::Unwrap<FileSystem>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}


}
}

extern "C" {
	void* _register_fs() {
		return (void*)dv8::fs::InitAll;
	}
}
