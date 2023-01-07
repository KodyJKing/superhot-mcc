namespace DX11Hook {

    typedef void ( *PresentCallback )( IDXGISwapChain* pSwapChain, ID3D11Device* pDevice );

    void addPresentHook();
    void addOnPresentCallback( PresentCallback cb );

}