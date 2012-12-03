/**
 * Coppermine 1.5.x Plugin - sView Gallery
 * Copyright (c) 2012 Kirill Gavrilov (www.sview.ru)
 */

/**
 * Supported stereoscopic modes.
 */
var ST_MODE_MONO       = 'idPicNormal';
var ST_MODE_LEFT       = 'idPicLeft';
var ST_MODE_EMBED      = 'idPicEmbed';
var ST_MODE_ANAGLYPH   = 'idPicAn';
var ST_MODE_INTERLACED = 'idPicRow';

function stGalleryNoFixed() {
  return ((window.XMLHttpRequest == undefined) && (ActiveXObject != undefined));
}

/**
 * Handler to show fullsize image.
 */
function stGalleryFull() {
  if(this.myIsFull) {
    // just remove fullsize
    document.body.removeChild(this);
    return;
  }

  // append fullsize object
  if(this.myStMode == ST_MODE_MONO) {
    document.body.appendChild(stGalleryMono(this.myStImg, true));
  } else if(this.myStMode == ST_MODE_ANAGLYPH) {
    document.body.appendChild(stGalleryCanvas(this.myStImg, true, this.myStMode));
  } else if(this.myStMode == ST_MODE_INTERLACED) {
    document.body.appendChild(stGalleryCanvas(this.myStImg, true, this.myStMode));
  } else if(this.myStMode == ST_MODE_LEFT) {
    document.body.appendChild(stGalleryLeft(this.myStImg, true));
  }
}

/**
 * Auxiliary function to retrieve mouse position.
 */
function stGalleryMousePos(theEvent) {
  var aPos = new Object();
  if(theEvent) {
    aPos.x = theEvent.pageX;
    aPos.y = theEvent.pageY;
  } else {
    // IE
    aPos.x = event.clientX; // + document.body.scrollLeft;
    aPos.y = event.clientY; // + document.body.scrollTop;
  }
  return aPos;
}

/**
 * Handler on fullsize image.
 */
function stGalleryFullDown(theEvent) {
  var aPos = stGalleryMousePos(theEvent);
  this.myMouseDownX = aPos.x;
  this.myMouseDownY = aPos.y;
  this.myMouseLastX = aPos.x;
  this.myMouseLastY = aPos.y;
  this.myMouseTrack = 1;
}

/**
 * Handler on fullsize image.
 */
function stGalleryFullUp(theEvent) {
  this.myMouseTrack = 0;
  if(this.myMouseLastX == this.myMouseDownX
  && this.myMouseLastY == this.myMouseDownY) {
    document.body.removeChild(this.parentNode);
  }
}
/**
 * Auxiliary function to set margin for fullscreen image.
 * This is dummy code for interlaced output to ensure that left/right
 * will not be swapped during dragging.
 */
function stGallerySetMargin(theStyle, theLeft, theTop) {
  var aLeft = theLeft;
  if(aLeft % 2 != 0) {
    aLeft += 1;
  }
  var aTop = theTop;
  if(aTop % 2 != 0) {
    aTop += 1;
  }

  theStyle.marginLeft = '-' + aLeft + 'px';
  theStyle.marginTop  = '-' + aTop  + 'px';
}

/**
 * Handler to move fullsize image.
 */
function stGalleryFullMove(theEvent) {
  if(!this.myMouseTrack) {
    return;
  }

  var aMouse = stGalleryMousePos(theEvent);
  var aMoveX = aMouse.x - this.myMouseLastX;
  var aMoveY = aMouse.y - this.myMouseLastY;
  if(aMoveX == 0 && aMoveY == 0) {
    return;
  }

  this.myMouseLastX = aMouse.x;
  this.myMouseLastY = aMouse.y;
  this.myMarginLeft -= aMoveX;
  this.myMarginTop  -= aMoveY;

  stGallerySetMargin(this.style, this.myMarginLeft, this.myMarginTop);
}

