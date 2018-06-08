#pragma once

#include <nerangake/objects.h>

namespace nrk {

class ObjectUser {
public:
    using HeapObject = object::HeapObject;
    using RawObject = object::RawObject;

    using Boolean = object::Boolean;
    using Fixnum = object::Fixnum;
    using Nil = object::Nil;
};

} // namespace nrk
