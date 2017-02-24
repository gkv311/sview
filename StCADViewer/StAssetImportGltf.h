/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016-2017
 */

#ifndef __StAssetImportGltf_h_
#define __StAssetImportGltf_h_

//#define RAPIDJSON_ASSERT
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/istreamwrapper.h>

#include <StStrings/StString.h>
#include <StFile/StFileNode.h>
#include <StSlots/StSignal.h>

#include <NCollection_DataMap.hxx>
#include <TCollection_AsciiString.hxx>

#include "StAssetDocument.h"

/**
 * Root elements within glTF JSON document.
 */
enum GltfRootElement {
    GltfRootElement_Asset,        //!< "asset"       element, mandatory
    GltfRootElement_Scenes,       //!< "scenes"      element, mandatory
    GltfRootElement_Scene,        //!< "scene"       element, mandatory
    GltfRootElement_Nodes,        //!< "nodes"       element, mandatory
    GltfRootElement_Meshes,       //!< "meshes"      element, mandatory
    GltfRootElement_Accessors,    //!< "accessors"   element, mandatory
    GltfRootElement_BufferViews,  //!< "bufferViews" element, mandatory
    GltfRootElement_Buffers,      //!< "buffers"     element, mandatory
    GltfRootElement_NB_MANDATORY, //!< number of mandatory elements
    // optional elements
    GltfRootElement_Animations = GltfRootElement_NB_MANDATORY,  //!< "animations" element
    GltfRootElement_Materials,    //!< "materials"  element,
    GltfRootElement_Programs,     //!< "programs"   element,
    GltfRootElement_Shaders,      //!< "shaders"    element,
    GltfRootElement_Skins,        //!< "skins"      element,
    GltfRootElement_Techniques,   //!< "techniques" element,
    GltfRootElement_Textures,     //!< "textures"   element,
    GltfRootElement_Images,       //!< "images"     element,
    GltfRootElement_NB            //!< overall number of elements
};

/**
 * Root elements within glTF JSON document - names array.
 */
static const char* GltfRootElementNames[GltfRootElement_NB] = {
    "asset",
    "scenes",
    "scene",
    "nodes",
    "meshes",
    "accessors",
    "bufferViews",
    "buffers",
    "animations",
    "materials",
    "programs",
    "shaders",
    "skins",
    "techniques",
    "textures",
    "images",
};

enum GltfPrimitiveMode {
    GltfPrimitiveMode_UNKNOWN       = -1, //!< unknown or invalid type
    GltfPrimitiveMode_Points        =  0, //!< GL_POINTS
    GltfPrimitiveMode_Lines         =  1, //!< GL_LINES
    GltfPrimitiveMode_LineLoop      =  2, //!< GL_LINE_LOOP
    GltfPrimitiveMode_LineStrip     =  3, //!< GL_LINE_STRIP
    GltfPrimitiveMode_Triangles     =  4, //!< GL_TRIANGLES
    GltfPrimitiveMode_TriangleStrip =  5, //!< GL_TRIANGLE_STRIP
    GltfPrimitiveMode_TriangleFan   =  6, //!< GL_TRIANGLE_FAN
};

enum GltfArrayType {
    GltfArrayType_UNKNOWN,  //!< unknown or invalid type
    GltfArrayType_Indices,  //!< "indices"    within "primitive"  element
    GltfArrayType_Position, //!< "POSITION"   within "attributes" element
    GltfArrayType_Normal,   //!< "NORMAL"     within "attributes" element
    GltfArrayType_Color,    //!< "COLOR"      within "attributes" element
    GltfArrayType_TCoord0,  //!< "TEXCOORD_0" within "attributes" element
    GltfArrayType_TCoord1,  //!< "TEXCOORD_1" within "attributes" element
    GltfArrayType_Joint,    //!< "JOINT"      within "attributes" element
    GltfArrayType_Weight,   //!< "WEIGHT"     within "attributes" element
};

