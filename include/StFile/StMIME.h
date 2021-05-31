/**
 * Copyright © 2009-2010 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StMIME_h__
#define __StMIME_h__

#include <StStrings/StString.h>
#include <StTemplates/StArrayList.h>

/**
 * MIME description storage class.
 */
class StMIME {

        private:

    StString mime; // mime type
    StString  ext; // extension
    StString desc; // type description

        public:

    /**
     * Create empty MIME description.
     */
    StMIME()
    : mime(),
      ext(),
      desc() {
        //
    }

    /**
     * Copy constructor.
     */
    StMIME(const StMIME& copy)
    : mime(copy.mime),
      ext(copy.ext),
      desc(copy.desc) {
        //
    }

    /**
     * Full constructor.
     * @param mime (const StString& ) - mime string (like this: 'image/jpg');
     * @param ext (const StString& ) - extension (like this: 'jpg');
     * @param desc (const StString& ) - mime description (like this: 'JPG is a JPEG image file').
     */
    StMIME(const StString& mime, const StString& ext, const StString& desc)
    : mime(mime),
      ext(ext),
      desc(desc) {
        //
    }

    /**
     * Create MIME description from formatted string (like this: 'image/jpg:jpg:info').
     * @param string (const StString& ) - formatted string.
     */
    StMIME(const StString& string)
    : mime(),
      ext(),
      desc() {
        if(string.isEmpty()) {
            return;
        }
        StHandle< StArrayList<StString> > splittedList = string.split(':');
        if(splittedList->size() > 2) {
            mime = splittedList->getValue(0);
            ext  = splittedList->getValue(1);
            desc = splittedList->getValue(2);
        }
    }

    ~StMIME() {
        //
    }

    bool isEmpty() const {
        return mime.isEmpty();
    }

    const StMIME& operator=(const StMIME& toCopy) {
        if(this != &toCopy) {
            mime = toCopy.mime;
            ext  = toCopy.ext;
            desc = toCopy.desc;
        }
        return (*this);
    }

    bool operator==(const StMIME& toCompare) const {
        if(this == &toCompare) {
            return true;
        }
        // we ignore extension and description!
        return (mime == toCompare.mime);
    }

    bool operator!=(const StMIME& toCompare) const {
        return !(operator==(toCompare));
    }

    bool operator>(const StMIME& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->mime > compare.mime;
    }

    bool operator<(const StMIME& compare) const {
        if(&compare == this) {
            return false;
        }
        return this->mime < compare.mime;
    }

    bool operator>=(const StMIME& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->mime >= compare.mime;
    }

    bool operator<=(const StMIME& compare) const {
        if(&compare == this) {
            return true;
        }
        return this->mime <= compare.mime;
    }

    const StString& getMIMEType() const {
        return mime;
    }

    const StString& getExtension() const {
        return ext;
    }

    const StString& getDescription() const {
        return desc;
    }

    StString toString() const {
        return mime + ':' + ext + ':' + desc;
    }

};

#endif //__StMIME_h__
