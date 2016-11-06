#include "RuleMatch.h"

#include <QRegExp>
#include <QString>
#include <QDebug>
#include <QByteArray>
#include <QStringBuilder>

RuleMatch::RuleMatch() :
    minRevision(-1), 
    maxRevision(-1), 
    annotate(false), 
    action(Ignore) 
{ 
    
}
        
bool RuleMatch::operator<(const RuleMatch other) const 
{
    if (filename != other.filename)
    {
        return filename < other.filename;
    }
         
    return lineNumber < other.lineNumber;
}
        
const QString RuleMatch::info() const 
{
    const QString info = filename % ":" % QByteArray::number(lineNumber) % " " % rx.pattern();
    return info;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug s, const RuleMatch& rule)
{
    s.nospace() << rule.info();
    return s.space();
}
#endif
