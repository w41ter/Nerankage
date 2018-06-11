#pragma once

#include <nerangake/object/heap_object.h>

namespace nrk {
namespace object {

class Float : public HeapObject {
public:
    enum FloatLayout { kData = kFieldStart };

    IMPLICIT_CONSTRUCTORS(Float);

    static Float *Create(double val);
    static Float *CreateGlobal(double val);

    static Float *Promote(const Fixnum *);
    static const Float *ConvertTo(const RawObject *);
    static const Float *ConvertTo(const HeapObject *);
    static int Compare(const Float *, const Float *);
    static Float *Add(const Float *, const Float *);
    static Float *Sub(const Float *, const Float *);
    static Float *Mul(const Float *, const Float *);
    static Float *Div(const Float *, const Float *);
    static Float *Pow(const Float *, const Float *);

    bool Zero() const;
    bool Normal() const;
    bool NInf() const;
    bool PInf() const;
    bool Inf() const;
    bool NaN() const;

private:
    void set_value(double v);
    double value() const;
};

static_assert(
    std::is_trivially_copyable<Float>::value,
    "class `Float` must be trivially copyable type.");

} // namespace object
} // namespace nrk
