#pragma once

#include <assert.h>

#include <stdexcept>
#include <vector>

#include <nerangake/memory/root_object_holder_interface.h>

namespace nrk {

class VMScene : public nrk::memory::RootObjectHolderInterface {
    using Stack = object::Stack;
    using CallInfo = object::CallInfo;

    VMScene(const VMScene &) = delete;
    VMScene &operator=(const VMScene &) = delete;

public:
    VMScene();
    ~VMScene();
    VMScene(VMScene &&) = default;
    VMScene &operator=(VMScene &&) = default;

    CallInfo *top();
    bool Empty();
    void Push(CallInfo *ci);
    CallInfo *Pop();
    Stack *stack() { return stack_; }

    virtual void ProcessRootObject(const Callback &cb) override;

private:
    const static int kMaxStackCount = 65535;

    uint32_t call_chain_size_;
    Stack *stack_;
    CallInfo *top_;
};

} // namespace nrk
