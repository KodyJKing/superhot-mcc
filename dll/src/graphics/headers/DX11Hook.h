namespace DX11Hook {

    typedef void ( *PresentCallback )( ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );

    void addPresentHook( HWND hwnd );
    void addOnPresentCallback( PresentCallback cb );

}