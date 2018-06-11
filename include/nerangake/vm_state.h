#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <vector>

#include <nerangake/memory/root_object_holder_interface.h>
#include <nerangake/state/executor_interface.h>
#include <nerangake/vm_scene.h>

namespace nrk {

class VMState : public state::ExecutorInterface,
                public memory::RootObjectHolderInterface {
    VMState(const VMState &) = delete;
    VMState &operator=(const VMState &) = delete;

public:
    VMState(const uint8_t *codes, size_t size);
    ~VMState();
    VMState(VMState &&) = default;
    VMState &operator=(VMState &&) = default;

    virtual void Execute() override;
    virtual void AddClosure(Closure *closure) override;
    virtual void SetUserClosure(
        const std::string &str, const UserDefFunc func) override;
    virtual void AddInteger(Fixnum *fixnum) override;
    virtual void AddFloat(Float *f) override;
    virtual void AddString(String *string) override;

    virtual bool IsUserClosureExists(const std::string &str) const override;

    virtual void ProcessRootObject(const Callback &cb) override;

private:
    CallInfo *LastCallInfo(VMScene *scene);

    bool Disptach(VMScene *scene, const uint8_t *pc);
    void ExecuteGoto(VMScene *scene);
    void ExecuteNot(VMScene *scene);
    void ExecuteInc(VMScene *scene);
    void ExecuteDec(VMScene *scene);
    void ExecuteAdd(VMScene *scene);
    void ExecuteSub(VMScene *scene);
    void ExecuteMul(VMScene *scene);
    void ExecuteDiv(VMScene *scene);
    void ExecuteMod(VMScene *scene);
    void ExecutePow(VMScene *scene);
    void ExecuteGT(VMScene *scene);
    void ExecuteGE(VMScene *scene);
    void ExecuteLT(VMScene *scene);
    void ExecuteLE(VMScene *scene);
    void ExecuteEQ(VMScene *scene);
    void ExecuteNE(VMScene *scene);
    void ExecuteMoveS(VMScene *scene);
    void ExecuteMoveI(VMScene *scene);
    void ExecuteMoveF(VMScene *scene);
    void ExecuteMoveN(VMScene *scene);
    void ExecuteMove(VMScene *scene);
    void ExecuteLoad(VMScene *scene);
    void ExecuteStore(VMScene *scene);
    void ExecuteLoadGlobal(VMScene *scene);
    void ExecuteStoreGlobal(VMScene *scene);
    void ExecuteLoadCaptured(VMScene *scene);
    void ExecuteStoreCaptured(VMScene *scene);
    void ExecuteIndex(VMScene *scene);
    void ExecuteSetIndex(VMScene *scene);
    void ExecuteIf(VMScene *scene);
    void ExecuteBEQ(VMScene *scene);
    void ExecuteBNE(VMScene *scene);
    void ExecuteBGT(VMScene *scene);
    void ExecuteBLT(VMScene *scene);
    void ExecuteBGE(VMScene *scene);
    void ExecuteBLE(VMScene *scene);
    void ExecuteBZ(VMScene *scene);
    void ExecuteBNZ(VMScene *scene);
    void ExecutePush(VMScene *scene);
    void ExecutePushN(VMScene *scene);
    void ExecutePop(VMScene *scene);
    void ExecuteCall(VMScene *scene);
    void ExecuteTailCall(VMScene *scene);
    void ExecuteReturn(VMScene *scene);
    void ExecuteReturnVoid(VMScene *scene);
    void ExecuteNewHash(VMScene *scene);
    void ExecuteNewArray(VMScene *scene);
    void ExecuteNewClosure(VMScene *scene);
    void ExecuteNewUserClosure(VMScene *scene);

    const uint8_t *code_;
    size_t size_;
    std::vector<Closure *> closures_;
    std::vector<Prototype *> prototypes_;
    std::vector<Fixnum *> fixnums_;
    std::vector<Float *> floats_;
    std::vector<String *> strings_;

    std::vector<RawObject *> globals_;

    std::vector<UserClosure *> user_closures_;
    std::unordered_map<std::string, unsigned> user_closure_map_;

    // main scene of current virtual machine
    VMScene *main_;
    VMScene *current_scene_;
    std::list<VMScene *> scenes_;
};

} // namespace nrk
