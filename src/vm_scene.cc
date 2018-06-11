#include <nerangake/vm_scene.h>

#include <nerangake/context.h>
#include <nerangake/object/stack.h>

namespace nrk {

VMScene::VMScene() : call_chain_size_(0), top_(nullptr) {
    Context::RegisterRootObjectHolder(this);
    stack_ = Stack::Create();
}

VMScene::~VMScene() { Context::CancelledRootObjectHolder(this); }

VMScene::CallInfo *VMScene::top() {
    assert(!Empty() && "try deref on nullptr");
    return top_;
}

bool VMScene::Empty() { return top_ == nullptr; }

void VMScene::Push(CallInfo *ci) {
    assert(ci && "nullptr exception.");
    assert(ci->parent() != nullptr && "using unreleased frame.");

    if (++call_chain_size_ > kMaxStackCount)
        throw std::runtime_error("stack overflow");

    ci->set_parent(top_);
    top_ = ci;
}

VMScene::CallInfo *VMScene::Pop() {
    assert(!Empty() && "current scene is empty.");

    --call_chain_size_;
    CallInfo *ci = top_;
    top_ = top_->parent();
    return ci;
}

void VMScene::ProcessRootObject(const Callback &cb) {
    top_ = ForwardingObject<CallInfo>(cb, top_);
    stack_ = ForwardingObject<Stack>(cb, stack_);
}

} // namespace nrk
