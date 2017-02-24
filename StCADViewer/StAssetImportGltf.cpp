/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2016-2017
 */

#ifdef _WIN32
    #define NOMINMAX
#endif

#include "StAssetImportGltf.h"

#include "StImageOcct.h"

#include <StStrings/StLogger.h>
#include <StTemplates/StArrayStreamBuffer.h>

#include <Graphic3d_Mat4d.hxx>
#include <Graphic3d_Vec.hxx>
#include <gp_Quaternion.hxx>
#include <NCollection_Buffer.hxx>
#include <OSD_OpenFile.hxx>
#include <Precision.hxx>

namespace
{
    /**
     * Material extension.
     */
    const char THE_KHR_materials_common[] = "KHR_materials_common";
    const char THE_KHR_binary_glTF[]      = "KHR_binary_glTF";

    //! Look-up table for decoding base64 stream.
    static const stUByte_t THE_BASE64_FROM[128] = {
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,  62, 255,  62, 255,  63,
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 255, 255,   0, 255, 255, 255,
        255,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
         15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 255, 255, 255, 255,  63,
        255,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
         41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 255, 255, 255, 255, 255
    };

    /**
     * Function decoding base64 stream.
     */
    Handle(NCollection_Buffer) decodeBase64(const stUByte_t* theStr,
                                            const size_t theLen) {
        Handle(NCollection_Buffer) aData = new NCollection_Buffer(NCollection_BaseAllocator::CommonBaseAllocator());
        if(!aData->Allocate (3 * theLen / 4)) {
            ST_ERROR_LOG("Fail to allocate memory.");
            return Handle(NCollection_Buffer)();
        }

        stUByte_t* aDataPtr = aData->ChangeData();
        const stUByte_t* anEnd = theStr + theLen;
        for(const stUByte_t* aByteIter = theStr; aByteIter < anEnd; aByteIter += 4) {
            // get values for each group of four base 64 characters
            const stUByte_t b4[4] = {
                aByteIter + 0 < anEnd && aByteIter[0] <= 'z' ? THE_BASE64_FROM[aByteIter[0]] : stUByte_t(0xFF),
                aByteIter + 1 < anEnd && aByteIter[1] <= 'z' ? THE_BASE64_FROM[aByteIter[1]] : stUByte_t(0xFF),
                aByteIter + 2 < anEnd && aByteIter[2] <= 'z' ? THE_BASE64_FROM[aByteIter[2]] : stUByte_t(0xFF),
                aByteIter + 3 < anEnd && aByteIter[3] <= 'z' ? THE_BASE64_FROM[aByteIter[3]] : stUByte_t(0xFF)
            };

            // transform into a group of three bytes
            const stUByte_t b3[3] = {
                stUByte_t(((b4[0] & 0x3F) << 2) + ((b4[1] & 0x30) >> 4)),
                stUByte_t(((b4[1] & 0x0F) << 4) + ((b4[2] & 0x3C) >> 2)),
                stUByte_t(((b4[2] & 0x03) << 6) + ((b4[3] & 0x3F) >> 0))
            };

            // add the byte to the return value if it isn't part of an '=' character (indicated by 0xFF)
            if(b4[1] != 0xFF) {
                *aDataPtr = b3[0];
                ++aDataPtr;
            }
            if(b4[2] != 0xFF) {
                *aDataPtr = b3[1];
                ++aDataPtr;
            }
            if(b4[3] != 0xFF) {
                *aDataPtr = b3[2];
                ++aDataPtr;
            }
        }

        return aData;
    }

    /**
     * Find member of the object in a safe way.
     */
    static const rapidjson::Document::GenericValue* findObjectMember(const rapidjson::Document::GenericValue& theObject,
                                                                     const char*  theName) {
        if(!theObject.IsObject()) {
            return NULL;
        }

        rapidjson::Document::ConstMemberIterator anIter = theObject.FindMember(theName);
        return anIter != theObject.MemberEnd()
             ? &anIter->value
             : NULL;
    }

}

/**
 * Image texture embedded into binary glTF file.
 */
class StGltfBinTexture : public StAssetTexture {

    DEFINE_STANDARD_RTTI_INLINE(StGltfBinTexture, StAssetTexture)

        public:

    /**
     * Constructor.
     */
    StGltfBinTexture(const StString& theUri,
                     const StString& theMime,
                     const int64_t theStart,
                     const int theLen)
    : StAssetTexture(theUri),
      myStart(theStart),
      myLen(theLen),
      myMime(theMime) {
        if(!theUri.isEmpty()) {
            const StString anId = StString("texture://") + theUri + "@offset=" + StString(theStart) + "@len=" + StString(theLen);
            myTexId = anId.toCString();
        }
    }

    /**
     * Constructor.
     */
    StGltfBinTexture(const StString& theUri,
                     const StString& theMime,
                     const Handle(NCollection_Buffer)& theBuffer)
    : StAssetTexture(theUri),
      myStart(0),
      myLen(0),
      myBuffer(theBuffer),
      myMime(theMime) {
        if(!theUri.isEmpty()) {
            const StString anId = StString("texture://") + theUri + "@base64";
            myTexId = anId.toCString();
        }
    }

    /**
     * Image getter.
     */
    virtual Handle(Image_PixMap) GetImage() const Standard_OVERRIDE {
        Handle(Image_PixMap) anImage;
        if(!myBuffer.IsNull()) {
            Handle(StImageOcct) anStImage = new StImageOcct();
            if(anStImage->Load(myImageUri, StMIME(myMime, StString(), StString()), myBuffer->ChangeData(), (int )myBuffer->Size())) {
                anImage = anStImage;
            }
        } else {
            std::ifstream aFile;
            OSD_OpenStream(aFile, myImageUri.toCString(), std::ios::in | std::ios::binary);
            if(!aFile.is_open() || !aFile.good()) {
                ST_ERROR_LOG(StString() + "Texture points to non existing file '" + myImageUri.toCString() + "'");
                return false;
            }

            aFile.seekg(myStart, std::ios_base::beg);
            if(!aFile.good()) {
                aFile.close();
                ST_ERROR_LOG(StString() + "Texture refers to non-existing location");
                return false;
            }

            Handle(NCollection_Buffer) aData = new NCollection_Buffer(NCollection_BaseAllocator::CommonBaseAllocator());
            if(!aData->Allocate(myLen)) {
                ST_ERROR_LOG("Fail to allocate memory.");
                return false;
            }

            if(!aFile.read((char* )aData->ChangeData(), myLen)) {
                ST_ERROR_LOG(StString() + "Texture refers to non-existing location");
                return false;
            }

            Handle(StImageOcct) anStImage = new StImageOcct();
            if(anStImage->Load(myImageUri, StMIME(myMime, StString(), StString()), aData->ChangeData(), myLen)) {
                anImage = anStImage;
            }
        }

        if(anImage.IsNull()) {
            return Handle(Image_PixMap)();
        }
        return anImage;
    }

    /**
     * Compare with another texture.
     */
    virtual bool isEqual(const StAssetTexture& theOther) const {
        return myTexId == theOther.GetId();
    }

        private:

    int64_t myStart;
    int     myLen;
    Handle(NCollection_Buffer) myBuffer;
    StString myMime;

};

void StAssetImportGltf::GltfElementMap::init(const TCollection_AsciiString& theRootName,
                                             const GenericValue* theRoot) {
    myRoot = theRoot;
    myChildren.Clear();
    if(theRoot == NULL) {
        return;
    }

    if(theRoot->IsObject()) {
        // glTF 1.0
        for(ConstMemberIterator aChildIter = theRoot->MemberBegin(); aChildIter != theRoot->MemberEnd(); ++aChildIter) {
            if(!aChildIter->name.IsString()) {
                continue;
            }

            const TCollection_AsciiString aKey(aChildIter->name.GetString());
            if(!myChildren.Bind(aKey, &aChildIter->value)) {
                ST_DEBUG_LOG("Warning! Invalid glTF syntax - key '" + aKey.ToCString() + "' is already defined in '" + theRootName.ToCString() + "'.");
            }
        }
    } else if(theRoot->IsArray()) {
        // glTF 2.0
        int aChildIndex = 0;
        for(rapidjson::Value::ConstValueIterator aChildIter = theRoot->Begin(); aChildIter != theRoot->End(); ++aChildIter, ++aChildIndex) {
            myChildren.Bind(TCollection_AsciiString(aChildIndex), aChildIter);
        }
    }
}

