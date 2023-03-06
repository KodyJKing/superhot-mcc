INCLUDE <src/utils/ASMMacros.asmlib>

_DATA SEGMENT

float_00 REAL4 0.0f
float_15 REAL4 15.0f

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
    
    PUBLIC postEntityUpdateHook
    postEntityUpdateHook PROC
    
        ; Original:
            mov rax,[rsp+30h]

        pushState
        
            sub rsp, 20h
            mov rcx, r12
            call postEntityUpdate
            add rsp, 20h

        popState
        
        jmp [postEntityUpdateHook_return]

    postEntityUpdateHook ENDP

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    extern fireRateFixHook_return: qword

    PUBLIC fireRateFixHook
    fireRateFixHook PROC
        
        ; Original
            ; F3 43 0F10 8C 01 38020000  - movss xmm1,[r9+r8+00000238]
            ; ==== hook ==== (22 bytes)
            ; halo1.dll+BDEF90:
            ; F3 0F10 52 08              - movss xmm2,[rdx+08]
            ; 0F57 DB                    - xorps xmm3,xmm3
            ; F3 0F5C 52 04              - subss xmm2,[rdx+04]
            ; F3 0F59 D1                 - mulss xmm2,xmm1
            ; F3 0F58 52 04              - addss xmm2,[rdx+04]
            ; ==============
            ; 0F2F 15 875AEA00           - comiss xmm2,[7FFCE8904A34]
            ; 76 0E                      - jna 7FFCE7A5EFBD
            ; F3 0F10 0D 0599D600        - movss xmm1,[7FFCE87C88BC]

        push rax

            ; If min ROF isn't 0, proceed normally.
            mov eax, dword ptr [rdx+04h]
            cmp eax, dword ptr [float_00]
            jne fireRateFixHook_normal

            ; or if max ROF isn't 0, proceed normally.
            mov eax, dword ptr [rdx+08h]
            cmp eax, dword ptr [float_00]
            jne fireRateFixHook_normal

            fireRateFixHook_limit:
                movss xmm2, dword ptr [float_15]
                xorps xmm3,xmm3
                subss xmm2, dword ptr [float_15]
                mulss xmm2,xmm1
                addss xmm2, dword ptr [float_15]
                jmp fireRateFixHook_end

            fireRateFixHook_normal:
                movss xmm2, dword ptr [rdx+08h]
                xorps xmm3,xmm3
                subss xmm2, dword ptr [rdx+04h]
                mulss xmm2,xmm1
                addss xmm2, dword ptr [rdx+04h]

            fireRateFixHook_end:

        pop rax

        jmp [fireRateFixHook_return]

    fireRateFixHook ENDP
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_TEXT ENDS

END