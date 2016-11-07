#include "ForwardingGitRepository.h"

ForwardingGitRepository::ForwardingGitRepository(const QString &n, GitRepository *r, const QString &p) : 
    name(n), 
    repo(r), 
    prefix(p) 
{
}

ForwardingGitRepository::~ForwardingGitRepository()
{
}

int ForwardingGitRepository::setupIncremental(int &) 
{ 
    return 1; 
}
    
void ForwardingGitRepository::restoreLog() 
{
}

void ForwardingGitRepository::reloadBranches() 
{ 
    return repo->reloadBranches(); 
}

int ForwardingGitRepository::createBranch(const QString &branch, int revnum, const QString &branchFrom, int revFrom) 
{ 
    return repo->createBranch(branch, revnum, branchFrom, revFrom); 
}

int ForwardingGitRepository::deleteBranch(const QString &branch, int revnum) 
{ 
    return repo->deleteBranch(branch, revnum); 
}

GitRepositoryTransaction* ForwardingGitRepository::newTransaction(const QString &branch, const QString &svnprefix, int revnum)
{
    GitRepositoryTransaction *t = repo->newTransaction(branch, svnprefix, revnum);
    return new ForwardingGitRepositoryTransaction(t, prefix);
}

void ForwardingGitRepository::createAnnotatedTag(const QString &name, const QString &svnprefix, int revnum, const QByteArray &author, uint dt, const QByteArray &log) 
{ 
    repo->createAnnotatedTag(name, svnprefix, revnum, author, dt, log); 
}

void ForwardingGitRepository::close()
{
	/* Nothing to do */	
}

void ForwardingGitRepository::finalizeTags() 
{ 
    /* loop that called this will invoke it on 'repo' too */ 
}

void ForwardingGitRepository::commit() 
{ 
    repo->commit(); 
}

bool ForwardingGitRepository::branchExists(const QString& branch) const 
{ 
    return repo->branchExists(branch); 
}

const QByteArray ForwardingGitRepository::branchNote(const QString& branch) const 
{ 
    return repo->branchNote(branch); 
}

void ForwardingGitRepository::setBranchNote(const QString& branch, const QByteArray& noteText) 
{ 
    repo->setBranchNote(branch, noteText); 
}

bool ForwardingGitRepository::hasPrefix() const 
{ 
    return !prefix.isEmpty() || repo->hasPrefix(); 
}

const QString& ForwardingGitRepository::getName() const
{ 
    return name; 
}

GitRepository* ForwardingGitRepository::getEffectiveRepository() 
{ 
    return repo->getEffectiveRepository(); 
}