StString StAssetImportGltf::formatSyntaxError(const StString& theFilePath,
                                              const StString& theLibDescr) {
    StString aFileName, aFolderName;
    StFileNode::getFolderAndFile(theFilePath, aFolderName, aFileName);
    ST_ERROR_LOG("Invalid syntax within glTF document \"" + theFilePath + "\" (" + theLibDescr + ')');
    return StString("Invalid syntax within glTF document\n\"") + aFileName + "\"\n" + theLibDescr;
}

bool StAssetImportGltf::probeFormatFromHeader(const char* theHeader,
                                              const StString& theExt) {
    return ::memcmp(theHeader, "glTF", 4) == 0
         || theExt.isEqualsIgnoreCase(stCString("gltf"));
}

bool StAssetImportGltf::probeFormatFromExtension(const StString& theExt) {
    return theExt.isEqualsIgnoreCase(stCString("gltf"))
        || theExt.isEqualsIgnoreCase(stCString("glb"));
}

StAssetImportGltf::StAssetImportGltf()
: myBinBodyOffset(0),
  myBinBodyLen(0),
  myIsBinary(false) {
    //
}

bool StAssetImportGltf::load(const Handle(StDocNode)& theParentNode,
                             const StString& theFile) {
    myFileName = theFile;
    StString aName;
    StFileNode::getFolderAndFile(theFile, myFolder, aName);
    if(!myFolder.isEmpty()) {
        myFolder += SYS_FS_SPLITTER;
    }

    std::ifstream aFile;
    OSD_OpenStream(aFile, theFile.toCString(), std::ios::in | std::ios::binary);
    if(!aFile.is_open() || !aFile.good()) {
        signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' is not found!"));
        return false;
    }

    char aGlbHeader[12] = {};
    aFile.read(aGlbHeader, sizeof(aGlbHeader));
    int64_t aJsonBodyOffset = 0;
    int64_t aJsonBodyLen    = 0;
    if(::strncmp(aGlbHeader, "glTF", 4) == 0) {
        myIsBinary = true;
        const uint32_t* aVer = (const uint32_t* )(aGlbHeader + 4);
        const uint32_t* aLen = (const uint32_t* )(aGlbHeader + 8);
        if(*aVer == 1) {
            if(*aLen < 20) {
                signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' has broken glTF format!"));
                return false;
            }

            char aHeader1[8] = {};
            aFile.read(aHeader1, sizeof(aHeader1));

            const uint32_t* aSceneLen    = (const uint32_t* )(aHeader1 + 0);
            const uint32_t* aSceneFormat = (const uint32_t* )(aHeader1 + 4);
            aJsonBodyOffset = 20;
            aJsonBodyLen    = int64_t(*aSceneLen);

            myBinBodyOffset = aJsonBodyOffset + aJsonBodyLen;
            myBinBodyLen    = int64_t(*aLen) - myBinBodyOffset;
            if(*aSceneFormat != 0) {
                signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' is written using unsupported Scene format!"));
                return false;
            }
        } else { //if (*aVer == 2) {
            if(*aVer != 2) {
                //signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' is written using unknown version " + int(*aVer) + "!"));
            }

            for(int aChunkIter = 0; !aFile.eof() && aChunkIter < 2; ++aChunkIter) {
                char aChunkHeader2[8] = {};
                if(int64_t(aFile.tellg()) + int64_t(sizeof(aChunkHeader2)) > int64_t(aLen)) {
                    break;
                }

                aFile.read(aChunkHeader2, sizeof(aChunkHeader2));
                if(!aFile.good()) {
                    signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' is written using unsupported format!"));
                    return false;
                }

                const uint32_t* aChunkLen  = (const uint32_t* )(aChunkHeader2 + 0);
                const uint32_t* aChunkType = (const uint32_t* )(aChunkHeader2 + 4);
                if(*aChunkType == 0x4E4F534A) {
                    aJsonBodyOffset = int64_t(aFile.tellg());
                    aJsonBodyLen    = int64_t(*aChunkLen);
                } else if(*aChunkType == 0x004E4942) {
                    myBinBodyOffset = int64_t(aFile.tellg());
                    myBinBodyLen    = int64_t(*aChunkLen);
                }
                if(*aChunkLen != 0) {
                    aFile.seekg(*aChunkLen, std::ios_base::cur);
                }
            }

            aFile.seekg(aJsonBodyOffset, std::ios_base::beg);
        }
    } else {
        aFile.seekg(0, std::ios_base::beg);
    }

    rapidjson::IStreamWrapper aFileStream(aFile);
    rapidjson::ParseResult aRes;
    if(myIsBinary) {
        aRes = ParseStream<rapidjson::kParseStopWhenDoneFlag, rapidjson::UTF8<>, rapidjson::IStreamWrapper>(aFileStream);
    } else {
        aRes = ParseStream(aFileStream);
    }

    if(aRes.IsError()) {
        if(aRes.Code() == rapidjson::kParseErrorDocumentEmpty) {
            signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' is empty!"));
            return false;
        }

        signals.onError(formatSyntaxError(myFileName, StString("File '") + theFile + "' defines invalid JSON document!\n"
                                                    + formatParseError(aRes.Code()) + "."));
        return false;
    }

    return gltfParse(theParentNode);
}

bool StAssetImportGltf::gltfParseRoots() {
    // find glTF root elements for smooth navigation
    GenericValue aNames[GltfRootElement_NB];
    for(int aRootNameIter = 0; aRootNameIter < GltfRootElement_NB; ++aRootNameIter) {
        aNames[aRootNameIter] = rapidjson::StringRef(GltfRootElementNames[aRootNameIter]);
    }

    for(ConstMemberIterator aRootIter = MemberBegin(); aRootIter != MemberEnd(); ++aRootIter) {
        for(int aRootNameIter = 0; aRootNameIter < GltfRootElement_NB; ++aRootNameIter) {
            if(myGltfRoots[aRootNameIter].isNull()
                 && aNames[aRootNameIter] == aRootIter->name) {
                // we will not modify JSON document, thus it is OK to keep the pointers
                myGltfRoots[aRootNameIter].init(GltfRootElementNames[aRootNameIter], &aRootIter->value);
                break;
            }
        }
    }

    for(int aRootNameIter = 0; aRootNameIter < GltfRootElement_NB_MANDATORY; ++aRootNameIter) {
        if(myGltfRoots[aRootNameIter].isNull()) {
            signals.onError(formatSyntaxError(myFileName, StString("Member '") + GltfRootElementNames[aRootNameIter] + "' is not found."));
            return false;
        }
    }
    return true;
}

void StAssetImportGltf::gltfParseAsset() {
    const GenericValue* anAsset = myGltfRoots[GltfRootElement_Asset].getRoot();
    if(anAsset == NULL) {
        return;
    }

    if(const GenericValue* aGenerator = findObjectMember(*anAsset, "generator")) {
        if(aGenerator->IsString()) {
            StString aStr = aGenerator->GetString();
        }
    }
    if(const GenericValue* aCopyRight = findObjectMember(*anAsset, "copyright")) {
        if(aCopyRight->IsString()) {
            StString aStr = aCopyRight->GetString();
        }
    }
}

