/**
 * Copyright © 2010-2013 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __APPLE__

#include <StFile/StFileNode.h>
#include <StStrings/StLogger.h>

#ifdef _WIN32
    #include <windows.h>
#elif defined(__ANDROID__)
    //
#elif defined(__linux__)
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
    #include <gtk/gtk.h>
#endif

/**
 * We put this function to external cpp file
 * to cut additional dependences for rarely used function
 * in commonly used class.
 * This needed only for Windows.
 */
bool StFileNode::openFileDialog(StString& theFilePath,
                                const StOpenFileName& theInfo,
                                bool theToSave) {
#ifdef _WIN32
    const StStringUtfWide aFolder = theInfo.Folder.toUtfWide();
    const StStringUtfWide aTitle  = theInfo.Title.toUtfWide();

    // use dummy \n symbol instead of NULL-termination symbol \0 inside string
    // because such string will be invalid for StString class
    const StStringUtfWide NULL_CHAR = '\n';

    StStringUtfWide aFilterString, anAllSupportedExt, anExtraSupportedExt;
    for(size_t aMimeId = 0; aMimeId < theInfo.Filter.size(); ++aMimeId) {
        const StMIME& aMime = theInfo.Filter[aMimeId];
        if(aMimeId > 0) {
            anAllSupportedExt += StStringUtfWide(';');
        }
        anAllSupportedExt += StStringUtfWide(L"*.") + aMime.getExtension().toUtfWide();
    }

    // fill 'All supported'
    if(!anAllSupportedExt.isEmpty() && theInfo.Filter.size() > 1) {
        if(!theInfo.FilterTitle.isEmpty()) {
            aFilterString += theInfo.FilterTitle.toUtfWide();
        } else {
            aFilterString += StStringUtfWide(L"All supported");
        }
        aFilterString += StStringUtfWide(L"\n");
        aFilterString += anAllSupportedExt + NULL_CHAR;
    }

    // fill 'Extra supported'
    for(size_t aMimeId = 0; aMimeId < theInfo.ExtraFilter.size(); ++aMimeId) {
        const StMIME& aMime = theInfo.ExtraFilter[aMimeId];
        if(aMimeId > 0) {
            anExtraSupportedExt += StStringUtfWide(';');
        }
        anExtraSupportedExt += StStringUtfWide(L"*.") + aMime.getExtension().toUtfWide();
    }
    if(!anExtraSupportedExt.isEmpty() && theInfo.ExtraFilter.size() > 1) {
        if(!theInfo.ExtraFilterTitle.isEmpty()) {
            aFilterString += theInfo.ExtraFilterTitle.toUtfWide();
        } else {
            aFilterString += StStringUtfWide(L"Extra supported");
        }
        aFilterString += StStringUtfWide(L"\n");
        aFilterString += anExtraSupportedExt + NULL_CHAR;
    }

    // fill MIME types
    for(size_t aMimeId = 0; aMimeId < theInfo.Filter.size(); ++aMimeId) {
        const StMIME& aMime = theInfo.Filter[aMimeId];
        if((aMimeId > 0) && (aMime.getDescription() == theInfo.Filter[aMimeId - 1].getDescription())) {
            // append extension to previous MIME (prevent duplication)
            aFilterString = aFilterString.subString(0, aFilterString.getLength() - 1); // backstep
            aFilterString += StStringUtfWide(L";*.") + aMime.getExtension().toUtfWide() + NULL_CHAR;
        } else {
            aFilterString += aMime.getDescription().toUtfWide() + NULL_CHAR;
            aFilterString += StStringUtfWide(L"*.")  + aMime.getExtension().toUtfWide() + NULL_CHAR;
        }
    }

    // fill 'Any File'
    aFilterString += StStringUtfWide(L"All Files (*)\n");
    aFilterString += StStringUtfWide(L"*\n"); // last string should be terminated by \0\0

    // replace dummy CR symbol within '\0' using 'hack' code
    stUtfWide_t* aBuffer;
    for(StUtfWideIter anIter = aFilterString.iterator(); *anIter != 0;) {
        aBuffer = (stUtfWide_t* )anIter.getBufferHere();
        ++anIter;
        if(*aBuffer == L'\n') {
            *aBuffer = L'\0';
        }
    }

    stUtfWide_t aFilePath [4096]; aFilePath[0]  = L'\0';
    stUtfWide_t aFileTitle[4096]; aFileTitle[0] = L'\0';
    StStringUtfWide aFilePathIn(theFilePath.toCString());
    if(!aFilePathIn.isEmpty()
     && aFilePathIn.getSize() < 4096) {
        stMemCpy(aFilePath, aFilePathIn.toCString(), (aFilePathIn.getSize() + 1) * sizeof(wchar_t));
    }
    OPENFILENAMEW anOpenStruct; stMemSet(&anOpenStruct, 0, sizeof(OPENFILENAMEW));
    anOpenStruct.lStructSize     = sizeof(OPENFILENAMEW);
    anOpenStruct.hwndOwner       = NULL;
    anOpenStruct.lpstrFilter     = aFilterString.toCString();
    anOpenStruct.lpstrInitialDir = aFolder.toCString();
    anOpenStruct.nFilterIndex    = 1;
    anOpenStruct.lpstrFile       = aFilePath;
    anOpenStruct.nMaxFile        = sizeof(aFilePath);
    anOpenStruct.lpstrFileTitle  = aFileTitle;
    anOpenStruct.nMaxFileTitle   = sizeof(aFileTitle);
    anOpenStruct.lpstrTitle      = aTitle.toCString();
    anOpenStruct.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    if(!theToSave) {
        if(GetOpenFileNameW(&anOpenStruct)
        && *anOpenStruct.lpstrFile != L'\0') {
            theFilePath = StString(anOpenStruct.lpstrFile);
            return true;
        }
    } else if(GetSaveFileNameW(&anOpenStruct)
           && *anOpenStruct.lpstrFile != L'\0') {
        theFilePath = StString(anOpenStruct.lpstrFile);
        return true;
    }
    return false;
#elif defined(__ANDROID__)
    //bool ST_NOT_IMPLEMENTED_FOR_ANDROID = true;
    return false;
#elif defined(__linux__)
    if(!StMessageBox::initGlobals()) {
        return false;
    }
    gdk_threads_enter();
    bool isFileSelected = false;
    GtkWidget* aDialog = gtk_file_chooser_dialog_new(theInfo.Folder.toCString(), NULL, (theToSave ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN),
                                                     GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, (theToSave ? GTK_STOCK_SAVE : GTK_STOCK_OPEN),
                                                     GTK_RESPONSE_ACCEPT, NULL);
    gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(aDialog), theInfo.Folder.toCString());
    if(!theFilePath.isEmpty()) {
        gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(aDialog), theFilePath.toCString());
    }

    GtkFileFilter* gtkFilter = gtk_file_filter_new();
    for(size_t aMimeId = 0; aMimeId < theInfo.Filter.size(); ++aMimeId) {
        const StMIME& aMime = theInfo.Filter[aMimeId];
        gtk_file_filter_add_pattern(gtkFilter, (StString("*.") + aMime.getExtension()).toCString());
    }

    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(aDialog), gtkFilter);
    if(gtk_dialog_run(GTK_DIALOG(aDialog)) == GTK_RESPONSE_ACCEPT) {
        char* aFileName = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(aDialog));
        theFilePath = StString(aFileName);
        g_free(aFileName);
        isFileSelected = true;
    }
    gtk_widget_destroy(aDialog);
    gdk_flush(); // we need this call!
    gdk_threads_leave();
    return isFileSelected;
#endif
}

#endif // __APPLE__
