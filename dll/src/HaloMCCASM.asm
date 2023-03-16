; INCLUDE <src/utils/ASMMacros.asmlib>

; _DATA SEGMENT
; _DATA ENDS

; _TEXT SEGMENT

;     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;     extern loadGameDLL: proc
;     extern loadGameDLLHook_return: qword
    
;     PUBLIC loadGameDLLHook
;     loadGameDLLHook PROC

;         ; mcc-win64-shipping.exe+42F459 - 48 0F43 4D 0F         - cmovae rcx,[rbp+0F]
;         ; mcc-win64-shipping.exe+42F45E - 33 D2                 - xor edx,edx
;         ; mcc-win64-shipping.exe+42F460 - 41 B8 00100000        - mov r8d,00001000
;         ; === BEGIN HOOK === (6 bytes)
;         ; mcc-win64-shipping.exe+42F466 - FF 15 94174502        - call qword ptr [mcc-win64-shipping.exe+2880C00] { ->KERNEL32.LoadLibraryExW }
;         ; === END HOOK =====
;         ; mcc-win64-shipping.exe+42F46C - 49 89 46 08           - mov [r14+08],rax
;         ; mcc-win64-shipping.exe+42F470 - 48 85 C0              - test rax,rax
;         ; mcc-win64-shipping.exe+42F473 - 75 0C                 - jne mcc-win64-shipping.exe+42F481
;         ; mcc-win64-shipping.exe+42F475 - FF 15 BD144502        - call qword ptr [mcc-win64-shipping.exe+2880938]

;         pushState
;         sub rsp, 20h
;         call loadGameDLL
;         add rsp, 20h
;         popState
        
;         jmp [loadGameDLLHook_return]

;     loadGameDLLHook ENDP

;     ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; _TEXT ENDS

END