void StAssetImportGltf::gltfParseMaterials() {
    const GenericValue* aMatList = myGltfRoots[GltfRootElement_Materials].getRoot();
    if(aMatList == NULL) {
        return;
    } else if(aMatList->IsObject()) {
        // glTF 1.0
        for(ConstMemberIterator aMatIter = aMatList->MemberBegin(); aMatIter != aMatList->MemberEnd(); ++aMatIter) {
            Handle(StGLMaterial) aMat = new StGLMaterial();
            const GenericValue& aMatNode = aMatIter->value;
            const GenericValue& aMatId   = aMatIter->name;
            const GenericValue* aNameVal = findObjectMember (aMatNode, "name");
            if(!gltfParseCommonMaterial(*aMat, aMatNode)) {
                if(!gltfParseStdMaterial(*aMat, aMatNode)) {
                    continue;
                }
            }

            if(aNameVal != NULL && aNameVal->IsString()) {
                aMat->Name = aNameVal->GetString();
            } else {
                aMat->Name = aMatId.GetString();
            }
            myMaterials.Bind(aMatId.GetString(), aMat);
        }
    } else if(aMatList->IsArray()) {
        // glTF 2.0
        int aMatIndex = 0;
        for(rapidjson::Value::ConstValueIterator aMatIter = aMatList->Begin(); aMatIter != aMatList->End(); ++aMatIter, ++aMatIndex) {
            Handle(StGLMaterial) aMat = new StGLMaterial();
            const GenericValue& aMatNode = *aMatIter;
            const GenericValue* aNameVal = findObjectMember(aMatNode, "name");
            if(!gltfParsePbrMaterial   (*aMat, aMatNode)
            && !gltfParseCommonMaterial(*aMat, aMatNode)
            && !gltfParseStdMaterial   (*aMat, aMatNode)) {
                continue;
            }

            if(aNameVal != NULL && aNameVal->IsString()) {
                aMat->Name = aNameVal->GetString();
            } else {
                aMat->Name = StString("mat_") + aMatIndex;
            }
            myMaterials.Bind(TCollection_AsciiString(aMatIndex), aMat);
        }
    }
}

bool StAssetImportGltf::gltfParseStdMaterial(StGLMaterial& theMat,
                                             const GenericValue& theMatNode) {
    const GenericValue* aTechVal = findObjectMember(theMatNode, "technique");
    const GenericValue* aValues  = findObjectMember(theMatNode, "values");
    if(aValues == NULL) {
        return false;
    }

    const GenericValue* anAmbVal = findObjectMember(*aValues, "ambient");
    const GenericValue* aDiffVal = findObjectMember(*aValues, "diffuse");
    const GenericValue* anEmiVal = findObjectMember(*aValues, "emission");
    const GenericValue* aSpecVal = findObjectMember(*aValues, "specular");
    const GenericValue* aShinVal = findObjectMember(*aValues, "shininess");
    if(anAmbVal == NULL
    && aDiffVal == NULL
    && anEmiVal == NULL
    && aSpecVal == NULL
    && aShinVal == NULL) {
        return false;
    }

    StGLVec4 anAmb, aDiff, anEmi, aSpec;
    if(anAmbVal != NULL && anAmbVal->IsString()) {
        // ambient texture
        gltfParseTexture(theMat, anAmbVal);
        theMat.AmbientColor = StGLVec4(1.0f, 1.0f, 1.0f, 1.0f);
    } else if(gltfReadVec4(anAmb, anAmbVal) && validateColor4(anAmb)) {
        theMat.AmbientColor = anAmb;
    }
    if(aDiffVal != NULL && aDiffVal->IsString()) {
        // diffuse texture
        gltfParseTexture(theMat, aDiffVal);
        theMat.DiffuseColor = StGLVec4(1.0f, 1.0f, 1.0f, 1.0f);
    } else if(gltfReadVec4(aDiff, aDiffVal) && validateColor4(aDiff)) {
        theMat.DiffuseColor = aDiff;
    }
    if(gltfReadVec4(anEmi, anEmiVal) && validateColor4(anEmi)) {
        theMat.EmissiveColor = anEmi;
    }
    if(gltfReadVec4(aSpec, aSpecVal) && validateColor4(aSpec)) {
        theMat.SpecularColor = aSpec;
    }
    if(aShinVal != NULL && aShinVal->IsNumber()) {
        const double aSpecular = aShinVal->GetDouble();
        if(aSpecular >= 0) {
            theMat.ChangeShine() = (float )stMin(aSpecular / 1000.0, 1.0);
        }
    }

    gltfParseTechnique(theMat, aTechVal);
    return true;
}

bool StAssetImportGltf::gltfParseCommonMaterial(StGLMaterial& theMat,
                                                const GenericValue& theMatNode) {
    const GenericValue* anExtVal = findObjectMember(theMatNode, "extensions");
    if(anExtVal == NULL) {
        return false;
    }

    const GenericValue* aMatCommon = findObjectMember(*anExtVal, THE_KHR_materials_common);
    if(aMatCommon == NULL) {
        return false;
    }

    if(!gltfParseStdMaterial(theMat, *aMatCommon)) {
        return false;
    }
    return true;
}

// =======================================================================
// function : gltfParsePbrMaterial
// purpose  :
// =======================================================================
bool StAssetImportGltf::gltfParsePbrMaterial(StGLMaterial& theMat,
                                             const GenericValue& theMatNode) {
    const GenericValue* aMetalRoughVal    = findObjectMember(theMatNode, "pbrMetallicRoughness");
    //const GenericValue* aNormTexVal       = findObjectMember(theMatNode, "normalTexture");
    const GenericValue* anEmissFactorVal  = findObjectMember(theMatNode, "emissiveFactor");
    //const GenericValue* anEmissTexVal     = findObjectMember(theMatNode, "emissiveTexture");
    //const GenericValue* anOcclusionTexVal = findObjectMember(theMatNode, "occlusionTexture");
    if(aMetalRoughVal == NULL) {
        return false;
    }

    /*if(const GenericValue* anExtVal = findObjectMember(theMatNode, "extensions")) {
        if(const GenericValue* anExtDefVal = findObjectMember(*anExtVal, "KHR_materials_pbrSpecularGlossiness")) {
            const GenericValue* aDiffTexVal = findObjectMember(*anExtDefVal, "diffuseTexture");
            const GenericValue* aSpecTexVal = findObjectMember(*anExtDefVal, "specularGlossinessTexture");
        }
    }*/

    const GenericValue* aBaseColorFactorVal = findObjectMember(*aMetalRoughVal, "baseColorFactor");
    const GenericValue* aBaseColorTexVal    = findObjectMember(*aMetalRoughVal, "baseColorTexture");
    //const GenericValue* aMetalRoughTexVal   = findObjectMember(*aMetalRoughVal, "metallicRoughnessTexture");

    StGLVec4 aBaseColorFactor;
    StGLVec3 anEmissiveFactor;
    if(gltfReadVec4(aBaseColorFactor, aBaseColorFactorVal) && validateColor4(aBaseColorFactor)) {
        //
    }
    if(gltfReadVec3(anEmissiveFactor, anEmissFactorVal) && validateColor3(anEmissiveFactor)) {
        //
    }
    if(aBaseColorTexVal != NULL && aBaseColorTexVal->IsObject()) {
        const GenericValue* aBaseColorTexIndexVal = findObjectMember(*aBaseColorTexVal, "index");
        if(aBaseColorTexIndexVal != NULL) {
            gltfParseTexture(theMat, aBaseColorTexIndexVal);
        }
    }

    return true;
}