/**
 * Just standard image object.
 * @param theStImg - stereoscopic image object;
 * @param isFull   - fullsize image or preview.
 */
function stGalleryMono(theStImg, isFull) {
  var anUrl   = isFull ? theStImg.myImgUrl   : theStImg.myPrvUrl;
  var aWidth  = isFull ? theStImg.myImgSizeX : theStImg.myPrvSizeX;
  var aHeight = isFull ? theStImg.myImgSizeY : theStImg.myPrvSizeY;

  // link to show fullsize image
  var aLink = document.createElement(isFull ? 'div' : 'a');
  aLink.myStImg  = theStImg;
  aLink.myStMode = ST_MODE_MONO;
  aLink.myIsFull = isFull;
  if(!isFull) {
    aLink.href    = 'javascript:void(0);';
    aLink.onclick = stGalleryFull;
  }

  // image object itself
  var anImg = new Image();
  anImg.border = '0';
  anImg.margin = '0';
  anImg.width  = aWidth;
  anImg.height = aHeight;
  anImg.src    = anUrl;

  if(!isFull) {
    aLink.appendChild(anImg);
    return aLink;
  }

  anImg.style.width      = aWidth  + 'px';
  anImg.style.height     = aHeight + 'px';
  anImg.style.overflow   = 'hidden';
  anImg.style.margin     = '0px';
  anImg.style.padding    = '0px';
  anImg.style.position   = 'absolute';
  anImg.style.top        = '50%';
  anImg.style.left       = '50%';
  anImg.style.marginTop  = '-' + (aHeight / 2) + 'px';
  anImg.style.marginLeft = '-' + (aWidth  / 4) + 'px';
  anImg.style.zIndex     = 3;

  // setup mouse events
  anImg.ondragstart = function() { return false; }; // block dragging
  anImg.myMouseTrack = 0;
  anImg.onmousemove = stGalleryFullMove;
  anImg.onmousedown = stGalleryFullDown;
  anImg.onmouseup   = stGalleryFullUp;

  aLink.appendChild(anImg);

  var anImgLoad = new Image();
  anImgLoad.onload = function() {
    anImg.style.width  = anImgLoad.width  + 'px';
    anImg.style.height = anImgLoad.height + 'px';
    anImg.width  = anImgLoad.width;
    anImg.height = anImgLoad.height;
    if(isFull) {
      if(stGalleryNoFixed()) {
        anImg.style.position = 'absolute';
      } else {
        anImg.style.position = 'fixed';
      }
      anImg.myMarginLeft = (anImgLoad.width  / 4);
      anImg.myMarginTop  = (anImgLoad.height / 2);
      anImg.style.marginLeft = '-' + anImg.myMarginLeft + 'px';
      anImg.style.marginTop  = '-' + anImg.myMarginTop  + 'px';
    }
  };
  anImgLoad.src = anUrl;

  return aLink;
}

/**
 * Generates standard image object cropped by div element.
 * @param theStImg - stereoscopic image object;
 * @param isFull   - fullsize image or preview.
 */