/**
 * Similar to Graphic3d_TypeOfData but does not define actual type and includes matrices.
 */
enum GltfAccessorLayout {
    GltfAccessorLayout_UNKNOWN, //!< unknown or invalid type
    GltfAccessorLayout_Scalar,  //!< "SCALAR"
    GltfAccessorLayout_Vec2,    //!< "VEC2"
    GltfAccessorLayout_Vec3,    //!< "VEC3"
    GltfAccessorLayout_Vec4,    //!< "VEC4"
    GltfAccessorLayout_Mat2,    //!< "MAT2"
    GltfAccessorLayout_Mat3,    //!< "MAT3"
    GltfAccessorLayout_Mat4,    //!< "MAT4"
};

/**
 * Accessor component type.
 */
enum GltfAccessorCompType {
    GltfAccessorCompType_UNKNOWN, //!< unknown or invalid type
    GltfAccessorCompType_Int8    = 5120, //!< GL_BYTE
    GltfAccessorCompType_UInt8   = 5121, //!< GL_UNSIGNED_BYTE
    GltfAccessorCompType_Int16   = 5122, //!< GL_SHORT
    GltfAccessorCompType_UInt16  = 5123, //!< GL_UNSIGNED_SHORT
    GltfAccessorCompType_UInt32  = 5125, //!< GL_UNSIGNED_INT
    GltfAccessorCompType_Float32 = 5126, //!< GL_FLOAT
};

/**
 * Accessor component type.
 */
enum GltfBufferViewTarget {
    GltfBufferViewTarget_UNKNOWN,                      //!< unknown or invalid type
    GltfBufferViewTarget_ARRAY_BUFFER         = 34962, //!< GL_ARRAY_BUFFER
    GltfBufferViewTarget_ELEMENT_ARRAY_BUFFER = 34963, //!< GL_ELEMENT_ARRAY_BUFFER
};

/**
 * Accessor structure.
 */
struct GltfAccessor {
    int64_t              ByteOffset;
    int32_t              ByteStride; // [0, 255]
    int64_t              Count;
    GltfAccessorLayout   Type;
    GltfAccessorCompType ComponentType;

    GltfAccessor() : ByteOffset(0), ByteStride(0), Count(0), Type(GltfAccessorLayout_UNKNOWN), ComponentType(GltfAccessorCompType_UNKNOWN) {}
};

/**
 * BufferView structure.
 */
struct GltfBufferView {
    int64_t ByteOffset;
    int64_t ByteLength;
    GltfBufferViewTarget Target;

    GltfBufferView() : ByteOffset(0), ByteLength(0), Target(GltfBufferViewTarget_UNKNOWN) {}
};

/**
 * Tool for importing asset from GLTF file.
 */
class StAssetImportGltf : public rapidjson::Document {

        public:

    /**
     * Detect file format from the file header.
     */
    ST_LOCAL static bool probeFormatFromHeader(const char* theHeader,
                                               const StString& theExt);

    /**
     * Detect file format from the file header.
     */
    ST_LOCAL static bool probeFormatFromExtension(const StString& theExt);

        public:

    /**
     * Empty constructor.
     */
    ST_LOCAL StAssetImportGltf();

    /**
     * Perform the import.
     */
    ST_LOCAL bool load(const Handle(StDocNode)& theParentNode,
                       const StString& theFile);

    /**
     * Parse glTF document.
     */
    ST_LOCAL bool gltfParse(const Handle(StDocNode)& theParentNode) {
        if(!gltfParseRoots()) {
            return false;
        }

        gltfParseAsset();
        gltfParseMaterials();
        return gltfParseScene(theParentNode);
    }

        protected:

    /**
     * Search mandatory root elements in the document.
     */
    bool gltfParseRoots();

    /**
     * Parse default scene.
     */
    bool gltfParseScene(const Handle(StDocNode)& theParentNode);

