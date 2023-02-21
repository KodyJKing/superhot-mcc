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

_TEXT ENDS

END