/**
 * Copyright © 2010-2015 Kirill Gavrilov
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StTranslations_h_
#define __StTranslations_h_

#include <StStrings/StLangMap.h>
#include <StThreads/StResourceManager.h>
#include <StSettings/StEnumParam.h>

/**
 * This class unify access to the translations for all modules.
 * It is automatically uses StSettings library and global settings section
 * to store / restore global language settings.
 */
class StTranslations : public StLangMap {

        public:

    /**
     * Main constructor.
     * @param theResMgr     file resources manager
     * @param theModuleName module name, the subfolder where translations files should be placed
     */
    ST_CPPEXPORT StTranslations(const StHandle<StResourceManager>& theResMgr,
                                const StString&                    theModuleName);

    /**
     * Destructor.
     */
    ST_CPPEXPORT virtual ~StTranslations();

    /**
     * Reload translation.
     */
    ST_CPPEXPORT void reload();

    /**
     * Returns active translation.
     */
    ST_CPPEXPORT const StString& getLanguage() const;

    /**
     * Returns 3-letter language code of active translation.
     */
    ST_LOCAL const StString& getLanguageCode() const {
        return myLangCode;
    }

    /**
     * Return list of available translations.
     */
    const StArrayList<StString>& getLanguagesList() const {
        return params.language->getValues();
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

        public: //! @name Properties

    struct {

        StHandle<StEnumParam> language; //!< language id in available translations list

    } params;

        private: //! @name private callback slots

    /**
     * Changes translation.
     */
    ST_LOCAL void setLanguage(const int32_t theNewLang);

    ST_LOCAL void updateLangCode(const int32_t theNewLang);

        private:

    ST_LOCAL static const StString DEFAULT_EXTENSION;
    ST_LOCAL static const StString DEFAULT_SUFFIX;

        private:

    StHandle<StResourceManager>
                          myResMgr;         //!< file resource manager
    StString              myModuleName;     //!< module name like 'StImageViewer'
    StArrayList<StString> myLangFolderList; //!< translation files
    StString              myLangCode;       //!< active language code
    bool                  myWasReloaded;    //!< flag indicates that translation was reloaded

};

// define StHandle template specialization
ST_DEFINE_HANDLE(StTranslations, StLangMap);

#endif //__StTranslations_h_
