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

#include "StImageOpenDialog.h"

#include "StImageViewer.h"
#include "StImageViewerStrings.h"

StImageOpenDialog::StImageOpenDialog(StImageViewer* thePlugin)
: myPlugin(thePlugin),
  myState(StImageOpenDialog::Dialog_Inactive) {
    //
}

StImageOpenDialog::~StImageOpenDialog() {
    if(!myThread.isNull()) {
        myThread->wait();
    }
}

bool StImageOpenDialog::openDialog(const size_t theNbFiles) {
    StMutexAuto aLock(myMutex);
    if(myState != StImageOpenDialog::Dialog_Inactive) {
        return false;
    }

    if(myPlugin->params.lastFolder.isEmpty()) {
        StHandle<StFileNode> aCurrFile = myPlugin->myPlayList->getCurrentFile();
        if(!aCurrFile.isNull()) {
            myPlugin->params.lastFolder = aCurrFile->isEmpty() ? aCurrFile->getFolderPath() : aCurrFile->getValue(0)->getFolderPath();
        }
    }

    myFolder = myPlugin->params.lastFolder;
    myState  = theNbFiles == 2 ? StImageOpenDialog::Dialog_ActiveDouble : StImageOpenDialog::Dialog_ActiveSingle;
    myThread = new StThread(openDialogThread, this);
    return true;
}

void StImageOpenDialog::resetResults() {
    StMutexAuto aLock(myMutex);
    if(myState != StImageOpenDialog::Dialog_HasFiles) {
        return;
    }

    if(!myThread.isNull()) {
        myThread->wait();
        myThread.nullify();
    }

    myState = Dialog_Inactive;
    myPathLeft .clear();
    myPathRight.clear();
}

void StImageOpenDialog::setPaths(const StString& thePathLeft,
                                 const StString& thePathRight) {
    StMutexAuto aLock(myMutex);
    if(myState != StImageOpenDialog::Dialog_Inactive) {
        return;
    }

    myPathLeft  = thePathLeft;
    myPathRight = thePathRight;
    if(!myPathLeft.isEmpty()) {
        myState = StImageOpenDialog::Dialog_HasFiles;
    }
}

SV_THREAD_FUNCTION StImageOpenDialog::openDialogThread(void* theArg) {
    StImageOpenDialog* aHandler = (StImageOpenDialog* )theArg;
    aHandler->dialogLoop();
    return SV_THREAD_RETURN 0;
}

void StImageOpenDialog::dialogLoop() {
    myPathLeft .clear();
    myPathRight.clear();
    StString aTitle = myPlugin->tr(myState == StImageOpenDialog::Dialog_ActiveDouble
                                 ? StImageViewerStrings::DIALOG_OPEN_LEFT
                                 : StImageViewerStrings::DIALOG_OPEN_FILE);

    StString aDummy;
    if(!StFileNode::openFileDialog(myFolder, aTitle, myPlugin->myLoader->getMimeListImages(), myPathLeft, false)) {
        StMutexAuto aLock(myMutex);
        myState = StImageOpenDialog::Dialog_Inactive;
        return;
    } else if(myState == StImageOpenDialog::Dialog_ActiveDouble) {
        aTitle = myPlugin->tr(StImageViewerStrings::DIALOG_OPEN_RIGHT);
        StFileNode::getFolderAndFile(myPathLeft, myFolder, aDummy);
        if(!StFileNode::openFileDialog(myFolder, aTitle, myPlugin->myLoader->getMimeListImages(), myPathRight, false)) {
            StMutexAuto aLock(myMutex);
            myState = StImageOpenDialog::Dialog_Inactive;
            return;
        }
    }

    StMutexAuto aLock(myMutex);
    myState = StImageOpenDialog::Dialog_HasFiles;
}
