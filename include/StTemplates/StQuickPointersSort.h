/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StQuickPointersSort_H__
#define __StQuickPointersSort_H__

#include <StTemplates/StTemplates.h>

/**
 * This is copy of the template StQuickSort
 * but assume array of pointers to objects with defined compare operators.
 * Example of use:
 *    MyObject* anArray[128];
 *    StQuickPointersSort<>::perform(anArray, 0, 128 - 1);
 * Notice: algorithm doesn't check pointers for NULL!
 */
template<typename PointerType>
class StQuickPointersSort {

        private:

    static size_t partition(PointerType* array, const size_t idLow, const size_t idHigh) {
        size_t idLeft(idLow), idRight(idHigh);
        PointerType pivot = array[idLow];

        while(idLeft < idRight) {
            while(*array[idRight] > *pivot){
                --idRight;
            }
            while((idLeft < idRight) && (*array[idLeft] <= *pivot)) {
                ++idLeft;
            }

            if(idLeft < idRight) {
                stSwap(array[idLeft], array[idRight]);
            }
        }

        array[idLow] = array[idRight];
        array[idRight] = pivot;
        return idRight;
    }

        public:

    static void perform(PointerType* array, size_t low, size_t high) {
        if(low < high) {
            size_t pivotPosition = partition(array, low, high);
            if(pivotPosition > 1) {
                perform(array, low, pivotPosition - 1);
            }
            perform(array, pivotPosition + 1, high);
        }
    }

};

#endif //__StQuickPointersSort_H__
