// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <windows.h>

#include <iostream>
#include <iomanip>
#include <filesystem>
#include <string>
#include <sstream>
#include <fstream>
#include <functional>
#include <unordered_map>
#include <mutex>
#include <unordered_set>
#include <set>
#include <exception>
#include <cstdio>
#include <memory>
#include <map>
#include <type_traits>
// #include <format>

#include <inttypes.h>
#include <intrin.h>

#define _USE_MATH_DEFINES
#include <math.h>

#include <DirectXMath.h>
using Vec2 = DirectX::XMFLOAT2;
using Vec3 = DirectX::XMFLOAT3;
using Vec4 = DirectX::XMFLOAT4;
using DirectX::XMMATRIX;

#include <D3D11.h>
#include <D3DX11.h>
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")

#endif //PCH_H
