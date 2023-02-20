INCLUDE <src/utils/ASMMacros.asmlib>

_DATA SEGMENT

_DATA ENDS

_TEXT SEGMENT

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    extern preEntityUpdate: proc
    extern preEntityUpdate_doUpdate: byte
    extern preEntityUpdateHook_return: qword
    extern preEntityUpdateHook_end: qword
    
    PUBLIC preEntityUpdateHook
    preEntityUpdateHook PROC

        ; Original:
            mov rax, [rsi+00F0h]
            test rax,rax

        pushState

            sub rsp, 20h
            mov rcx, r12
            call preEntityUpdate
            add rsp, 20h

            cmp byte ptr [preEntityUpdate_doUpdate], 1
            je __doUpdate
                popState
                jmp [preEntityUpdateHook_end]
            __doUpdate:

        popState

        jmp [preEntityUpdateHook_return]

    preEntityUpdateHook ENDP

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    extern postEntityUpdate: proc
    extern postEntityUpdateHook_return: qword
    extern postEntityUpdateHook_jmp: qword
    
    PUBLIC postEntityUpdateHook
    postEntityUpdateHook PROC
    
        ; Original:
            cmp dword ptr [rbp+7Ch], -01

        pushState
        
            sub rsp, 20h
            mov rcx, r12
            call postEntityUpdate
            add rsp, 20h

        popState

        ; Original:
            jne __dontJump1
                jmp [postEntityUpdateHook_jmp]
            __dontJump1:
        
        jmp [postEntityUpdateHook_return]

    postEntityUpdateHook ENDP

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_TEXT ENDS

END