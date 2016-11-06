#include "LoggingQProcess.h"

#include "commandline/CommandLineParser.h"

LoggingQProcess::LoggingQProcess(const QString filename) : QProcess(), log() 
{
    if(CommandLineParser::instance()->contains("debug-rules"))
    {
        logging = true;
        QString name = filename;
        name.replace('/', '_');
        name.prepend("gitlog-");
        log.setFileName(name);
        log.open(QIODevice::WriteOnly);
    } 
    else 
    {
        logging = false;
    }
};
    
LoggingQProcess::~LoggingQProcess() 
{
    if(logging) 
    {
        log.close();
    }
};

qint64 LoggingQProcess::write(const char *data) 
{
    Q_ASSERT(state() == QProcess::Running);
    if(logging) 
    {
        log.write(data);
    }
        
    return QProcess::write(data);
}
    
qint64 LoggingQProcess::write(const char *data, qint64 length) 
{
    Q_ASSERT(state() == QProcess::Running);
    if(logging) 
    {
        log.write(data);
    }
        
    return QProcess::write(data, length);
}
    
qint64 LoggingQProcess::write(const QByteArray &data) 
{
    Q_ASSERT(state() == QProcess::Running);
    if(logging) 
    {
        log.write(data);
    }
        
    return QProcess::write(data);
}
    
qint64 LoggingQProcess::writeNoLog(const char *data) 
{
    Q_ASSERT(state() == QProcess::Running);
    return QProcess::write(data);
}
    
qint64 LoggingQProcess::writeNoLog(const char *data, qint64 length) 
{
    Q_ASSERT(state() == QProcess::Running);
    return QProcess::write(data, length);
}
    
qint64 LoggingQProcess::writeNoLog(const QByteArray &data) 
{
    Q_ASSERT(state() == QProcess::Running);
    return QProcess::write(data);
}
    
bool LoggingQProcess::putChar( char c) 
{
    Q_ASSERT(state() == QProcess::Running);
    if(logging) 
    {
        log.putChar(c);
    }
        
    return QProcess::putChar(c);
}
