INCLUDE <src/utils/ASMMacros.asmlib>

_DATA SEGMENT
_DATA ENDS

_TEXT SEGMENT

    extern onPresentCalled: proc
    extern presentHook_jmp: qword
    PUBLIC presentHook
    presentHook PROC
        pushState
        sub rsp, 20h
        call onPresentCalled
        add rsp, 20h
        popState
        jmp [presentHook_jmp]
    presentHook ENDP

    extern onResizeBuffers: proc
    extern resizeBuffersHook_jmp: qword
    PUBLIC resizeBuffersHook
    resizeBuffersHook PROC
        pushState
        sub rsp, 20h
        call onResizeBuffers
        add rsp, 20h
        popState
        jmp [resizeBuffersHook_jmp]
    resizeBuffersHook ENDP

    extern onResizeBuffers: proc
    extern setRenderTargetsHook_return: qword
    extern setRenderTargetsHook_depthStencilView: qword
    PUBLIC setRenderTargetsHook
    setRenderTargetsHook PROC
        ; Original code:
            push rbx
            push rbp
            push rsi
            push rdi
        cmp [setRenderTargetsHook_depthStencilView], 0
        jne dont_save
            mov [setRenderTargetsHook_depthStencilView], r9
        dont_save:
        jmp [setRenderTargetsHook_return]
    setRenderTargetsHook ENDP

_TEXT ENDS

END