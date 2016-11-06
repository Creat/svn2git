#include "Rule.h"

Rule::Rule() :
    lineNumber(0) 
{
    
}

const QString& Rule::getFilename() const
{
    return filename;
}

int Rule::getLineNumber() const
{
    return lineNumber;
}
    

void Rule::setFilename(const QString& newFilename)
{
    filename = newFilename;
}

void Rule::setLineNumber(const int newLineNumber)
{
    lineNumber = newLineNumber;
}
