/**
 * Copyright © 2009-2011 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StArray_H__
#define __StArray_H__

#include <StStrings/StString.h>
#include <StTemplates/StQuickSort.h>

/**
 * This template declare Array class.
 * Memory allocated as flat array.
 *
 * Requirements - Elements' class should provide:
 *   - empty constructor (major);
 *   - copy constructor (major);
 *   - assignment operator = (major);
 *   - equal operator == (major);
 *   - toString() method (recommended).
 */
template<typename Element_t>
class StArray {

        protected:

    size_t      mySize;  //!< array size
    Element_t* myArray; //!< array buffer (may be bigger than array size!)

        public:

    /**
     * Initialize the array with requested number of elements.
     * @param theSize (size_t ) - the array size (number of elements).
     */
    StArray(size_t theSize)
    : mySize(theSize),
      myArray(NULL) {
        ST_DEBUG_ASSERT(theSize > 0);
        size_t aSize = (theSize > 1) ? getAligned(theSize) : 1;
        myArray = new Element_t[aSize];
    }

    /**
     * Initialize the array with specified default value.
     */
    StArray(size_t theSize, const Element_t& theDefValue)
    : mySize(theSize),
      myArray(NULL) {
        ST_DEBUG_ASSERT(theSize > 0);
        size_t aSize = (theSize > 1) ? getAligned(theSize) : 1;
        myArray = new Element_t[aSize];
        for(size_t anItem = 0; anItem < mySize; ++anItem) {
            myArray[anItem] = theDefValue;
        }
    }

    /**
     * Copy constructor.
     */
    StArray(const StArray& theCopy)
    : mySize(theCopy.mySize) {
        myArray = new Element_t[theCopy.mySize];
        for(size_t anElem = 0; anElem < mySize; ++anElem) {
            myArray[anElem] = theCopy.myArray[anElem];
        }
    }

    /**
     * Assignment operator.
     */
    const StArray& operator=(const StArray& theCopy) {
        if(this != &theCopy) {
            mySize = theCopy.mySize;
            delete[] myArray;
            myArray = new Element_t[mySize];
            for(size_t anElem = 0; anElem < mySize; ++anElem) {
                myArray[anElem] = theCopy.myArray[anElem];
            }
        }
        return (*this);
    }

    /**
     * Deallocate the array.
     */
    virtual ~StArray() {
        delete[] myArray;
    }

    /**
     * @param theElement (Element_t& );
     * @return true if this list contains the specified element.
     */
    virtual bool contains(const Element_t& theElement) const {
        for(size_t anElem = 0; anElem < mySize; ++anElem) {
            if(myArray[anElem] == theElement) {
                return true;
            }
        }
        return false;
    }

    virtual bool contains(const Element_t& theElement, size_t& theFoundId) const {
        for(size_t anElem = 0; anElem < mySize; ++anElem) {
            if(myArray[anElem] == theElement) {
                theFoundId = anElem;
                return true;
            }
        }
        return false;
    }

    /**
     * @param theIndex (size_t& ) - element's index to get;
     * @return element at specified position (constant link).
     */
    const Element_t& getValue(const size_t& theIndex) const {
        ST_DEBUG_ASSERT(theIndex < mySize);
        return myArray[theIndex];
    }

    const Element_t& operator[](const size_t& theIndex) const {
        return getValue(theIndex);
    }

    Element_t& changeValue(const size_t& theIndex) {
        ST_DEBUG_ASSERT(theIndex < mySize);
        return myArray[theIndex];
    }

    Element_t& operator[](const size_t& theIndex) {
        return changeValue(theIndex);
    }

    /**
     * Fast method to access to the first element.
     */
    const Element_t& getFirst() const {
        ST_DEBUG_ASSERT(mySize > 0);
        return myArray[0];
    }

    /**
     * Fast method to access to the first element.
     */
    Element_t& changeFirst() {
        ST_DEBUG_ASSERT(mySize > 0);
        return myArray[0];
    }

    /**
     * Fast method to access to the last element.
     */
    const Element_t& getLast() const {
        ST_DEBUG_ASSERT(mySize > 0);
        return myArray[mySize - 1];
    }

    /**
     * Fast method to access to the last element.
     */
    Element_t& changeLast() {
        ST_DEBUG_ASSERT(mySize > 0);
        return myArray[mySize - 1];
    }

    /**
     * @param theIndex (size_t ) - element's index to set;
     * @param theElement (Element_t& ) - new element to set.
     */
    StArray& set(const size_t theIndex, Element_t& theElement) {
        if(theIndex >= mySize) {
            ST_DEBUG_ASSERT(theIndex < mySize);
            return (*this);
        }
        myArray[theIndex] = theElement;
        return (*this);
    }

    /**
     * Do array sort, from minimum to maximum element value.
     */
    virtual void sort() {
        if(mySize > 0) {
            StQuickSort<Element_t>::perform(myArray, 0, mySize - 1);
        }
    }

    /**
     * @return (size_t ) the number of elements in this list.
     */
    size_t size() const {
        return mySize;
    }

    /**
     * Equality operator.
     */
    bool operator==(const StArray& theOther) const {
        if(this == &theOther) {
            return true;
        } else if(mySize != theOther.mySize) {
            return false;
        }

        for(size_t anElem = 0; anElem < mySize; ++anElem) {
            if(myArray[anElem] != theOther.myArray[anElem]) {
                return false;
            }
        }
        return true;
    }

    /**
     * Inequality operator.
     */
    bool operator!=(const StArray& theOther) const {
        return !(*this == theOther);
    }

    /**
     * Base template for to string method. Will be instantiated only when needed.
     */
    StString toString(const StString& theSplitter = StString('\n')) const;

};

/**
 * Base template for to string method. Will be instantiated only when needed.
 */
template<typename Type> inline
StString StArray<Type>::toString(const StString& theSplitter) const {
    StString aString;
    if(mySize == 0) {
        return aString;
    }
    for(size_t anElem = 0;;) {
        aString += myArray[anElem++].toString();
        if(anElem < mySize) {
            aString += theSplitter;
        } else {
            break;
        }
    }
    return aString;
}

#endif //__StArray_H__
