#include "../../pch.h"
#include "./headers/MathUtils.h"

namespace MathUtils {

    INT_PTR signedDifference( UINT_PTR a, UINT_PTR b ) {
        if ( a > b ) return (INT_PTR) ( a - b );
        return -(INT_PTR) ( b - a );
    }

}