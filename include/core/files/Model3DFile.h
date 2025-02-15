/*
 * Model3DFile.hpp
 *
 *  Created on: 21 апр. 2017 г.
 *      Author: sadko
 */

#ifndef CORE_FILES_MODEL3DFILE_H_
#define CORE_FILES_MODEL3DFILE_H_

#include <core/types.h>
#include <core/status.h>
#include <core/3d/Scene3D.h>

namespace lsp
{
    /** Model file used to load 3D objects
     *
     */
    class Model3DFile
    {
        public:
            Model3DFile();
            ~Model3DFile();

        public:
            /** Create new scene and load file contents to the scene
             *
             * @param scene pointer to store loaded scene
             * @param path location of the file
             * @return status of the operation
             */
            static status_t load(Scene3D **scene, const char *path);

            /** Load file to the passed scene
             *
             * @param scene scene to store contents
             * @param path location of the file
             * @param clear issue clear() on the scene
             * @return status of the operation
             */
            static status_t load(Scene3D *scene, const char *path, bool clear);
    };

} /* namespace lsp */

#endif /* CORE_FILES_MODEL3DFILE_H_ */
