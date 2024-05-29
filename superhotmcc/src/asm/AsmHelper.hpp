#pragma once

#include "asmjit/x86.h"

#define ARG_0_i(a, arg) a.mov(asmjit::x86::rcx, arg)
#define ARG_1_i(a, arg) a.mov(asmjit::x86::rdx, arg)
#define ARG_2_i(a, arg) a.mov(asmjit::x86::r8, arg)
#define ARG_3_i(a, arg) a.mov(asmjit::x86::r9, arg)

#define ARG_0_f(a, arg) a.movss(asmjit::x86::xmm0, arg)
#define ARG_1_f(a, arg) a.movss(asmjit::x86::xmm1, arg)
#define ARG_2_f(a, arg) a.movss(asmjit::x86::xmm2, arg)
#define ARG_3_f(a, arg) a.movss(asmjit::x86::xmm3, arg)

#define START_CALL(a) a.sub(asmjit::x86::rsp, 0x28);
#define END_CALL(a, func) a.call(func); \
                          a.add(asmjit::x86::rsp, 0x28);

#define CALL_0(a, func) a.sub(asmjit::x86::rsp, 0x28); \
                        a.call(func); \
                        a.add(asmjit::x86::rsp, 0x28);
#define CALL_1(a, func, arg) a.sub(asmjit::x86::rsp, 0x28); \
                             a.mov(asmjit::x86::rcx, arg); \
                             a.call(func); \
                             a.add(asmjit::x86::rsp, 0x28);
#define CALL_2(a, func, arg1, arg2) a.sub(asmjit::x86::rsp, 0x28); \
                                    a.mov(asmjit::x86::rcx, arg1); \
                                    a.mov(asmjit::x86::rdx, arg2); \
                                    a.call(func); \
                                    a.add(asmjit::x86::rsp, 0x28);
#define CALL_3(a, func, arg1, arg2, arg3) a.sub(asmjit::x86::rsp, 0x28); \
                                          a.mov(asmjit::x86::rcx, arg1); \
                                          a.mov(asmjit::x86::rdx, arg2); \
                                          a.mov(asmjit::x86::r8, arg3); \
                                          a.call(func); \
                                          a.add(asmjit::x86::rsp, 0x28);
#define CALL_4(a, func, arg1, arg2, arg3, arg4) a.sub(asmjit::x86::rsp, 0x28); \
                                                a.mov(asmjit::x86::rcx, arg1); \
                                                a.mov(asmjit::x86::rdx, arg2); \
                                                a.mov(asmjit::x86::r8, arg3); \
                                                a.mov(asmjit::x86::r9, arg4); \
                                                a.call(func); \
                                                a.add(asmjit::x86::rsp, 0x28);

namespace AsmHelper {
    void push(asmjit::x86::Assembler& a);
    void pop(asmjit::x86::Assembler& a);
    asmjit::x86::Mem rbpFromStack();
    asmjit::x86::Mem rbxFromStack();
}