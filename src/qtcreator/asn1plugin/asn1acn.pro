#############################################################################
##
## Copyright (C) 2017-2019 N7 Space sp. z o. o.
## Contact: http://n7space.com
##
## This file is part of ASN.1/ACN Plugin for QtCreator.
##
## Plugin was developed under a programme and funded by
## European Space Agency.
##
## This Plugin is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This Plugin is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <http://www.gnu.org/licenses/>.
##
#############################################################################

# Qt Creator linking

IDE_SOURCE_TREE = $$QTC_SOURCE
isEmpty(IDE_SOURCE_TREE): IDE_SOURCE_TREE = "/opt/qt-creator-dev/qt-creator"

IDE_BUILD_TREE = $$QTC_BUILD
isEmpty(IDE_BUILD_TREE): IDE_BUILD_TREE = "/opt/qt-creator-dev/build-debug"

QTC_PLUGIN_NAME = Asn1Acn
QTC_LIB_DEPENDS += \
    utils

QTC_PLUGIN_DEPENDS += \
    texteditor \
    coreplugin \
    projectexplorer

QTC_PLUGIN_RECOMMENDS += \
    # optional plugin dependencies. nothing here at this time

include($$IDE_SOURCE_TREE/src/qtcreatorplugin.pri)

### Plugin ###

CONFIG += object_parallel_to_source

DEFINES += ASN1ACN_LIBRARY
INCLUDEPATH += $$PWD/src
VPATH += $$PWD/src

# asn1acn plugin files

SOURCES += \
    completion/autocompleter.cpp \
    completion/asncompletionassist.cpp \
    completion/asnsnippets.cpp \
    completion/acnsnippets.cpp \
    completion/proposalsbuilder.cpp \
    completion/asnkeywordproposalsbuilder.cpp \
    completion/usertypesproposalsbuilder.cpp \
    completion/acnkeywordproposalsbuilder.cpp \
    completion/completionassist.cpp \
    completion/acncompletionassist.cpp \
    completion/keywordproposalsbuilder.cpp \
    completion/importfindingvisitor.cpp \
    \
    data/sourcelocation.cpp \
    data/node.cpp \
    data/root.cpp \
    data/file.cpp \
    data/project.cpp \
    data/typeassignment.cpp \
    data/valueassignment.cpp \
    data/typereference.cpp \
    data/definitions.cpp \
    data/visitor.cpp \
    \
    data/types/type.cpp \
    data/types/userdefinedtype.cpp \
    data/types/labeltype.cpp \
    data/types/builtintypes.cpp \
    \
    tree-views/outline-visitors/childrencountingvisitor.cpp \
    tree-views/outline-visitors/childreturningvisitor.cpp \
    tree-views/outline-visitors/indexfindingvisitor.cpp \
    \
    tree-views/combo-visitors/childrencountingvisitor.cpp \
    tree-views/combo-visitors/childreturningvisitor.cpp \
    tree-views/combo-visitors/indexfindingvisitor.cpp \
    \
    tree-views/typestree-visitors/parentreturningvisitor.cpp \
    tree-views/typestree-visitors/childrencountingvisitor.cpp \
    tree-views/typestree-visitors/childreturningvisitor.cpp \
    tree-views/typestree-visitors/indexfindingvisitor.cpp \
    \
    tree-views/displayrolevisitor.cpp \
    tree-views/decorationrolevisitor.cpp \
    tree-views/mutablerootmodel.cpp \
    tree-views/outlinemodel.cpp \
    tree-views/typestreemodel.cpp \
    tree-views/model.cpp \
    tree-views/combomodel.cpp \
    tree-views/treeviewwidget.cpp \
    tree-views/activatehandler.cpp \
    tree-views/indexupdater.cpp \
    tree-views/outlineindexupdater.cpp \
    tree-views/typestreeindexupdater.cpp \
    tree-views/synchronizedindexupdater.cpp \
    tree-views/unsynchronizedindexupdater.cpp \
    tree-views/outlinecombo.cpp \
    tree-views/outlinewidget.cpp \
    tree-views/typestreewidget.cpp \
    tree-views/expansionstaterestorer.cpp \
    \
    options-pages/service.cpp \
    options-pages/libraries.cpp \
    options-pages/fuzzer.cpp \
    options-pages/servicewidget.cpp \
    options-pages/librarieswidget.cpp \
    options-pages/fuzzerwidget.cpp \
    \
    settings/settings.cpp \
    settings/service.cpp \
    settings/libraries.cpp \
    settings/fuzzer.cpp \
    \
    fuzzer/fuzzerparamsdialog.cpp \
    fuzzer/fuzzerparamswidget.cpp \
    fuzzer/fuzzerrunner.cpp \
    fuzzer/fuzzerparamsprovider.cpp \
    \
    libraries/modulemetadataparser.cpp \
    libraries/generalmetadataparser.cpp \
    libraries/componentimporter.cpp \
    libraries/componentdirectorywatcher.cpp \
    libraries/componentlibraryprocessor.cpp \
    libraries/generalmetadataprocessor.cpp \
    libraries/componentlibrarydispatcher.cpp \
    libraries/librarystorage.cpp \
    libraries/metadatamodel.cpp \
    libraries/filemodel.cpp \
    libraries/metadatacheckstatehandler.cpp \
    \
    libraries/wizard/importcomponentwizard.cpp \
    libraries/wizard/selectsourcepage.cpp \
    libraries/wizard/selectcomponentspage.cpp \
    libraries/wizard/summarypage.cpp \
    libraries/wizard/metadatacomponentselector.cpp \
    libraries/wizard/filecomponentselector.cpp \
    libraries/wizard/relationslabelscontroller.cpp \
    libraries/wizard/vcshandler.cpp \
    libraries/wizard/modulestreeview.cpp \
    \
    messages/messageutils.cpp \
    messages/messagemanager.cpp \
    \
    icd/icdbuilder.cpp \
    \
    asn1acn.cpp \
    asneditor.cpp \
    asndocument.cpp \
    acneditor.cpp \
    acndocument.cpp \
    editor.cpp \
    asn1sccdocumentprocessor.cpp \
    parseddatastorage.cpp \
    astxmlparser.cpp \
    projectwatcher.cpp \
    document.cpp \
    asn1acnjsextension.cpp \
    asn1sccserviceprovider.cpp \
    asn1sccparseddocumentbuilder.cpp \
    projectcontenthandler.cpp \
    indenter.cpp \
    tools.cpp \
    linkcreator.cpp \
    referencefinder.cpp \
    filesourcereader.cpp \
    errormessageparser.cpp \
    typeslocator.cpp \
    modelvalidityguard.cpp \
    usagesfinder.cpp \
    toolsmenuimportitemcontroller.cpp \
    projectmenuimportitemcontroller.cpp\
    kitinformation.cpp \
    kitconfigwidget.cpp \
    selectionpositionresolver.cpp \
    asn1acnerrorparser.cpp \
    asn1acnstepscache.cpp \
    asn1acnbuildstep.cpp \
    asn1acnbuildsteprunner.cpp

