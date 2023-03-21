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

        ; halo1.dll+B89892 - 48 0FBF 00            - movsx rax,word ptr [rax]
        ; halo1.dll+B89896 - 48 8D 35 A382E400     - lea rsi,[halo1.dll+19D1B40]
        ; halo1.dll+B8989D - 0FB7 FB               - movzx edi,bx
        ; halo1.dll+B898A0 - 48 8B 34 C6           - mov rsi,[rsi+rax*8]
        ; === BEGIN HOOK === (10 bytes)
        ; halo1.dll+B898A4 - 48 8B 86 F0000000     - mov rax,[rsi+000000F0]
        ; halo1.dll+B898AB - 48 85 C0              - test rax,rax
        ; === END HOOK =====
        ; halo1.dll+B898AE - 74 22                 - je halo1.dll+B898D2
        ; halo1.dll+B898B0 - 48 8B 50 58           - mov rdx,[rax+58]
        ; halo1.dll+B898B4 - 48 85 D2              - test rdx,rdx
        ; halo1.dll+B898B7 - 74 05                 - je halo1.dll+B898BE
        ; halo1.dll+B898B9 - 41 8B CC              - mov ecx,r12d
        ; halo1.dll+B898BC - FF D2                 - call rdx

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
    
        ; halo1.dll+B89A25 - 66 FF C7              - inc di
        ; halo1.dll+B89A28 - 0FBF C7               - movsx eax,di
        ; halo1.dll+B89A2B - 41 3B 87 40010000     - cmp eax,[r15+00000140]
        ; halo1.dll+B89A32 - 7C 8D                 - jl halo1.dll+B899C1
        ; halo1.dll+B89A34 - 33 DB                 - xor ebx,ebx
        ; halo1.dll+B89A36 - 4C 8B 7C 24 20        - mov r15,[rsp+20]
        ; === BEGIN HOOK === (5 bytes)
        ; halo1.dll+B89A3B - 48 8B 44 24 30        - mov rax,[rsp+30]
        ; === END HOOK =====
        ; halo1.dll+B89A40 - 48 8B BC 24 C0000000  - mov rdi,[rsp+000000C0]
        ; halo1.dll+B89A48 - 48 8B B4 24 B8000000  - mov rsi,[rsp+000000B8]
        ; halo1.dll+B89A50 - 8B 08                 - mov ecx,[rax]
        ; halo1.dll+B89A52 - 83 F9 FF              - cmp ecx,-01
        ; halo1.dll+B89A55 - 74 05                 - je halo1.dll+B89A5C

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
        
        ; halo1.dll+BDEF7B - F3 41 0F10 88 FC010000  - movss xmm1,[r8+000001FC]
        ; halo1.dll+BDEF84 - EB 0A                 - jmp halo1.dll+BDEF90
        ; halo1.dll+BDEF86 - F3 43 0F10 8C 01 38020000  - movss xmm1,[r9+r8+00000238]
        ; ==== BEGIN HOOK ==== (22 bytes)
        ; halo1.dll+BDEF90 - F3 0F10 52 08         - movss xmm2,[rdx+08]
        ; halo1.dll+BDEF95 - 0F57 DB               - xorps xmm3,xmm3
        ; halo1.dll+BDEF98 - F3 0F5C 52 04         - subss xmm2,[rdx+04]
        ; halo1.dll+BDEF9D - F3 0F59 D1            - mulss xmm2,xmm1
        ; halo1.dll+BDEFA1 - F3 0F58 52 04         - addss xmm2,[rdx+04]
        ; ==== END HOOK ======
        ; halo1.dll+BDEFA6 - 0F2F 15 875AEA00      - comiss xmm2,[halo1.dll+1A84A34]
        ; halo1.dll+BDEFAD - 76 0E                 - jna halo1.dll+BDEFBD
        ; halo1.dll+BDEFAF - F3 0F10 0D 0599D600   - movss xmm1,[halo1.dll+19488BC]

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

    extern damageScaleForEntity: proc
    extern damageHealthHook_return: qword
    
    PUBLIC damageHealthHook
    damageHealthHook PROC

        ; halo1.dll+C1907E - 0FB6 AC 24 20010000   - movzx ebp,byte ptr [rsp+00000120]
        ; halo1.dll+C19086 - 40 80 FD 01           - cmp bpl,01
        ; halo1.dll+C1908A - 0F85 DA000000         - jne halo1.dll+C1916A
        ; === BEGIN HOOK === (12 bytes)
        ; halo1.dll+C19090 - F3 0F10 83 9C000000   - movss xmm0,[rbx+0000009C]
        ; halo1.dll+C19098 - F3 0F5C C6            - subss xmm0,xmm6
        ; === END HOOK =====
        ; halo1.dll+C1909C - F3 0F11 83 9C000000   - movss [rbx+0000009C],xmm0

        pushState
            sub rsp, 20h
            mov rcx, r14
            call damageScaleForEntity
            add rsp, 20h

            mulss xmm6, xmm0
        popState

        movss xmm0, dword ptr [rbx+09Ch]
        subss xmm0, xmm6
        
        jmp [damageHealthHook_return]

    damageHealthHook ENDP

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    extern damageShieldHook_return: qword
    
    PUBLIC damageShieldHook
    damageShieldHook PROC

        ; halo1.dll+C197C5 - B8 00080000           - mov eax,00000800
        ; halo1.dll+C197CA - 66 44 85 C8           - test ax,r9w
        ; halo1.dll+C197CE - 75 08                 - jne halo1.dll+C197D8
        ; === BEGIN HOOK === (8 bytes)
        ; halo1.dll+C197D0 - F3 0F5C CA            - subss xmm1,xmm2
        ; halo1.dll+C197D4 - F3 0F11 0F            - movss [rdi],xmm1
        ; === END HOOK =====
        ; halo1.dll+C197D8 - 41 F6 C1 02           - test r9l,02
        ; halo1.dll+C197DC - 75 35                 - jne halo1.dll+C19813
        ; halo1.dll+C197DE - F3 0F10 83 84010000   - movss xmm0,[rbx+00000184]
        ; halo1.dll+C197E6 - 0F2F C1               - comiss xmm0,xmm

        ; Get entity handle from stack to pass to damageScaleForEntity
        push rcx
            mov ecx, dword ptr [rsp + 00E0h]
            pushState
                sub rsp, 20h
                call damageScaleForEntity
                add rsp, 20h

                mulss xmm2, xmm0
            popState
        pop rcx

        subss xmm1, xmm2
        movss dword ptr [rdi], xmm1
        
        jmp [damageShieldHook_return]

    damageShieldHook ENDP

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_TEXT ENDS

END