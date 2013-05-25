/**
 * Copyright © 2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StNode_H__
#define __StNode_H__

#include <StTemplates/StArrayList.h>
#include <StTemplates/StQuickPointersSort.h>

class StNode;

template<>
inline void StArray<StNode*>::sort() {
    if(mySize > 0) {
        StQuickPointersSort<StNode*>::perform(myArray, 0, mySize - 1);
    }
}

class StNode : public StArrayList<StNode*> {

        public:

    StNode(const StCString& theSubPath  = stCString(""),
           StNode*          theParent   = NULL,
           int              theNodeType = 0)
    : StArrayList<StNode*>(size_t(1)), parent(theParent), subPath(theSubPath), nodeType(theNodeType) {
        //
    }

    virtual ~StNode() {
        clear();
    }

    virtual StArrayList<StNode*>& clear() {
        for(size_t index = 0; index < size(); index++) {
            // call destructor for each element
            delete changeValue(index);
        }
        StArrayList<StNode*>::clear();
        return (*this);
    }

    StNode* getParent() const {
        return parent;
    }

    void reParent(StNode* theNewParent) {
        size_t aPos = 0;
        if(parent != NULL
        && parent->contains(this, aPos)) {
            parent->remove(aPos);
        }
        theNewParent->add(this);
        parent = theNewParent;
    }

    int getType() const {
        return nodeType;
    }

    virtual const StString& getSubPath() const {
        return subPath;
    }

    virtual void setSubPath(const StString& subPath) {
        this->subPath = subPath;
    }

    virtual StString getPath() const {
        if(getParent() != NULL) {
            StString parentPath = getParent()->getPath();
            if(parentPath.isEmpty()) {
                return subPath;
            } else {
                return parentPath + StString(SYS_FS_SPLITTER) + subPath;
            }
        }
        return subPath;
    }

    bool operator==(const StNode& compare) const {
        return (this == &compare)
            || ((this->nodeType == compare.nodeType) && (this->getPath() == compare.getPath()));
    }

    bool operator!=(const StNode& compare) const {
        return !this->operator==(compare);
    }

    bool operator>(const StNode& compare) const {
        if(this == &compare) {
            return false;
        }
        if(this->nodeType == compare.nodeType) {
            return this->subPath > compare.subPath;
        } else {
            return (this->nodeType > compare.nodeType);
        }
    }

    bool operator<(const StNode& compare) const {
        if(this == &compare) {
            return false;
        }
        if(this->nodeType == compare.nodeType) {
            return this->subPath < compare.subPath;
        } else {
            return (this->nodeType < compare.nodeType);
        }
    }

    bool operator>=(const StNode& compare) const {
        if(this == &compare) {
            return true;
        }
        if(this->nodeType == compare.nodeType) {
            return this->subPath >= compare.subPath;
        } else {
            return this->nodeType > compare.nodeType;
        }
    }

    bool operator<=(const StNode& compare) const {
        if(this == &compare) {
            return true;
        }
        if(this->nodeType == compare.nodeType) {
            return this->subPath <= compare.subPath;
        } else {
            return this->nodeType < compare.nodeType;
        }
    }

    //virtual StString toString() const { return getPath(); }

        private:

    StNode*   parent;
    StString subPath;
    int     nodeType;

};

#endif //__StNode_H__
