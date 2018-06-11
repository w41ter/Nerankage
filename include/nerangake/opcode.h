#pragma once

namespace nrk {

/** memory layout of instruction
 * +----------------------------+
 * | OP(8) | A(8) | B(8) | C(8) |
 * +----------------------------+
 * | OP(8) | A(8) |     Bx(16)  |
 * +----------------------------+
 * | OP(8) |         Ax(24)     |
 * +----------------------------+
 *
 * NOTICE:
 *      Need to ensure that Bx, Ax byte order
 *  are litte-endian.
 **/
enum class OPCode {
    kGoto = 0, // PC += Ax

    // operator
    // single
    kNot, // A = !B
    kInc, // A = B + 1
    kDec, // A = B - 1

    // binary
    kAdd, // A = B + C
    kSub, // A = B - C
    kMul, // A = B * C
    kDiv, // A = B / C
    kMod, // A = B % C
    kPow, // A = B ^ C

    // relop
    kGT, // A = B > C
    kGE, // A = B >= C
    kLT, // A = B < C
    kLE, // A = B <= C
    kEQ, // A = B == C
    kNE, // A = B != C

    // move
    kMoveS, // A = String(Bx)
    kMoveI, // A = Integer(Bx)
    kMoveF, // A = Float(Bx)
    kMoveN, // A = Nil
    kMove,  // A = B

    // memory
    kLoad,          // A = stack[B]
    kStore,         // stack[A] = B
    kLoadGlobal,    // A = global[Bx]
    kStoreGlobal,   // global[Bx] = A
    kLoadCaptured,  // A = captureds[Bx]
    kStoreCaptured, // captureds[Bx] = A
    kIndex,         // A = B[C]
    kSetIndex,      // A[B] = C

    // condition jmp
    kIf,  // if A PC += Bx;
    kBEQ, // if (A == B) PC += C;
    kBNE, // if (A != B) PC += C;
    kBGT, // if (A > B) PC += C;
    kBLT, // if (A < B) PC += C;
    kBGE, // if (A >= B) PC += C;
    kBLE, // if (A <= B) PC += C;
    kBZ,  // if (A == Nil) PC += C;
    kBNZ, // if (A != Nil) PC += C;

    // call
    kPush,  // stack.push(A)
    kPushN, // stack.push(A) B times
    kPop,   // stack.pop A
    kCall,  // [A...B) = call stack[top] C
    kTailCall,
    kReturn,     // return [A...B)
    kReturnVoid, // return

    kNewHash,     // A = Hash
    kNewArray,    // A = Vector
    kNewClosure,  // A = Prototype[Bx]
    kUserClosure, // A = UserClosure[Bx]
    kHalt,        // stop
};

} // namespace nrk
