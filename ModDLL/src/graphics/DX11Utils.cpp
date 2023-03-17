#include "./headers/DX11Utils.h"
#include "../utils/headers/common.h"
#include "../utils/headers/MathUtils.h"
#include "../utils/headers/Vec.h"

void safePrintErrorMessage( ID3D10Blob* errorBlob ) {
    if ( errorBlob )
        std::cout << (char*) errorBlob->GetBufferPointer() << std::endl;
}

void fitViewportToWindow( ID3D11DeviceContext* pCtx, HWND hwnd ) {
    RECT rect;
    GetClientRect( hwnd, &rect );
    D3D11_VIEWPORT vp{};
    vp.TopLeftX = 0.0;
    vp.TopLeftY = 0.0;
    vp.Width = (float) ( rect.right - rect.left );
    vp.Height = (float) ( rect.bottom - rect.top );
    vp.MinDepth = 0.0;
    vp.MaxDepth = 1.0;
    pCtx->RSSetViewports( 1, &vp );
}

ID3D10Blob* compileShader(
    ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile
) {
    ID3D10Blob* errorBlob;
    ID3D10Blob* shaderBlob;
    auto hr = D3DX11CompileFromMemory(
        source, std::strlen( source ),
        0, 0, 0,
        functionName, profile,
        0, 0, 0,
        &shaderBlob, &errorBlob,
        0
    );
    safePrintErrorMessage( errorBlob );
    safeRelease( errorBlob );
    throwIfFail( "Compiling shader", hr );
    return shaderBlob;
}

ID3D11VertexShader* compileVertexShader(
    ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile,
    const D3D11_INPUT_ELEMENT_DESC* layout, UINT layoutSize,
    ID3D11InputLayout** pVertLayout
) {
    ID3D11VertexShader* shaderObject;
    auto shaderBlob = compileShader( pDevice, source, functionName, profile );
    auto hr = pDevice->CreateVertexShader( shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &shaderObject );
    throwIfFail( "Creating vertex shader", hr );
    hr = pDevice->CreateInputLayout(
        layout, layoutSize,
        shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(),
        pVertLayout
    );
    safeRelease( shaderBlob );
    throwIfFail( hr );
    return shaderObject;
}


ID3D11PixelShader* compilePixelShader(
    ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile
) {
    ID3D11PixelShader* shaderObject;
    auto shaderBlob = compileShader( pDevice, source, functionName, profile );
    auto hr = pDevice->CreatePixelShader( shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), NULL, &shaderObject );
    throwIfFail( "Creating pixel shader", hr );
    safeRelease( shaderBlob );
    return shaderObject;
}

void copyToBuffer(
    ID3D11DeviceContext* pCtx,
    ID3D11Buffer* pBuffer,
    void* pData, size_t bytes
) {
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    auto hr = pCtx->Map( pBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource );
    throwIfFail( "Mapping D3D Buffer", hr );
    std::memcpy( mappedResource.pData, pData, bytes );
    pCtx->Unmap( pBuffer, 0 );
}

XMMATRIX cameraMatrix(
    const Vec3 camPos, const Vec3 camForward, float fov,
    float clippingNear, float clippingFar,
    float viewportWidth, float viewportHeight
) {
    float aspect = viewportWidth / viewportHeight;
    XMMATRIX perspective = XMMatrixPerspectiveFovRH( fov, aspect, clippingNear, clippingFar );
    XMMATRIX view = XMMatrixLookToRH( XMLoadFloat3( &camPos ), XMLoadFloat3( &camForward ), { 0.0f, 0.0f, 1.0f, 0.0f } );
    return view * perspective;
}

Vec3 worldToScreen(
    const Vec3 point,
    const Vec3 camPos, const Vec3 camForward, float fov,
    float clippingNear, float clippingFar,
    float viewportWidth, float viewportHeight
) {
    float aspect = viewportWidth / viewportHeight;
    XMMATRIX perspective = XMMatrixPerspectiveFovRH( fov, aspect, clippingNear, clippingFar );
    XMMATRIX view = XMMatrixLookToRH( XMLoadFloat3( &camPos ), XMLoadFloat3( &camForward ), { 0.0f, 0.0f, 1.0f, 0.0f } );
    static XMMATRIX world = XMMatrixIdentity();
    Vec3 result;
    XMStoreFloat3(
        &result,
        XMVector3Project(
            XMLoadFloat3( &point ),
            0.0f, 0.0f,
            viewportWidth, viewportHeight,
            0.0f, 1.0f,
            perspective, view, world
        )
    );

    auto diff = Vec::sub( point, camPos );
    auto depth = Vec::dot( diff, camForward );
    result.z = MathUtils::unlerp( clippingNear, clippingFar, depth );

    return result;
}
