#pragma once

#include <QtGlobal>

#if defined(ASN1SCCPLUGIN_LIBRARY)
#  define ASN1SCCPLUGINSHARED_EXPORT Q_DECL_EXPORT
#else
#  define ASN1SCCPLUGINSHARED_EXPORT Q_DECL_IMPORT
#endif