function stGalleryLeft(theStImg, isFull) {
  var anUrl   = isFull ? theStImg.myImgUrl   : theStImg.myPrvUrl;
  var aWidth  = isFull ? theStImg.myImgSizeX : theStImg.myPrvSizeX;
  var aHeight = isFull ? theStImg.myImgSizeY : theStImg.myPrvSizeY;

  // link to show fullsize image
  var aLink = document.createElement(isFull ? 'div' : 'a');
  aLink.myStImg  = theStImg;
  aLink.myStMode = ST_MODE_LEFT;
  aLink.myIsFull = isFull;
  if(!isFull) {
    aLink.href    = 'javascript:void(0);';
    aLink.onclick = stGalleryFull;
  }

  // div to crop half of image
  var aDiv = document.createElement('div');
  aDiv.style.width    = (aWidth/2) + 'px';
  aDiv.style.height   = aHeight + 'px';
  aDiv.style.overflow = 'hidden';
  aDiv.style.margin   = '0px';
  aDiv.style.padding  = '0px';
  if(isFull) {
    if(stGalleryNoFixed()) {
      aDiv.style.position = 'absolute';
    } else {
      aDiv.style.position = 'fixed';
    }
    aDiv.style.top        = '50%';
    aDiv.style.left       = '50%';
    aDiv.style.marginTop  = '-' + (aHeight / 2) + 'px';
    aDiv.style.marginLeft = '-' + (aWidth  / 4) + 'px';
    aDiv.style.zIndex     = 3;

    // setup mouse events
    aDiv.ondragstart = function() { return false; }; // block dragging
    aDiv.myMouseTrack = 0;
    aDiv.onmousemove = stGalleryFullMove;
    aDiv.onmousedown = stGalleryFullDown;
    aDiv.onmouseup   = stGalleryFullUp;
  }

  // image block itself
  var anImg = new Image();
  anImg.border = '0';
  anImg.margin = '0';
  anImg.width  = aWidth;
  anImg.height = aHeight;
  anImg.src    = anUrl;
  anImg.ondragstart = function() { return false; }; // block dragging

  aDiv.appendChild(anImg);
  aLink.appendChild(aDiv);

  var anImgLoad = new Image();
  anImgLoad.onload = function() {
    aDiv.style.width  = (anImgLoad.width / 2) + 'px';
    aDiv.style.height =  anImgLoad.height     + 'px';
    anImg.width  = anImgLoad.width;
    anImg.height = anImgLoad.height;
    if(isFull) {
      aDiv.myMarginLeft     = (anImgLoad.width  / 4);
      aDiv.myMarginTop      = (anImgLoad.height / 2);
      aDiv.style.marginLeft = '-' + aDiv.myMarginLeft + 'px';
      aDiv.style.marginTop  = '-' + aDiv.myMarginTop  + 'px';
    }
  };
  anImgLoad.src = anUrl;

  return aLink;
}

/**
 * Generates canvas block (HTML5).
 * @param theStImg  - stereoscopic image object;
 * @param isFull    - fullsize image or preview;
 * @param theStMode - mode to activate.
 */
function stGalleryCanvas(theStImg, isFull, theStMode) {
  var anUrl   = isFull ? theStImg.myImgUrl   : theStImg.myPrvUrl;
  var aWidth  = isFull ? theStImg.myImgSizeX : theStImg.myPrvSizeX;
  var aHeight = isFull ? theStImg.myImgSizeY : theStImg.myPrvSizeY;

  // link to show fullsize image
  var aLink = document.createElement(isFull ? 'div' : 'a');
  aLink.myStImg  = theStImg;
  aLink.myStMode = theStMode;
  aLink.myIsFull = isFull;
  if(!isFull) {
    aLink.href     = 'javascript:void(0);';
    aLink.onclick  = stGalleryFull;
  }

  // div to crop half of image
  var aDiv = document.createElement('div');
  aDiv.style.width    = (aWidth/2) + 'px';
  aDiv.style.height   = aHeight + 'px';
  aDiv.style.overflow = 'hidden';
  aDiv.style.margin   = '0px';
  aDiv.style.padding  = '0px';
  if(isFull) {
    if(stGalleryNoFixed()) {
      aDiv.style.position = 'absolute';
    } else {
      aDiv.style.position = 'fixed';
    }
    aDiv.style.top        = '50%';
    aDiv.style.left       = '50%';
    aDiv.style.marginTop  = '-' + (aHeight / 2) + 'px';
    aDiv.style.marginLeft = '-' + (aWidth  / 4) + 'px';
    aDiv.style.zIndex     = 3;

    // setup mouse events
    aDiv.ondragstart = function() { return false; }; // block dragging
    aDiv.myMouseTrack = 0;
    aDiv.onmousemove = stGalleryFullMove;
    aDiv.onmousedown = stGalleryFullDown;
    aDiv.onmouseup   = stGalleryFullUp;
  }

  // canvas block itself
  var aCanvas = document.createElement('canvas');
  aCanvas.width  = aWidth;
  aCanvas.height = aHeight;
  aCanvas.ondragstart = function() { return false; }; // block dragging

  aDiv.appendChild(aCanvas);
  aLink.appendChild(aDiv);

  // bind handler to perform rendering when image will be loaded
  var anImg = new Image();
  anImg.onload = function() {
    aDiv.style.width  = (anImg.width / 2) + 'px';
    aDiv.style.height =  anImg.height     + 'px';
    aCanvas.width  = anImg.width;
    aCanvas.height = anImg.height;
    if(isFull) {
      aDiv.myMarginLeft     = (anImg.width  / 4);
      aDiv.myMarginTop      = (anImg.height / 2);
      aDiv.style.marginLeft = '-' + aDiv.myMarginLeft + 'px';
      aDiv.style.marginTop  = '-' + aDiv.myMarginTop  + 'px';
    }
    stGalleryDraw(anImg, aCanvas, theStMode)
  };
  anImg.src = anUrl;

  return aLink;
}

