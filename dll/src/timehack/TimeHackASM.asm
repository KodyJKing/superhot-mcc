INCLUDE <src/utils/ASMMacros.asmlib>

_DATA SEGMENT

_DATA ENDS

_TEXT SEGMENT


    extern entityUpdateHook_return: qword
    extern entityUpdateHook_end: qword
    extern entityUpdateHook_shouldUpdate: proc
    ;
    PUBLIC entityUpdateHook
    entityUpdateHook PROC

        ; Original:
            mov rax, [rsi+00F0h]
            test rax,rax

        pushState

            sub rsp, 20h
            mov rcx, r12
            call entityUpdateHook_shouldUpdate
            add rsp, 20h

            test rax, rax
            jnz __else_1
                popState
                jmp [entityUpdateHook_end]
            __else_1:

        popState

        jmp [entityUpdateHook_return]

    entityUpdateHook ENDP


_TEXT ENDS

END