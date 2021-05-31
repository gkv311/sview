/**
 * StGLWidgets, small C++ toolkit for writing GUI using OpenGL.
 * Copyright © 2014-2017 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StGLTable_h_
#define __StGLTable_h_

#include <StGLWidgets/StGLTextArea.h>

#include <StSettings/StParam.h>
#include <StSlots/StAction.h>
#include <StStrings/StLangMap.h>

class StGLTable;

/**
 * Table element definition.
 */
class StGLTableItem : public StGLWidget {

        public:

    ST_LOCAL int  getColSpan() const { return myColSpan; }
    ST_LOCAL int  getRowSpan() const { return myRowSpan; }

    ST_LOCAL void setColSpan(int theValue) { myColSpan = theValue; }
    ST_LOCAL void setRowSpan(int theValue) { myRowSpan = theValue; }

    ST_LOCAL const StGLWidget* getItem() const {
        return myChildren.getStart();
    }

    ST_LOCAL StGLWidget* getItem() {
        return myChildren.getStart();
    }

    /**
     * Default constructor.
     */
    ST_CPPEXPORT StGLTableItem(StGLTable* theParent);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLTableItem();

        private:

    int myColSpan; //!< columns span
    int myRowSpan; //!< rows    span

};

// dummy
template<> inline void StArray<             StGLTableItem*  >::sort() {}
template<> inline void StArray< StArrayList<StGLTableItem*> >::sort() {}

class StDictionary;

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
                           const int   theTop,
                           StGLCorner  theCorner = StGLCorner(ST_VCORNER_TOP, ST_HCORNER_LEFT));

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StGLTable();

    ST_CPPEXPORT virtual void stglResize() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual bool stglInit() ST_ATTR_OVERRIDE;
    ST_CPPEXPORT virtual void stglDraw(unsigned int theView) ST_ATTR_OVERRIDE;

    /**
     * Initialize table dimensions.
     * Reset previous content.
     */
    ST_CPPEXPORT void setupTable(const int theNbRows,
                                 const int theNbColumns);

    /**
     * Access table element at specified position.
     */
    ST_LOCAL const StGLTableItem& getElement(const int theRowId,
                                             const int theColId) const {
        return *myTable.getValue(theRowId).getValue(theColId);
    }

    /**
     * Access table element at specified position.
     */
    ST_LOCAL StGLTableItem& changeElement(const int theRowId,
                                          const int theColId) {
        return *myTable.changeValue(theRowId).changeValue(theColId);
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
    ST_CPPEXPORT void fillFromMap(const StDictionary& theMap,
                                  const StGLVec3&     theTextColor,
                                  const int           theMaxWidth,
                                  const int           theCol1MaxWidth,
                                  const int           theRowId = 0,
                                  const int           theColId = 0);

    /**
     * Initialize table or fill subpart of existing table from parameters list.
     * @param theParams       array of parameters
     * @param theTextColor    text color
     * @param theMaxWidth     maximum width of these two columns
     * @param theRowId        row    of top-bottom table corner to fill from
     * @param theColId        column of top-bottom table corner to fill from
     */
    ST_CPPEXPORT void fillFromParams(const StParamsList& theParams,
                                     const StGLVec3&     theTextColor,
                                     const int           theMaxWidth,
                                     const int           theRowId = 0,
                                     const int           theColId = 0);

    /**
     * Initialize the table from per-action hot-keys.
     * @param theActions      actions map
     * @param theLangMap      map of translation strings
     * @param theSlot1        callback slot to change first  hot-key for specified action ID
     * @param theSlot2        callback slot to change second hot-key for specified action ID
     * @param theMaxWidth     maximum width of these three columns
     * @param theRowId        row    of top-bottom table corner to fill from
     * @param theColId        column of top-bottom table corner to fill from
     */
    ST_CPPEXPORT void fillFromHotKeys(const std::map< int, StHandle<StAction> >&      theActions,
                                      const StLangMap&                                theLangMap,
                                      const StHandle< StSlot<void (const size_t )> >& theSlot1,
                                      const StHandle< StSlot<void (const size_t )> >& theSlot2,
                                      int                theMaxWidth = 0,
                                      const int          theRowId = 0,
                                      const int          theColId = 0);

    /**
     * Update hot-keys values.
     */
    ST_CPPEXPORT void updateHotKeys(const std::map< int, StHandle<StAction> >& theActions,
                                    const int theRowId = 0,
                                    const int theColId = 0);

    /**
     * Re-compute position of table elements.
     */
    ST_CPPEXPORT void updateLayout();

    /**
     * Return extra margins within each item in the table.
     */
    ST_LOCAL const StMarginsI& getItemMargins() const {
        return myItemMargins;
    }

    /**
     * Return extra margins within each item in the table.
     */
    ST_LOCAL StMarginsI& changeItemMargins() {
        return myItemMargins;
    }

        protected: //! @name protected fields

    StArrayList< StArrayList<StGLTableItem*> >
                     myTable;         //!< table content

    StArrayList<int> myRowBottoms;    //!< array of rows    bottoms
    StArrayList<int> myColRights;     //!< array of columns rights

    StMarginsI       myItemMargins;   //!< margins for all table elements

    bool             myIsInitialized; //!< cached initialization state

};

#endif // __StGLTable_h_
