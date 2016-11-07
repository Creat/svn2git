#include "ForwardingGitRepositoryTransaction.h"

#include <QIODevice>

ForwardingGitRepositoryTransaction::ForwardingGitRepositoryTransaction(GitRepositoryTransaction* t, const QString& p) : 
    txn(t), 
    prefix(p) 
{
    
}
    
ForwardingGitRepositoryTransaction::~ForwardingGitRepositoryTransaction() 
{ 
    delete txn; 
}

void ForwardingGitRepositoryTransaction::commit() 
{ 
    txn->commit(); 
}

void ForwardingGitRepositoryTransaction::setAuthor(const QByteArray& author) 
{ 
    txn->setAuthor(author); 
}

void ForwardingGitRepositoryTransaction::setDateTime(uint dt) 
{ 
    txn->setDateTime(dt); 
}

void ForwardingGitRepositoryTransaction::setLog(const QByteArray& log)
{ 
    txn->setLog(log); 
}

void ForwardingGitRepositoryTransaction::noteCopyFromBranch (const QString& prevbranch, int revFrom)
{ 
    txn->noteCopyFromBranch(prevbranch, revFrom); 
}

void ForwardingGitRepositoryTransaction::deleteFile(const QString& path) 
{ 
    txn->deleteFile(prefix + path); 
}

QIODevice* ForwardingGitRepositoryTransaction::addFile(const QString& path, int mode, qint64 length)  
{ 
    return txn->addFile(prefix + path, mode, length); 
}

void ForwardingGitRepositoryTransaction::commitNote(const QByteArray& noteText, bool append, const QByteArray& commit) 
{ 
    return txn->commitNote(noteText, append, commit); 
}