HEADERS += \
    completion/autocompleter.h \
    completion/asncompletionassist.h \
    completion/asnsnippets.h \
    completion/acnsnippets.h \
    completion/proposalsbuilder.h \
    completion/asnkeywordproposalsbuilder.h \
    completion/usertypesproposalsbuilder.h \
    completion/acnkeywordproposalsbuilder.h \
    completion/completionassist.h \
    completion/acncompletionassist.h \
    completion/keywordproposalsbuilder.h \
    completion/completiontypedefs.h \
    completion/importfindingvisitor.h \
    \
    data/sourcelocation.h \
    data/typeassignment.h \
    data/valueassignment.h \
    data/definitions.h \
    data/typereference.h \
    data/errormessage.h \
    data/node.h \
    data/root.h \
    data/file.h \
    data/project.h \
    data/visitor.h \
    data/visitorwithvalue.h \
    data/importedtype.h \
    data/importedvalue.h \
    \
    data/types/type.h \
    data/types/userdefinedtype.h \
    data/types/labeltype.h \
    data/types/builtintypes.h \
    \
    tree-views/outline-visitors/childrencountingvisitor.h \
    tree-views/outline-visitors/childreturningvisitor.h \
    tree-views/outline-visitors/indexfindingvisitor.h \
    \
    tree-views/combo-visitors/childrencountingvisitor.h \
    tree-views/combo-visitors/childreturningvisitor.h \
    tree-views/combo-visitors/indexfindingvisitor.h \
    \
    tree-views/typestree-visitors/parentreturningvisitor.h \
    tree-views/typestree-visitors/childrencountingvisitor.h \
    tree-views/typestree-visitors/childreturningvisitor.h \
    tree-views/typestree-visitors/indexfindingvisitor.h \
    \
    tree-views/displayrolevisitor.h \
    tree-views/decorationrolevisitor.h \
    tree-views/mutablerootmodel.h \
    tree-views/outlinemodel.h \
    tree-views/typestreemodel.h \
    tree-views/typestreeindexupdater.h \
    tree-views/model.h \
    tree-views/combomodel.h \
    tree-views/treeviewwidget.h \
    tree-views/activatehandler.h \
    tree-views/indexupdater.h \
    tree-views/outlineindexupdater.h \
    tree-views/unsynchronizedindexupdater.h \
    tree-views/synchronizedindexupdater.h \
    tree-views/outlinecombo.h \
    tree-views/outlinewidget.h \
    tree-views/typestreewidget.h \
    tree-views/expansionstaterestorer.h \
    tree-views/indexupdaterfactory.h \
    tree-views/typestreeindexupdaterfactory.h \
    tree-views/outlineindexupdaterfactory.h \
    \
    options-pages/service.h \
    options-pages/libraries.h \
    options-pages/fuzzer.h \
    options-pages/servicewidget.h \
    options-pages/librarieswidget.h \
    options-pages/fuzzerwidget.h \
    \
    settings/settings.h \
    settings/service.h \
    settings/libraries.h \
    settings/fuzzer.h \
    \
    fuzzer/fuzzerparamsdialog.h \
    fuzzer/fuzzerparamswidget.h \
    fuzzer/fuzzerrunner.h \
    fuzzer/fuzzerparamsprovider.h \
    \
    libraries/modulemetadataparser.h \
    libraries/generalmetadataparser.h \
    libraries/componentimporter.h \
    libraries/componentdirectorywatcher.h \
    libraries/componentlibraryprocessor.h \
    libraries/generalmetadataprocessor.h \
    libraries/componentlibrarydispatcher.h \
    libraries/librarystorage.h \
    libraries/metadatamodel.h \
    libraries/filemodel.h \
    libraries/metadatacheckstatehandler.h \
    \
    libraries/wizard/importcomponentwizard.h \
    libraries/wizard/selectsourcepage.h \
    libraries/wizard/selectcomponentspage.h \
    libraries/wizard/summarypage.h \
    libraries/wizard/metadatacomponentselector.h \
    libraries/wizard/componentselector.h \
    libraries/wizard/filecomponentselector.h \
    libraries/wizard/relationslabelscontroller.h \
    libraries/wizard/vcshandler.h \
    libraries/wizard/modulestreeview.h \
    \
    libraries/metadata/element.h \
    libraries/metadata/reference.h \
    libraries/metadata/submodule.h \
    libraries/metadata/module.h \
    libraries/metadata/library.h \
    libraries/metadata/general.h \
    libraries/metadata/librarynode.h \
    \
    messages/messageutils.h \
    messages/messagemanager.h \
    \
    icd/icdbuilder.h \
    \
    asn1acn_global.h \
    asn1acnconstants.h \
    asn1acn.h \
    asneditor.h \
    asndocument.h \
    acneditor.h \
    acndocument.h \
    editor.h \
    asn1sccdocumentprocessor.h \
    parseddatastorage.h \
    astxmlparser.h \
    projectwatcher.h \
    document.h \
    asn1acnjsextension.h \
    asn1sccserviceprovider.h \
    asn1sccparseddocumentbuilder.h \
    projectcontenthandler.h \
    indenter.h \
    tr.h \
    linkcreator.h \
    referencefinder.h \
    tools.h \
    parsingserviceprovider.h \
    parseddocumentbuilder.h \
    documentprocessor.h \
    sourcereader.h \
    filesourcereader.h \
    errormessageparser.h \
    typeslocator.h \
    modelvalidityguard.h \
    usagesfinder.h \
    toolsmenuimportitemcontroller.h \
    projectmenuimportitemcontroller.h \
    kitinformation.h \
    kitconfigwidget.h \
    selectionpositionresolver.h \
    asn1acnerrorparser.h \
    asn1acnstepscache.h \
    asn1acnbuildstep.h \
    asn1acnbuildsteprunner.h

