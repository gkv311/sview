/**
 * Copyright © 2010-2013 Kirill Gavrilov
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file license-boost.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

#ifndef __StTranslations_h_
#define __StTranslations_h_

#include <StStrings/StLangMap.h>
#include <StThreads/StProcess.h>
#include <StFile/StFolder.h>
#include <StSettings/StSettings.h>

/**
 * This class unify access to the translations for all modules.
 * It is automatically uses StSettings library and global settings section
 * to store / restore global language settings.
 */
class StTranslations : public StLangMap {

        public:

    /**
     * Main constructor.
     * @param theModuleName (const StString& ) - module name, the subfolder where translations files should be placed.
     */
    ST_CPPEXPORT StTranslations(const StString& theModuleName);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StTranslations();

    /**
     * Returns active translation.
     */
    ST_CPPEXPORT StString getLanguage() const;

    /**
     * Return list of available translations.
     */
    const StArrayList<StString>& getLanguagesList() const {
        return myLangList;
    }

    /**
     * Returns true if language was changed from last initialization.
     * Means that GUI elements should update the text.
     * Call resetReloaded() method when change was processed.
     */
    bool wasReloaded() const {
        return myWasReloaded;
    }

    /**
     * Reset reloaded flag.
     */
    void resetReloaded() {
        myWasReloaded = false;
    }

        public: //!< Properties

    struct {

        StHandle<StInt32Param> language; //!< language id in available translations list

    } params;

        private: //!< private callback slots

    /**
     * Changes translation.
     */
    ST_LOCAL void setLanguage(const int32_t theNewLang);

        private:

    StString              myModuleName;  //!< module name like 'StImageViewer'
    StArrayList<StString> myLangList;    //!< available (found) translations
    bool                  myWasReloaded; //!< flag indicates that translation was reloaded

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StTranslations, StLangMap);

#endif //__StTranslations_h_