bool StAssetImportGltf::gltfParseTechnique(StGLMaterial& theMat,
                                           const GenericValue* theTechniqueId) {
    // default values
    if(theTechniqueId == NULL) {
        return true;
    }

    const TCollection_AsciiString aTechId = getKeyString(*theTechniqueId);
    if(aTechId.IsEqual("PHONG")
    || aTechId.IsEqual("BLINN")
    || aTechId.IsEqual("LAMBERT")) {
        // KHR_materials_common extension - actually does NOT specify states!
        theMat.SetCullBackFaces(true);
        return true;
    }

    const GenericValue* aTechNode = myGltfRoots[GltfRootElement_Techniques].findChild(*theTechniqueId);
    if(aTechNode == NULL) {
        signals.onError(formatSyntaxError(myFileName, StString("Technique node '") + aTechId.ToCString() + "' is not found."));
        return false;
    }

    const GenericValue* aStatesVal = findObjectMember(*aTechNode, "states");
    if(aStatesVal != NULL) {
        const GenericValue* aStatesOnVal = findObjectMember(*aStatesVal, "enable");
        if(aStatesOnVal != NULL
        && aStatesOnVal->IsArray()) {
            theMat.SetCullBackFaces(false);
            const int aNbStates = aStatesOnVal->Size();
            for(int aStateIter = 0; aStateIter < aNbStates; ++aStateIter) {
                const GenericValue& aGenVal = (*aStatesOnVal)[aStateIter];
                if(!aGenVal.IsInt()) {
                    continue;
                }
                const int aStateId = aGenVal.GetInt();
                switch(aStateId) {
                    case 2884:  // CULL_FACE
                        theMat.SetCullBackFaces(true);
                        break;
                    case 3042:  // BLEND
                    case 2929:  // DEPTH_TEST
                    case 32823: // POLYGON_OFFSET_FILL
                    case 32926: // SAMPLE_ALPHA_TO_COVERAGE
                    case 3089:  // SCISSOR_TEST
                        break;
                }
            }
        }
    }

    return true;
}

bool StAssetImportGltf::gltfParseTexture(StGLMaterial& theMat,
                                         const GenericValue* theTextureId) {
    if(theTextureId == NULL
    || myGltfRoots[GltfRootElement_Textures].isNull()
    || myGltfRoots[GltfRootElement_Images].isNull()) {
        return false;
    }

    const StString aTextureId = getKeyString(*theTextureId).ToCString();
    const GenericValue* aTexNode = myGltfRoots[GltfRootElement_Textures].findChild(*theTextureId);
    if(aTexNode == NULL) {
        signals.onError(formatSyntaxError(myFileName, StString("Texture node '") + aTextureId + "' is not found."));
        return false;
    }

    const GenericValue* aSrcVal  = findObjectMember(*aTexNode, "source");
    const GenericValue* aTargVal = findObjectMember(*aTexNode, "target");
    if(aSrcVal == NULL) {
        signals.onError(formatSyntaxError(myFileName, StString("Invalid texture node '") + aTextureId + "' without a 'source' property."));
        return false;
    }
    if(aTargVal != NULL
    && aTargVal->IsNumber()
    && aTargVal->GetInt() != 3553) { // GL_TEXTURE_2D
        return false;
    }

    const GenericValue* anImgNode = myGltfRoots[GltfRootElement_Images].findChild(*aSrcVal);
    if(anImgNode == NULL) {
        signals.onError(formatSyntaxError(myFileName, StString("Invalid texture node '") + aTextureId
                                                    + "' points to non-existing image '" + getKeyString(*aSrcVal).ToCString() + "'."));
        return false;
    }

    const GenericValue* anUriVal = findObjectMember(*anImgNode, "uri");
    if(myIsBinary) {
        const GenericValue* aBinVal = anImgNode;
        if(const GenericValue* anExtVal = findObjectMember(*anImgNode, "extensions")) {
            const GenericValue* aBinValExt = findObjectMember(*anExtVal, THE_KHR_binary_glTF);
            if(aBinValExt != NULL) {
                aBinVal = aBinValExt;
            }
        }

        const GenericValue* aBufferViewName = findObjectMember(*aBinVal, "bufferView");
        const GenericValue* aMimeTypeVal    = findObjectMember(*aBinVal, "mimeType");
        //const GenericValue* aWidthVal       = findObjectMember(*aBinVal, "width");
        //const GenericValue* aHeightVal      = findObjectMember(*aBinVal, "height");
        if(aBufferViewName != NULL) {
            const StString aBufferViewNameStr(getKeyString(*aBufferViewName).ToCString());
            const GenericValue* aBufferView = myGltfRoots[GltfRootElement_BufferViews].findChild(*aBufferViewName);
            if(aBufferView == NULL || !aBufferView->IsObject()) {
                signals.onError(formatSyntaxError(myFileName, StString("Invalid texture node '") + aTextureId
                                                            + "' points to invalid buffer view '" + aBufferViewNameStr + "'."));
                return false;
            }

            const GenericValue* aBufferName = findObjectMember(*aBufferView, "buffer");
            const GenericValue* aByteLength = findObjectMember(*aBufferView, "byteLength");
            const GenericValue* aByteOffset = findObjectMember(*aBufferView, "byteOffset");
            if(aBufferName != NULL &&  aBufferName->IsString()
            && !IsEqual(aBufferName->GetString(), "binary_glTF")) {
                signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + aBufferViewNameStr + "' does not define binary_glTF buffer."));
                return false;
            } else if(aByteOffset == NULL || !aByteOffset->IsNumber()) {
                signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + aBufferViewNameStr + "' does not define byteOffset."));
                return false;
            }

            GltfBufferView aBuffView;
            aBuffView.ByteOffset = (int64_t )aByteOffset->GetDouble();
            aBuffView.ByteLength = aByteLength != NULL && aByteLength->IsNumber()
                                  ? (int64_t )aByteLength->GetDouble()
                                  : 0;
            if(aBuffView.ByteLength < 0
            || aBuffView.ByteLength > std::numeric_limits<int>::max()) {
                signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + aBufferViewNameStr + "' defines invalid byteLength."));
                return false;
            } else if(aBuffView.ByteOffset < 0) {
                signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + aBufferViewNameStr + "' defines invalid byteOffset."));
                return false;
            }

            const int64_t anOffset = myBinBodyOffset + aBuffView.ByteOffset;
            StString aMime;
            if(aMimeTypeVal != NULL && aMimeTypeVal->IsString()) {
                aMime = aMimeTypeVal->GetString();
            }
            theMat.Texture = new StGltfBinTexture(myFileName, aMime, anOffset, (int )aBuffView.ByteLength);
            return true;
        }
    }

    if(anUriVal == NULL || !anUriVal->IsString()) {
        return false;
    }

    const char* anUriData = anUriVal->GetString();
    if(::strncmp(anUriData, "data:", 5) == 0) { // data:image/png;base64,DATA
        const char* aDataStart = anUriData + 5;
        for(const char* aDataIter = aDataStart; aDataIter != '\0'; ++aDataIter) {
            if(stAreEqual(aDataIter, ";base64,", 8)) {
                const char* aBase64End  = anUriData + anUriVal->GetStringLength();
                const char* aBase64Data = aDataIter + 8;
                const int aBase64Len = int(aBase64End - aBase64Data);
                const StString aMime(aDataStart, aDataIter - aDataStart);
                Handle(NCollection_Buffer) aData = decodeBase64((const stUByte_t* )aBase64Data, aBase64Len);
                theMat.Texture = new StGltfBinTexture(myFileName + "@" + aSrcVal->GetString(), aMime, aData);
                return true;
            }
        }
        return false;
    }

    theMat.Texture = new StAssetTexture(myFolder + anUriVal->GetString());
    return true;
}

bool StAssetImportGltf::gltfParseScene(const Handle(StDocNode)& theParentNode) {
    // search default scene
    const GenericValue* aDefScene = myGltfRoots[GltfRootElement_Scenes].findChild(*myGltfRoots[GltfRootElement_Scene].getRoot());
    if(aDefScene == NULL) {
        signals.onError(formatSyntaxError(myFileName, "Default scene is not found."));
        return false;
    }

    const GenericValue* aSceneNodes = findObjectMember(*aDefScene, "nodes");
    if(aSceneNodes == NULL || !aSceneNodes->IsArray()) {
        signals.onError(formatSyntaxError(myFileName, StString() + "Empty scene '"
                                                    + getKeyString(*myGltfRoots[GltfRootElement_Scene].getRoot()).ToCString() + "'."));
        return false;
    }

    return gltfParseSceneNodes(theParentNode, *aSceneNodes);
}

