namespace DX11Hook {

    typedef void ( *PresentCallback )( ID3D11DeviceContext* pCtx, ID3D11Device* pDevice, IDXGISwapChain* pSwapChain );

    void hook( HWND hwnd, IDXGISwapChain* pSwapChainActual );
    void addOnPresentCallback( PresentCallback cb );
    void removeOnPresentCallback( PresentCallback cb );
    void cleanup();

}