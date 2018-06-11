#pragma once

#include <list>
#include <memory>

#include <nerangake/gc/gc_interface.h>
#include <nerangake/memory/allocator_interface.h>
#include <nerangake/memory/root_object_holder_interface.h>
#include <nerangake/state/executor_interface.h>

namespace nrk {

class Context {
    Context() = default;

    Context(const Context &) = delete;
    Context &operator=(const Context &) = delete;

public:
    using GCInterface = gc::GCInterface;
    using AllocatorInterface = memory::AllocatorInterface;
    using RootObjectHolderInterface = memory::RootObjectHolderInterface;
    using ExecutorInterface = state::ExecutorInterface;

    typedef std::list<RootObjectHolderInterface *> RootObjectHolders;
    typedef RootObjectHolders::iterator iterator;

    Context(Context &&) = default;
    Context &operator=(Context &&) = default;
    ~Context() = default;

    static Context &Instance() {
        static Context instance;
        return instance;
    }

    static void set_allocator(AllocatorInterface *allocator) {
        Instance().allocator_ = allocator;
    }

    static void set_executor(ExecutorInterface *executor) {
        Instance().executor_ = executor;
    }

    static void set_gc(GCInterface *gc) { Instance().gc_ = gc; }

    static AllocatorInterface *allocator() {
        assert(
            Instance().allocator_ &&
            "allocator of current context not intialized");

        return Instance().allocator_;
    }

    static ExecutorInterface *executor() { return Instance().executor_; }

    static GCInterface *gc() {
        assert(Instance().gc_ && "gc of current context not intialized");

        return Instance().gc_;
    }

    static void RegisterRootObjectHolder(RootObjectHolderInterface *i) {
        RootObjectHolders &holders = Instance().root_object_holders_;
        for (RootObjectHolderInterface *holder : holders) {
            if (i == holder) return;
        }
        holders.push_back(i);
    }

    static void CancelledRootObjectHolder(RootObjectHolderInterface *i) {
        RootObjectHolders &holders = Instance().root_object_holders_;
        auto it = holders.begin();
        auto end = holders.end();
        while (it != end) {
            if (*it == i) {
                holders.erase(it);
                return;
            }
        }
    }

    static iterator root_object_holder_begin() {
        return Instance().root_object_holders_.begin();
    }

    static iterator root_object_holder_end() {
        return Instance().root_object_holders_.end();
    }

private:
    AllocatorInterface *allocator_;
    ExecutorInterface *executor_;
    GCInterface *gc_;

    RootObjectHolders root_object_holders_;
};

} // namespace nrk
