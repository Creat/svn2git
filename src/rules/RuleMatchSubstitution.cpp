#include "RuleMatchSubstitution.h"

bool RuleMatchSubstitution::isValid()
{ 
    return !pattern.isEmpty(); 
}

QString& RuleMatchSubstitution::apply(QString &string) 
{ 
    return string.replace(pattern, replacement); 
}