bool StAssetImportGltf::gltfParseSceneNodes(const Handle(StDocNode)& theParentNode,
                                            const GenericValue& theSceneNodes) {
    if(!theSceneNodes.IsArray()) {
        signals.onError(formatSyntaxError(myFileName, "Scene nodes is not array."));
        return false;
    }

    for(rapidjson::Value::ConstValueIterator aSceneNodeIter = theSceneNodes.Begin();
        aSceneNodeIter != theSceneNodes.End(); ++aSceneNodeIter) {
        const GenericValue* aSceneNode = myGltfRoots[GltfRootElement_Nodes].findChild(*aSceneNodeIter);
        if(aSceneNode == NULL) {
            signals.onError(formatSyntaxError(myFileName, StString("Scene refers to non-existing node '") + getKeyString(*aSceneNodeIter).ToCString() + "'."));
            return true;
        }

        if(!gltfParseSceneNode(theParentNode, getKeyString(*aSceneNodeIter), *aSceneNode)) {
            return false;
        }
    }
    return true;
}

bool StAssetImportGltf::gltfParseSceneNode(const Handle(StDocNode)& theParentNode,
                                           const TCollection_AsciiString& theSceneNodeName,
                                           const GenericValue& theSceneNode) {
    const GenericValue* aName         = findObjectMember(theSceneNode, "name");
    //const GenericValue* aJointName    = findObjectMember(theSceneNode, "jointName");
    const GenericValue* aChildren     = findObjectMember(theSceneNode, "children");
    const GenericValue* aMeshes       = findObjectMember(theSceneNode, "meshes");
    const GenericValue* aMesh         = findObjectMember(theSceneNode, "mesh");
    //const GenericValue* aCamera       = findObjectMember(theSceneNode, "camera");
    const GenericValue* aTrsfMatVal   = findObjectMember(theSceneNode, "matrix");
    const GenericValue* aTrsfRotVal   = findObjectMember(theSceneNode, "rotation");
    const GenericValue* aTrsfScaleVal = findObjectMember(theSceneNode, "scale");
    const GenericValue* aTrsfTransVal = findObjectMember(theSceneNode, "translation");
    Handle(StDocObjectNode) aNewNode;
    if(mySceneNodeMap.Find(theSceneNodeName, aNewNode)) {
        theParentNode->ChangeChildren().Append(aNewNode);
        return true;
    }

    aNewNode = new StDocObjectNode();
    theParentNode->ChangeChildren().Append(aNewNode);
    if(aName != NULL && aName->IsString()) {
        aNewNode->setNodeName(aName->GetString());
    }
    mySceneNodeMap.Bind(theSceneNodeName, aNewNode);

    const bool hasTrs = aTrsfRotVal != NULL || aTrsfScaleVal != NULL || aTrsfTransVal != NULL;
    if(aTrsfMatVal != NULL) {
        if(hasTrs) {
            pushSceneNodeError(theSceneNodeName.ToCString(), "defines ambiguous transformation");
            return false;
        }
        else if(!aTrsfMatVal->IsArray() || aTrsfMatVal->Size() != 16) {
            pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid transformation matrix array");
            return false;
        }

        Graphic3d_Mat4d aMat4;
        for(int aColIter = 0; aColIter < 4; ++aColIter) {
            for(int aRowIter = 0; aRowIter < 4; ++aRowIter) {
                const GenericValue& aGenVal = (*aTrsfMatVal)[aColIter * 4 + aRowIter];
                if(!aGenVal.IsNumber()) {
                    pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid transformation matrix");
                    return false;
                }
                aMat4.SetValue(aRowIter, aColIter, aGenVal.GetDouble());
            }
        }

        if(!aMat4.IsIdentity()) {
            gp_Trsf aTrsf;
            aTrsf.SetValues(aMat4.GetValue(0, 0), aMat4.GetValue(0, 1), aMat4.GetValue(0, 2), aMat4.GetValue(0, 3),
                            aMat4.GetValue(1, 0), aMat4.GetValue(1, 1), aMat4.GetValue(1, 2), aMat4.GetValue(1, 3),
                            aMat4.GetValue(2, 0), aMat4.GetValue(2, 1), aMat4.GetValue(2, 2), aMat4.GetValue(2, 3));
            if(aTrsf.Form() != gp_Identity) {
                aNewNode->setNodeTransformation(aTrsf);
            }
        }
    } else if(hasTrs) {
        gp_Trsf aTrsf;
        if(aTrsfRotVal != NULL) {
            if(!aTrsfRotVal->IsArray() || aTrsfRotVal->Size() != 4) {
                pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid rotation quaternion");
                return false;
            }

            Graphic3d_Vec4d aRotVec4;
            for(int aCompIter = 0; aCompIter < 4; ++aCompIter) {
                const GenericValue& aGenVal = (*aTrsfRotVal)[aCompIter];
                if(!aGenVal.IsNumber()) {
                    pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid rotation");
                    return false;
                }
                aRotVec4[aCompIter] = aGenVal.GetDouble();
            }
            const gp_Quaternion aQuaternion(aRotVec4.x(), aRotVec4.y(), aRotVec4.z(), aRotVec4.w());
            if(Abs(aQuaternion.X())       > gp::Resolution()
            && Abs(aQuaternion.Y())       > gp::Resolution()
            && Abs(aQuaternion.Z())       > gp::Resolution()
            && Abs(aQuaternion.W() - 1.0) > gp::Resolution()) {
                aTrsf.SetRotation(aQuaternion);
            }
        }

        if(aTrsfTransVal != NULL) {
            if(!aTrsfTransVal->IsArray() || aTrsfTransVal->Size() != 3) {
                pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid translation vector");
                return false;
            }

            gp_XYZ aTransVec;
            for(int aCompIter = 0; aCompIter < 3; ++aCompIter) {
                const GenericValue& aGenVal = (*aTrsfTransVal)[aCompIter];
                if(!aGenVal.IsNumber()) {
                    pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid translation");
                    return false;
                }
                aTransVec.SetCoord(aCompIter + 1, aGenVal.GetDouble());
            }
            aTrsf.SetTranslationPart(aTransVec);
        }

        if(aTrsfScaleVal != NULL) {
            Graphic3d_Vec3d aScaleVec;
            if(!aTrsfScaleVal->IsArray() || aTrsfScaleVal->Size() != 3) {
                pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid scale vector");
                return false;
            }
            for(int aCompIter = 0; aCompIter < 3; ++aCompIter) {
                const GenericValue& aGenVal = (*aTrsfScaleVal)[aCompIter];
                if(!aGenVal.IsNumber()) {
                    pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid scale");
                    return false;
                }
                aScaleVec[aCompIter] = aGenVal.GetDouble();
                if(Abs(aScaleVec[aCompIter]) <= gp::Resolution()) {
                    pushSceneNodeError(theSceneNodeName.ToCString(), "defines invalid scale");
                    return false;
                }
            }
            if(Abs(aScaleVec.x() - aScaleVec.y()) > Precision::Confusion()
            && Abs(aScaleVec.y() - aScaleVec.z()) > Precision::Confusion()
            && Abs(aScaleVec.x() - aScaleVec.z()) > Precision::Confusion()) {
                ST_DEBUG_LOG("glTF reader, scene node '"
                            + theSceneNodeName.ToCString() + "' defines unsupported scaling "
                            + aScaleVec.x() + " " + aScaleVec.y() + " " + aScaleVec.z());
            }
            if(Abs (aScaleVec.x() - 1.0) > Precision::Confusion()) {
                aTrsf.SetScaleFactor(aScaleVec.x());
            }
        }

        if(aTrsf.Form() != gp_Identity) {
            aNewNode->setNodeTransformation(aTrsf);
        }
    }

    if(aChildren != NULL && !gltfParseSceneNodes(aNewNode, *aChildren)) {
        return false;
    }

    if(aMeshes != NULL && aMeshes->IsArray()) {
        for(rapidjson::Value::ConstValueIterator aMeshIter = aMeshes->Begin(); aMeshIter != aMeshes->End(); ++aMeshIter) {
            const GenericValue* aMeshItem = myGltfRoots[GltfRootElement_Meshes].findChild(*aMeshIter);
            if(aMeshItem == NULL) {
                pushSceneNodeError(theSceneNodeName.ToCString(), "refers to non-existing mesh");
                return false;
            }

            if(!gltfParseMesh(aNewNode, getKeyString(*aMeshIter), *aMeshItem)) {
                return false;
            }
        }
    }
    if(aMesh != NULL) {
        // glTF 2.0
        const GenericValue* aMeshItem = myGltfRoots[GltfRootElement_Meshes].findChild(*aMesh);
        if(aMeshItem == NULL) {
            pushSceneNodeError(theSceneNodeName.ToCString(), "refers to non-existing mesh");
            return false;
        }

        if(!gltfParseMesh(aNewNode, getKeyString(*aMesh), *aMeshItem)) {
            return false;
        }
    }

    return true;
}

