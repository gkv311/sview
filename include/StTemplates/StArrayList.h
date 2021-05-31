/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StArrayList_H__
#define __StArrayList_H__

#include "StArray.h"

/**
 * This template declare simple-to-use size-scaled Array class.
 * This mean you can work with this class like with List (methods add() and remove()),
 * but memory allocated as array (not a single-linked or double-linked list!).
 * You can manage array-allocation size on construction. When array is full, it automatically upscaled + 8 items
 * and array copying processed.
 */
template<typename Element_t>
class StArrayList : public StArray<Element_t> {

        private:

    size_t mySizeMax;

        public:

    /**
     * Create an empty list.
     */
    StArrayList(size_t theReserveSize = 16)
    : StArray<Element_t>(theReserveSize),
      mySizeMax(0) {
        mySizeMax = StArray<Element_t>::mySize;
        StArray<Element_t>::mySize = 0;
    }

    /**
     * Copy constructor.
     */
    StArrayList(const StArrayList& theCopy)
    : StArray<Element_t>(theCopy.mySizeMax),
      mySizeMax(theCopy.mySizeMax) {
        StArray<Element_t>::mySize = theCopy.mySize;
        // copy only significant elements
        for(size_t anElem = 0; anElem < theCopy.mySize; ++anElem) {
            StArray<Element_t>::myArray[anElem] = theCopy.myArray[anElem];
        }
    }

    /**
     * Assignment operator.
     */
    const StArrayList& operator=(const StArrayList& theCopy) {
        if(this != &theCopy) {
            StArray<Element_t>::mySize = theCopy.mySize;
            mySizeMax = theCopy.mySizeMax;
            delete[] StArray<Element_t>::myArray;
            StArray<Element_t>::myArray = new Element_t[mySizeMax];
            for(size_t anElem = 0; anElem < StArray<Element_t>::mySize; ++anElem) {
                StArray<Element_t>::myArray[anElem] = theCopy.myArray[anElem];
            }
        }
        return (*this);
    }

    /**
     * Deallocate the list.
     */
    virtual ~StArrayList() {
        //
    }

    /**
     * Method to initialize the array list as array.
     * You can access elements within the specified size limit.
     * This method will destroy all current data!
     */
    void initArray(size_t theArraySize) {
        StArray<Element_t>::mySize = theArraySize;
        mySizeMax = (theArraySize > 1) ? getAligned(theArraySize) : 1;
        delete[] StArray<Element_t>::myArray;
        StArray<Element_t>::myArray = new Element_t[mySizeMax];
    }

    /**
     * Method to initialize the array list as list with reserved capability.
     * You should add element by element to fill the list!
     * This method will destroy all current data!
     */
    void initList(size_t theListSize) {
        StArray<Element_t>::mySize = 0;
        mySizeMax = (theListSize > 1) ? getAligned(theListSize) : 1;
        delete[] StArray<Element_t>::myArray;
        StArray<Element_t>::myArray = new Element_t[mySizeMax];
    }

    /**
     * @param theIndex (size_t ) - list position to add new element;
     * @param theElement (const Element_t& ) - element to add;
     * @return this list.
     */
    StArrayList& add(const size_t theIndex, const Element_t& theElement) {
        if(theIndex >= mySizeMax) {
            size_t aNewSize = getAligned(theIndex + 7); // increment with 8 elements...
            Element_t* aNewArray = new Element_t[aNewSize];
            for(size_t anElem = 0; anElem < mySizeMax; ++anElem) {
                aNewArray[anElem] = StArray<Element_t>::myArray[anElem];
            }
            // perform copy before deleting (make it safe for adding self-element)
            aNewArray[theIndex] = theElement;
            if(theIndex >= StArray<Element_t>::mySize) {
                StArray<Element_t>::mySize = theIndex + 1;
            }
            delete[] StArray<Element_t>::myArray;
            StArray<Element_t>::myArray = aNewArray;
            mySizeMax = aNewSize;
        } else {
            StArray<Element_t>::myArray[theIndex] = theElement;
            if(theIndex >= StArray<Element_t>::mySize) {
                StArray<Element_t>::mySize = theIndex + 1;
            }
        }
        return (*this);
    }

    /**
     * @param theElement (const Element_t& ) - element to add at the end of list;
     * @return this list.
     */
    StArrayList& add(const Element_t& theElement) {
        size_t aLastId = StArray<Element_t>::mySize;
        return add(aLastId, theElement);
    }

    /**
     * Removes the element at the specified position in this list.
     */
    StArrayList& remove(const size_t theIndex) {
        if(theIndex >= StArray<Element_t>::mySize) {
            return (*this);
        }
        // make sure the object destroyed (like handle)
        StArray<Element_t>::myArray[theIndex] = Element_t();

        for(size_t aMoveFromId = theIndex + 1; aMoveFromId < StArray<Element_t>::mySize; ++aMoveFromId) {
            // move object to the top... slow!
            StArray<Element_t>::myArray[aMoveFromId - 1] = StArray<Element_t>::myArray[aMoveFromId];
        }
        --StArray<Element_t>::mySize;
        return (*this);
    }

    /**
     * Removes all of the elements from this list;
     * @return this list.
     */
    virtual StArrayList& clear() {
        for(size_t anElem = 0; anElem < StArray<Element_t>::mySize; ++anElem) {
            StArray<Element_t>::myArray[anElem] = Element_t();
        }
        StArray<Element_t>::mySize = 0;
        return (*this);
    }

    /**
     * @return true if this list contains no elements.
     */
    bool isEmpty() const {
        return StArray<Element_t>::mySize == 0;
    }

};

#endif //__StArrayList_H__
