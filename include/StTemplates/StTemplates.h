/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StTemplates_H__
#define __StTemplates_H__

/**
 * Redefine memory allocation functions as templates
 * (just to improve readability).
 */
template<typename TypePtr>
inline TypePtr stMemAlloc(const size_t& bytesCount) {
    return (TypePtr )::stMemAlloc(bytesCount);
}

template<typename TypePtr>
inline TypePtr stMemRealloc(TypePtr thePtr,
                            const size_t& bytesCount) {
    return (TypePtr )::stMemRealloc(thePtr, bytesCount);
}

template<typename TypePtr>
inline TypePtr stMemAllocAligned(const size_t& bytesCount,
                                 const size_t& align = ST_ALIGNMENT) {
    return (TypePtr )::stMemAllocAligned(bytesCount, align);
}

template<typename TypePtr>
inline TypePtr stMemAllocZeroAligned(const size_t& bytesCount,
                                     const size_t& align = ST_ALIGNMENT) {
    return (TypePtr )::stMemAllocZeroAligned(bytesCount, align);
}

template<typename TypePtr>
inline TypePtr stMemReallocAligned(TypePtr ptrAligned,
                                   const size_t& bytesCount,
                                   const size_t& align = ST_ALIGNMENT) {
    return (TypePtr )::stMemReallocAligned(ptrAligned, bytesCount, align);
}

/**
 * Auxiliary functions.
 */
template<typename Type>
void stSwap(Type& x, Type& y) {
    Type temp = x;
    x = y;
    y = temp;
}

template<typename Type>
inline Type stMin(const Type value1, const Type value2) {
    return (value1 < value2) ? value1 : value2;
}

template<typename Type>
inline Type stMin(const Type value1, const Type value2, const Type value3) {
    return stMin(stMin(value1, value2), value3);
}

template<typename Type>
inline Type stMax(const Type value1, const Type value2) {
    return (value1 > value2) ? value1 : value2;
}

template<typename Type>
inline Type stMax(const Type value1, const Type value2, const Type value3) {
    return stMax(stMax(value1, value2), value3);
}

/**template<typename Type>
inline void stClamp(Type &x, Type min, Type max) {
    if(x < min) {
        x = min;
    }
    if(x > max) {
        x = max;
    }
}*/

#endif //__StTemplates_H__
