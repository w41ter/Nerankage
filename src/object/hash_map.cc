#include <nerangake/object/hash_map.h>

#include <assert.h>

#include <stdexcept>

#include <nerangake/context.h>

namespace nrk {
namespace object {

static void Children(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception");

    HashNode *node = HeapObject::Cast<HashNode>(obj);
    node->Children(cb);
}

static bool HashNodeEquals(const HeapObject *n1, const HeapObject *n2) {
    const HashNode *node1 = HeapObject::Cast<HashNode>(n1),
                   *node2 = HeapObject::Cast<HashNode>(n2);
    return HeapObject::Equals(node1->key(), node2->key()) &&
        HeapObject::Equals(node1->value(), node2->value());
}

static ObjectMethodTable *HashNodeVTable() {
    static ObjectMethodTable table = {
        &HashNodeEquals,
        nullptr,
        &Children,
    };
    return &table;
}

HashNode *HashNode::Create(const RawObject *key, Element value, Element next) {
    HashNode *node = Allocate<HashNode>(Size());
    node->set_key(key);
    node->set_value(value);
    node->set_next(next);
    node->set_vtable(HashNodeVTable());
    node->set_type(kHashNode);
    return node;
}

void HashNode::Children(const ForwardingCallback &cb) {
    const RawObject *key = this->key();
    RawObject *value = this->value();
    RawObject *next = this->next();

    if (key->IsObject())
        key = ForwardingObject<HeapObject>(cb, HeapObject::From(key));

    if (value->IsObject())
        value = ForwardingObject<HeapObject>(cb, HeapObject::From(value));

    if (next->IsObject())
        next = ForwardingObject<HeapObject>(cb, HeapObject::From(next));

    set_key(key);
    set_value(value);
    set_next(next);
}

static void HashMapChildren(HeapObject *obj, const ForwardingCallback &cb) {
    assert(obj && "nullptr exception");

    HashMap *map = HeapObject::Cast<HashMap>(obj);
    map->Children(cb);
}

static const ObjectMethodTable *HashMapVTable() {
    static ObjectMethodTable table = {nullptr, nullptr, &HashMapChildren};
    return &table;
}

HashMap *HashMap::Create() {
    const size_t length = 16;
    const double load_factor = 0.75;
    size_t size = sizeof(double) + sizeof(uint32_t) + sizeof(Array *);

    HeapObject *obj = Allocate<HeapObject>(size);

    GCInterface *gc = Context::gc();
    gc->Push(&obj);
    Array *buckets = Array::Create(length);
    gc->Pop();

    HashMap *map = Cast<HashMap>(obj);
    map->set_length(0);
    map->set_load_factor(load_factor);
    map->set_buckets(buckets);
    map->set_type(kHashMap);
    map->set_vtable(HashMapVTable());

    return map;
}

void HashMap::Set(const RawObject *key, Element value) {
    if (value->IsNil()) {
        Remove(key);
    } else {
        SetWithoutUpdate(key, value);
        Update();
    }
}

HeapObject::Element HashMap::Find(const RawObject *key) {
    Element link = GetHashNodeLink(key);
    while (!link->IsNil()) {
        HeapObject *obj = HeapObject::From(link);
        HashNode *node = Cast<HashNode>(obj);
        if (HeapObject::Equals(node->key(), key)) return node->value();
        link = node->next();
    }
    return Nil::Create();
}

void HashMap::Remove(const RawObject *key) {
    RawObject *link = GetHashNodeLink(key);
    if (link->IsNil()) return;

    HeapObject *obj = HeapObject::From(link);
    HashNode *node = Cast<HashNode>(obj);
    if (HeapObject::Equals(node->key(), key)) {
        SetHashNodeLink(key, node->next());
        return;
    }

    while (true) {
        RawObject *object = node->next();
        if (object->IsNil()) return;
        HashNode *next = Cast<HashNode>(From(object));
        if (HeapObject::Equals(next->key(), key)) {
            node->set_next(next->next());
            return;
        }
        node = next;
    }
}

void HashMap::SetWithoutUpdate(const RawObject *key, Element value) {
    RawObject *link = GetHashNodeLink(key), *start = link;

    while (!link->IsNil()) {
        HashNode *node = Cast<HashNode>(From(link));
        if (HeapObject::Equals(key, node->key())) {
            node->set_value(value);
            return;
        }
        link = node->next();
    }

    // Arrived here, indicating that there is no node with key,
    // to create it
    HashNode *node =
        HashNode::Create(const_cast<RawObject *>(key), value, start);
    SetHashNodeLink(key, node);
}

void HashMap::Update() {
    if (IsNeedExpand())
        Expand();
    else if (IsNeedShrink())
        Shrink();
}

void HashMap::Rehash(size_t capacity) {
    Array *old_buckets = buckets();
    Array *new_buckets = Array::Create(capacity);

    set_buckets(new_buckets);

    size_t length = old_buckets->length();
    for (size_t i = 0; i < length; ++i) {
        RawObject *link = old_buckets->Get(i);
        while (!link->IsNil()) {
            HashNode *node = Cast<HashNode>(From(link));
            link = node->next();

            RawObject *target_key = node->key();
            RawObject *target_link = GetHashNodeLink(target_key);
            node->set_next(target_link);
            SetHashNodeLink(target_key, node);
        }
    }
}

void HashMap::Expand() {
    size_t cap = capacity() * 2;
    Rehash(cap);
}

void HashMap::Shrink() {
    size_t cap = capacity() / 2;
    Rehash(cap);
}

bool HashMap::IsNeedExpand() {
    size_t thresold = load_factor() * capacity();

    return thresold <= length();
}

bool HashMap::IsNeedShrink() {
    size_t thresold = (1.0 - load_factor()) * capacity();
    return capacity() != 16 && thresold > length();
}

RawObject *HashMap::GetHashNodeLink(const RawObject *key) {
    uint32_t hash = HeapObject::HashCode(key) & capacity();

    return buckets()->Get(hash);
}

void HashMap::SetHashNodeLink(const RawObject *key, Element link) {
    size_t hash = HeapObject::HashCode(key) % capacity();

    buckets()->Set(hash, link);
}

void HashMap::ValidateKeyType(const RawObject *key) {
    if (!key->IsFixnum() ||
        !(key->IsObject() && HeapObject::From(key)->IsString())) {
        throw std::runtime_error("only support Fixnum & String");
    }
}

void HashMap::Children(const ForwardingCallback &cb) {
    Array *buckets = this->buckets();
    buckets = ForwardingObject<Array>(cb, buckets);
    set_buckets(buckets);
}

} // namespace object
} // namespace nrk
