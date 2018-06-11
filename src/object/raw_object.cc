#include <nerankage/object/raw_object.h>

#include <assert.h>

#include <algorithm>
#include <stdexcept>

#include <nerankage/context.h>
#include <nerankage/objects.h>

namespace nrk {
namespace object {

/**
 * Can compare type Fixnum, Float, and if only one is Float, then promote
 * the other to Float type and compare again.
 *
 * @param lhs
 * @param rhs
 * @return int  the result of compare:
 *              lhs == rhs: 0, lhs > rhs: 1, lhs < rhs: -1.
 */
static int Compare(const RawObject* lhs, const RawObject* rhs) {
    const Float *a = Float::ConvertTo(lhs), *b = Float::ConvertTo(rhs);
    return Float::Compare(a, b);
}

/**
 * The Not () operation results in reverse to the True () result.
 *
 * @param val
 * @return Boolean
 */
RawObject* RawObject::Not(const RawObject* val) {
    assert(val && "nullptr exception");
    return Boolean::Create(!RawObject::True(val));
}

/**
 * Fixnum
 */
RawObject* RawObject::Add(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Fixnum::Create(a->value() + b->value());
    } else {
        const Float *a = Float::ConvertTo(lhs), *b = Float::ConvertTo(rhs);
        return Float::Add(a, b);
    }
}

RawObject* RawObject::Sub(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Fixnum::Create(a->value() - b->value());
    } else {
        const Float *a = Float::ConvertTo(lhs), *b = Float::ConvertTo(rhs);
        return Float::Sub(a, b);
    }
}

RawObject* RawObject::Mul(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Fixnum::Create(a->value() * b->value());
    } else {
        const Float *a = Float::ConvertTo(lhs), *b = Float::ConvertTo(rhs);
        return Float::Mul(a, b);
    }
}

RawObject* RawObject::Div(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        if (b->value() == 0) {
            throw std::runtime_error("div zero error");
        }
        return Fixnum::Create(a->value() / b->value());
    } else {
        const Float *a = Float::ConvertTo(lhs), *b = Float::ConvertTo(rhs);
        return Float::Div(a, b);
    }
}

RawObject* RawObject::Mod(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        int32_t a = lhs->As<Fixnum>()->value();
        int32_t b = lhs->As<Fixnum>()->value();
        if (b == 0) {
            throw std::runtime_error("div zero exception.");
        }
        return Fixnum::Create(a % b);
    } else {
        throw std::runtime_error("MOD only support Fixnum.");
    }
}

RawObject* RawObject::Pow(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    GCInterface *gc = Context::gc();
    HeapObject *obj = const_cast<Float*>(Float::ConvertTo(lhs));
    gc->Push(&obj);
    const Float *b = Float::ConvertTo(rhs);
    gc->Pop();
    
    const Float *a = HeapObject::Cast<Float>(obj);

    return Float::Pow(a, b);
}

RawObject* RawObject::GT(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Boolean::Create(a->value() > b->value());
    } else {
        return Boolean::Create(Compare(lhs, rhs) > 0);
    }
}

RawObject* RawObject::GE(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Boolean::Create(a->value() >= b->value());
    } else {
        return Boolean::Create(Compare(lhs, rhs) >= 0);
    }
}

RawObject* RawObject::LT(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Boolean::Create(a->value() < b->value());
    } else {
        return Boolean::Create(Compare(lhs, rhs) < 0);
    }
}

RawObject* RawObject::LE(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Boolean::Create(a->value() <= b->value());
    } else {
        return Boolean::Create(Compare(lhs, rhs) <= 0);
    }
}

RawObject* RawObject::EQ(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    return Boolean::Create(HeapObject::Equals(lhs, rhs));
}

