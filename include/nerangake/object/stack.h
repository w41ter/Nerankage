#pragma once

#include <nerangake/object/array.h>
#include <nerangake/object/heap_object.h>

namespace nrk {
namespace object {

/**
 * Object's layout
 * - depth (uint32_t)   the number array of current_top
 * - offset (uint32_t)  next_availiable pos in array's offset.
 * - top (RawObject)
 * - Array (array)
 *
 * array[0] -> parent
 * array[1:SPLICE_SIZE] -> data
 **/
class Stack : public HeapObject {
public:
    enum StackLayout {
        kDepth = kFieldStart,
        kOffset = kDepth + sizeof(uint32_t),
        kTop = kOffset + sizeof(uint32_t),
        kArray = kDepth + sizeof(Element)
    };

    IMPLICIT_CONSTRUCTORS(Stack);

    /**
     * Stack::Create on old space
     */
    static Stack* Create();
    static size_t Size();

    Element top();

    void Push(Element e);
    void PushN(Element e, uint8_t length);
    Element Pop();
    void Pop(uint8_t n);

    bool Empty() const;
    const Element Get(unsigned offset) const;
    Element Get(unsigned offset);
    void Set(unsigned offset, Element e);
    size_t Length() const;

    void Children(const ForwardingCallback& cb);

private:
    void set_top(Element e);

    void Extend();
    void Shrink();

    Array* current_buffer();
    const Array* current_buffer() const;
    void set_current_buffer(Array* array);
    uint32_t depth() const;
    void set_depth(uint32_t dep);
    uint32_t offset() const;
    void set_offset(uint32_t offset);
};

static_assert(
    std::is_trivially_copyable<Stack>::value,
    "class `Stack` must be trivially copyable type.");

} // namespace object
} // namespace nrk
