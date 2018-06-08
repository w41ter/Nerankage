#pragma once

namespace nrk {
namespace memory {

enum class AllocateType {
    Static,
    Heap,
};

class AllocatorInterface {
public:
    virtual ~AllocatorInterface() {}

    virtual void* Allocate(AllocateType type, size_t size) = 0;

protected:
    AllocatorInterface() = default;
};

} // namespace memory
} // namespace nrk