/**
 * Error text for embed object.
 */
function stGalleryEmbedError() {
  var aTable = document.createElement('table');
  aTable.border = '0';
  aTable.width  = '100%';
  aTable.height = '100%';

  var aLink = document.createElement('a');
  aLink.href = 'http://www.sview.ru/download';
  aLink.innerHTML = 'This view requires browser plugin.<br />Download sView with plugin for popular browsers now.';

  var aCell = aTable.insertRow(0).insertCell(0);
  aCell.style.verticalAlign = 'middle'; // doesn't work in FF...

  aCell.style.textAlign     = 'center';
  aCell.appendChild(aLink);

  return aTable;
}

/**
 * Generates embed object block.
 * @param theStImg - stereoscopic image object;
 */
function stGalleryEmbed(theStImg) {
  var anUrl   = theStImg.myImgUrl;
  var aWidth  = theStImg.myPrvSizeX;
  var aHeight = theStImg.myPrvSizeY;

  // div element to enforce correct layout
  var aDiv = document.createElement('div');
  aDiv.style.width    = (aWidth/2) + 'px';
  aDiv.style.height   = aHeight + 'px';
  aDiv.style.overflow = 'hidden';
  aDiv.style.margin   = '0px';
  aDiv.style.padding  = '0px';

  if(navigator.mimeTypes['image/x-jps']
  && navigator.mimeTypes['image/x-jps'].enabledPlugin) {
    // embed object itself
    var anObject = document.createElement('object');
    anObject.type   = 'image/x-jps';
    anObject.data   = anUrl;
    anObject.width  = (aWidth/2);
    anObject.height = aHeight;
    anObject.setAttribute('data-prv-url', theStImg.myPrvUrl);

    aDiv.appendChild(anObject);
  } else {
    var isIE = (navigator.appVersion.indexOf("MSIE") != -1) ? true : false;
    if(isIE) {
      // Internet Explorer (a painful hacks):
      //  - doesn't support navigator.mimeTypes (we can test only predefined plugins using 'new ActiveXObject', not MIME types!);
      //  - doesn't correctly instantiate plugin using correct javascript DOM syntax (we use innerHTML instead);
      //  - and ActiveX components don't work without child <param name="src" value="url" />.
      aDiv.innerHTML = '<object type="image/x-jps" width="' + (aWidth/2) + '" height="' + aHeight + '">'
                     + '<param name="src"          value="' + anUrl + '" />'
                     + '<param name="type"         value="image/x-jps" />'
                     + '<param name="data-prv-url" value="' + theStImg.myPrvUrl + '" />'
                     + '<table border=0 height=100%><tr><td align="center" valign="middle">'
                     + '<a href=http://www.sview.ru/download>This view requires browser plugin.<br />Download sView with plugin for popular browsers now.</a>'
                     + '</td></tr></table>'
                     + '</object>';
    } else {
      // just show error message
      aDiv.appendChild(stGalleryEmbedError());
    }
  }

  return aDiv;
}

