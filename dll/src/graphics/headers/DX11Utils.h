#pragma once

#include "../../../pch.h"
#include "../../utils/headers/common.h"

void fitViewportToWindow( ID3D11DeviceContext* pCtx, HWND hwnd );

void safePrintErrorMessage( ID3D10Blob* errorBlob );

ID3D10Blob* compileShader( ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile );
ID3D11VertexShader* compileVertexShader( ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile, const D3D11_INPUT_ELEMENT_DESC* layout, UINT layoutSize, ID3D11InputLayout** pVertLayout );
ID3D11PixelShader* compilePixelShader( ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile );

void copyToBuffer( ID3D11DeviceContext* pCtx, ID3D11Buffer* pBuffer, void* pData, size_t bytes );

// template <typename T>
// void createShader(
//     ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile,
//     ID3D10Blob** ppShaderBlob, T** ppShader
// ) {
//     T* pShader;
//     auto pShaderBlob = compileShader( pDevice, source, functionName, profile );
//     HRESULT hr;
//     if constexpr ( std::is_base_of<ID3D11VertexShader, T>::value )
//         hr = pDevice->CreateVertexShader( pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &pShader );
//     else if constexpr ( std::is_base_of<ID3D11PixelShader, T>::value )
//         hr = pDevice->CreatePixelShader( pShaderBlob->GetBufferPointer(), pShaderBlob->GetBufferSize(), NULL, &pShader );
//     throwIfFail( "Creating shader object", hr );
//     if ( ppShaderBlob )
//         *ppShaderBlob = pShaderBlob;
//     if ( ppShader )
//         *ppShader = pShader;
// }