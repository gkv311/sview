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

#include "StMovieOpenDialog.h"

#include "StMoviePlayer.h"
#include "StMoviePlayerStrings.h"
#include "StVideo/StVideo.h"

#include <StGL/StPlayList.h>

StMovieOpenDialog::StMovieOpenDialog(StMoviePlayer* thePlugin)
: myPlugin(thePlugin),
  myState(StMovieOpenDialog::Dialog_Inactive) {
    //
}

StMovieOpenDialog::~StMovieOpenDialog() {
    if(!myThread.isNull()) {
        myThread->wait();
    }
}

bool StMovieOpenDialog::openDialog(const StMovieOpenDialog::DialogState theMode) {
    StMutexAuto aLock(myMutex);
    if(myState != StMovieOpenDialog::Dialog_Inactive) {
        return false;
    }

    if(myPlugin->params.lastFolder.isEmpty()) {
        StHandle<StFileNode> aCurrFile = myPlugin->myPlayList->getCurrentFile();
        if(!aCurrFile.isNull()) {
            myPlugin->params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }

    myFolder = myPlugin->params.lastFolder;
    myState  = theMode;
    myThread = new StThread(openDialogThread, this);
    return true;
}

void StMovieOpenDialog::resetResults() {
    StMutexAuto aLock(myMutex);
    if(myState != StMovieOpenDialog::Dialog_HasFiles) {
        return;
    }

    if(!myThread.isNull()) {
        myThread->wait();
        myThread.nullify();
    }

    myState = Dialog_Inactive;
    myPathVideoL.clear();
    myPathVideoR.clear();
    myPathAudio .clear();
    myPathSubs  .clear();
}

void StMovieOpenDialog::setPaths(const StString& thePathLeft,
                                 const StString& thePathRight) {
    StMutexAuto aLock(myMutex);
    if(myState != StMovieOpenDialog::Dialog_Inactive) {
        return;
    }

    myPathVideoL = thePathLeft;
    myPathVideoR = thePathRight;
    myPathAudio.clear();
    myPathSubs .clear();
    if(!myPathVideoL.isEmpty()) {
        myState = StMovieOpenDialog::Dialog_HasFiles;
    }
}

SV_THREAD_FUNCTION StMovieOpenDialog::openDialogThread(void* theArg) {
    StMovieOpenDialog* aHandler = (StMovieOpenDialog* )theArg;
    aHandler->dialogLoop();
    return SV_THREAD_RETURN 0;
}

void StMovieOpenDialog::dialogLoop() {
    myPathVideoL.clear();
    myPathVideoR.clear();
    myPathAudio .clear();
    myPathSubs  .clear();

    StString aTitle;
    const StMIMEList* aMimeList = &myPlugin->myVideo->getMimeListVideo();
    switch(myState) {
        case Dialog_DoubleMovie:
            aTitle = myPlugin->tr(StMoviePlayerStrings::DIALOG_OPEN_LEFT);
            break;
        case Dialog_Audio:
            aTitle    = "Choose audio file to attach";
            aMimeList = &myPlugin->myVideo->getMimeListAudio();
            break;
        case Dialog_Subtitles:
            aTitle    = "Choose subtitles file to attach";
            aMimeList = &myPlugin->myVideo->getMimeListSubtitles();
            break;
        case Dialog_SingleMovie:
        default:
            aTitle = myPlugin->tr(StMoviePlayerStrings::DIALOG_OPEN_FILE);
            break;
    }

    StString aFilePath, aDummy;
    if(!StFileNode::openFileDialog(myFolder, aTitle, *aMimeList, aFilePath, false)) {
        StMutexAuto aLock(myMutex);
        myState = StMovieOpenDialog::Dialog_Inactive;
        return;
    }

    switch(myState) {
        case Dialog_DoubleMovie: {
            aTitle =  myPlugin->tr(StMoviePlayerStrings::DIALOG_OPEN_RIGHT);
            StFileNode::getFolderAndFile(aFilePath, myFolder, aDummy);
            myPathVideoL = aFilePath;
            if(!StFileNode::openFileDialog(myFolder, aTitle, *aMimeList, myPathVideoR, false)) {
                StMutexAuto aLock(myMutex);
                myState = StMovieOpenDialog::Dialog_Inactive;
                return;
            }
            break;
        }
        case Dialog_Audio: {
            myPathAudio = aFilePath;
            break;
        }
        case Dialog_Subtitles: {
            myPathSubs = aFilePath;
            break;
        }
        case Dialog_SingleMovie:
        default: {
            myPathVideoL = aFilePath;
            break;
        }
    }

    StMutexAuto aLock(myMutex);
    myState = StMovieOpenDialog::Dialog_HasFiles;
}