FORMS += \
    fuzzer/fuzzerparams.ui \
    \
    options-pages/service.ui \
    options-pages/libraries.ui \
    options-pages/fuzzer.ui \
    \
    libraries/wizard/import_component.ui \
    libraries/wizard/select_component.ui \
    libraries/wizard/summary.ui

RESOURCES += \
    asn1acn.qrc

DISTFILES += \
    LICENSE \
    README.md \
    schemas/asn1-lib-general-schema.json \
    schemas/asn1-lib-module-schema.json

# test files

equals(TEST, 1) {

SOURCES += \
    libraries/tests/modulemetadataparser_tests.cpp \
    libraries/tests/generalmetadataparser_tests.cpp \
    libraries/tests/metadatamodel_tests.cpp \
    libraries/tests/filemodel_tests.cpp \
    libraries/tests/metadatacheckstatehandler_tests.cpp \
    \
    3rdparty/tests/modeltest.cpp \
    \
    tree-views/tests/outlinemodel_tests.cpp \
    tree-views/tests/combomodel_tests.cpp \
    tree-views/tests/typestreemodel_tests.cpp \
    tree-views/tests/displayrolevisitor_tests.cpp \
    tree-views/tests/outlineindexupdater_tests.cpp \
    \
    tests/astxmlparser_tests.cpp \
    tests/errormessageparser_tests.cpp \
    tests/parseddocumentbuilder_tests.cpp \
    tests/documentprocessor_tests.cpp \
    tests/projectcontenthandler_tests.cpp \
    tests/parseddatastorage_tests.cpp \
    tests/autocompleter_tests.cpp \
    tests/modelvalidityguard_tests.cpp \
    tests/linkcreator_tests.cpp \
    tests/indenter_tests.cpp \
    tests/selectionpositionresolver_test.cpp \
    \
    tests/networkreply.cpp \
    tests/parsingserviceproviderstub.cpp \
    tests/parseddocumentbuilderstub.cpp \
    tests/documentprocessorstub.cpp \
    tests/sourcereadermock.cpp

HEADERS += \
    libraries/tests/modulemetadataparser_tests.h \
    libraries/tests/generalmetadataparser_tests.h \
    libraries/tests/metadatamodel_tests.h \
    libraries/tests/filemodel_tests.h \
    libraries/tests/metadatacheckstatehandler_tests.h \
    \
    tree-views/tests/outlinemodel_tests.h \
    tree-views/tests/combomodel_tests.h \
    tree-views/tests/typestreemodel_tests.h \
    tree-views/tests/displayrolevisitor_tests.h \
    tree-views/tests/outlineindexupdater_tests.h \
    \
    3rdparty/tests/modeltest.h \
    \
    tests/astxmlparser_tests.h \
    tests/errormessageparser_tests.h \
    tests/parseddocumentbuilder_tests.h \
    tests/documentprocessor_tests.h \
    tests/projectcontenthandler_tests.h \
    tests/parseddatastorage_tests.h \
    tests/autocompleter_tests.h \
    tests/modelvalidityguard_tests.h \
    tests/linkcreator_tests.h \
    tests/indenter_tests.h \
    tests/selectionpositionresolver_test.h \
    \
    tests/networkreply.h \
    tests/parsingserviceproviderstub.h \
    tests/parseddocumentbuilderstub.h \
    tests/documentprocessorstub.h \
    tests/sourcereadermock.h
}