bool StAssetImportGltf::gltfParseMesh(const Handle(StDocNode)& theParentNode,
                                      const TCollection_AsciiString& theMeshName,
                                      const GenericValue& theMesh) {
    const GenericValue* aName  = findObjectMember(theMesh, "name");
    const GenericValue* aPrims = findObjectMember(theMesh, "primitives");
    if(!aPrims->IsArray()) {
        signals.onError(formatSyntaxError(myFileName, StString("Primitive array attributes within Mesh '")
                                                             + theMeshName.ToCString() + "' is not an array."));
        return false;
    }

    Handle(StDocMeshNode) aMeshNode;
    if(myMeshMap.Find(theMeshName, aMeshNode)) {
        theParentNode->ChangeChildren().Append(aMeshNode);
        return true;
    }

    aMeshNode = new StDocMeshNode();
    myMeshMap.Bind(theMeshName, aMeshNode);
    if(aName != NULL && aName->IsString()) {
        aMeshNode->setNodeName(aName->GetString());
    }
    theParentNode->ChangeChildren().Append(aMeshNode);
    for(rapidjson::Value::ConstValueIterator aPrimArrIter = aPrims->Begin(); aPrimArrIter != aPrims->End(); ++aPrimArrIter) {
        if(!gltfParsePrimArray(aMeshNode, theMeshName, *aPrimArrIter)) {
            return false;
        }
    }
    return true;
}

bool StAssetImportGltf::gltfParsePrimArray(const Handle(StDocMeshNode)& theMeshNode,
                                           const TCollection_AsciiString& theMeshName,
                                           const GenericValue& thePrimArray) {
    const GenericValue* anAttribs = findObjectMember(thePrimArray, "attributes");
    const GenericValue* anIndices = findObjectMember(thePrimArray, "indices");
    const GenericValue* aMaterial = findObjectMember(thePrimArray, "material");
    const GenericValue* aModeVal  = findObjectMember(thePrimArray, "mode");
    GltfPrimitiveMode aMode = GltfPrimitiveMode_Triangles;
    if(anAttribs == NULL || !anAttribs->IsObject()) {
        signals.onError(formatSyntaxError(myFileName, StString("Primitive array within Mesh '")
                                                             + theMeshName.ToCString() + "' defines no attributes."));
        return false;
    }
    else if(aModeVal != NULL) {
        aMode = GltfPrimitiveMode_UNKNOWN;
        if(aModeVal->IsInt()) {
            aMode = (GltfPrimitiveMode )aModeVal->GetInt();
        }
        if(aMode < GltfPrimitiveMode_Points
        || aMode > GltfPrimitiveMode_TriangleFan) {
            signals.onError(formatSyntaxError(myFileName, StString("Primitive array within Mesh '")
                                                                 + theMeshName.ToCString() + "' has unknown mode."));
            return false;
        }
    }
    if(aMode != GltfPrimitiveMode_Triangles) {
        ST_DEBUG_LOG("Primitive array within Mesh '" + theMeshName.ToCString() + "' skipped due to unsupported mode.");
        return true;
    }

    Handle(StPrimArray) aPrimArray = new StPrimArray();
    theMeshNode->ChangePrimitiveArrays().Append(aPrimArray);
    if(aMaterial != NULL) {
        // assign material
        myMaterials.Find(getKeyString(*aMaterial), aPrimArray->Material);
    }

    bool hasPositions = false;
    for(rapidjson::Value::ConstMemberIterator anAttribIter = anAttribs->MemberBegin(); anAttribIter != anAttribs->MemberEnd(); ++anAttribIter) {
        const TCollection_AsciiString anAttribId = getKeyString(anAttribIter->value);
        if(anAttribId.IsEmpty()) {
            signals.onError(formatSyntaxError(myFileName, StString("Primitive array attribute accessor key within Mesh '")
                                                                 + theMeshName.ToCString() + "' is not a string."));
            return false;
        }

        GltfArrayType aType = gltfParseAttribType(anAttribIter->name.GetString());
        if(aType == GltfArrayType_UNKNOWN) {
            // just ignore unknown attributes
            continue;
        }

        const GenericValue* anAccessor = myGltfRoots[GltfRootElement_Accessors].findChild(anAttribIter->value);
        if(anAccessor == NULL || !anAccessor->IsObject()) {
            signals.onError(formatSyntaxError(myFileName, StString("Primitive array attribute accessor key '") + anAttribId.ToCString()
                                                                 + "' points to non-existing object."));
            return false;
        } else if(!gltfParseAccessor(aPrimArray, anAttribId, *anAccessor, aType, aMode)) {
            return false;
        } else if(aType == GltfArrayType_Position) {
            hasPositions = true;
        }
    }
    if(!hasPositions) {
        signals.onError(formatSyntaxError(myFileName, StString("Primitive array within Mesh '")
                                                             + theMeshName.ToCString() + "' does not define vertex positions."));
        return false;
    }

    if(anIndices != NULL) {
        const TCollection_AsciiString anIndicesId = getKeyString(*anIndices);
        if(anIndicesId.IsEmpty()) {
            signals.onError(formatSyntaxError(myFileName, StString("Primitive array indices accessor key within Mesh '")
                                                                 + theMeshName.ToCString() + "' is not a string."));
            return false;
        }

        const GenericValue* anAccessor = myGltfRoots[GltfRootElement_Accessors].findChild(*anIndices);
        if(anAccessor == NULL || !anAccessor->IsObject()) {
            signals.onError(formatSyntaxError(myFileName, StString("Primitive array indices accessor key '") + anIndicesId.ToCString()
                                                                + "' points to non-existing object."));
            return false;
        } else if(!gltfParseAccessor(aPrimArray, anIndicesId, *anAccessor, GltfArrayType_Indices, aMode)) {
            return false;
        }
    }

    return true;
}

