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

	using Array = object::Array;
    using CallInfo = object::CallInfo;
    using Closure = object::Closure;
    using Float = object::Float;
    using HashMap = object::HashMap;
    using Prototype = object::Prototype;
    using Stack = object::Stack;
    using String = object::String;
    using Vector = object::Vector;
    using UserClosure = object::UserClosure;
};

} // namespace nrk