### Static files ###

STATIC_FILES += \
    snippets/asn1.xml \
    snippets/acn.xml \
    generic-highlighter/syntax/asn1.xml \
    generic-highlighter/syntax/acn.xml \
    templates/wizards/files/acn/wizard.json \
    templates/wizards/files/acn/template.acn \
    templates/wizards/files/asn1/wizard.json \
    templates/wizards/files/asn1/template.asn \
    templates/wizards/files/asn1acn/wizard.json \
    templates/wizards/files/asn1acn/template.asn \
    templates/wizards/files/asn1acn/template.acn \
    templates/wizards/projects/asn1acn/wizard.json \
    templates/wizards/projects/asn1acn/template.asn \
    templates/wizards/projects/asn1acn/template.acn \
    templates/wizards/projects/asn1acn/file.pro \
    templates/wizards/projects/asn1acn/generateFromAsn1.pri \
    templates/wizards/projects/asn1acn/handleAsn1AcnBuild.pri \
    templates/wizards/projects/asn1acn/updateSourcesList.pri \
    templates/wizards/projects/asn1acn/CMakeLists.txt \
    templates/wizards/projects/asn1acn/asn1sccSettings.cmake \
    templates/wizards/projects/asn1acn/configureAsn1SccTarget.cmake \
    templates/wizards/projects/asn1acn/createFilesList.cmake

STATIC_BASE = $$PWD
STATIC_OUTPUT_BASE = $$IDE_DATA_PATH
STATIC_INSTALL_BASE = $$INSTALL_DATA_PATH

include($$IDE_SOURCE_TREE/qtcreatordata.pri)
