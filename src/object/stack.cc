#include <nerangake/object/stack.h>

#include <assert.h>

#include <nerangake/context.h>

namespace nrk {
namespace object {

static void Children(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception");

    Stack *stack = HeapObject::Cast<Stack>(obj);
    stack->Children(cb);
}

static const ObjectMethodTable *VTable() {
    static ObjectMethodTable table = {nullptr, nullptr, &Children};
    return &table;
}

static const int STACK_SPLICE_LENGTH = 64;

static Array *NewArray() { return Array::Create(STACK_SPLICE_LENGTH + 1); }

size_t Stack::Size() {
    return sizeof(uint32_t) + sizeof(uint32_t) + sizeof(uintptr_t) +
        sizeof(uintptr_t);
}

Stack *Stack::Create() {
    HeapObject *obj = HeapObject::Static<HeapObject>(Size());

    GCInterface *gc = Context::gc();
    gc->Push(&obj);
    Array *array = NewArray();
    gc->Pop();

    Stack *stack = Cast<Stack>(obj);
    stack->set_type(kStack);
    stack->set_top(Nil::Create());
    stack->set_depth(0);
    stack->set_offset(1);
    stack->set_current_buffer(array);
    stack->set_vtable(VTable());

    return stack;
}

Stack::Element Stack::top() {
    assert(!Empty() && "try ref empty stack");

    return GetFieldAs<Element, kTop>();
}

void Stack::Pop(uint8_t n) {
    assert(n < Length() && "out of range.");

    for (uint8_t i = 0; i < n; ++i) Pop();
}

void Stack::Push(Element e) {
    uint32_t off = offset();
    if (off > STACK_SPLICE_LENGTH) {
        Extend();
        off = 1;
    }

    Array *array = current_buffer();
    array->Set(off, e);
    set_offset(off + 1);
    set_top(e);
}

void Stack::PushN(Element e, uint8_t length) {
    for (uint8_t i = 0; i < length; ++i) { Stack::Push(e); }
}

Stack::Element Stack::Pop() {
    assert(!Empty() && "stack are empty");

    uint32_t off = offset();
    if (off == 1) {
        Shrink();
        off = STACK_SPLICE_LENGTH + 1;
    }

    Element e = top();
    off -= 1;

    if (off == 1) {
        RawObject *raw = current_buffer()->Get(0);
        if (raw->IsNil()) {
            set_top(Nil::Create());
        } else {
            HeapObject *obj = HeapObject::From(raw);
            assert(obj->IsArray() && "error");
            Array *parent = Cast<Array>(obj);
            set_top(parent->Get(STACK_SPLICE_LENGTH));
        }
    } else {
        set_top(current_buffer()->Get(off - 1));
    }
    set_offset(off);
    return e;
}

bool Stack::Empty() const {
    return STACK_SPLICE_LENGTH * depth() + offset() == 1;
}

const Stack::Element Stack::Get(unsigned length) const {
    assert(length < Length() && "out of stack range");

    const Array *array = current_buffer();
    uint32_t splice_length = offset() - 1;
    int idx = length;
    do {
        if (idx < (int)splice_length) {
            return array->Get(splice_length - idx);
        } else {
            idx -= splice_length;
            RawObject *raw = array->Get(0);
            assert(raw->IsObject() && From(raw)->IsArray());
            array = Cast<Array>(From(raw));
            splice_length = STACK_SPLICE_LENGTH;
        }
    } while (idx >= 0);

    throw std::logic_error("unreachable");
}

Stack::Element Stack::Get(unsigned length) {
    const Stack *thiz = this;
    return const_cast<Element>(thiz->Get(length));
}

void Stack::Set(unsigned length, Element e) {
    Array *array = current_buffer();
    uint32_t splice_length = offset() - 1;
    int idx = length;
    do {
        if (idx < (int)splice_length) {
            array->Set(splice_length - idx, e);
            break;
        } else {
            idx -= splice_length;
            RawObject *raw = array->Get(0);
            assert(raw->IsObject() && From(raw)->IsArray());
            array = Cast<Array>(From(raw));
            splice_length = STACK_SPLICE_LENGTH;
        }
    } while (idx >= 0);
}

size_t Stack::Length() const {
    return depth() * (STACK_SPLICE_LENGTH - 1) + offset();
}

void Stack::set_top(Element e) { SetField<kTop>(e); }

void Stack::Extend() {
    Array *next = Array::Create(STACK_SPLICE_LENGTH);
    next->Set(0, current_buffer());
    set_current_buffer(next);
    set_offset(1);
}

void Stack::Shrink() {
    RawObject *raw = current_buffer()->Get(0);
    assert(raw->IsObject() && Cast<Array>(From(raw)) && "error");
    Array *parent = Cast<Array>(From(raw));
    set_current_buffer(parent);
    set_offset(STACK_SPLICE_LENGTH);
}

Array *Stack::current_buffer() { return GetFieldAs<Array *, kArray>(); }

const Array *Stack::current_buffer() const {
    return GetFieldAs<Array *, kArray>();
}

void Stack::set_current_buffer(Array *array) {
    SetField<kArray, Array *>(array);
}

uint32_t Stack::depth() const { return GetFieldAs<uint32_t, kDepth>(); }

void Stack::set_depth(uint32_t dep) { SetField<kDepth, uint32_t>(dep); }

uint32_t Stack::offset() const { return GetFieldAs<uint32_t, kOffset>(); }

void Stack::set_offset(uint32_t offset) { SetField<kOffset>(offset); }

void Stack::Children(const ForwardingCallback &cb) {
    Array *buffer = current_buffer();
    buffer = ForwardingObject<Array>(cb, buffer);
    set_current_buffer(buffer);

    RawObject *t = top();
    if (t->IsObject())
        t = ForwardingObject<HeapObject>(cb, HeapObject::From(t));
    set_top(t);
}

} // namespace object
} // namespace nrk
