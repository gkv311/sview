/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StGLTable_h_
#define __StGLTable_h_

#include <StGLWidgets/StGLTextArea.h>

/**
 * Table element definition.
 */
struct StGLTableItem {
    StGLWidget* Item;
    int         ColSpan;
    int         RowSpan;

    StGLTableItem() : Item(NULL), ColSpan(1), RowSpan(1) {}

    bool operator==(const StGLTableItem& theOther) const { return Item == theOther.Item; }
    bool operator!=(const StGLTableItem& theOther) const { return Item != theOther.Item; }
};

// dummy
template<> inline void StArray<             StGLTableItem  >::sort() {}
template<> inline void StArray< StArrayList<StGLTableItem> >::sort() {}

class StArgumentsMap;

/**
 * Widget represents table of widgets.
 */
class StGLTable : public StGLWidget {

        public:

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StGLTable(StGLWidget* theParent,
                           const int   theLeft,
                           const int   theTop);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLTable();

    ST_CPPEXPORT virtual const StString& getClassName();
    ST_CPPEXPORT virtual void stglResize();
    ST_CPPEXPORT virtual bool stglInit();
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView);

    /**
     * Initialize table dimensions.
     * Reset previous content.
     */
    ST_CPPEXPORT void setupTable(const int theNbRows,
                                 const int theNbColumns);

    /**
     * Setup table element at specified position.
     */
    ST_CPPEXPORT void setElement(const int   theRowId,
                                 const int   theColId,
                                 StGLWidget* theItem,
                                 const int   theRowSpan = 1,
                                 const int   theColSpan = 1);

    /**
     * Access table element at specified position.
     */
    ST_LOCAL const StGLTableItem& getElement(const int theRowId,
                                             const int theColId) const {
        return myTable.getValue(theRowId).getValue(theColId);
    }

    /**
     * Access table element at specified position.
     */
    ST_LOCAL StGLTableItem& changeElement(const int theRowId,
                                          const int theColId) {
        return myTable.changeValue(theRowId).changeValue(theColId);
    }

    /**
     * Initialize table or fill subpart of existing table from strings map.
     * @param theMap          map of string pairs
     * @param theTextColor    text color
     * @param theMaxWidth     maximum width of these two columns
     * @param theCol1MaxWidth maximum width of first column
     * @param theRowId        row    of top-bottom table corner to fill from
     * @param theColId        column of top-bottom table corner to fill from
     */
    ST_CPPEXPORT void fillFromMap(const StArgumentsMap& theMap,
                                  const StGLVec3&       theTextColor,
                                  const int             theMaxWidth,
                                  const int             theCol1MaxWidth,
                                  const int             theRowId = 0,
                                  const int             theColId = 0);

        protected: //! @name protected fields

    StArrayList< StArrayList<StGLTableItem> >
                     myTable;         //!< table content

    StArrayList<int> myRowBottoms;    //!< array of rows    bottoms
    StArrayList<int> myColRights;     //!< array of columns rights

    int              myMarginLeft;    //!< left   margin for all table elements
    int              myMarginRight;   //!< right  margin for all table elements
    int              myMarginTop;     //!< tio    margin for all table elements
    int              myMarginBottom;  //!< bottom margin for all table elements

    bool             myIsInitialized; //!< cached initialization state

};

#endif // __StGLTable_h_
