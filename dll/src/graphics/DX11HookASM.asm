; INCLUDE <src/utils/ASMMacros.ASM>

; _DATA SEGMENT
; _DATA ENDS

; _TEXT SEGMENT

; ; Begin: Present Hook
; ;    ASM Present hook is currently unused, but I'm keeping as an example of ASM/C++ interop.

;     extern onPresentCalled: proc

;     PUBLIC presentHook
;     presentHook PROC

;         pushState
;         sub rsp, 20h
;         ; swapChain pointer is already in rcx
;         call onPresentCalled
;         add rsp, 20h
;         popState

;         ret
;     presentHook ENDP

; ; End: Present Hook

; _TEXT ENDS

END