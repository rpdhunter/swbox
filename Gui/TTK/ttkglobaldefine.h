#ifndef TTKGLOBALDEFINE_H
#define TTKGLOBALDEFINE_H

#include <QObject>

///
#ifdef TTK_LIBRARY
#  define TTK_EXPORT
#endif

#define TTK_BUILD_EXAMPLE

#ifdef TTK_EXPORT
#  define TTK_CORE_EXPORT Q_DECL_EXPORT
#else
#  define TTK_CORE_IMPORT Q_DECL_IMPORT
#endif

#endif // TTKGLOBALDEFINE_H
