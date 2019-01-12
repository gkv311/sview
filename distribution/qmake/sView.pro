# This is a Solution project for sView.
# It is intended only for code navigation using Qt Creator, not for building project!
#
# To use this project, open it in Qt Creator, "Run qmake" once,
# close the project and open it anew, until the list of sView modules will not appear in Projects list.
# Then "Run qmake" and "Build Project" which would redirect to Makefile in sView root folder.
TEMPLATE = subdirs

# Iterate over Modules and generate sub-projects
aSolTkList = StShared StGLWidgets StCore StOutAnaglyph StOutDual StOutInterlace StOutPageFlip StOutIZ3D StOutDistorted StImageViewer StMoviePlayer StDiagnostics StCADViewer StBrowserPlugin sview StTests
for (aSolToolkit, aSolTkList) {
  eval(sviewtkgen_$${aSolToolkit}.input  = $$_PRO_FILE_PWD_/sViewToolkit.pro.in)
  eval(sviewtkgen_$${aSolToolkit}.output = $$_PRO_FILE_PWD_/$${aSolToolkit}/$${aSolToolkit}.pro)
  eval(sviewkgen_$${aSolToolkit}.config = verbatim)
  eval(QMAKE_SUBSTITUTES += sviewtkgen_$${aSolToolkit})
  SUBDIRS += $${aSolToolkit}

  !equals(aSolToolkit, StShared) {
    #$${aSolToolkit}.depends += StShared
  }
}

aTxtListTxt = $$files($$_PRO_FILE_PWD_/../../license-*.txt)
aTxtListMd1 = $$files($$_PRO_FILE_PWD_/../../*.md)
aTxtListMd2 = $$files($$_PRO_FILE_PWD_/../../docs/*.md)
OTHER_FILES += $$aTxtListTxt
OTHER_FILES += $$aTxtListMd1
OTHER_FILES += $$aTxtListMd2
OTHER_FILES += $$_PRO_FILE_PWD_/../../Makefile
