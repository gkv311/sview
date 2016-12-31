/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016
 */

#ifndef __StAssetNodeIterator_h_
#define __StAssetNodeIterator_h_

#include "StAssetNodeIterator.h"

/**
 * Smart document iterator through nodes of requested type.
 */
class StAssetNodeIterator {

        public:

    /**
     * Main constructor.
     */
    StAssetNodeIterator(const Handle(StDocNode)& theRoot,
                        const StDocNodeType theToFind)
    : myTop(-1), myToFind(theToFind), myToAvoid(StDocNodeType_Unknown), myHasMore(false) {
        init(theRoot, theToFind);
    }

    /**
     * Initialize the iterator.
     */
    void init(const Handle(StDocNode)& theRoot,
              const StDocNodeType theToFind) {
        for(int aStackIter = 0; aStackIter <= myTop; ++aStackIter) {
            myNodeStack.SetValue(aStackIter, NCollection_Sequence<Handle(StDocNode)>::Iterator());
        }
        myTrsf    = gp_Trsf();
        myTop    = -1;
        myRoot   = theRoot;
        myToFind = theToFind;
        if(theRoot.IsNull()
        || myToFind == StDocNodeType_Unknown) {
            myHasMore = false;
            return;
        }

        const StDocNodeType aRootNodeType = theRoot->nodeType();
        if(isLessComplexNodeType(aRootNodeType, theToFind)) {
            // the first Shape is less complex, nothing to find
            myHasMore = false;
        } else if(!isSameNodeType(aRootNodeType, theToFind)) {
            // type is more complex search inside
            myHasMore = true;
            next();
        } else {
            // type is found
            myHasMore = true;
            myTrsf = theRoot->nodeTransformation();
        }
    }

    /**
     * Return true, if iterator points to the valid node.
     */
    bool more() const { return myHasMore; }

    /**
     * Return the current node.
     */
    const Handle(StDocNode)& value() const {
        if(!myHasMore) {
            //
        }
        if(myTop >= 0) {
            return myNodeStack.Value(myTop).Value();
        } else {
            return myRoot;
        }
    }

    /**
     * Return transformation to the current node.
     */
    const gp_Trsf& location() const {
        return myTrsf;
    }

    /**
     * Go to the next node.
     */
    void next() {
        if(!myHasMore) {
            // assert
            return;
        }

        if(myTop < 0) {
            const StDocNodeType aNodeType = myRoot->nodeType();
            if(isSameNodeType(myToFind, aNodeType)) {
                // already visited once
                myHasMore = false;
                return;
            } else if(toAvoidNodeType(myToAvoid, aNodeType)) {
                // avoid the top-level
                myHasMore = false;
                return;
            } else {
                // push and try to find
                myTop = 0;
                myNodeStack.SetValue(0, NCollection_Sequence<Handle(StDocNode)>::Iterator(myRoot->Children()));
            }
        } else {
            myNodeStack.ChangeValue(myTop).Next();
        }

        for(;;) {
            if(myNodeStack.Value(myTop).More()) {
                const Handle(StDocNode)& aNodeTop = myNodeStack.Value(myTop).Value();
                const StDocNodeType aNodeType = aNodeTop->nodeType();
                if(isSameNodeType(myToFind, aNodeType)) {
                    myHasMore = true;

                    myTrsf = gp_Trsf();
                    for(int aStackIter = 0; aStackIter <= myTop; ++aStackIter) {
                        const Handle(StDocNode)& aNode = myNodeStack.Value(aStackIter).Value();
                        myTrsf = myTrsf * aNode->nodeTransformation();
                    }
                    return;
                } else if(isLessComplexNodeType(myToFind, aNodeType)
                      && !toAvoidNodeType(myToAvoid, aNodeType)) {
                    ++myTop;
                    myNodeStack.SetValue(myTop, NCollection_Sequence<Handle(StDocNode)>::Iterator(aNodeTop->Children()));
                } else {
                    myNodeStack.ChangeValue(myTop).Next();
                }
            } else {
                myNodeStack.SetValue(myTop, NCollection_Sequence<Handle(StDocNode)>::Iterator());
                --myTop;
                if(myTop < 0) {
                    break;
                }

                myNodeStack.ChangeValue(myTop).Next();
            }
        }
        myHasMore = false;
    }

        protected:

    //! Function to compare two types of nodes
    static bool isSameNodeType(StDocNodeType theType1, StDocNodeType theType2) {
        return theType1 == theType2;
    }

    static bool toAvoidNodeType(StDocNodeType theType1, StDocNodeType theType2) {
        return theType1 != StDocNodeType_Unknown
            && theType1 == theType2;
    }

    static bool isLessComplexNodeType(StDocNodeType theType1, StDocNodeType theType2) {
        return theType1 > theType2;
    }

        protected:

    Handle(StDocNode) myRoot;
    NCollection_Vector<NCollection_Sequence<Handle(StDocNode)>::Iterator> myNodeStack;
    int           myTop;
    StDocNodeType myToFind;
    StDocNodeType myToAvoid;
    bool          myHasMore;
    gp_Trsf       myTrsf;

};

#endif // __StAssetNodeIterator_h_