bool StAssetImportGltf::gltfParseAccessor(const Handle(StPrimArray)& thePrimArray,
                                          const TCollection_AsciiString& theName,
                                          const GenericValue&     theAccessor,
                                          const GltfArrayType     theType,
                                          const GltfPrimitiveMode theMode) {
    GltfAccessor aStruct;
    const GenericValue* aTypeStr        = findObjectMember(theAccessor, "type");
    const GenericValue* aBufferViewName = findObjectMember(theAccessor, "bufferView");
    const GenericValue* aByteOffset     = findObjectMember(theAccessor, "byteOffset");
    const GenericValue* aByteStride     = findObjectMember(theAccessor, "byteStride");
    const GenericValue* aCompType       = findObjectMember(theAccessor, "componentType");
    const GenericValue* aCount          = findObjectMember(theAccessor, "count");
    if(aTypeStr == NULL || !aTypeStr->IsString()) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' does not define type."));
        return false;
    }
    aStruct.Type = gltfParseAccessorType(aTypeStr->GetString());
    if(aStruct.Type == GltfAccessorLayout_UNKNOWN) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' has invalid type."));
        return false;
    }

    if(aBufferViewName == NULL) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' does not define bufferView."));
        return false;
    }
    if(aByteOffset == NULL || !aByteOffset->IsNumber()) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' does not define byteOffset."));
        return false;
    }
    if(aCompType == NULL || !aCompType->IsInt()) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' does not define componentType."));
        return false;
    }
    aStruct.ComponentType = (GltfAccessorCompType )aCompType->GetInt();
    if(aStruct.ComponentType != GltfAccessorCompType_Int8
    && aStruct.ComponentType != GltfAccessorCompType_UInt8
    && aStruct.ComponentType != GltfAccessorCompType_Int16
    && aStruct.ComponentType != GltfAccessorCompType_UInt16
    && aStruct.ComponentType != GltfAccessorCompType_UInt32
    && aStruct.ComponentType != GltfAccessorCompType_Float32) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' defines invalid componentType value."));
        return false;
    }

    if(aCount == NULL || !aCount->IsNumber()) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' does not define count."));
        return false;
    }

    aStruct.ByteOffset = (int64_t )aByteOffset->GetDouble();
    aStruct.ByteStride = aByteStride != NULL && aByteStride->IsInt()
                       ? aByteStride->GetInt()
                       : 0;
    aStruct.Count = (int64_t )aCount->GetDouble();

    if(aStruct.ByteOffset < 0) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' defines invalid byteOffset."));
        return false;
    } else if(aStruct.ByteStride < 0
           || aStruct.ByteStride > 255) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' defines invalid byteStride."));
        return false;
    } else if(aStruct.Count < 1) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' defines invalid count."));
        return false;
    }

    //const GenericValue* aMax = findObjectMember(theAccessor, "max");
    //const GenericValue* aMin = findObjectMember(theAccessor, "min");
    const GenericValue* aBufferView = myGltfRoots[GltfRootElement_BufferViews].findChild(*aBufferViewName);
    if(aBufferView == NULL || !aBufferView->IsObject()) {
        signals.onError(formatSyntaxError(myFileName, StString("Accessor '") + theName.ToCString() + "' refers to non-existing bufferView."));
        return false;
    }

    return gltfParseBufferView(thePrimArray, getKeyString(*aBufferViewName), *aBufferView, aStruct, theType, theMode);
}

bool StAssetImportGltf::gltfParseBufferView(const Handle(StPrimArray)& thePrimArray,
                                            const TCollection_AsciiString& theName,
                                            const GenericValue&     theBufferView,
                                            const GltfAccessor&     theAccessor,
                                            const GltfArrayType     theType,
                                            const GltfPrimitiveMode theMode) {
    GltfBufferView aBuffView;
    const GenericValue* aBufferName = findObjectMember(theBufferView, "buffer");
    const GenericValue* aByteLength = findObjectMember(theBufferView, "byteLength");
    const GenericValue* aByteOffset = findObjectMember(theBufferView, "byteOffset");
    const GenericValue* aTarget     = findObjectMember(theBufferView, "target");
    if(aBufferName == NULL) {
        signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + theName.ToCString() + "' does not define buffer."));
        return false;
    } else if(aByteOffset == NULL || !aByteOffset->IsNumber()) {
        signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + theName.ToCString() + "' does not define byteOffset."));
        return false;
    }

    aBuffView.ByteOffset = (int64_t )aByteOffset->GetDouble();
    aBuffView.ByteLength = aByteLength != NULL && aByteLength->IsNumber()
                         ? (int64_t )aByteLength->GetDouble()
                         : 0;
    if(aTarget != NULL && aTarget->IsInt()) {
        aBuffView.Target = (GltfBufferViewTarget )aTarget->GetInt();
        if(aBuffView.Target != GltfBufferViewTarget_ARRAY_BUFFER
        && aBuffView.Target != GltfBufferViewTarget_ELEMENT_ARRAY_BUFFER) {
            signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + theName.ToCString() + "' defines invalid target."));
            return false;
        }
    }

    if(aBuffView.ByteLength < 0) {
        signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + theName.ToCString() + "' defines invalid byteLength."));
        return false;
    } else if(aBuffView.ByteOffset < 0) {
        signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + theName.ToCString() + "' defines invalid byteOffset."));
        return false;
    }

    const GenericValue* aBuffer = myGltfRoots[GltfRootElement_Buffers].findChild(*aBufferName);
    if(aBuffer == NULL || !aBuffer->IsObject()) {
        signals.onError(formatSyntaxError(myFileName, StString("BufferView '") + theName.ToCString() + "' refers to non-existing buffer."));
        return false;
    }

    return gltfParseBuffer(thePrimArray, getKeyString(*aBufferName), *aBuffer, theAccessor, aBuffView, theType, theMode);
}

bool StAssetImportGltf::gltfParseBuffer(const Handle(StPrimArray)& thePrimArray,
                                        const TCollection_AsciiString& theName,
                                        const GenericValue&     theBuffer,
                                        const GltfAccessor&     theAccessor,
                                        const GltfBufferView&   theView,
                                        const GltfArrayType     theType,
                                        const GltfPrimitiveMode theMode) {
    //const GenericValue* aType       = findObjectMember(theBuffer, "type");
    //const GenericValue* aByteLength = findObjectMember(theBuffer, "byteLength");
    const GenericValue* anUriVal      = findObjectMember(theBuffer, "uri");

    int64_t anOffset = theView.ByteOffset + theAccessor.ByteOffset;
    bool isBinary = false;
    if(myIsBinary) {
        isBinary = theName.IsEqual("binary_glTF") // glTF 1.0
                || anUriVal == NULL;              // glTF 2.0
    }

    if(isBinary) {
        std::ifstream aFile;
        OSD_OpenStream(aFile, myFileName.toCString(), std::ios::in | std::ios::binary);
        if(!aFile.is_open() || !aFile.good()) {
            signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' refers to non-existing file '" + myFileName + "'."));
            return false;
        }

        anOffset += myBinBodyOffset;
        aFile.seekg(anOffset, std::ios_base::beg);
        if(!aFile.good()) {
            aFile.close();
            signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' refers to non-existing location."));
            return false;
        }

        return gltfReadBuffer(thePrimArray, theName, theAccessor, aFile, theType, theMode);
    }

    if(anUriVal == NULL || !anUriVal->IsString()) {
        signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' does not define uri."));
        return false;
    }

    const char* anUriData = anUriVal->GetString();
    if(::strncmp(anUriData, "data:application/octet-stream;base64,", 37) == 0) {
        Handle(NCollection_Buffer) aData = decodeBase64((const stUByte_t* )anUriData + 37, anUriVal->GetStringLength() - 37);
        StArrayStreamBuffer aStreamBuffer((const char* )aData->Data(), aData->Size());
        std::istream aStream(&aStreamBuffer);
        aStream.seekg(anOffset, std::ios_base::beg);
        return gltfReadBuffer(thePrimArray, theName, theAccessor, aStream, theType, theMode);
    } else {
        StString anUri = anUriData;
        if(anUri.isEmpty()) {
            signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' does not define uri."));
            return false;
        }

        std::ifstream aFile;
        StString aPath = myFolder + anUri;
        OSD_OpenStream(aFile, aPath.toCString(), std::ios::in | std::ios::binary);
        if(!aFile.is_open() || !aFile.good()) {
            signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' refers to non-existing file '" + anUri + "'."));
            return false;
        }

        aFile.seekg(anOffset, std::ios_base::beg);
        if(!aFile.good()) {
            aFile.close();
            signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' refers to invalid location."));
            return false;
        }

        return gltfReadBuffer(thePrimArray, theName, theAccessor, aFile, theType, theMode);
    }
}

