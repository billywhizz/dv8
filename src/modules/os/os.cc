#include "os.h"

namespace dv8
{

namespace os
{
using dv8::builtins::Environment;

int on_signal(jsys_descriptor* signal) {
	struct signalfd_siginfo info;
	ssize_t r = read(signal->fd, &info, sizeof info);
	if (r != sizeof info) {
    perror("read signalfd");
    return -1;
  }
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  OS *os = (OS *)signal->data;
  Local<Value> argv[1] = {Number::New(isolate, info.ssi_signo)};
  Local<Function> Callback = Local<Function>::New(isolate, os->onSignal);
  Local<Value> ret = Callback->Call(context, context->Global(), 1, argv).ToLocalChecked();
  uint32_t close = ret->Uint32Value(context).ToChecked();
  if (close == 1) {
    jsys_descriptor_free(signal);
  }
	fprintf(stderr, "on_signal %s (%u)\n", strsignal((int)info.ssi_signo), info.ssi_signo);
	fprintf(stderr, "  signo   = %u\n", info.ssi_signo);
	fprintf(stderr, "  errno   = %i\n", info.ssi_errno);
	fprintf(stderr, "  code    = %i\n", info.ssi_code);
	fprintf(stderr, "  pid     = %u\n", info.ssi_pid);
	fprintf(stderr, "  uid     = %u\n", info.ssi_uid);
	fprintf(stderr, "  fd      = %i\n", info.ssi_fd);
	fprintf(stderr, "  tid     = %u\n", info.ssi_tid);
	fprintf(stderr, "  band    = %u\n", info.ssi_band);
	fprintf(stderr, "  overrun = %u\n", info.ssi_overrun);
	fprintf(stderr, "  trapno  = %u\n", info.ssi_trapno);
	fprintf(stderr, "  status  = %i\n", info.ssi_status);
	fprintf(stderr, "  int     = %i\n", info.ssi_int);
	fprintf(stderr, "  ptr     = %lu\n", info.ssi_ptr);
	fprintf(stderr, "  utime   = %lu\n", info.ssi_utime);
	fprintf(stderr, "  stime   = %lu\n", info.ssi_stime);
	fprintf(stderr, "  addr    = %lu\n", info.ssi_addr);
  return 0;
}

void InitAll(Local<Object> exports)
{
  OS::Init(exports);
}

void OS::Init(Local<Object> exports)
{
  Isolate *isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);

  tpl->SetClassName(String::NewFromUtf8(isolate, "OS").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  DV8_SET_PROTOTYPE_METHOD(isolate, tpl, "onSignal", OS::OnSignal);
  
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, SIGPIPE), "SIGPIPE", tpl);
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, SIGTERM), "SIGTERM", tpl);
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, SIGINT), "SIGINT", tpl);
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, SIGUSR1), "SIGUSR1", tpl);
  DV8_SET_CONSTANT(isolate, Integer::New(isolate, SIGCHLD), "SIGCHLD", tpl);

  DV8_SET_EXPORT(isolate, tpl, "OS", exports);
}

void OS::New(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  HandleScope handle_scope(isolate);
  if (args.IsConstructCall()) {
    OS *obj = new OS();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  }
}

void OS::Destroy(const v8::WeakCallbackInfo<ObjectWrap> &data) {
  Isolate *isolate = data.GetIsolate();
  v8::HandleScope handleScope(isolate);
  ObjectWrap *wrap = data.GetParameter();
  OS* sock = static_cast<OS *>(wrap);
		#if TRACE
		fprintf(stderr, "OS::Destroy\n");
		#endif
}

void OS::OnSignal(const FunctionCallbackInfo<Value> &args)
{
  Isolate *isolate = args.GetIsolate();
  v8::HandleScope handleScope(isolate);
  Local<Context> context = isolate->GetCurrentContext();
  Environment *env = static_cast<Environment *>(context->GetAlignedPointerFromEmbedderData(kModuleEmbedderDataIndex));
  OS *os = ObjectWrap::Unwrap<OS>(args.Holder());
  if (args.Length() > 0)
  {
    if (args[0]->IsFunction())
    {
      Local<Function> onSignal = Local<Function>::Cast(args[0]);
      os->onSignal.Reset(isolate, onSignal);
      int signum = SIGTERM;
      if (args.Length() > 1)
      {
        Local<Context> context = isolate->GetCurrentContext();
        signum = args[1]->Uint32Value(context).ToChecked();
      }
      jsys_signal_add(env->loop, signum);
      jsys_descriptor *sig = jsys_signal_watcher_create(env->loop);
      jsys_loop_add_flags(env->loop, sig, EPOLLIN);
      sig->callback = dv8::os::on_signal;
      sig->data = os;
      os->handle = sig;
      args.GetReturnValue().Set(Integer::New(isolate, 0));
    }
  }
}

} // namespace os
} // namespace dv8

extern "C" {
	void* _register_os() {
		return (void*)dv8::os::InitAll;
	}
}
