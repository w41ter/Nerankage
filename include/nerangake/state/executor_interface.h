#pragma once

#include <string>

#include <nerangake/object_user.h>

namespace nrk {
namespace state {

class ExecutorInterface : public ObjectUser {
public:
    virtual ~ExecutorInterface() {}

    virtual void Execute() = 0;
    virtual void AddClosure(Closure *closure) = 0;
    virtual void SetUserClosure(
        const std::string &str, const UserDefFunc func) = 0;
    virtual void AddInteger(Fixnum *fixnum) = 0;
    virtual void AddFloat(Float *f) = 0;
    virtual void AddString(String *string) = 0;

    virtual bool IsUserClosureExists(const std::string &str) const = 0;

protected:
    ExecutorInterface() = default;
};

} // namespace state
} // namespace nrk
