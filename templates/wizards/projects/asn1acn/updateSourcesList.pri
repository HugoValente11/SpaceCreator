######################################################################
# Copyright (C) 2017-2019 N7 Space sp. z o. o.
# Contact: http://n7space.com
#
# This file is part of ASN.1/ACN Plugin for QtCreator.
#
# Plugin was developed under a programme and funded by
# European Space Agency.
#
# This Plugin is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This Plugin is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#######################################################################

ASN1SCC_C_MAIN_HEADER = $$clean_path($${ASN1SCC_C_DIR}/asn1crt.h)
ASN1SCC_ADA_MAIN_HEADER = $$clean_path($${ASN1SCC_ADA_DIR}/adaasn1rtl.ads)

defineReplace(createDependencyOrder) {
  file = $${1}
  dependencyRoot = $${2}

  equals(file, dependencyRoot) {
     return($$file)
  }

  stripedFile = $$replace(file, " ", "")

  eval($${stripedFile}_custom.target = $$relative_path($${file},$${OUT_PWD}))
  eval($${stripedFile}_custom.depends = $${dependencyRoot})
  eval(export($${stripedFile}_custom.target))
  eval(export($${stripedFile}_custom.depends))

  QMAKE_EXTRA_TARGETS += $${stripedFile}_custom
  export(QMAKE_EXTRA_TARGETS)

  return($$file)
}

defineReplace(createFileNames) {
    allFiles = $${1}

    for(file, allFiles) {
        fileBaseName = $$basename(file)
        splitted = $$split(fileBaseName, ".")
        extension = $$last(splitted)

        equals(extension, asn)|equals(extension, asn1) {
            fileNames += $$first(splitted)
        }
    }

    return($$fileNames)
}

defineReplace(createEmptyFiles) {
    files = $${1}
    contents = "File will be generated by asn1scc during build"

    for(file, files) {
        write_file($$file, contents)
        unix|macx {
            system("touch -t 197001020000 $$shell_quote($${file})")
        } else {
            touch($$file, $${QMAKE_QMAKE})
        }
    }
    return($$files)
}

defineReplace(createGeneratedFilesList) {
    sourcesNames = $${1}
    extension = $${2}
    dependencyRoot = $${3}
    targetDir = $${4}

    for(name, sourcesNames) {
        source = $$clean_path($${targetDir}/$${name}.$${extension})
        sources += $$createDependencyOrder($$source, $$dependencyRoot)
    }

    return($$createEmptyFiles($$sources))
}

defineReplace(createGeneratedFilesListC) {
    l = $$createGeneratedFilesList($${1}, $${2}, $${ASN1SCC_C_MAIN_HEADER}, $${ASN1SCC_C_DIR})
    return($$l)
}

defineReplace(createGeneratedFilesListAda) {
    l = $$createGeneratedFilesList($${1}, $${2}, $${ASN1SCC_ADA_MAIN_HEADER}, $${ASN1SCC_ADA_DIR})
    return($$l)
}

names = $$createFileNames($$DISTFILES)

generateC {
    cNames = $$names
    !isEmpty(names) {
        cNames += asn1crt real

        contains(ASN1SCC_C_OPTIONS, --acn-enc) {
            cNames += acn
        }
    }

    SOURCES += $$createGeneratedFilesListC($$cNames, "c")
    HEADERS += $$createGeneratedFilesListC($$cNames, "h")
    HEADERS += $$createEmptyFiles($${ASN1SCC_C_MAIN_HEADER})

    INCLUDEPATH += $$ASN1SCC_C_DIR

    PRE_TARGETDEPS += $${ASN1SCC_C_MAIN_HEADER}
}

generateAda {
    adaNames = $$lower($$names)
    !isEmpty(names) {
        adaNames += adaasn1rtl
    }

    SOURCES += $$createGeneratedFilesListAda($$adaNames, "adb")
    HEADERS += $$createGeneratedFilesListAda($$adaNames, "ads")
    HEADERS += $$createEmptyFiles($${ASN1SCC_ADA_MAIN_HEADER})

    PRE_TARGETDEPS += $${ASN1SCC_ADA_MAIN_HEADER}
}