    /**
     * Parse document metadata.
     */
    void gltfParseAsset();

        protected:

    /**
     * Parse materials defined in the document.
     */
    void gltfParseMaterials();

    /**
     * Parse standard material.
     */
    bool gltfParseStdMaterial (StGLMaterial& theMat,
                               const GenericValue& theMatNode);

    /**
     * Parse common material (KHR_materials_common extension).
     */
    bool gltfParseCommonMaterial (StGLMaterial& theMat,
                                  const GenericValue& theMatNode);

    /**
     * Parse pbrMetallicRoughness material.
     */
    bool gltfParsePbrMaterial(StGLMaterial& theMat,
                              const GenericValue& theMatNode);

    /**
     * Parse material technique.
     */
    bool gltfParseTechnique(StGLMaterial& theMat,
                            const GenericValue* theTechniqueId);

    /**
     * Parse texture definition.
     */
    bool gltfParseTexture(StGLMaterial& theMat,
                          const GenericValue* theTextureId);

        protected:

    /**
     * Parse scene array of nodes recursively.
     */
    bool gltfParseSceneNodes(const Handle(StDocNode)& theParentNode,
                             const GenericValue& theSceneNodes);

    /**
     * Parse scene node recursively.
     */
    bool gltfParseSceneNode(const Handle(StDocNode)& theParentNode,
                            const TCollection_AsciiString& theSceneNodeName,
                            const GenericValue& theSceneNode);

    /**
     * Parse mesh element.
     */
    bool gltfParseMesh(const Handle(StDocNode)& theParentNode,
                       const TCollection_AsciiString& theMeshName,
                       const GenericValue& theMesh);

    /**
     * Parse GltfArrayType from string.
     */
    static GltfArrayType gltfParseAttribType(const char* theType) {
        if(IsEqual("POSITION", theType)) {
            return GltfArrayType_Position;
        } else if(IsEqual("NORMAL", theType)) {
            return GltfArrayType_Normal;
        } else if(IsEqual("COLOR", theType)) {
            return GltfArrayType_Color;
        } else if(IsEqual("TEXCOORD_0", theType)) {
            return GltfArrayType_TCoord0;
        } else if(IsEqual("TEXCOORD_1", theType)) {
            return GltfArrayType_TCoord1;
        } else if(IsEqual("JOINT", theType)) {
            return GltfArrayType_Joint;
        } else if(IsEqual("WEIGHT", theType)) {
            return GltfArrayType_Weight;
        }
        return GltfArrayType_UNKNOWN;
    }

    /**
     * Parse primitive array.
     */
    bool gltfParsePrimArray(const Handle(StDocMeshNode)& theMeshNode,
                            const TCollection_AsciiString& theMeshName,
                            const GenericValue& thePrimArray);

    /**
     * Parse GltfAccessorLayout from string.
     */
    static GltfAccessorLayout gltfParseAccessorType(const char* theType) {
        if(IsEqual("SCALAR", theType)) {
            return GltfAccessorLayout_Scalar;
        } else if(IsEqual("VEC2", theType)) {
            return GltfAccessorLayout_Vec2;
        } else if(IsEqual("VEC3", theType)) {
            return GltfAccessorLayout_Vec3;
        } else if(IsEqual("VEC4", theType)) {
            return GltfAccessorLayout_Vec4;
        } else if(IsEqual("MAT2", theType)) {
            return GltfAccessorLayout_Mat2;
        } else if(IsEqual("MAT3", theType)) {
            return GltfAccessorLayout_Mat3;
        } else if(IsEqual("MAT4", theType)) {
            return GltfAccessorLayout_Mat4;
        }
        return GltfAccessorLayout_UNKNOWN;
    }

    /**
     * Parse accessor.
     */
    bool gltfParseAccessor(const Handle(StPrimArray)& thePrimArray,
                           const TCollection_AsciiString& theName,
                           const GenericValue&     theAccessor,
                           const GltfArrayType     theType,
                           const GltfPrimitiveMode theMode);

