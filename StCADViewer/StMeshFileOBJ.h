/**
 * This source is a part of sView program.
 *
 * Copyright © Kirill Gavrilov, 2011-2013
 */

#ifndef __StMeshFileOBJ_h_
#define __StMeshFileOBJ_h_

#include <StTemplates/StHandle.h>
#include <StGLMesh/StGLMesh.h>

class ST_LOCAL StMeshFileOBJ {

        private:

    StHandle<StGLMesh> myMesh;

        public:

    /**
     * Empty constructor.
     */
    StMeshFileOBJ();

    /**
     * Read the mesh from OBJ file.
     */
    bool load(const StString& theFilePath);

    /**
     * Return imported mesh.
     */
    const StHandle<StGLMesh>& getResult() const {
        return myMesh;
    }

};

#endif //__StMeshFileOBJ_h_
