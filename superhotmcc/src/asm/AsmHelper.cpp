#include "asmjit/x86.h"
#include "AsmHelper.hpp"

namespace AsmHelper {

    using namespace asmjit::x86;

    void push(asmjit::x86::Assembler& a) {
       // Save rsp to rbp
        a.push(rbp);
        a.lea(rbp, qword_ptr(rsp, 8));
        // Save rflags
        a.pushfq();
        // Save volatile registers.
        a.push(rax);
        a.push(rcx);
        a.push(rdx);
        a.push(r8);
        a.push(r9);
        a.push(r10);
        a.push(r11);
        // Align stack to 16 bytes
        a.push(rbx);
        a.mov(rbx, rsp);
        a.and_(rsp, 0xfffffffffffffff0);
        // Save volatile xmm registers.
        a.sub(rsp, 0x60);
        a.movdqa(ptr(rsp), xmm0);
        a.movdqa(ptr(rsp, 0x10), xmm1);
        a.movdqa(ptr(rsp, 0x20), xmm2);
        a.movdqa(ptr(rsp, 0x30), xmm3);
        a.movdqa(ptr(rsp, 0x40), xmm4);
        a.movdqa(ptr(rsp, 0x50), xmm5);
    }
    
    void pop(asmjit::x86::Assembler& a) {
        // Restore volatile xmm registers.
        a.movdqa(xmm0, ptr(rsp));
        a.movdqa(xmm1, ptr(rsp, 0x10));
        a.movdqa(xmm2, ptr(rsp, 0x20));
        a.movdqa(xmm3, ptr(rsp, 0x30));
        a.movdqa(xmm4, ptr(rsp, 0x40));
        a.movdqa(xmm5, ptr(rsp, 0x50));
        a.add(rsp, 0x60);
        // Restore stack pointer.
        a.mov(rsp, rbx);
        a.pop(rbx);
        // Restore volatile registers
        a.pop(r11);
        a.pop(r10);
        a.pop(r9);
        a.pop(r8);
        a.pop(rdx);
        a.pop(rcx);
        a.pop(rax);
        // Restore rflags
        a.popfq();
        // Restore rbp
        a.pop(rbp);
    }
}