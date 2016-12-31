/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2016
 */

#ifndef __StAssetImportShape_h_
#define __StAssetImportShape_h_

#include <StStrings/StString.h>
#include <StFile/StFileNode.h>
#include <StSlots/StSignal.h>

#include <Standard_Type.hxx>

#include "StAssetDocument.h"

class TDF_Label;
class TDocStd_Document;
class TopLoc_Location;
class TopoDS_Shape;
class TDocStd_Application;
class XCAFDoc_ColorTool;
class XCAFPrs_Style;
class XSControl_WorkSession;

/**
 * Tool for importing asset from BRep shape.
 */
class StAssetImportShape {

        public:

    /**
     * Supported file formats.
     */
    enum FileFormat {
        FileFormat_UNKNOWN,
        FileFormat_BREP,
        FileFormat_BINBREP,
        FileFormat_XBF,
        FileFormat_STEP,
        FileFormat_IGES,
    };

        public:

    /**
     * Detect file format from the file header.
     */
    ST_LOCAL static FileFormat probeFormatFromHeader(const char* theHeader,
                                                     const StString& theExt);

    /**
     * Detect file format from the file header.
     */
    ST_LOCAL static FileFormat probeFormatFromExtension(const StString& theExt);

    /**
     * Initialize global data exchange parameters.
     */
    ST_LOCAL static void initStatic();

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StAssetImportShape();

    /**
     * Perform the import.
     */
    ST_LOCAL bool load(const Handle(StDocNode)& theParentNode,
                       const StString& theFile,
                       const FileFormat theFormat);

        protected:

    /**
     * Fill in XDE document from IGES file.
     */
    ST_LOCAL bool loadIGES(const StString& theFile);

    /**
     * Fill in XDE document from STEP file.
     */
    ST_LOCAL bool loadSTEP(const StString& theFile);

    /**
     * Add the XDE label into Asset document.
     */
    ST_LOCAL void addNodeRecursive(const Handle(StDocNode)& theParentTreeItem,
                                   XCAFDoc_ColorTool&       theColorTool,
                                   const TDF_Label&         theLabel,
                                   const TopLoc_Location&   theParentTrsf,
                                   const XCAFPrs_Style&     theParentStyle);

    /**
     * Add the BRep shape into Asset document.
     */
    ST_LOCAL bool addMeshNode(const Handle(StDocNode)& theParentTreeItem,
                              const TDF_Label&         theShapeLabel,
                              const XCAFPrs_Style&     theParentStyle);

    /**
     * Reset XDE document.
     */
    ST_LOCAL void reset();

    /**
     * Clear working session.
     */
    ST_LOCAL void clearSession(const Handle(XSControl_WorkSession)& theWorkSession);

        public:

    struct {
        /**
         * Emit callback Slot on error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StCString& )> onError;
    } signals;

        protected:

    Handle(TDocStd_Application) myXCAFApp;
    Handle(TDocStd_Document)    myXCAFDoc;

};

#endif // __StAssetImportShape_h_