/**
 * Handler-function to switch to specified mode.
 */
function stGallerySwitchHandler() {
  if(document.getElementsByClassName == undefined) {
    stGallerySwitch(this.myStImg, this.myStMode);
    return;
  }

  // switch all stereo images on the page
  var aList = document.getElementsByClassName('StImage');
  for(var anIter = 0, aLen = aList.length; anIter < aLen; ++anIter) {
    stGallerySwitch(aList[anIter], this.myStMode);
  }
}

/**
 * Generates switch mode link.
 * @param theStImg        - stereoscopic image object;
 * @param theCell         - parent cell;
 * @param theStModeActive - current mode;
 * @param theLinkStMode   - mode to activate by this link;
 * @param theStModeName   - mode title.
 */
function stGalleryModeLink(theStImg, theCell, theStModeActive, theLinkStMode, theStModeName) {
  theCell.width  = '100p';
  theCell.align  = 'center';
  theCell.style.border = '0';

  var aLink = document.createElement('a');
  aLink.myStImg  = theStImg;
  aLink.myStMode = theLinkStMode;
  aLink.href     = 'javascript:void(0);';
  if(theStModeActive != theLinkStMode) {
    aLink.onclick = stGallerySwitchHandler;
  }
  aLink.appendChild(document.createTextNode(theStModeName));
  theCell.appendChild(aLink);
}

/**
 * Just generates download link.
 * @param theStImg - stereoscopic image object;
 * @param theCell  - parent cell.
 */
function stGalleryDownLink(theStImg, theCell) {
  theCell.width  = '100p';
  theCell.align  = 'center';
  theCell.style.border = '0';

  var aLink = document.createElement('a');
  aLink.href = theStImg.myImgUrl;
  aLink.appendChild(document.createTextNode('Download'));
  theCell.appendChild(aLink);
}

/**
 * Function switch presentation mode for specified Picture.
 * @param theStImg  - stereoscopic image object;
 * @param theStMode - mode to activate.
 */
