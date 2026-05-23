#pragma once

#include <QtCore/qglobal.h>

#if defined(LOGSERVICE_LIBRARY)
#define LOGSERVICE_EXPORT Q_DECL_EXPORT
#else
#define LOGSERVICE_EXPORT Q_DECL_IMPORT
#endif