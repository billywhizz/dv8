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

	void FileSystem::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "FileSystem").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		// Sync Methods
		DV8_SET_METHOD(isolate, tpl, "unlink", FileSystem::Unlink);
		DV8_SET_METHOD(isolate, tpl, "mkdir", FileSystem::Mkdir);
		DV8_SET_METHOD(isolate, tpl, "rmdir", FileSystem::Rmdir);
		DV8_SET_METHOD(isolate, tpl, "fstat", FileSystem::FStat);
		DV8_SET_METHOD(isolate, tpl, "rename", FileSystem::Rename);
		DV8_SET_METHOD(isolate, tpl, "fsync", FileSystem::FSync);
		DV8_SET_METHOD(isolate, tpl, "ftruncate", FileSystem::FTruncate);
		DV8_SET_METHOD(isolate, tpl, "copy", FileSystem::Copy);
		DV8_SET_METHOD(isolate, tpl, "sendfile", FileSystem::SendFile);
		DV8_SET_METHOD(isolate, tpl, "readdir", FileSystem::Readdir);

	
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_RDONLY), "O_RDONLY", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_WRONLY), "O_WRONLY", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_RDWR), "O_RDWR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_APPEND), "O_APPEND", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_CLOEXEC), "O_CLOEXEC", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_CREAT), "O_CREAT", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_TRUNC), "O_TRUNC", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, O_EXCL), "O_EXCL", tpl);

		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IRUSR), "S_IRUSR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IWUSR), "S_IWUSR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IXUSR), "S_IXUSR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IRGRP), "S_IRGRP", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IWGRP), "S_IWGRP", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IXGRP), "S_IXGRP", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IROTH), "S_IROTH", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IWOTH), "S_IWOTH", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IXOTH), "S_IXOTH", tpl);

		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IRWXO), "S_IRWXO", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IRWXG), "S_IRWXG", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IRWXU), "S_IRWXU", tpl);

		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFMT), "S_IFMT", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFSOCK), "S_IFSOCK", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFLNK), "S_IFLNK", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFREG), "S_IFREG", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFBLK), "S_IFBLK", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFDIR), "S_IFDIR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFCHR), "S_IFCHR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, S_IFIFO), "S_IFIFO", tpl);

		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_BLK), "DT_BLK", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_CHR), "DT_CHR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_DIR), "DT_DIR", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_FIFO), "DT_FIFO", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_LNK), "DT_LNK", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_REG), "DT_REG", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_SOCK), "DT_SOCK", tpl);
		DV8_SET_CONSTANT(isolate, Integer::New(isolate, DT_UNKNOWN), "DT_UNKNOWN", tpl);

		DV8_SET_EXPORT(isolate, tpl, "FileSystem", exports);
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

	void File::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		#if TRACE
		fprintf(stderr, "File::Destroy\n");
		#endif
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
		obj->fd = fd;
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

	void FileSystem::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			FileSystem* obj = new FileSystem();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void FileSystem::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
		Isolate *isolate = data.GetIsolate();
		v8::HandleScope handleScope(isolate);
		ObjectWrap *wrap = data.GetParameter();
		FileSystem* sock = static_cast<FileSystem *>(wrap);
		#if TRACE
		fprintf(stderr, "FileSystem::Destroy\n");
		#endif
	}

	void FileSystem::Readdir(const FunctionCallbackInfo<Value> &args) {
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		String::Utf8Value path(isolate, args[0]);
		v8::Local<v8::Array> answer = args[1].As<v8::Array>();
		DIR* directory = opendir(*path);
		args.GetReturnValue().Set(Integer::New(isolate, -1));
		if (!directory) return;
		dirent* entry = readdir(directory);
		int i = 0;
		while (entry) {
			Local<Object> o = Object::New(isolate);
			o->Set(context, String::NewFromUtf8(isolate, "name", v8::NewStringType::kNormal).ToLocalChecked(), String::NewFromUtf8(isolate, entry->d_name).ToLocalChecked());
			o->Set(context, String::NewFromUtf8(isolate, "type", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, entry->d_type));
			o->Set(context, String::NewFromUtf8(isolate, "ino", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, entry->d_ino));
			o->Set(context, String::NewFromUtf8(isolate, "off", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, entry->d_off));
			o->Set(context, String::NewFromUtf8(isolate, "reclen", v8::NewStringType::kNormal).ToLocalChecked(), Integer::New(isolate, entry->d_reclen));
      answer->Set(context, i++, o);
			entry = readdir(directory);
			if (i == 1023) break;
		}
		closedir(directory);
		args.GetReturnValue().Set(Integer::New(isolate, i));
	}

	void FileSystem::Unlink(const FunctionCallbackInfo<Value> &args)
	{		
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		uv_fs_t req;
		String::Utf8Value path(isolate, args[0]);
		int rc = uv_fs_unlink(env->loop, &req, *path, NULL);
		args.GetReturnValue().Set(Integer::New(isolate, rc));
		uv_fs_req_cleanup(&req);
	}

	void FileSystem::Mkdir(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		uv_fs_t req;
		String::Utf8Value path(isolate, args[0]);
		int mode = S_IRWXO | S_IRWXG | S_IRWXU;
		int argc = args.Length();
		if (argc > 1) {
			mode = args[1]->Int32Value(context).ToChecked();
		}
		int rc = uv_fs_mkdir(env->loop, &req, *path, mode, NULL);
		args.GetReturnValue().Set(Integer::New(isolate, rc));
		uv_fs_req_cleanup(&req);
	}

	void FileSystem::Rmdir(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		uv_fs_t req;
		String::Utf8Value path(isolate, args[0]);
		int rc = uv_fs_rmdir(env->loop, &req, *path, NULL);
		args.GetReturnValue().Set(Integer::New(isolate, rc));
		uv_fs_req_cleanup(&req);
	}

	void FileSystem::FStat(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		File* obj = ObjectWrap::Unwrap<File>(args[0].As<v8::Object>());
		v8::Local<v8::BigUint64Array> answer = args[1].As<v8::BigUint64Array>();
		Local<ArrayBuffer> ab = answer->Buffer();
		uint64_t *fields = static_cast<uint64_t *>(ab->GetContents().Data());
		uv_fs_t req;
		int rc = uv_fs_fstat(env->loop, &req, obj->fd, NULL);
		if (rc == 0) {
			const uv_stat_t* const s = static_cast<const uv_stat_t*>(req.ptr);
			fields[0] = s->st_dev;
			fields[1] = s->st_mode;
			fields[2] = s->st_nlink;
			fields[3] = s->st_uid;
			fields[4] = s->st_gid;
			fields[5] = s->st_rdev;
			fields[6] = s->st_ino;
			fields[7] = s->st_size;
			fields[8] = s->st_blksize;
			fields[9] = s->st_blocks;
			fields[10] = s->st_flags;
			fields[11] = s->st_gen;
			args.GetReturnValue().Set(Integer::New(isolate, 0));
		}
		args.GetReturnValue().Set(Integer::New(isolate, rc));
		uv_fs_req_cleanup(&req);
	}

	void FileSystem::Rename(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FSync(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::FTruncate(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::Copy(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

	void FileSystem::SendFile(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}

}
}

extern "C" {
	void* _register_fs() {
		return (void*)dv8::fs::InitAll;
	}
}
