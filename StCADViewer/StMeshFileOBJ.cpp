/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#include "StMeshFileOBJ.h"

#include <StStrings/StStringStream.h>
#include <StFile/StRawFile.h>
#include <StGLCore/StGLCore20.h>

StMeshFileOBJ::StMeshFileOBJ()
: myMesh() {
    //
}

static char* nextLine(char* theBuffer) {
    if(theBuffer == NULL) {
        return NULL;
    }
    char* aPtr = theBuffer;
    while(*aPtr != '\0' && *aPtr != '\n') {
        ++aPtr;
    }
    if(*aPtr == '\0') {
        return NULL;
    }
    return ++aPtr;
}

bool StMeshFileOBJ::load(const StString& theFilePath) {
    myMesh.nullify();

    // read the file into RAM
    StRawFile aFile(theFilePath);
    if(!aFile.readFile()) {
        // no file - no mesh
        return false;
    }

    // determine arrays lengths
    char* aBuffer = (char* )aFile.getBuffer();
    size_t aLen = aFile.getSize();
    size_t aVertNb  = 0;
    size_t aNormNb  = 0;
    size_t aFacesNb = 0;
    for(char* aLine = aBuffer; aLine != NULL; aLine = nextLine(aLine)) {
        if(aLine[0] == 'v' && aLine[1] == ' ') {
            // vertex
            ++aVertNb;
            aLine += 2;
        } else if(aLine[0] == 'v' && aLine[1] == 'n' && aLine[2] == ' ') {
            // normal
            ++aNormNb;
            aLine += 3;
        } else if(aLine[0] == 'f' && aLine[1] == ' ') {
            // face
            ++aFacesNb;
            aLine += 2;
        }
    }

ST_DEBUG_LOG("aVertNb= " + aVertNb + "; aFacesNb= " + aFacesNb + "; aNormNb= " + aNormNb);
    if(aVertNb == 0 || aFacesNb == 0) {
        // invalid or unsupported data
        return false;
    }

    // allocate arrays
    myMesh = new StGLMesh(GL_TRIANGLES);
    StArrayList<StGLVec3>& aVertices = myMesh->changeVertices();
    StArrayList<StGLVec3>& aNormals  = myMesh->changeNormals();
    StArrayList<GLuint>&   anIndices = myMesh->changeIndices();
    aVertices.initList(aVertNb);
    anIndices.initList(aFacesNb * 6); // OBJ files often contains quads, reserve additional memory for quads -> triangles splitting

    // temporary arrays
    StArrayList<StGLVec3> aNormalsTmp;
    StArrayList<GLuint> aNormalIndices;
    aNormalsTmp.initList(aNormNb);
    aNormalIndices.initList(anIndices.size());

    // temporary variables
    StGLVec3 aVert, aNorm;
    GLuint aVertIndex = 0;
    GLuint aNormIndex = 0;
    GLuint aTmpIndex1 = 0;
    GLuint aTmpIndex2 = 0;
    size_t anIndexSet = 0;
    size_t anIndexIter = 0;
    bool isTermination = false;

    // make sure floats are read using C locale
    StStringStream aStream;
    aStream.setCLocale();

    // fill arrays
    for(char* aLine = aBuffer; aLine != NULL; aLine = nextLine(aLine)) {
        if(aLine[0] == 'v' && aLine[1] == ' ') {
            // vertex
            aLine += 2;
            aStream.setInputBuffer(aLine, aLen - size_t(aLine - aBuffer));
            aStream >> aVert.x(); aStream >> aVert.y(); aStream >> aVert.z();
            aVertices.add(aVert);
        } else if(aLine[0] == 'v' && aLine[1] == 'n' && aLine[2] == ' ') {
            // normal
            aLine += 3;
            aStream.setInputBuffer(aLine, aLen - size_t(aLine - aBuffer));
            aStream >> aNorm.x(); aStream >> aNorm.y(); aStream >> aNorm.z();
            aNormalsTmp.add(aNorm);
        } else if(aLine[0] == 'f' && aLine[1] == ' ') {
            // face indices
            aLine += 2;
            char* anEnd = aLine;
            anIndexIter = 0;
            anIndexSet = 0;
            aVertIndex = 0;
            aNormIndex = 0;
            for(;; ++anEnd) {
                isTermination = *anEnd == '\0' || *anEnd == '\n';
                if(*anEnd != '/' && *anEnd != ' ' && !isTermination) {
                    continue;
                }
                ++anIndexIter;
                if(aLine != anEnd) {
                    aStream.setInputBuffer(aLine, size_t(anEnd - aLine));
                    switch(anIndexIter) {
                        case 1: {
                            // vertex index
                            aStream >> aVertIndex;
                            ++anIndexSet;
                            break;
                        }
                        case 2: {
                            // texture coordinates index
                            //anIndices.add(anIndex - 1);
                            break;
                        }
                        case 3: {
                            // normal index
                            aStream >> aNormIndex;
                            break;
                        }
                        default: break;
                    }
                    if(*anEnd != '/') {
                        if(aVertIndex == 0 || aVertIndex > aVertNb) {
                            // index out of bounds - ignore line
                            break;
                        }

                        // indices set ends
                        if(anIndexSet <= 3) {
                            // got triangle
                            anIndices.add(aVertIndex - 1); // in OBJ indices starts from 1
                            if(aNormIndex > 0 && aNormIndex <= aNormNb) {
                                aNormalIndices.add(aNormIndex - 1); // in OBJ indices starts from 1
                            }
                        } else if(anIndices.size() >= 3) {
                            // got polygon, split it to triangles
                            aTmpIndex1 = anIndices.getValue(anIndices.size() - 3);
                            aTmpIndex2 = anIndices.getLast();
                            anIndices.add(aTmpIndex1);
                            anIndices.add(aTmpIndex2);
                            anIndices.add(aVertIndex - 1); // in OBJ indices starts from 1
                            if(aNormIndex > 0 && aNormIndex <= aNormNb
                            && aNormalIndices.size() >= 3) {
                                aTmpIndex1 = aNormalIndices.getValue(aNormalIndices.size() - 3);
                                aTmpIndex2 = aNormalIndices.getLast();
                                aNormalIndices.add(aTmpIndex1);
                                aNormalIndices.add(aTmpIndex2);
                                aNormalIndices.add(aNormIndex - 1); // in OBJ indices starts from 1
                            }
                        }
                        anIndexIter = 0;
                    }
                }
                if(isTermination) {
                    break;
                }
                aLine = anEnd + 1;
            }
        }
    }

    if(aNormalIndices.size() == anIndices.size() && !aNormalsTmp.isEmpty()) {
        // move collected normals to the mesh
        StArray<bool> anIndMap(aVertices.size());
        stMemSet(&anIndMap.changeFirst(), false, sizeof(bool) * aVertices.size());
        aNormals.initArray(aVertices.size());
        size_t aNormIndex = 0;
        size_t aVertIndex = 0;
        for(size_t anIndexId = 0; anIndexId < aNormalIndices.size(); ++anIndexId) {
            aVertIndex = anIndices[anIndexId];
            if(!anIndMap[aVertIndex]) {
                aNormIndex = aNormalIndices[anIndexId];
                aNormals.changeValue(aVertIndex) = aNormalsTmp[aNormIndex];
                anIndMap[aVertIndex] = true;
            }
        }
    } else {
        // compute coarse normals
        myMesh->computeNormals();
    }

    // compute bounding sphere
    myMesh->computeMesh();

    return true;
}
