/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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

template<typename Type>
inline Type stClamp(const Type theValue,
                    const Type theMin,
                    const Type theMax) {
    if(theValue < theMin) {
        return theMin;
    } else if(theValue > theMax) {
        return theMax;
    }
    return theValue;
}

#endif //__StTemplates_H__