function stGallerySwitch(theStImg, theStMode) {
  // save choise to cookies
  document.cookie = 'stGalleryMode=' + theStMode;

  // try to fix memory leaks in IE...
  // ActiveX element doesn't destroyed immediately!
  var anOldObject = null;
  for(var aChildIter = 0; aChildIter < theStImg.childNodes.length; ++aChildIter) {
    var aChild = theStImg.childNodes[aChildIter];
    for(var aSubChildIter = 0; aSubChildIter < aChild.childNodes.length; ++aSubChildIter) {
      var aSubChild = aChild.childNodes[aSubChildIter];
      if(aSubChild.nodeName == "OBJECT") {
        anOldObject = aSubChild;
        aSubChild.parentNode.removeChild(aSubChild);
        break;
      }
    }
  }
  while(theStImg.hasChildNodes()) {
    theStImg.removeChild(theStImg.lastChild);
  }

  // create new stereoscopic image object without previous children
  var aStImg = theStImg.cloneNode(false);

  aStImg.className  = 'StImage';
  aStImg.myHideMono = theStImg.getAttribute('data-hide-mono');
  aStImg.myPrvUrl   = theStImg.getAttribute('data-prv-url');
  aStImg.myPrvSizeX = theStImg.getAttribute('data-prv-width');
  aStImg.myPrvSizeY = theStImg.getAttribute('data-prv-height');
  aStImg.myImgUrl   = theStImg.getAttribute('data-img-url');
  aStImg.myImgSizeX = theStImg.getAttribute('data-img-width');
  aStImg.myImgSizeY = theStImg.getAttribute('data-img-height');
  aStImg.myTitle    = theStImg.getAttribute('data-img-title');
  if(!aStImg.myPrvUrl) {
    aStImg.myPrvUrl   = aStImg.myImgUrl;
    aStImg.myPrvSizeX = aStImg.myImgSizeX;
    aStImg.myPrvSizeY = aStImg.myImgSizeY;
  }
  if(!aStImg.myPrvSizeX || !aStImg.myPrvSizeY) {
    aStImg.myPrvSizeX = aStImg.myImgSizeX;
    aStImg.myPrvSizeY = aStImg.myImgSizeY;
  }

  // add switch buttons
  var aTable = document.createElement('table');
  aTable.border = '0';
  aTable.cellpadding = '1';
  aTable.cellspacing = '1';
  aTable.width       = '514p';

  var aRow = aTable.insertRow(0);
  aRow.style.border = '0';
  stGalleryDownLink(aStImg, aRow.insertCell(0));
  stGalleryModeLink(aStImg, aRow.insertCell(0), theStMode, ST_MODE_EMBED,      '3D Stereo');
  stGalleryModeLink(aStImg, aRow.insertCell(0), theStMode, ST_MODE_INTERLACED, 'Interlaced');
  stGalleryModeLink(aStImg, aRow.insertCell(0), theStMode, ST_MODE_ANAGLYPH,   'Anaglyph');
  stGalleryModeLink(aStImg, aRow.insertCell(0), theStMode, ST_MODE_LEFT,       'Left');
  if(!aStImg.myHideMono) {
    stGalleryModeLink(aStImg, aRow.insertCell(0), theStMode, ST_MODE_MONO,     'Mono');
  }

  aStImg.appendChild(aTable);

  // generate children for active mode
  if(theStMode == ST_MODE_MONO) {
    aStImg.appendChild(stGalleryMono(aStImg, false));
  } else if(theStMode == ST_MODE_ANAGLYPH) {
    aStImg.appendChild(stGalleryCanvas(aStImg, false, theStMode));
  } else if(theStMode == ST_MODE_INTERLACED) {
    aStImg.appendChild(stGalleryCanvas(aStImg, false, theStMode));
  } else if(theStMode == ST_MODE_EMBED) {
    aStImg.appendChild(stGalleryEmbed(aStImg));
  } else if(theStMode == ST_MODE_LEFT) {
    aStImg.appendChild(stGalleryLeft(aStImg, false));
  }

  // add switch buttons
  var aTableBot = document.createElement('table');
  aTableBot.border      = '0';
  aTableBot.cellpadding = '1';
  aTableBot.cellspacing = '1';
  aTableBot.width       = '514p';
  var aRowBot  = aTableBot.insertRow(0);
  aRowBot.style.border = '0';
  var aCellBot = aRowBot.insertCell(0);

  var aBotText = aStImg.myTitle ? ('<p>' + aStImg.myTitle + '</p>') : '';
  aCellBot.innerHTML = aBotText;
  aCellBot.align     = 'center';
  aCellBot.style.border = '0';
  aStImg.appendChild(aTableBot);

  // replace original stereoscopic image object with new one
  theStImg.parentNode.replaceChild(aStImg, theStImg);

  if(!anOldObject) {
    return;
  }

  try {
    // call dispose if plugin supports is to release resources explicitly
    anOldObject.Dispose();
  } catch(theErr) {
    //alert(theErr);
  }
  anOldObject = null;
}

/**
 * Function read presentation mode from cookies.
 */
function stGalleryReadMode() {
  var aCookies = document.cookie.split(';');
  for(var aCookIter = 0; aCookIter < aCookies.length; aCookIter++) {
    var aName = aCookies[aCookIter].substr(0, aCookies[aCookIter].indexOf('='));
    aName = aName.replace(/^\s+|\s+$/g,"");
    if(aName == 'stGalleryMode') {
      var aValue = aCookies[aCookIter].substr(aCookies[aCookIter].indexOf('=') + 1);
      return unescape(aValue);
    }
  }
  return ST_MODE_ANAGLYPH;
}

/**
 * Function load presentation mode from cookies for specified Picture.
 * @param theElem - initialization image object.
 */
