#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source "${DIR}/setup-env.sh"

echo "Preparing distro"

echo "Copy lib"
if [[ $BUILD_OS_NAME == 'linux' ]]; then
    mkdir -p ${DIST_DIR}/lib/qtcreator/plugins
    cp ${QTC_BUILD_DIR}/lib/qtcreator/plugins/lib${PLUGIN_NAME}.so ${DIST_DIR}/lib/qtcreator/plugins
    RESOURCE_DIST_DIR=${DIST_DIR}/share/qtcreator
    LIBEXEC_DIST_DIR=${DIST_DIR}/libexec/qtcreator
    OS_TAR_SUFFIX="linux-x64"
elif [[ $BUILD_OS_NAME == 'osx' ]]; then
    mkdir -p ${DIST_DIR}/Contents/PlugIns
    cp "${QTC_BUILD_DIR}/Qt Creator.app/Contents/PlugIns/lib${PLUGIN_NAME}.dylib" ${DIST_DIR}/Contents/PlugIns
    RESOURCE_DIST_DIR=${DIST_DIR}/Contents/Resources
    LIBEXEC_DIST_DIR=${RESOURCE_DIST_DIR}
    OS_TAR_SUFFIX="osx"
fi

echo "Downloading asn1scc to distribute with plugin"
wget -O ${DOWNLOAD_DIR}/asn1scc.7z "https://n7space-my.sharepoint.com/:u:/p/kgrochowski/EZSwzkHV-C1Mn1nppkUqKIcBjFpzsn67G_N_1RMW2Yr7dQ?download=1"
mkdir -p ${LIBEXEC_DIST_DIR}
7zr x -o${LIBEXEC_DIST_DIR} ${DOWNLOAD_DIR}/asn1scc.7z

if [[ $BUILD_OS_NAME == 'linux' ]]; then
    echo "Downloading asn1scc.Fuzzer to distribute with plugin"
    wget -O ${DOWNLOAD_DIR}/fuzzer.7z "https://github.com/n7space/asn1scc.Fuzzer/releases/download/${FUZZER_VERSION}/asn1scc-Fuzzer-${FUZZER_VERSION}-linux-x64.tar.gz"
    mkdir -p ${LIBEXEC_DIST_DIR}
    pushd ${LIBEXEC_DIST_DIR}
    tar -xvf ${DOWNLOAD_DIR}/fuzzer.7z
    popd
fi

echo "Downloading PUS-C lib to distribute with plugin"
wget -O ${DOWNLOAD_DIR}/pusc-lib.7z "https://github.com/n7space/asn1-pusc-lib/releases/download/${PUSC_VERSION}/Asn1Acn-PusC-Library-${PUSC_VERSION}.7z"
PUSC_LIB_DIR=${RESOURCE_DIST_DIR}/asn1acn/libs/PUS-C
mkdir -p ${PUSC_LIB_DIR}
7zr x -o${PUSC_LIB_DIR} ${DOWNLOAD_DIR}/pusc-lib.7z

echo "Copy static files"
mkdir -p ${RESOURCE_DIST_DIR}
cp -r ${PROJECT_DIR}/generic-highlighter ${RESOURCE_DIST_DIR}
cp -r ${PROJECT_DIR}/templates ${RESOURCE_DIST_DIR}
cp -r ${PROJECT_DIR}/snippets ${RESOURCE_DIST_DIR}

cp ${PROJECT_DIR}/LICENSE ${DIST_DIR}/${PLUGIN_NAME}-LICENSE
cp ${PROJECT_DIR}/README.md ${DIST_DIR}/${PLUGIN_NAME}-README.md

echo "Creating package"
pushd ${DIST_DIR}
tar czf ${PROJECT_DIR}/${PLUGIN_NAME}-${TRAVIS_TAG}-${OS_TAR_SUFFIX}.tar.gz *
popd
