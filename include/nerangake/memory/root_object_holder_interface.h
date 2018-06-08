#pragma once

#include <functional>
#include <type_traits>

#include <nerangake/gc_interface.h>
#include <nerangake/object_user.h>

namespace nrk {
namespace memory {

class RootObjectHolderInterface : public ObjectUser {
public:
    typedef std::function<HeapObject *(HeapObject *)> Callback;

    virtual ~RootObjectHolderInterface() = default;

    virtual void ProcessRootObject(const Callback &cb) = 0;

protected:
    template <
        typename Type,
        typename = typename std::enable_if<
            std::is_base_of<HeapObject, Type>::value, Type>::type>
    Type *ForwardingObject(const Callback &cb, HeapObject *obj) {
        return HeapObject::Cast<Type>(cb(obj));
    }

    RootObjectHolderInterface() = default;
};

} // namespace memory
} // namespace nrk
