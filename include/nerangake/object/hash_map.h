#pragma once

#include <nerangake/object/array.h>

namespace nrk {
namespace object {

/**
 * Object's model
 * - key    sizeof(RawObject*)
 * - value  sizeof(RawObject*)
 * - next   sizeof(RawObject*)
 */
class HashNode : public HeapObject {
public:
    enum HashNodeLayout {
        kKey = kFieldStart,
        kValue = kKey + sizeof(Element),
        kNext = kValue + sizeof(Element),
    };

    IMPLICIT_CONSTRUCTORS(HashNode);

    static size_t Size() { return sizeof(Element) * 3; }
    static HashNode *Create(const RawObject *key, Element value, Element next);

    static HashNode *Create(RawObject *key, Element value) {
        return Create(key, value, Nil::Create());
    }

    void set_key(const RawObject *key) { SetField<kKey>(key); }

    void set_value(Element value) { SetField<kValue>(value); }

    void set_next(Element next) { SetField<kNext>(next); }

    const Element key() const { return GetFieldAs<Element, kKey>(); }

    Element value() { return GetFieldAs<Element, kValue>(); }

    const Element value() const { return GetFieldAs<Element, kValue>(); }

    Element next() { return GetFieldAs<Element, kNext>(); }

    void Children(const ForwardingCallback &cb);
};

/**
 * Object's model:
 * - loadfactor (float)
 * - size uint32_t
 * - Array (array)
 */
class HashMap : public HeapObject {
public:
    enum HashMapOffset {
        kLoadFactor = kFieldStart,
        kLength = kLoadFactor + sizeof(double),
        kBuckets = kLength + sizeof(uint32_t),
    };

    IMPLICIT_CONSTRUCTORS(HashMap);

    static HashMap *Create();

    uint32_t length() const { return GetFieldAs<uint32_t, kLength>(); }

    void set_load_factor(double load_factor) {
        SetField<kLoadFactor>(load_factor);
    }

    double load_factor() const { return GetFieldAs<double, kLoadFactor>(); }

    Array *buckets() { return GetFieldAs<Array *, kBuckets>(); }

    const Array *buckets() const { return GetFieldAs<Array *, kBuckets>(); }

    void set_buckets(Array *buckets) { SetField<kBuckets>(buckets); }

    void Set(const RawObject *key, Element value);
    Element Find(const RawObject *key);
    void Remove(const RawObject *key);

    void Children(const ForwardingCallback &cb);

private:
    void set_length(uint32_t size) { SetField<kLength>(size); }

    uint32_t capacity() const { return buckets()->length(); }

    void ValidateKeyType(const RawObject *key);
    void SetWithoutUpdate(const RawObject *obj, Element value);
    void Update();
    void Expand();
    void Shrink();
    void Rehash(size_t capacity);
    bool IsNeedExpand();
    bool IsNeedShrink();

    Element GetHashNodeLink(const RawObject *key);
    void SetHashNodeLink(const RawObject *key, Element link);
};

static_assert(
    std::is_trivially_copyable<HashNode>::value,
    "class `HashNode` must be trivially copyable type.");
static_assert(
    std::is_trivially_copyable<HashMap>::value,
    "class `HashMap` must be trivially copyable type.");

} // namespace object
} // namespace nrk