    /**
     * Parse buffer view.
     */
    bool gltfParseBufferView(const Handle(StPrimArray)& thePrimArray,
                             const TCollection_AsciiString& theName,
                             const GenericValue&     theBufferView,
                             const GltfAccessor&     theAccessor,
                             const GltfArrayType     theType,
                             const GltfPrimitiveMode theMode);

    /**
     * Parse buffer.
     */
    bool gltfParseBuffer(const Handle(StPrimArray)& thePrimArray,
                         const TCollection_AsciiString& theName,
                         const GenericValue&     theBuffer,
                         const GltfAccessor&     theAccessor,
                         const GltfBufferView&   theView,
                         const GltfArrayType     theType,
                         const GltfPrimitiveMode theMode);

    /**
     * Read buffer.
     */
    bool gltfReadBuffer(const Handle(StPrimArray)& thePrimArray,
                        const TCollection_AsciiString& theName,
                        const GltfAccessor&     theAccessor,
                        std::istream&           theStream,
                        const GltfArrayType     theType,
                        const GltfPrimitiveMode theMode);

protected:

    /**
     * Read vec4 from specified item.
     */
    static bool gltfReadVec4(StGLVec4& theVec4,
                             const GenericValue* theVal) {
        if(theVal == NULL || !theVal->IsArray() || theVal->Size() != 4) {
            return false;
        }

        for(int aCompIter = 0; aCompIter < 4; ++aCompIter) {
            const GenericValue& aGenVal = (*theVal)[aCompIter];
            if(!aGenVal.IsNumber()) {
                return false;
            }
            theVec4[aCompIter] = aGenVal.GetFloat();
        }
        return true;
    }

    /**
     * Validate color.
     */
    static bool validateColor4(const StGLVec4& theVec) {
        return theVec.r() >= 0.0f && theVec.r() <= 1.0f
            && theVec.g() >= 0.0f && theVec.g() <= 1.0f
            && theVec.b() >= 0.0f && theVec.b() <= 1.0f
            && theVec.a() >= 0.0f && theVec.a() <= 1.0f;
    }

    /**
     * Read vec3 from specified item.
     */
    static bool gltfReadVec3(StGLVec3& theVec3,
                             const GenericValue* theVal) {
        if(theVal == NULL || !theVal->IsArray() || theVal->Size() != 3) {
            return false;
        }

        for(int aCompIter = 0; aCompIter < 3; ++aCompIter) {
            const GenericValue& aGenVal = (*theVal)[aCompIter];
            if(!aGenVal.IsNumber()) {
                return false;
            }
            theVec3[aCompIter] = aGenVal.GetFloat();
        }
        return true;
    }

    /**
     * Validate color
     */
    static bool validateColor3(const StGLVec3& theVec) {
        return theVec.r() >= 0.0f && theVec.r() <= 1.0f
            && theVec.g() >= 0.0f && theVec.g() <= 1.0f
            && theVec.b() >= 0.0f && theVec.b() <= 1.0f;
    }

        protected:

