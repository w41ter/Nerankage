#include <nerangake/vm_state.h>

#include <nerangake/context.h>
#include <nerangake/instruction.h>
#include <nerangake/opcode.h>

namespace nrk {

VMState::VMState(const uint8_t *codes, size_t size)
    : code_(codes), size_(size) {
    Context::RegisterRootObjectHolder(this);
    main_ = new VMScene();
    current_scene_ = main_;
    scenes_.push_back(main_);
}

VMState::~VMState() {
    Context::CancelledRootObjectHolder(this);
    for (VMScene *scene : scenes_) delete scene;
}

void VMState::Execute() {
    bool halt = false;
    while (!halt) {
        CallInfo *ci = current_scene_->top();

        if (ci->is_light_func()) {
            const UserClosure *callee = ci->user_callee();
            const auto func = callee->callable();
            func(current_scene_, ci->begin(), ci->end(), ci->num_of_params());
        } else {
            halt = Disptach(current_scene_, ci->saved_pc());
        }
    }
}

bool VMState::Disptach(VMScene *scene, const uint8_t *pc) {
    assert(pc && "nullptr exception");
    switch (Instruction::OP(pc)) {
        case OPCode::kGoto:
            ExecuteGoto(scene);
            break;
        case OPCode::kNot:
            ExecuteNot(scene);
            break;
        case OPCode::kInc:
            ExecuteInc(scene);
            break;
        case OPCode::kDec:
            ExecuteDec(scene);
            break;
        case OPCode::kAdd:
            ExecuteAdd(scene);
            break;
        case OPCode::kSub:
            ExecuteSub(scene);
            break;
        case OPCode::kMul:
            ExecuteMul(scene);
            break;
        case OPCode::kDiv:
            ExecuteDiv(scene);
            break;
        case OPCode::kMod:
            ExecuteMod(scene);
            break;
        case OPCode::kPow:
            ExecutePow(scene);
            break;
        case OPCode::kGT:
            ExecuteGT(scene);
            break;
        case OPCode::kGE:
            ExecuteGE(scene);
            break;
        case OPCode::kLT:
            ExecuteLT(scene);
            break;
        case OPCode::kLE:
            ExecuteLE(scene);
            break;
        case OPCode::kEQ:
            ExecuteEQ(scene);
            break;
        case OPCode::kNE:
            ExecuteNE(scene);
            break;
        case OPCode::kMoveS:
            ExecuteMoveS(scene);
            break;
        case OPCode::kMoveI:
            ExecuteMoveI(scene);
            break;
        case OPCode::kMoveF:
            ExecuteMoveF(scene);
            break;
        case OPCode::kMoveN:
            ExecuteMoveN(scene);
            break;
        case OPCode::kMove:
            ExecuteMove(scene);
            break;
        case OPCode::kLoad:
            ExecuteLoad(scene);
            break;
        case OPCode::kStore:
            ExecuteStore(scene);
            break;
        case OPCode::kLoadGlobal:
            ExecuteLoad(scene);
            break;
        case OPCode::kStoreGlobal:
            ExecuteStore(scene);
            break;
        case OPCode::kLoadCaptured:
            ExecuteLoad(scene);
            break;
        case OPCode::kStoreCaptured:
            ExecuteStore(scene);
            break;
        case OPCode::kIndex:
            ExecuteIndex(scene);
            break;
        case OPCode::kSetIndex:
            ExecuteSetIndex(scene);
            break;
        case OPCode::kIf:
            ExecuteIf(scene);
            break;
        case OPCode::kBEQ:
            ExecuteBEQ(scene);
            break;
        case OPCode::kBNE:
            ExecuteBNE(scene);
            break;
        case OPCode::kBGT:
            ExecuteBGT(scene);
            break;
        case OPCode::kBGE:
            ExecuteBGE(scene);
            break;
        case OPCode::kBLT:
            ExecuteBLT(scene);
            break;
        case OPCode::kBLE:
            ExecuteBLE(scene);
            break;
        case OPCode::kBZ:
            ExecuteBZ(scene);
            break;
        case OPCode::kBNZ:
            ExecuteBNZ(scene);
            break;
        case OPCode::kPush:
            ExecutePush(scene);
            break;
        case OPCode::kPushN:
            ExecutePushN(scene);
            break;
        case OPCode::kPop:
            ExecutePop(scene);
            break;
        case OPCode::kCall:
            ExecuteCall(scene);
            break;
        case OPCode::kTailCall:
            ExecuteTailCall(scene);
            break;
        case OPCode::kReturn:
            ExecuteReturn(scene);
            break;
        case OPCode::kReturnVoid:
            ExecuteReturnVoid(scene);
            break;
        case OPCode::kNewHash:
            ExecuteNewHash(scene);
            break;
        case OPCode::kNewArray:
            ExecuteNewArray(scene);
            break;
        case OPCode::kNewClosure:
            ExecuteNewClosure(scene);
            break;
        case OPCode::kUserClosure:
            ExecuteNewUserClosure(scene);
            break;
        case OPCode::kHalt:
            return true;
    }
    return false;
}

VMState::CallInfo *VMState::LastCallInfo(VMScene *scene) {
    CallInfo *current = scene->top();
    assert(current == nullptr);

    CallInfo *result = current->parent();
    while (result != nullptr && result->is_light_func()) {
        result = result->parent();
    }
    if (result == nullptr) {
        throw std::runtime_error("could not found last non-light func.");
    }
    return result;
}

void VMState::ExecuteGoto(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    int32_t offset = static_cast<int32_t>(Instruction::Ax(ci->saved_pc()));

    scene->top()->SetNextPC(offset);
}

void VMState::ExecuteNot(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *b = ci->reg(B);
    RawObject *a = RawObject::Not(b);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteInc(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *b = ci->reg(B);
    RawObject *a = RawObject::Add(b, Fixnum::Create(1));
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteDec(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *b = ci->reg(B);
    RawObject *a = RawObject::Sub(b, Fixnum::Create(1));
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteAdd(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::Add(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteSub(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::Sub(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteMul(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::Mul(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteDiv(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::Div(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteMod(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::Mod(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecutePow(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::Pow(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteGT(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::GT(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteGE(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::GE(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteLT(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::LT(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteLE(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::LE(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteEQ(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::EQ(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteNE(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject *a = RawObject::NE(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteMoveS(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(strings_.size() < Bx && "index out of string poll size");

    String *string = strings_[Bx];
    ci->set_reg(A, string);
    ci->SetNextPC(1);
}

void VMState::ExecuteMoveI(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(fixnums_.size() < Bx && "index out of integer poll size");

    Fixnum *fixnum = fixnums_[Bx];
    ci->set_reg(A, fixnum);
    ci->SetNextPC(1);
}

void VMState::ExecuteMoveF(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(floats_.size() < Bx && "index out of float poll size");

    Float *f = floats_[Bx];
    ci->set_reg(A, f);
    ci->SetNextPC(1);
}

void VMState::ExecuteMoveN(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);

    ci->set_reg(A, Nil::Create());
    ci->SetNextPC(1);
}

void VMState::ExecuteMove(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *b = ci->reg(B);
    ci->set_reg(A, b);
    ci->SetNextPC(1);
}

void VMState::ExecuteLoad(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *a = scene->stack()->Get(B);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteIndex(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *c = ci->reg(C);
    RawObject *b = ci->reg(B);
    RawObject *a = RawObject::Index(b, c);
    ci->set_reg(A, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteStore(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *b = ci->reg(B);
    scene->stack()->Set(A, b);

    ci->SetNextPC(1);
}

void VMState::ExecuteLoadGlobal(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(Bx < globals_.size() && "out of globals range");

    RawObject *b = globals_[Bx];
    ci->set_reg(A, b);
    ci->SetNextPC(1);
}

void VMState::ExecuteStoreGlobal(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(Bx < globals_.size() && "out of globals range");

    globals_[Bx] = ci->reg(A);
    ci->SetNextPC(1);
}

void VMState::ExecuteLoadCaptured(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(Bx < globals_.size() && "out of globals range");

    RawObject *b = ci->captured(Bx);
    ci->set_reg(A, b);
    ci->SetNextPC(1);
}

void VMState::ExecuteStoreCaptured(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(Bx < globals_.size() && "out of globals range");

    RawObject *a = ci->reg(A);
    ci->set_captured(Bx, a);
    ci->SetNextPC(1);
}

void VMState::ExecuteSetIndex(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = ci->reg(C);
    RawObject::SetIndex(a, b, c);

    ci->SetNextPC(1);
}

void VMState::ExecuteIf(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    RawObject *a = ci->reg(A);
    ci->SetNextPC(RawObject::True(a) ? Bx : 1);
}

void VMState::ExecuteBEQ(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = RawObject::EQ(a, b);

    ci->SetNextPC(RawObject::True(c) ? C : 1);
}

void VMState::ExecuteBNE(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = RawObject::NE(a, b);

    ci->SetNextPC(RawObject::True(c) ? C : 1);
}

void VMState::ExecuteBGT(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = RawObject::GT(a, b);

    ci->SetNextPC(RawObject::True(c) ? C : 1);
}

void VMState::ExecuteBLT(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = RawObject::LT(a, b);

    ci->SetNextPC(RawObject::True(c) ? C : 1);
}

void VMState::ExecuteBGE(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = RawObject::GE(a, b);

    ci->SetNextPC(RawObject::True(c) ? C : 1);
}

void VMState::ExecuteBLE(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *a = ci->reg(A);
    RawObject *b = ci->reg(B);
    RawObject *c = RawObject::LE(a, b);

    ci->SetNextPC(RawObject::True(c) ? C : 1);
}

void VMState::ExecuteBZ(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *a = ci->reg(A);

    ci->SetNextPC(RawObject::NZ(a) ? 1 : B);
}

void VMState::ExecuteBNZ(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *a = ci->reg(A);

    ci->SetNextPC(RawObject::NZ(a) ? B : 1);
}

void VMState::ExecutePush(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);

    RawObject *a = ci->reg(A);
    scene->stack()->Push(a);

    ci->SetNextPC(1);
}

void VMState::ExecutePushN(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);

    RawObject *a = ci->reg(A);
    scene->stack()->PushN(a, B);

    ci->SetNextPC(1);
}

void VMState::ExecutePop(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);

    scene->stack()->Pop(A);

    ci->SetNextPC(1);
}

void VMState::ExecuteCall(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t C = Instruction::C(pc);

    RawObject *raw = scene->stack()->top();
    CallInfo *next;
    if (raw->IsObject()) {
        HeapObject *obj = HeapObject::From(raw);
        if (obj->IsClosure()) {
            Closure *closure = HeapObject::Cast<Closure>(obj);
            next = CallInfo::Create(closure, A, B, C);
        } else if (obj->IsUserClosure()) {
            UserClosure *closure = HeapObject::Cast<UserClosure>(obj);
            next = CallInfo::Create(closure, A, B, C);
        } else {
            goto ERROR;
        }
    } else {
        goto ERROR;
    }

    scene->Push(next);

    ci->SetNextPC(1);
    return;

ERROR:
    throw std::runtime_error("`object` not callable");
}

void VMState::ExecuteTailCall(VMScene *scene) {
    assert(scene && "nullptr exception");

    throw std::runtime_error("not support");
}

void VMState::ExecuteReturn(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint8_t B = Instruction::B(pc);
    uint8_t begin = ci->begin();
    uint8_t end = ci->end();

    CallInfo *last = LastCallInfo(scene);
    uint8_t need_length = end - begin;
    uint8_t actual_length = B - A;
    uint8_t first = need_length > actual_length ? actual_length : need_length;
    for (uint8_t i = 0; i < first; ++i) {
        last->set_reg(begin + i, ci->reg(A + i));
    }
    for (uint8_t i = first; i < need_length; ++i) {
        last->set_reg(begin + i, Nil::Create());
    }

    scene->Pop();
}

void VMState::ExecuteReturnVoid(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    
    uint8_t begin = ci->begin();
    uint8_t end = ci->end();

    CallInfo *last = LastCallInfo(scene);
    uint8_t need_length = end - begin;
    for (uint8_t i = 0; i < need_length; ++i) {
        last->set_reg(begin + i, Nil::Create());
    }

    scene->Pop();
}

void VMState::ExecuteNewHash(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    ci->set_reg(A, HashMap::Create());

    ci->SetNextPC(1);
}

void VMState::ExecuteNewArray(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    ci->set_reg(A, Vector::Create());

    ci->SetNextPC(1);
}

void VMState::ExecuteNewClosure(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(Bx < prototypes_.size() && "out of prototypes range");

    const Prototype *proto = prototypes_[Bx];
    uint16_t num_of_captureds = proto->num_of_captureds();
    CallInfo *last_ci = LastCallInfo(scene);
    Closure *closure = Closure::Create(proto, num_of_captureds);

    for (uint16_t i = 0; i < num_of_captureds; ++i) {
        const auto &captured = proto->captured(i);
        RawObject *obj;

        if (!captured.instack) {
            // If it is not on the stack, then access the top callinfo's
            // captured list directly and take out the value.
            obj = last_ci->captured(captured.index);
        } else {
            // On the stack, then access the corresponding stack location.
            obj = scene->stack()->Get(captured.index);
        }
        closure->set_captured(i, obj);
    }

    ci->set_reg(A, closure);
    ci->SetNextPC(1);
}

void VMState::ExecuteNewUserClosure(VMScene *scene) {
    assert(scene && "nullptr exception");

    CallInfo *ci = scene->top();
    const uint8_t *pc = ci->saved_pc();

    uint8_t A = Instruction::A(pc);
    uint16_t Bx = Instruction::Bx(pc);

    assert(Bx < user_closures_.size() && "out of user_closure size");

    ci->set_reg(A, user_closures_[Bx]);
    ci->SetNextPC(1);
}

void VMState::AddClosure(Closure *closure) {
    assert(closure && "nullptr exception");

    closures_.push_back(closure);
}

void VMState::SetUserClosure(const std::string &str, const UserDefFunc func) {
    unsigned idx;
    if (user_closure_map_.count(str))
        idx = user_closure_map_[str];
    else {
        idx = user_closure_map_.size();
        user_closure_map_[str] = idx;
    }
    UserClosure *closure = UserClosure::Create(func);
    user_closures_.push_back(closure);
}

void VMState::AddInteger(Fixnum *fixnum) {
    assert(fixnum && "nullptr exception");

    fixnums_.push_back(fixnum);
}

void VMState::AddFloat(Float *f) {
    assert(f && "nullptr exception");

    floats_.push_back(f);
}

void VMState::AddString(String *string) {
    assert(string && "nullptr exception");

    strings_.push_back(string);
}

bool VMState::IsUserClosureExists(const std::string &str) const {
    return user_closure_map_.count(str);
}

void VMState::ProcessRootObject(const Callback &cb) {
    for (auto &closure : closures_)
        closure = ForwardingObject<Closure>(cb, closure);
    for (auto &proto : prototypes_)
        proto = ForwardingObject<Prototype>(cb, proto);
    for (auto &f : floats_) f = ForwardingObject<Float>(cb, f);
    for (auto &str : strings_) str = ForwardingObject<String>(cb, str);
    for (auto &global : globals_) {
        if (global->IsObject()) {
            HeapObject *obj = HeapObject::From(global);
            global = ForwardingObject<HeapObject>(cb, obj);
        }
    }
    for (auto &fn : user_closures_) fn = ForwardingObject<UserClosure>(cb, fn);
}

}  // namespace vm
}  // namespace script
