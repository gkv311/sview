/**
 * Copyright © 2014 Kirill Gavrilov <kirill@sview.ru>
 *
 * This code is licensed under MIT license (see docs/license-mit.txt for details).
 */

#ifndef __StExifTags_h_
#define __StExifTags_h_

#include <StStrings/StString.h>

#include <map>

struct StExifTag {
    uint16_t  Tag;
    StCString Name;
};

class StExifTagsMap {

        public:

    ST_CPPEXPORT  StExifTagsMap();
    ST_CPPEXPORT ~StExifTagsMap();

    ST_CPPEXPORT const StExifTag* findImageTag(const uint16_t theTag) const;
    ST_CPPEXPORT const StExifTag* findMpoTag  (const uint16_t theTag) const;
    ST_CPPEXPORT const StExifTag* findFujiTag (const uint16_t theTag) const;
    ST_CPPEXPORT const StExifTag* findCanonTag(const uint16_t theTag) const;
    ST_CPPEXPORT const StExifTag* findOlympTag(const uint16_t theTag) const;

        private:

    std::map<uint16_t, const StExifTag*> myImageTags;
    std::map<uint16_t, const StExifTag*> myMpoTags;
    std::map<uint16_t, const StExifTag*> myFujiTags;
    std::map<uint16_t, const StExifTag*> myCanonTags;
    std::map<uint16_t, const StExifTag*> myOlympTags;

};

namespace StExifTags {

    extern const StExifTag IMAGE_TAGS[];
    extern const StExifTag MPO_TAGS[];
    extern const StExifTag FUJI_TAGS[];
    extern const StExifTag CANON_TAGS[];
    extern const StExifTag OLYMP_TAGS[];

    enum Image {
        Image_Orientation = 0x0112,
        Image_DateTime    = 0x0132,
        Image_MakerNote   = 0x927C, // matches TAG_MAKER_NOTE
    };

    enum Fuji {
        Fuji_Parallax = 0xB211,
    };

};


#endif // __StExifTags_h_
