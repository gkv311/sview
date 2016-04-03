/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * StImageViewer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StImageViewer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StImageOpenDialog_h_
#define __StImageOpenDialog_h_

#include <StStrings/StString.h>
#include <StThreads/StThread.h>
#include <StThreads/StMutex.h>

class StImageViewer;

/**
 * Auxiliary class to create standard non-blocking open file dialog in dedicated thread.
 */
class StImageOpenDialog {

        public:

    enum DialogState {
        Dialog_Inactive,     //!< dialog is not opened
        Dialog_ActiveSingle, //!< dialog is opened and waiting for user input (one file)
        Dialog_ActiveDouble, //!< dialog is opened and waiting for user input (two files)
        Dialog_HasFiles,     //!< dialog has been closed and waiting for processing results
    };

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StImageOpenDialog(StImageViewer* thePlugin);

    /**
     * Destructor.
     */
    ST_LOCAL ~StImageOpenDialog();

    /**
     * Create open file dialog.
     */
    ST_LOCAL bool openDialog(const size_t theNbFiles);

    /**
     * Return true for Dialog_HasFiles state.
     */
    ST_LOCAL bool hasResults() {
        StMutexAuto aLock(myMutex);
        return myState == StImageOpenDialog::Dialog_HasFiles;
    }

    /**
     * Reset results.
     */
    ST_LOCAL void resetResults();

    /**
     * Return path to the left file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathLeft()  const { return myPathLeft; }

    /**
     * Set paths to open.
     */
    ST_LOCAL void setPaths(const StString& thePathLeft,
                           const StString& thePathRight);

    /**
     * Return path to the right file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathRight() const { return myPathRight; }

        private:

    /**
     * Thread function wrapper.
     */
    static SV_THREAD_FUNCTION openDialogThread(void* theArg);

    /**
     * Thread function.
     */
    ST_LOCAL void dialogLoop();

        private:

    StImageViewer*     myPlugin;
    StHandle<StThread> myThread;
    StMutex            myMutex;
    StString           myFolder;
    StString           myPathLeft;
    StString           myPathRight;
    DialogState        myState;

};

#endif // __StImageOpenDialog_h_
