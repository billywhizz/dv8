#ifndef DV8_Inspector_H
#define DV8_Inspector_H

#include <dv8.h>

namespace dv8 {

namespace inspector {

enum {
  kModuleEmbedderDataIndex,
  kInspectorClientIndex
};

class InspectorFrontend final : public V8Inspector::Channel {
 public:

  explicit InspectorFrontend(Local<Context> context) {
    isolate_ = context->GetIsolate();
    context_.Reset(isolate_, context);
  }

  ~InspectorFrontend() override = default;

 private:

  void sendResponse(int callId, std::unique_ptr<StringBuffer> message) override {
    Send(message->string());
  }

  void sendNotification(std::unique_ptr<StringBuffer> message) override {
    Send(message->string());
  }

  void flushProtocolNotifications() override {}

  void Send(const v8_inspector::StringView& string) {
    v8::Isolate::AllowJavascriptExecutionScope allow_script(isolate_);
    int length = static_cast<int>(string.length());
    Local<String> message = (string.is8Bit() ? v8::String::NewFromOneByte(isolate_, reinterpret_cast<const uint8_t*>(string.characters8()), v8::NewStringType::kNormal, length) : v8::String::NewFromTwoByte(isolate_, reinterpret_cast<const uint16_t*>(string.characters16()), v8::NewStringType::kNormal, length)).ToLocalChecked();
    Local<String> callback_name = v8::String::NewFromUtf8(isolate_, "receive", v8::NewStringType::kNormal).ToLocalChecked();
    Local<Context> context = context_.Get(isolate_);
    Local<Value> callback = context->Global()->Get(context, callback_name).ToLocalChecked();
    if (callback->IsFunction()) {
      v8::TryCatch try_catch(isolate_);
      Local<Value> args[] = {message};
      Local<Function>::Cast(callback)->Call(context, Undefined(isolate_), 1, args);
    }
  }

  Isolate* isolate_;
  Global<Context> context_;
};

class InspectorClient : public V8InspectorClient {
 public:
  InspectorClient(Local<Context> context, bool connect) {
    if (!connect) return;
    isolate_ = context->GetIsolate();
    channel_.reset(new InspectorFrontend(context));
    inspector_ = V8Inspector::create(isolate_, this);
    session_ = inspector_->connect(1, channel_.get(), StringView());
    context->SetAlignedPointerInEmbedderData(kInspectorClientIndex, this);
    inspector_->contextCreated(V8ContextInfo(context, kContextGroupId, StringView()));
    Local<Value> function = FunctionTemplate::New(isolate_, SendInspectorMessage)->GetFunction(context).ToLocalChecked();
    Local<String> function_name = String::NewFromUtf8(isolate_, "send", NewStringType::kNormal).ToLocalChecked();
    context->Global()->Set(context, function_name, function).FromJust();
    context_.Reset(isolate_, context);
  }

  void runMessageLoopOnPause(int context_group_id) override {
    Local<String> callback_name = v8::String::NewFromUtf8(isolate_, "onRunMessageLoop", v8::NewStringType::kNormal).ToLocalChecked();
    Local<Context> context = context_.Get(isolate_);
    Local<Value> callback = context->Global()->Get(context, callback_name).ToLocalChecked();
    if (callback->IsFunction()) {
      v8::TryCatch try_catch(isolate_);
      Local<Value> args[] = {};
      Local<Function>::Cast(callback)->Call(context, Undefined(isolate_), 0, args);
    }
  }

  void quitMessageLoopOnPause() override {
    Local<String> callback_name = v8::String::NewFromUtf8(isolate_, "onQuitMessageLoop", v8::NewStringType::kNormal).ToLocalChecked();
    Local<Context> context = context_.Get(isolate_);
    Local<Value> callback = context->Global()->Get(context, callback_name).ToLocalChecked();
    if (callback->IsFunction()) {
      v8::TryCatch try_catch(isolate_);
      Local<Value> args[] = {};
      Local<Function>::Cast(callback)->Call(context, Undefined(isolate_), 0, args);
    }
  }

 private:

  static V8InspectorSession* GetSession(Local<Context> context) {
    InspectorClient* inspector_client = static_cast<InspectorClient*>(context->GetAlignedPointerFromEmbedderData(kInspectorClientIndex));
    return inspector_client->session_.get();
  }

  Local<Context> ensureDefaultContextInGroup(int group_id) override {
    return context_.Get(isolate_);
  }

  static void SendInspectorMessage(const v8::FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    v8::HandleScope handle_scope(isolate);
    Local<Context> context = isolate->GetCurrentContext();
    args.GetReturnValue().Set(Undefined(isolate));
    Local<String> message = args[0]->ToString(context).ToLocalChecked();
    V8InspectorSession* session = InspectorClient::GetSession(context);
    int length = message->Length();
    std::unique_ptr<uint16_t[]> buffer(new uint16_t[length]);
    message->Write(isolate, buffer.get(), 0, length);
    StringView message_view(buffer.get(), length);
    session->dispatchProtocolMessage(message_view);
    args.GetReturnValue().Set(True(isolate));
  }

  static const int kContextGroupId = 1;
  std::unique_ptr<V8Inspector> inspector_;
  std::unique_ptr<V8InspectorSession> session_;
  std::unique_ptr<V8Inspector::Channel> channel_;
  Global<Context> context_;
  Isolate* isolate_;
};

class Inspector : public dv8::ObjectWrap {
	public:
		static void Init(v8::Local<v8::Object> exports);

	private:

		Inspector() {
		}

		~Inspector() {
		}

		static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
		static void Setup(const v8::FunctionCallbackInfo<v8::Value>& args);

};

}
}
#endif
