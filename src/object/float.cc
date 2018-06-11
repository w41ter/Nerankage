#include <nerangake/object/float.h>

#include <assert.h>

#include <cmath>
#include <stdexcept>

namespace nrk {
namespace object {

static const double FLOAT_EQUAL_SIZE = 0.0000000000000001;

static bool Equals(const HeapObject *lhs, const HeapObject *rhs) {
    const Float *a = Float::ConvertTo(lhs), *b = Float::ConvertTo(rhs);
    return Float::Compare(a, b) == 0;
}

static const ObjectMethodTable *VTable() {
    static ObjectMethodTable table = {&Equals, nullptr, nullptr};
    return &table;
}

Float *Float::Create(double val) {
    size_t size = sizeof(val);
    Float *db = Allocate<Float>(size);

    db->set_value(val);
    db->set_type(kFloat);
    db->set_vtable(VTable());
    return db;
}

Float *Float::CreateGlobal(double val) {
    size_t size = sizeof(val);
    Float *db = Allocate<Float>(size);

    db->set_value(val);
    db->set_type(kFloat);
    db->set_vtable(VTable());
    return db;
}

Float *Float::Promote(const Fixnum *num) {
    int32_t inum = num->value();
    return Float::Create(static_cast<double>(inum));
}

int Float::Compare(const Float *lhs, const Float *rhs) {
    assert(lhs && rhs && "nullptr exception");

    double a = lhs->value(), b = rhs->value();
    double sum = a - b;
    if (std::abs(sum) <= FLOAT_EQUAL_SIZE) return 0;
    return (sum > 0) ? 1 : -1;
}

Float *Float::Add(const Float *lhs, const Float *rhs) {
    assert(lhs && rhs && "nullptr exception");

    double a = lhs->value(), b = rhs->value();
    return Float::Create(a + b);
}

Float *Float::Sub(const Float *lhs, const Float *rhs) {
    assert(lhs && rhs && "nullptr exception");

    double a = lhs->value(), b = rhs->value();
    return Float::Create(a - b);
}

Float *Float::Mul(const Float *lhs, const Float *rhs) {
    assert(lhs && rhs && "nullptr exception");

    double a = lhs->value(), b = rhs->value();
    return Float::Create(a * b);
}

Float *Float::Div(const Float *lhs, const Float *rhs) {
    assert(lhs && rhs && "nullptr exception");

    double a = lhs->value(), b = rhs->value();
    return Float::Create(a / b);
}

Float *Float::Pow(const Float *lhs, const Float *rhs) {
    assert(lhs && rhs && "nullptr exception");

    double a = lhs->value(), b = rhs->value();
    return Float::Create(pow(a, b));
}

bool Float::Zero() const {
    double val = value();
    return std::fpclassify(val) == FP_ZERO;
}

bool Float::Normal() const { return std::isnormal(value()); }

bool Float::NInf() const {
    double val = value();
    return std::isinf(val) && val < 0.0;
}

bool Float::PInf() const {
    double val = value();
    return std::isinf(val) && val > 0.0;
}

bool Float::Inf() const { return PInf() || NInf(); }

bool Float::NaN() const {
    double val = value();
    return std::isnan(val);
}

void Float::set_value(double v) { SetField<kData>(v); }

double Float::value() const { return GetFieldAs<double, kData>(); }

const Float *Float::ConvertTo(const RawObject *val) {
    if (val->IsFixnum()) {
        return Float::Promote(val->As<Fixnum>());
    } else if (val->IsObject()) {
        const HeapObject *obj = HeapObject::From(val);
        return ConvertTo(obj);
    }

    throw std::runtime_error("cannot convert to Float object");
}

const Float *Float::ConvertTo(const HeapObject *obj) {
    if (obj->IsFloat()) { return HeapObject::Cast<Float>(obj); }

    throw std::runtime_error("cannot convert to Float object");
}

} // namespace object
} // namespace nrk

