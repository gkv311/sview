/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StQuickSort_H__
#define __StQuickSort_H__

#include <StTemplates/StTemplates.h>

/**
 * Perform sorting of plain array with QuickSort algorithm.
 */
template<typename Type>
class StQuickSort {

        private:

    static size_t partition(Type* array, const size_t idLow, const size_t idHigh) {
        size_t idLeft(idLow), idRight(idHigh);
        Type pivot = array[idLow];

        while(idLeft < idRight) {
            while(array[idRight] > pivot){
                --idRight;
            }
            while((idLeft < idRight) && (array[idLeft] <= pivot)) {
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

    static void perform(Type* array, size_t low, size_t high) {
        if(low < high) {
            size_t pivotPosition = partition(array, low, high);
            if(pivotPosition > 1) {
                perform(array, low, pivotPosition - 1);
            }
            perform(array, pivotPosition + 1, high);
        }
    }

};

#endif //__StQuickSort_H__
