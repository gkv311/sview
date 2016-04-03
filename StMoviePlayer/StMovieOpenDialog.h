/**
 * Copyright © 2016 Kirill Gavrilov <kirill@sview.ru>
 *
 * StMoviePlayer program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * StMoviePlayer program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __StMovieOpenDialog_h_
#define __StMovieOpenDialog_h_

#include <StStrings/StString.h>
#include <StThreads/StThread.h>
#include <StThreads/StMutex.h>

class StMoviePlayer;

/**
 * Auxiliary class to create standard non-blocking open file dialog in dedicated thread.
 */
class StMovieOpenDialog {

        public:

    enum DialogState {
        Dialog_Inactive,    //!< dialog is not opened
        Dialog_SingleMovie, //!< dialog is opened and waiting for user input (one video file)
        Dialog_DoubleMovie, //!< dialog is opened and waiting for user input (two video files)
        Dialog_Audio,       //!< dialog is opened and waiting for user input (audio file)
        Dialog_Subtitles,   //!< dialog is opened and waiting for user input (subtitles file)
        Dialog_HasFiles,    //!< dialog has been closed and waiting for processing results
    };

        public:

    /**
     * Main constructor.
     */
    ST_LOCAL StMovieOpenDialog(StMoviePlayer* thePlugin);

    /**
     * Destructor.
     */
    ST_LOCAL ~StMovieOpenDialog();

    /**
     * Create open file dialog.
     */
    ST_LOCAL bool openDialog(const StMovieOpenDialog::DialogState theMode);

    /**
     * Return true for Dialog_HasFiles state.
     */
    ST_LOCAL bool hasResults() {
        StMutexAuto aLock(myMutex);
        return myState == StMovieOpenDialog::Dialog_HasFiles;
    }

    /**
     * Reset results.
     */
    ST_LOCAL void resetResults();

    /**
     * Return path to the left file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathLeft()  const { return myPathVideoL; }

    /**
     * Return path to the right file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathRight() const { return myPathVideoR; }

    /**
     * Return path to the audio file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathAudio() const { return myPathAudio; }

    /**
     * Return path to the subtitles file.
     * Should NOT be called within Active state.
     */
    ST_LOCAL const StString& getPathSubtitles() const { return myPathSubs; }

    /**
     * Set paths to open.
     */
    ST_LOCAL void setPaths(const StString& thePathLeft,
                           const StString& thePathRight);

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

    StMoviePlayer*     myPlugin;
    StHandle<StThread> myThread;
    StMutex            myMutex;
    StString           myFolder;
    StString           myPathVideoL;
    StString           myPathVideoR;
    StString           myPathAudio;
    StString           myPathSubs;
    DialogState        myState;

};

#endif // __StMovieOpenDialog_h_
