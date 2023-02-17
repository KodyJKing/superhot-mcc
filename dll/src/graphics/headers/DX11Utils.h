#pragma once

#include "../../../pch.h"
#include "../../utils/headers/common.h"

using namespace DirectX;

void fitViewportToWindow( ID3D11DeviceContext* pCtx, HWND hwnd );

void safePrintErrorMessage( ID3D10Blob* errorBlob );

ID3D10Blob* compileShader( ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile );
ID3D11VertexShader* compileVertexShader( ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile, const D3D11_INPUT_ELEMENT_DESC* layout, UINT layoutSize, ID3D11InputLayout** pVertLayout );
ID3D11PixelShader* compilePixelShader( ID3D11Device* pDevice, LPCSTR source, LPCSTR functionName, LPCSTR profile );

void copyToBuffer( ID3D11DeviceContext* pCtx, ID3D11Buffer* pBuffer, void* pData, size_t bytes );

XMMATRIX cameraMatrix(
    const Vec3 camPos, const Vec3 camForward, float fov,
    float clippingNear, float clippingFar,
    float viewportWidth, float viewportHeight
);

Vec3 worldToScreen(
    const Vec3 point,
    const Vec3 camPos, const Vec3 camForward, float fov,
    float clippingNear, float clippingFar,
    float viewportWidth, float viewportHeight
);