    /**
     * Return text description for an error code.
     */
    static const char* formatParseError(rapidjson::ParseErrorCode theCode) {
        switch(theCode) {
            case rapidjson::kParseErrorNone:                          return "";
            case rapidjson::kParseErrorDocumentEmpty:                 return "Empty Document";
            case rapidjson::kParseErrorDocumentRootNotSingular:       return "The document root must not follow by other values";
            case rapidjson::kParseErrorValueInvalid:                  return "Invalid value";
            case rapidjson::kParseErrorObjectMissName:                return "Missing a name for object member";
            case rapidjson::kParseErrorObjectMissColon:               return "Missing a colon after a name of object member";
            case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket: return "Missing a comma or '}' after an object member";
            case rapidjson::kParseErrorArrayMissCommaOrSquareBracket: return "Missing a comma or ']' after an array element";
            case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex: return "Incorrect hex digit after \\u escape in string";
            case rapidjson::kParseErrorStringUnicodeSurrogateInvalid: return "The surrogate pair in string is invalid";
            case rapidjson::kParseErrorStringEscapeInvalid:           return "Invalid escape character in string";
            case rapidjson::kParseErrorStringMissQuotationMark:       return "Missing a closing quotation mark in string";
            case rapidjson::kParseErrorStringInvalidEncoding:         return "Invalid encoding in string";
            case rapidjson::kParseErrorNumberTooBig:                  return "Number is too big to be stored in double";
            case rapidjson::kParseErrorNumberMissFraction:            return "Miss fraction part in number";
            case rapidjson::kParseErrorNumberMissExponent:            return "Miss exponent in number";
            case rapidjson::kParseErrorTermination:                   return "Parsing was terminated";
            case rapidjson::kParseErrorUnspecificSyntaxError:         return "Unspecific syntax error";
        }
        return "UNKOWN syntax error";
    }

        protected:

    static StString formatSyntaxError(const StString& theFilePath,
                                      const StString& theLibDescr);

    void pushSceneNodeError(const StString& theSceneNode,
                            const StString& theError) {
        signals.onError(formatSyntaxError(myFileName, StString("Scene node '") + theSceneNode + "' " + theError + "."));
    }

    /**
     * Return the string representation of the key in the document - either string key (glTF 1.0) or integer index within array (glTF 2.0).
     */
    static TCollection_AsciiString getKeyString(const rapidjson::Document::GenericValue& theValue) {
        if(theValue.IsString()) {
            return TCollection_AsciiString(theValue.GetString());
        } else if(theValue.IsInt()) {
            return TCollection_AsciiString(theValue.GetInt());
        }
        return TCollection_AsciiString();
    }

        protected:

    /**
     * Auxiliary structure for fast look-up of document sub-nodes of specified node.
     */
    class GltfElementMap {

            public:

        /**
          * Empty constructor.
          */
        GltfElementMap() : myRoot (NULL) {}

        /**
          * Return TRUE if this element is NULL.
          */
        bool isNull() const { return myRoot == NULL; }

        /**
          * Access this node.
          */
        const GenericValue* getRoot() const { return myRoot; }

        /**
          * Find the child node with specified key.
          */
        const GenericValue* findChild(const rapidjson::Document::GenericValue& theKey) {
            const TCollection_AsciiString aKey = getKeyString(theKey);
            if(aKey.IsEmpty()) {
                return NULL;
            }

            const GenericValue* aNode = NULL;
            return myChildren.Find(aKey, aNode)
                  ? aNode
                  : NULL;
        }

        /**
          * Initialize the element.
          */
        void init(const TCollection_AsciiString& theRootName,
                  const GenericValue* theRoot);

            private:

        NCollection_DataMap<TCollection_AsciiString, const GenericValue*, TCollection_AsciiString> myChildren;
        const GenericValue* myRoot;

    };

        public:

    struct {
        /**
         * Emit callback Slot on error.
         * @param theUserData (const StString& ) - error description.
         */
        StSignal<void (const StCString& )> onError;
    } signals;

        protected:

    NCollection_DataMap<TCollection_AsciiString, Handle(StDocObjectNode)> mySceneNodeMap;
    NCollection_DataMap<TCollection_AsciiString, Handle(StDocMeshNode)>   myMeshMap;
    NCollection_DataMap<TCollection_AsciiString, Handle(StGLMaterial)>    myMaterials;

    int64_t  myBinBodyOffset;  //!< offset to binary body
    int64_t  myBinBodyLen;     //!< binary body length
    bool     myIsBinary;       //!< binary document

    GltfElementMap myGltfRoots[GltfRootElement_NB]; //!< glTF format root elements
    StString myFileName;
    StString myFolder;

};

#endif // __StAssetImportGltf_h_
