; _DATA SEGMENT

; _DATA ENDS

; _TEXT SEGMENT

; pushState MACRO
;     pushfq
;     push rax
;     push rbx
;     push rcx
;     push rdx
;     push rsi
;     push rdi
;     push rbp
;     push rsp
;     push r8
;     push r9
;     push r10
;     push r11
;     push r12
;     push r13
;     push r14
;     push r15
; ENDM

; popState MACRO
;     pop r15
;     pop r14
;     pop r13
;     pop r12
;     pop r11
;     pop r10
;     pop r9
;     pop r8
;     pop rsp
;     pop rbp
;     pop rdi
;     pop rsi
;     pop rdx
;     pop rcx
;     pop rbx
;     pop rax
;     popfq
; ENDM

; ; Begin: Present Hook

;     extern onPresentCalled: proc

;     PUBLIC presentHook
;     presentHook PROC
;         sub rsp, 20h
;         ; swapChain pointer is already in rcx
;         call onPresentCalled
;         add rsp, 20h
;         ret
;     presentHook ENDP

; ; End: Present Hook

; _TEXT ENDS

END