RawObject* RawObject::NE(const RawObject* lhs, const RawObject* rhs) {
    assert(lhs && rhs && "nullptr exception");

    if (lhs->IsFixnum() && rhs->IsFixnum()) {
        const Fixnum *a = lhs->As<Fixnum>(), *b = rhs->As<Fixnum>();
        return Boolean::Create(a->value() != b->value());
    } else {
        return Boolean::Create(Compare(lhs, rhs) != 0);
    }
}

/**
 * Index has only a few possible types, Vector, String, HashMap:
 * Among them, HashMap supports index as Fixnum and String, The other
 * only support Fixnum.
 *
 * @param object    target to set index.
 * @param index
 * @return          object[index] if exists, otherwise Nil.
 */
RawObject* RawObject::Index(RawObject* object, const RawObject* index) {
    assert(object && index && "nullptr exception");

    if (object->IsObject()) {
        HeapObject* obj = HeapObject::From(object);
        if (obj->IsHashMap()) {
            HashMap* map = HeapObject::Cast<HashMap>(obj);
            return map->Find(index);
        } else if (obj->IsVector()) {
            Vector* vector = HeapObject::Cast<Vector>(obj);
            int32_t idx;
            if (index->IsFixnum() &&
                (idx = index->As<Fixnum>()->value()) >= 0) {
                return vector->Get(static_cast<unsigned>(idx));
            } else {
                throw std::runtime_error("error index");
            }
        } else if (obj->IsString()) {
            String* string = HeapObject::Cast<String>(obj);
            int32_t idx;
            if (index->IsFixnum() &&
                (idx = index->As<Fixnum>()->value()) >= 0) {
                return Fixnum::Create(string->At(static_cast<unsigned>(idx)));
            } else {
                throw std::runtime_error("error index");
            }
        }
    }

    throw std::runtime_error("object not indexable");
}

/**
 * Set_index has only a few possible types, Vector, HashMap:
 * Among them, HashMap supports index as Fixnum and String, The other
 * only support Fixnum.
 *
 * @param object    target to set index.
 * @param index
 * @param val
 */
void RawObject::SetIndex(RawObject* object, const RawObject* index,
                         RawObject* val) {
    assert(object && index && val && "nullptr exception");

    if (object->IsObject()) {
        HeapObject* obj = HeapObject::From(object);
        if (obj->IsHashMap()) {
            HashMap* map = HeapObject::Cast<HashMap>(obj);
            map->Set(index, val);
            return;
        } else if (obj->IsVector()) {
            Vector* vector = HeapObject::Cast<Vector>(obj);
            int32_t idx;
            if (index->IsFixnum() &&
                (idx = index->As<Fixnum>()->value()) >= 0) {
                vector->Set(static_cast<unsigned>(idx), val);
                return;
            } else {
                throw std::runtime_error("error index");
            }
        }
    }

    throw std::runtime_error("object not indexable");
}

/**
 * In addition to the value associated with Boolean, the rest is consistent with
 * NZ().
 *
 * @param val   the value need to validate.
 * @return      true of false.
 */
bool RawObject::True(const RawObject* val) {
    assert(val && "nullptr exception");

    if (val->IsBoolean()) {
        return val->As<Boolean>()->value();
    } else {
        return RawObject::NZ(val);
    }
}

/**
 * Specifies when the built-in object is zero:
 * - Nil: Forever zero;
 * - Fixnum: value is zero;
 * - Float: value is zero;
 * - Others: never nonzero;
 *
 * @param val   the value need to validate.
 * @return      true if val isn't zero, otherwise false.
 */
bool RawObject::NZ(const RawObject* val) {
    assert(val && "nullptr exception");

    if (val->IsFixnum()) {
        return val->As<Fixnum>()->value() != 0;
    } else if (val->IsObject()) {
        const HeapObject* obj = HeapObject::From(val);
        if (!obj->IsFloat()) return true;
        return !HeapObject::Cast<Float>(obj)->Zero();
    } else {
        return !val->IsNil();
    }
}

}  // namespace object
}  // namespace nrk
