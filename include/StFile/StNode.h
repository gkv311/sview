/**
 * Copyright © 2010-2015 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
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

    virtual void setSubPath(const StString& theSubPath) {
        this->subPath = theSubPath;
    }

    virtual StString getPath() const {
        if(getParent() == NULL) {
            return subPath;
        }

        const StString parentPath = getParent()->getPath();
        if(parentPath.isEmpty()) {
            return subPath;
        }

        return parentPath.isEndsWith(SYS_FS_SPLITTER)
             ? parentPath + subPath
             : parentPath + StString(SYS_FS_SPLITTER) + subPath;
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
