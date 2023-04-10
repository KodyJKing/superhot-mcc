INCLUDE <src/utils/ASMMacros.asmlib>

_DATA SEGMENT

float_00 REAL4 0.0f
float_15 REAL4 15.0f

_DATA ENDS

_TEXT SEGMENT

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    extern copyViewProjMatrix: proc
    extern setViewProjMatrixHook_return: qword
    
    PUBLIC setViewProjMatrixHook
    setViewProjMatrixHook PROC

        ; halo1.dll+22FCBC - CC                    - int 3 
        ; halo1.dll+22FCBD - CC                    - int 3 
        ; halo1.dll+22FCBE - CC                    - int 3 
        ; halo1.dll+22FCBF - CC                    - int 3 
        ; === BEGIN HOOK === (11 bytes)
        ; halo1.dll+22FCC0 - 48 83 EC 18           - sub rsp,18
        ; halo1.dll+22FCC4 - 48 8B 82 D8000000     - mov rax,[rdx+000000D8]
        ; === END HOOK =====
        ; halo1.dll+22FCCB - F3 0F10 5C 24 40      - movss xmm3,[rsp+40]
        ; halo1.dll+22FCD1 - 0F28 CB               - movaps xmm1,xmm3
        ; halo1.dll+22FCD4 - 0F28 D3               - movaps xmm2,xmm3
        ; halo1.dll+22FCD7 - F3 0F5C 4C 24 48      - subss xmm1,[rsp+48]
        ; halo1.dll+22FCDD - F3 0F59 5C 24 48      - mulss xmm3,[rsp+48]

        sub rsp, 18h
        mov rax, [rdx+00D8h]

        cmp rbx, 02
        je dontCopy

        pushState
            sub rsp, 28h
            mov rcx, r9 ; (move param 4 into param 1)
            call copyViewProjMatrix
            add rsp, 28h
        popState

        dontCopy:

        jmp [setViewProjMatrixHook_return]

    setViewProjMatrixHook ENDP

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

_TEXT ENDS

END