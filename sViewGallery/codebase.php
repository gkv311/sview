<?php
/**
 * Coppermine 1.5.x Plugin - sView Gallery
 * Copyright (c) 2012 Kirill Gavrilov (www.sview.ru)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

if(!defined('IN_COPPERMINE')) die('Not in Coppermine...');

// Add filter
$thisplugin->add_filter('html_image',        'embed_stereo');
$thisplugin->add_filter('html_image_reduced','embed_stereo');

function embed_stereo($pic_html) {
    global $CONFIG, $CURRENT_PIC_DATA, $CURRENT_ALBUM_DATA, $JS;///, $LINEBREAK;

    // Include Javascript files
    $JS['includes'][] = 'plugins/sViewGallery/js/stGallery.js';

    // Idiotic check from themes.inc.php
    $useIntermediate = false;
    $aResizeMethod = $CONFIG['picture_use'] == "thumb" ? ($CONFIG['thumb_use'] == "ex" ? "any" : $CONFIG['thumb_use']) : $CONFIG['picture_use'];
    if($aResizeMethod == 'ht' && $CURRENT_PIC_DATA['pheight'] > $CONFIG['picture_width']) {
        $useIntermediate = true;
    } elseif($aResizeMethod == 'wd' && $CURRENT_PIC_DATA['pwidth'] > $CONFIG['picture_width']) {
        $useIntermediate = true;
    } elseif($aResizeMethod == 'any' && max($CURRENT_PIC_DATA['pwidth'], $CURRENT_PIC_DATA['pheight']) > $CONFIG['picture_width']) {
        $useIntermediate = true;
    }

    // This is preferred to always use full image for embed view
    $aPicUrlFull = get_pic_url($CURRENT_PIC_DATA, 'fullsize');
    $aPicUrlThmb = get_pic_url($CURRENT_PIC_DATA, 'thumb');
    $aPicUrlNorm = $aPicUrlFull;
    if($CONFIG['make_intermediate'] && $useIntermediate) {
        $aPicUrlNorm = get_pic_url($CURRENT_PIC_DATA, 'normal');
    }
    $aPid = $CURRENT_PIC_DATA['pid'];

    // Analyze the file extension using regular expressions.
    $aMatchesExt = array();
    if(!preg_match("/(.+)\.(.*?)\Z/", $aPicUrlFull, $aMatchesExt)) {
        $aMatchesExt[1] = 'invalid_fname';
        $aMatchesExt[2] = 'xxx';
    }
    $aPicExtension = strtolower($aMatchesExt[2]);

    // Detect stereoscopic image
    $isStereo = ($aPicExtension=='jps' || $aPicExtension=='pns' || $aPicExtension=='mpo');

    $aPreviewSize = cpg_getimagesize(urldecode($aPicUrlNorm));
    $aFullSize    = cpg_getimagesize(urldecode($aPicUrlFull));
    //$anEmbSizeX = $CONFIG['picture_width'];
    //$anEmbSizeY = intval((10 * $anEmbSizeX) / 16);
    $aNormSizeX = $aPreviewSize[0];
    $aNormSizeY = $aPreviewSize[1];
    $anEmbSizeX = intval($aNormSizeX / 2);
    $anEmbSizeY = $aNormSizeY;

    $aFullSizeX = $aFullSize[0];
    $aFullSizeY = $aFullSize[1];

    $aTitle  = $CURRENT_ALBUM_DATA['title']." - ";
    $aTitle .= $CURRENT_PIC_DATA['title'] ? $CURRENT_PIC_DATA['title'] : strtr(preg_replace("/(.+)\..*?\Z/", "\\1", htmlspecialchars($CURRENT_PIC_DATA['filename'])), "_", " ");

    # filter to match INTTEXT
    #$aTitleBB = preg_replace('/[^a-z0-9-.\ \+\x7F-\xFF]+/is', '_', $aTitle);
    $aTitleBB = preg_replace('/[^a-z0-9-.\ \+\p{L}]+/uis', '_', $aTitle);

    // stereoscopic image object managed by special javascript code
    $pic_html  = "<div id='stImgCpg".$aPid."'";
    $pic_html .= " data-prv-url='$aPicUrlNorm' data-prv-width='$aNormSizeX' data-prv-height='$aNormSizeY'";
    $pic_html .= " data-img-url='$aPicUrlFull' data-img-width='$aFullSizeX' data-img-height='$aFullSizeY'";
    #$pic_html .= " data-img-title='$aTitle'";
    $pic_html .= ">";
    $pic_html .= "<img src='$aPicUrlNorm' width='$aNormSizeX' height='$aNormSizeY' onload='stGalleryInit(this);' border='0' margin='0' />";
    $pic_html .= "</div>";

    // generated BBcodes
    $aCpgRoot = $CONFIG['ecards_more_pic_target'];
    $pic_html .= "<br /><a href='javascript:void(0);' onclick=\"stGalleryShowBBCodes".$aPid."(this);\">Show BBCodes</a>";
    $pic_html .= "<table id='idBBCodes".$aPid."' style='display: none;'>";
    $pic_html .= "<tr><td width=150p align='center'>BBcode thumbnail:</td><td>";
    $pic_html .= "<textarea id='idBB4PicT".$aPid."' rows='2' readonly='readonly' style='width: 100%' onClick='stGallerySelectAll(this)'";
    $pic_html .= " title='Use this for having a having a linkable thumbnail in any PHPBB forum'>";
    $pic_html .= "[url=URL_TO_THERE][img]${aCpgRoot}${aPicUrlThmb}[/img][/url]";
    $pic_html .= "</textarea>";
    $pic_html .= "</td></tr>";

    $pic_html .= "<tr><td width=150p align='center'>BBcode preview:</td><td width=600p>";
    $pic_html .= "<textarea id='idBB4PicP".$aPid."' rows='2' readonly='readonly' style='width: 100%' onClick='stGallerySelectAll(this)'";
    $pic_html .= " title='Use this for having a larger sized linkable thumbnail in any PHPBB forum'>";
    $pic_html .= "[url=URL_TO_THERE][img]${aCpgRoot}${aPicUrlNorm}[/img][/url]";
    $pic_html .= "</textarea>";
    $pic_html .= "</td></tr>";

    $pic_html .= "<tr><td width=150p align='center'>BBcode stImg:</td><td width=600p>";
    $pic_html .= "<textarea rows='2' readonly='readonly' style='width: 100%' onClick='stGallerySelectAll(this)'";
    $pic_html .= " title='Use this to embed stereoscopic image preview in forums with stImg tag defined'>";
    $pic_html .= "[stimg='${aTitleBB}' url=${aCpgRoot}${aPicUrlFull}]${aCpgRoot}${aPicUrlNorm}[/stimg]";
    $pic_html .= "</textarea>";
    $pic_html .= "</td></tr>";

    $pic_html .= "<tr><td width=150p align='center'>HTML preview:</td><td width=600p>";
    $pic_html .= "<textarea id='idHTML4PicP".$aPid."' rows='2' readonly='readonly' style='width: 100%' onClick='stGallerySelectAll(this)'";
    $pic_html .= " title='Use this to embed image preview using common HTML tags'>";
    $pic_html .= "<a href='URL_TO_THERE'><img src='${aCpgRoot}${aPicUrlNorm}  width='$aNormSizeX' height='$aNormSizeY' /></a>";
    $pic_html .= "</textarea>";
    $pic_html .= "</td></tr>";

    $pic_html .= "<tr><td width=150p align='center'>HTML stImg:</td><td width=600p>";
    $pic_html .= "<textarea rows='2' readonly='readonly' style='width: 100%' onClick='stGallerySelectAll(this)'";
    $pic_html .= " title='Use this to embed stereoscopic image preview if \"stGallery.js\" is installed on your website'>";
    $pic_html .= "<div align='center' data-prv-url='${aCpgRoot}${aPicUrlNorm}' data-img-url='${aCpgRoot}${aPicUrlFull}' data-img-title='$aTitle'>";
    $pic_html .= "<img src='${aCpgRoot}${aPicUrlNorm}' onload='stGalleryInit(this);' border='0' margin='0' /></div>";
    $pic_html .= "</textarea>";
    $pic_html .= "</td></tr>";
    $pic_html .= "</table>";

    $pic_html .= "<script>";
    $pic_html .= "var aBbT = document.getElementById('idBB4PicT".$aPid."');";
    $pic_html .= "aBbT.value = aBbT.value.replace('URL_TO_THERE', document.URL);";
    $pic_html .= "var aBbP = document.getElementById('idBB4PicP".$aPid."');";
    $pic_html .= "aBbP.value = aBbP.value.replace('URL_TO_THERE', document.URL);";
    $pic_html .= "var aHtmlP = document.getElementById('idHTML4PicP".$aPid."');";
    $pic_html .= "aHtmlP.value = aHtmlP.value.replace('URL_TO_THERE', document.URL);";
    $pic_html .= "function stGalleryShowBBCodes".$aPid."(theLink) {";
    $pic_html .= "  theLink.style.display = 'none';";
    $pic_html .= "  document.getElementById('idBBCodes".$aPid."').style.display = 'block';";
    $pic_html .= "}";
    $pic_html .= "function stGallerySelectAll(theTextArea) {";
    $pic_html .= "  theTextArea.focus();";
    $pic_html .= "  theTextArea.select();";
    $pic_html .= "}";
    $pic_html .= "</script>";

    return $pic_html;
}

?>
