#ifndef QTQSMARTBULBGLOBAL_H
#define QTQSMARTBULBGLOBAL_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#ifndef QT_STATIC
#  if defined(QT_BUILD_SMARTBULB_LIB)
#    define Q_SMARTBULB_EXPORT Q_DECL_EXPORT
#  else
#    define Q_SMARTBULB_EXPORT Q_DECL_IMPORT
#  endif
#else
#  define Q_SMARTBULB_EXPORT
#endif

namespace QSmartBulb
{
    
}

QT_END_NAMESPACE

#endif //QTQSMARTBULBGLOBAL_H
