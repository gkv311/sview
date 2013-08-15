/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#include <StGLWidgets/StGLSubtitles.h>
#include <StGLWidgets/StGLRootWidget.h>

namespace {
    static const StString CLASS_NAME("StGLSubtitles");
};

StGLSubtitles::StSubShowItems::StSubShowItems()
: StArrayList<StHandle <StSubItem> >(8) {
    //
}

bool StGLSubtitles::StSubShowItems::pop(const double thePTS) {
    bool isChanged = false;
    for(size_t anId = size() - 1; anId < size_t(-1); --anId) {
        // filter outdated and forward items
        const StHandle<StSubItem>& anItem = getValue(anId);
        if(anItem->myTimeEnd < thePTS || anItem->myTimeStart > thePTS) {
            remove(anId);
            isChanged = true;
        }
    }
    if(!isChanged) {
        return isChanged;
    } else if(isEmpty()) {
        myText.clear();
        return isChanged;
    }

    // update active text
    myText = getFirst()->myText;
    for(size_t anId = 1; anId < size(); ++anId) {
        const StHandle<StSubItem>& anItem = getValue(anId);
        myText += StString('\n');
        myText += anItem->myText;
    }
    return isChanged;
}

void StGLSubtitles::StSubShowItems::add(const StHandle<StSubItem>& theItem) {
    if(!myText.isEmpty()) {
        myText += StString('\n');
    }
    myText += theItem->myText;
    StArrayList<StHandle <StSubItem> >::add(theItem);
}

const StString& StGLSubtitles::getClassName() {
    return CLASS_NAME;
}

StGLSubtitles::StGLSubtitles(StGLWidget* theParent,
                             const StHandle<StSubQueue>& theSubQueue)
: StGLTextArea(theParent,
               0, -theParent->getRoot()->scale(100),
               StGLCorner(ST_VCORNER_BOTTOM, ST_HCORNER_CENTER),
               theParent->getRoot()->scale(800), theParent->getRoot()->scale(160),
               StGLTextArea::SIZE_DOUBLE),
  myQueue(theSubQueue),
  myShowItems(),
  myPTS(0.0) {
    if(myQueue.isNull()) {
        myQueue = new StSubQueue();
    }

    myToDrawShadow = true;
    setTextColor(StGLVec3(1.0f, 1.0f, 1.0f));
    setBorder(false);

    myFormatter.setupAlignment(StGLTextFormatter::ST_ALIGN_X_CENTER,
                               StGLTextFormatter::ST_ALIGN_Y_BOTTOM);
}

StGLSubtitles::~StGLSubtitles() {
    //
}

void StGLSubtitles::stglUpdate(const StPointD_t& ) {
    bool isChanged = myShowItems.pop(myPTS);
    for(StHandle<StSubItem> aNewSubItem = myQueue->pop(myPTS); !aNewSubItem.isNull(); aNewSubItem = myQueue->pop(myPTS)) {
        isChanged = true;
        myShowItems.add(aNewSubItem);
    }
    if(isChanged) {
        setText(myShowItems.myText);
        StString aLog;
        /**for(size_t anId = 0; anId < myShowItems.size(); ++anId) {
            aLog += ST_STRING(" from ") + myShowItems[anId]->myTimeStart + " to " + myShowItems[anId]->myTimeEnd + "\n";
        }
        ST_DEBUG_LOG("(" + myPTS + ") myShowItems.myText= '" + myShowItems.myText + "'\n" + aLog);*/
    }
}

void StGLSubtitles::stglResize(const StRectI_t& theWinRectPx) {
    changeRectPx().right() = (getParent()->getRectPx().width() / 5) * 3;
    myTextWidth = (GLfloat )getRectPx().width();
    myToRecompute = true;
    StGLTextArea::stglResize(theWinRectPx);
}

const StHandle<StSubQueue>& StGLSubtitles::getQueue() const {
    return myQueue;
}

void StGLSubtitles::setPTS(const double thePTS) {
    myPTS = thePTS;
}
