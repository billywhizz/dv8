#ifndef DV8_ENV_H
#define DV8_ENV_H

#include <dv8.h>
#include <jsys.h>

namespace dv8
{
const int kModuleEmbedderDataIndex = 32;
const int kInspectorClientIndex = 33;


namespace builtins
{
class Environment
{
  public:
    jsys_loop *loop;
    int argc;
    char** argv;

    inline void AssignToContext(v8::Local<v8::Context> context)
    {
        context->SetAlignedPointerInEmbedderData(kModuleEmbedderDataIndex, this);
    }
    Environment()
    {
    }
    ~Environment()
    {
    }
};

} // namespace builtins
} // namespace dv8
#endif
