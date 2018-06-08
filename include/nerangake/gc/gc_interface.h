#pragma once

#include <nerangake/object_user.h>

namespace nrk {
namespace gc {

enum class GCType {
    Generation,
};

class GCInterface : public ObjectUser {
public:
    typedef std::function<void(HeapObject **)> ProcessObjectCallback;

    virtual ~GCInterface() {}

    virtual void MajorGC() = 0;
    virtual void MinorGC() = 0;
    virtual void FullGC() = 0;
    virtual void Push(HeapObject **) = 0;
    virtual HeapObject **Pop() = 0;
    virtual void WriteBarrier(HeapObject *, RawObject **, HeapObject *) = 0;

    static GCInterface &Create(GCType type);

protected:
    GCInterface() = default;
};

} // namespace gc
} // namespace nrk
