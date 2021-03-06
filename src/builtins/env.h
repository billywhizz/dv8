#ifndef DV8_ENV_H
#define DV8_ENV_H

#include <dv8.h>

namespace dv8
{
const int kModuleEmbedderDataIndex = 32;
const int kInspectorClientIndex = 33;


namespace builtins
{
class Environment
{
  public:
    uv_loop_t *loop;
    v8::Persistent<v8::Function> onExit;
    v8::Persistent<v8::Function> onUnhandledRejection;
    v8::Persistent<v8::Object> err;
    js_error *error;

    inline void AssignToContext(v8::Local<v8::Context> context)
    {
        context->SetAlignedPointerInEmbedderData(kModuleEmbedderDataIndex, this);
    }
    Environment()
    {
    }

  private:
    ~Environment()
    {
    }
};

} // namespace builtins
} // namespace dv8
#endif
