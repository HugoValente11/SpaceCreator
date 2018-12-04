# TODO: make it sharable with main project, now it must be actualized manually

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/../../../../msceditor \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools

QT += core gui widgets xml

SOURCES += \
    $$_PRO_FILE_PWD_/../../../../msceditor/mainwindow.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/mainmodel.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/basetool.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/pointertool.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/instancecreatortool.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/messagecreatortool.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/basecreatortool.cpp \
    $$_PRO_FILE_PWD_/../../../../msceditor/commandlineparser.cpp

HEADERS += \
    $$_PRO_FILE_PWD_/../../../../msceditor/mainwindow.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/mainmodel.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/basetool.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/tooltypes.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/pointertool.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/instancecreatortool.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/messagecreatortool.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/tools/basecreatortool.h \
    $$_PRO_FILE_PWD_/../../../../msceditor/commandlineparser.h

FORMS += \
    $$_PRO_FILE_PWD_/../../../../msceditor/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

INCLUDEPATH += \
    $$_PRO_FILE_PWD_/../../../../libs/asn1editor \
    $$_PRO_FILE_PWD_/../../../../libs/mscwidgets \
    $$_PRO_FILE_PWD_/../../../../libs/msclibrary

DEPENDPATH += \
    $$_PRO_FILE_PWD_/../../../../libs/asn1editor \
    $$_PRO_FILE_PWD_/../../../../libs/mscwidgets \
    $$_PRO_FILE_PWD_/../../../../libs/msclibrary

BuildType = Release
CONFIG(debug, debug|release) {
    BuildType = Debug
} # Profiling, etc (if any) are defaulted to Release
BuildTypeLower = $$lower($$BuildType)

LibSuffix = a
LibPrefix = lib

win32:!win32-g++ {
    LibSuffix = lib
    LibPrefix = ""
}

win32 {
    PRE_TARGETDEPS += \
        $$OUT_PWD/../../../../libs/asn1editor/$$BuildTypeLower/$${LibPrefix}asn1editor.$$LibSuffix \
        $$OUT_PWD/../../../../libs/mscwidgets/$$BuildTypeLower/$${LibPrefix}mscwidgets.$$LibSuffix \
        $$OUT_PWD/../../../../libs/msclibrary/$$BuildTypeLower/$${LibPrefix}msclibrary.$$LibSuffix

    LIBS += \
        -L$$OUT_PWD/../../../../libs/asn1editor/$$BuildTypeLower/ -lasn1editor \
        -L$$OUT_PWD/../../../../libs/mscwidgets/$$BuildTypeLower/ -lmscwidgets \
        -L$$OUT_PWD/../../../../libs/msclibrary/$$BuildTypeLower/ -lmsclibrary

    DEFINES += EXAMPLES_STORAGE_PATH=\\\"$$_PRO_FILE_PWD_/../../../../examples/\\\"
} else:unix {
    PRE_TARGETDEPS += \
        $$OUT_PWD/../../../../libs/asn1editor/$${LibPrefix}asn1editor.$$LibSuffix \
        $$OUT_PWD/../../../../libs/mscwidgets/$${LibPrefix}mscwidgets.$$LibSuffix \
        $$OUT_PWD/../../../../libs/msclibrary/$${LibPrefix}msclibrary.$$LibSuffix

    LIBS += \
        -L$$OUT_PWD/../../../../libs/asn1editor/ -lasn1editor \
        -L$$OUT_PWD/../../../../libs/msclibrary/ -lmsclibrary \
        -L$$OUT_PWD/../../../../libs/mscwidgets/ -lmscwidgets

message($$OUT_PWD)
    DEFINES += EXAMPLES_STORAGE_PATH=\\\"$$_PRO_FILE_PWD_/../../../../examples/\\\"
}

# Add ANTLR runtime

include("$$_PRO_FILE_PWD_/../../../../link_antlr_rt.pri")