function stGalleryInit(theElem) {
  var aStImg = theElem.parentNode;

  var aSizeX = aStImg.getAttribute('data-img-width');
  var aSizeY = aStImg.getAttribute('data-img-height');
  if(!aSizeX || !aSizeY) {
    // read from image with onload event
    aStImg.setAttribute('data-img-width',  theElem.width);
    aStImg.setAttribute('data-img-height', theElem.height);
  }

  stGallerySwitch(aStImg, stGalleryReadMode());
}

/**
 * Function load presentation mode from cookies for specified Picture.
 * @param theStImgId - stereoscopic image object id.
 */
function stGalleryInitId(theStImgId) {
  var aStImg = document.getElementById(theStImgId);
  stGallerySwitch(aStImg, stGalleryReadMode());
}

/**
 * Function creates Anaglyph view for SideBySide data.
 * Result stored in left half of original data.
 * @param theImgData - image object data.
 */
function stGalleryAnaglyphDraw(theImgData) {
  var aData     = theImgData.data;
  var aWidth    = Math.floor(theImgData.width / 2);
  var aSbsWidth = theImgData.width;
  var aSlice    = aWidth * 4;
  for(var aRow = 0; aRow < theImgData.height; ++aRow) {
    var aRowStart = aRow * aSbsWidth * 4;
    for(var aCol = 0; aCol < aWidth; ++aCol) {
        var aPixel = aRowStart + aCol * 4;
//        aData[aPixel + 1] = aData[aPixel + aSlice + 1];
//        aData[aPixel + 2] = aData[aPixel + aSlice + 2];
        aData[aPixel] = aData[aPixel + aSlice];
    }
  }
}

/**
 * Function creates Row-interleaved view for SideBySide data.
 * Result stored in left half of original data.
 * @param theImgData - image object data.
 */
function stGalleryRowDraw(theImgData) {
  var aData     = theImgData.data;
  var aWidth    = Math.floor(theImgData.width / 2);
  var aSbsWidth = theImgData.width;
  var aSlice    = aWidth * 4;
  for(var aRow = 0; aRow < theImgData.height; ++aRow) {
    var aRowStart = aRow * aSbsWidth * 4;
    if(aRow % 2 == 0) {
      continue;
    }
    for(var aCol = 0; aCol < aWidth; ++aCol) {
        var aPixel = aRowStart + aCol * 4;
        aData[aPixel    ] = aData[aPixel + aSlice    ];
        aData[aPixel + 1] = aData[aPixel + aSlice + 1];
        aData[aPixel + 2] = aData[aPixel + aSlice + 2];
    }
  }
}

/**
 * Function render Anaglyph view for SideBySide image using canvas.
 * @param theImgObj - image object, should be loaded;
 * @param theCanvas - canvas element in DOM;
 * @param theStMode - stereo mode.
 */
function stGalleryDraw(theImgObj, theCanvas, theStMode) {
  if(!theCanvas.getContext) {
    // replace canvas with error
    var aTable = document.createElement('table');
    aTable.border = '0';
    aTable.width  = '100%';
    aTable.height = '100%';

    var aCell = aTable.insertRow(0).insertCell(0);
    aCell.style.verticalAlign = 'middle';
    aCell.appendChild(document.createTextNode('This view requires HTML5-compliant browser'));

    var aParent = theCanvas.parentNode;
    aParent.replaceChild(aTable, theCanvas);
    return;
  }

  var aContext  = theCanvas.getContext('2d');
  aContext.drawImage(theImgObj, 0, 0);
  var anImgData = aContext.getImageData(0, 0, theCanvas.width, theCanvas.height);
  if(theStMode == ST_MODE_ANAGLYPH) {
    stGalleryAnaglyphDraw(anImgData);
  } else if(theStMode == ST_MODE_INTERLACED) {
    stGalleryRowDraw(anImgData);
  }
  aContext.putImageData(anImgData, 0, 0);
}
