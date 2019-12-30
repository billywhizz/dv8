const { dirname } = require('path')
const { writeFile, stat, mkdir } = require('fs')
const { promisify } = require('util')

const [writeFileAsync, statAsync, mkdirAsync] = [promisify(writeFile), promisify(stat), promisify(mkdir)]

async function mkdirpAsync(p, mode = 0o777) {
	try {
		await mkdirAsync(p, mode);
	} catch (error) {
		switch (error.code) {
			case 'ENOENT':
				await mkdirp(dirname(p), mode);
				await mkdirp(p, mode);
				break;
			default:
				const stats = await statAsync(p);
				if (!stats.isDirectory()) {
					throw error;
				}
				break;
		}
	}
}

function getSource(name, className) {
	return `#include "${name}.h"

namespace dv8 {

namespace ${name} {
	using dv8::builtins::Environment;

	void InitAll(Local<Object> exports)
	{
		${className}::Init(exports);
	}

	void ${className}::Init(Local<Object> exports) {
		Isolate* isolate = exports->GetIsolate();
		Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	
		tpl->SetClassName(String::NewFromUtf8(isolate, "${className}").ToLocalChecked());
		tpl->InstanceTemplate()->SetInternalFieldCount(1);
	
		DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "hello", ${className}::Hello);
	
		DV8_SET_EXPORT(isolate, tpl, "${className}", exports);
	}

	void ${className}::New(const FunctionCallbackInfo<Value>& args) {
		Isolate* isolate = args.GetIsolate();
		HandleScope handle_scope(isolate);
		if (args.IsConstructCall()) {
			${className}* obj = new ${className}();
			obj->Wrap(args.This());
			args.GetReturnValue().Set(args.This());
		}
	}

	void ${className}::Hello(const FunctionCallbackInfo<Value> &args)
	{
		Isolate *isolate = args.GetIsolate();
		Local<Context> context = isolate->GetCurrentContext();
		Environment* env = static_cast<Environment*>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
		v8::HandleScope handleScope(isolate);
		${className}* obj = ObjectWrap::Unwrap<${className}>(args.Holder());
		args.GetReturnValue().Set(Integer::New(isolate, 0));
	}
	
}
}	

extern "C" {
	void* _register_${name}() {
		return (void*)dv8::${name}::InitAll;
	}
}
`
}

function getHeader(name, className) {
	return `#ifndef DV8_${className}_H
#define DV8_${className}_H

#include <dv8.h>

namespace dv8 {

namespace ${name} {

using v8::Array;
using v8::Context;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;

void InitAll(Local<Object> exports);

class ${className} : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:

		${className}() {
		}

		~${className}() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Hello(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
`
}

async function run(args) {
	const [name, className] = args
	console.log(`name: ${name}, className: ${className}`)
	await mkdirpAsync(`./src/modules/${name}`)
	await writeFileAsync(`./src/modules/${name}/${name}.cc`, getSource(name, className))
	await writeFileAsync(`./src/modules/${name}/${name}.h`, getHeader(name, className))
}

run(process.argv.slice(2)).catch(console.error)