bool StAssetImportGltf::gltfReadBuffer(const Handle(StPrimArray)& thePrimArray,
                                       const TCollection_AsciiString& theName,
                                       const GltfAccessor&     theAccessor,
                                       std::istream&           theStream,
                                       const GltfArrayType     theType,
                                       const GltfPrimitiveMode theMode) {
    if(theMode != GltfPrimitiveMode_Triangles) {
        ST_DEBUG_LOG("Buffer '" + theName.ToCString() + "' skipped unsupported primitive array.");
        return true;
    }

    switch(theType) {
        case GltfArrayType_Indices: {
            if(theAccessor.Type != GltfAccessorLayout_Scalar
            || theAccessor.Count <= 0) {
                break;
            } else if((theAccessor.Count / 3) > std::numeric_limits<int>::max()) {
                signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' defines too big array."));
                return false;
            }

            const size_t aNbTris = size_t(theAccessor.Count / 3);
            if(theAccessor.ComponentType == GltfAccessorCompType_UInt16) {
                StVec3<uint16_t> aVec3_16u;
                const int aNbSkipBytes = theAccessor.ByteStride != 0
                                       ? (theAccessor.ByteStride - sizeof(uint16_t))
                                       : 0;
                thePrimArray->Indices.resize(aNbTris * 3);
                for(size_t aTriIter = 0; aTriIter < aNbTris; ++aTriIter) {
                    theStream.read((char* )&aVec3_16u[0], sizeof(uint16_t));
                    if(aNbSkipBytes != 0) {
                        theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                    }

                    theStream.read((char* )&aVec3_16u[1], sizeof(uint16_t));
                    if(aNbSkipBytes != 0) {
                        theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                    }

                    theStream.read((char* )&aVec3_16u[2], sizeof(uint16_t));
                    if(aNbSkipBytes != 0) {
                        theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                    }

                    if((size_t )aVec3_16u[0] >= thePrimArray->Positions.size()
                    || (size_t )aVec3_16u[1] >= thePrimArray->Positions.size()
                    || (size_t )aVec3_16u[2] >= thePrimArray->Positions.size()) {
                        signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' refers to invalid indices."));
                        return false;
                    }

                    thePrimArray->Indices[aTriIter * 3 + 0] = aVec3_16u[0];
                    thePrimArray->Indices[aTriIter * 3 + 1] = aVec3_16u[1];
                    thePrimArray->Indices[aTriIter * 3 + 2] = aVec3_16u[2];
                }
            } else if(theAccessor.ComponentType == GltfAccessorCompType_UInt32) {
                StVec3<uint32_t> aVec3_32u;
                const int aNbSkipBytes = theAccessor.ByteStride != 0
                                       ? (theAccessor.ByteStride - sizeof(uint32_t))
                                       : 0;
                thePrimArray->Indices.resize(aNbTris * 3);
                for(size_t aTriIter = 0; aTriIter < aNbTris; ++aTriIter) {
                    theStream.read((char* )&aVec3_32u[0], sizeof(uint32_t));
                    if(aNbSkipBytes != 0) {
                        theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                    }

                    theStream.read((char* )&aVec3_32u[1], sizeof(uint32_t));
                    if(aNbSkipBytes != 0) {
                        theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                    }

                    theStream.read((char* )&aVec3_32u[2], sizeof(uint32_t));
                    if(aNbSkipBytes != 0) {
                        theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                    }

                    if((size_t )aVec3_32u[0] >= thePrimArray->Positions.size()
                    || (size_t )aVec3_32u[1] >= thePrimArray->Positions.size()
                    || (size_t )aVec3_32u[2] >= thePrimArray->Positions.size()) {
                        signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' refers to invalid indices."));
                        return false;
                    }

                    thePrimArray->Indices[aTriIter * 3 + 0] = aVec3_32u[0];
                    thePrimArray->Indices[aTriIter * 3 + 1] = aVec3_32u[1];
                    thePrimArray->Indices[aTriIter * 3 + 2] = aVec3_32u[2];
                }
            } else {
                break;
            }
            break;
        }
        case GltfArrayType_Position: {
            if(theAccessor.ComponentType != GltfAccessorCompType_Float32
            || theAccessor.Type != GltfAccessorLayout_Vec3) {
                break;
            } else if(theAccessor.Count > std::numeric_limits<int>::max()) {
                signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' defines too big array."));
                return false;
            }

            const size_t aNbNodes = size_t(theAccessor.Count);
            StGLVec4 aVec4(0.0f, 0.0f, 0.0f, 1.0f);
            const int aNbSkipBytes = theAccessor.ByteStride != 0
                                   ? (theAccessor.ByteStride - sizeof(StGLVec3))
                                   : 0;
            thePrimArray->Positions.resize(aNbNodes);
            for(size_t aVertIter = 0; aVertIter < aNbNodes; ++aVertIter) {
                theStream.read((char* )aVec4.getData(), sizeof(StGLVec3));
                thePrimArray->Positions[aVertIter] = aVec4.xyz();
                if(aNbSkipBytes != 0) {
                    theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                }
            }

            break;
        }
        case GltfArrayType_Normal: {
            if(theAccessor.ComponentType != GltfAccessorCompType_Float32
            || theAccessor.Type != GltfAccessorLayout_Vec3) {
                break;
            } else if(theAccessor.Count > std::numeric_limits<int>::max()) {
                signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' defines too big array."));
                return false;
            }

            const size_t aNbNodes = size_t(theAccessor.Count);
            StGLVec4 aVec4(0.0f, 0.0f, 0.0f, 0.0f);
            const int aNbSkipBytes = theAccessor.ByteStride != 0
                                   ? (theAccessor.ByteStride - sizeof(StGLVec3))
                                   : 0;
            thePrimArray->Normals.resize(aNbNodes);
            for(size_t aVertIter = 0; aVertIter < aNbNodes; ++aVertIter) {
                theStream.read((char* )aVec4.getData(), sizeof(StGLVec3));
                thePrimArray->Normals[aVertIter] = aVec4.xyz();
                if(aNbSkipBytes != 0) {
                    theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                }
            }

            break;
        }
        case GltfArrayType_TCoord0: {
            if(theAccessor.ComponentType != GltfAccessorCompType_Float32
            || theAccessor.Type != GltfAccessorLayout_Vec2) {
                break;
            } else if(theAccessor.Count > std::numeric_limits<int>::max()) {
                signals.onError(formatSyntaxError(myFileName, StString("Buffer '") + theName.ToCString() + "' defines too big array."));
                return false;
            }

            const size_t aNbNodes = size_t(theAccessor.Count);
            StGLVec2 aVec2(0.0f, 0.0f);
            const int aNbSkipBytes = theAccessor.ByteStride != 0
                                   ? (theAccessor.ByteStride - sizeof(StGLVec2))
                                   : 0;
            thePrimArray->TexCoords0.resize(aNbNodes);
            for(size_t aVertIter = 0; aVertIter < aNbNodes; ++aVertIter) {
                theStream.read((char* )aVec2.getData(), sizeof(StGLVec2));

                thePrimArray->TexCoords0[aVertIter] = aVec2;
                if(aNbSkipBytes != 0) {
                    theStream.seekg(aNbSkipBytes, std::ios_base::cur);
                }
            }
            break;
        }
        case GltfArrayType_Color:
        case GltfArrayType_TCoord1:
        case GltfArrayType_Joint:
        case GltfArrayType_Weight: {
            return true;
        }
        case GltfArrayType_UNKNOWN: {
            return false;
        }
    }
    return true;
}
