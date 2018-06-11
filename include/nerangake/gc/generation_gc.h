#pragma once

#include <set>
#include <stack>
#include <unordered_map>

#include <nerangake/gc/gc_interface.h>
#include <nerangake/memory/allocator_interface.h>
#include <nerangake/memory/root_object_holder_interface.h>

namespace nrk {
namespace gc {

class GenerationGC : public nrk::memory::AllocatorInterface,
                     public GCInterface {
    using ForwardingMap = std::unordered_map<HeapObject *, HeapObject *>;

    GenerationGC(const GenerationGC &) = delete;
    GenerationGC &operator=(const GenerationGC &) = delete;

public:
    GenerationGC(size_t size);
    virtual ~GenerationGC();

    virtual void *Allocate(size_t size) override {
        return AllocateInNewSpace(size);
    }

    virtual void *Static(size_t size) override {
        return AllocateInOldSpace(size);
    }

    virtual void Push(HeapObject **obj) override { temp_stack_.push(obj); }

    virtual HeapObject **Pop() override {
        HeapObject **obj = temp_stack_.top();
        temp_stack_.pop();
        return obj;
    }

    virtual void MajorGC() override;
    virtual void MinorGC() override;
    virtual void FullGC() override;
    virtual void WriteBarrier(
        HeapObject *, RawObject **, HeapObject *) override;

private:
    void AllocationFail();
    void ProcessRootObjects(const RootObjectHolderInterface::Callback &cb);

    HeapObject *Mark(HeapObject *obj);
    void RecordForwarding(ForwardingMap &map);
    void ResetReferences(ForwardingMap &map);
    void Compact(ForwardingMap &map);
    uint8_t *AllocateInOldSpace(size_t size);

    void Promote(HeapObject *obj);
    void ProcessTransboundaryReference();

    HeapObject *CopyIntoAnotherSpace(HeapObject *obj);
    // HeapObject *CopyAndSet(HeapObject **pObj);
    HeapObject *AllocateInNewSpace(size_t size);

    static const uint8_t MAX_AGE = 64;

    const size_t space_size_;
    uint8_t *const start_;
    uint8_t *const survivor1_start_;
    uint8_t *const survivor2_start_;
    uint8_t *const old_start_;
    uint8_t *const end_;

    std::set<HeapObject *> record_set_;
    std::stack<HeapObject **> temp_stack_;

    uint8_t *from_, *to_, *to_free_;
    uint8_t *new_free_, *old_free_;
};

} // namespace gc
} // namespace nrk
