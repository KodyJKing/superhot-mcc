#pragma once
#include "../../../pch.h"
namespace MathUtils {
    float lerp( float start, float end, float t );
    float unlerp( float start, float end, float x );
    INT_PTR signedDifference( UINT_PTR a, UINT_PTR b );
    float randf();
    float guassian();
}