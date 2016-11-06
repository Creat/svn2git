#include "GitProcessCache.h"

void GitProcessCache::touch(FastImportGitRepository* repo)
{
    remove(repo);

    // if the cache is too big, remove from the front
    while (size() >= maxSimultaneousProcesses)
    {
        takeFirst()->closeFastImport();
    }

    // append to the end
    append(repo);
}
