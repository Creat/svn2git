/*
 *  Copyright (C) 2007  Thiago Macieira <thiago@kde.org>
 *  Copyright (C) 2007  Daniel Dewald <daniel.dewald@innogames.com>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LOGGING_Q_PROCESS_H
#define LOGGING_Q_PROCESS_H

#include <QFile>
#include <QProcess>

class LoggingQProcess : public QProcess
{
   
public:
    
    LoggingQProcess(const QString filename);
    ~LoggingQProcess();
    
    qint64 write(const char *data);
    qint64 write(const char *data, qint64 length);
    qint64 write(const QByteArray &data);
    qint64 writeNoLog(const char *data);
    qint64 writeNoLog(const char *data, qint64 length);
    qint64 writeNoLog(const QByteArray &data);
    bool putChar( char c);
    
private:
    
    QFile log;
    bool logging;
};

#endif
