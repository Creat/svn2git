svn-all-fast-export aka svn2git
===============================
This project contains all the tools required to do a conversion of an svn repository (server side, not a checkout) to one or more git repositories.

You can find more description and usage examples at https://techbase.kde.org/Projects/MoveToGit/UsingSvn2Git


How does it work
----------------
The svn2git repository gets you an application that will do the actual conversion.
The conversion exists of looping over each and every commit in the subversion repository and matching the changes to a ruleset after which the changes are applied to a certain path in a git repo.
The ruleset can specify which git repository to use and thus you can have more than one git repository as a result of running the conversion.
Also noteworthy is that you can have a rule that, for example, changes in svnrepo/branches/foo/2.1/ will appear as a git-branch in a repository.

If you have a proper ruleset the tool will create the git repositories for you and show progress while converting commit by commit.

After it is done you likely want to run `git repack -a -d -f` to compress the pack file as it can get quite big.

Building the tool
-----------------
Create a folder (e.g. build) that houses your build data. Change into it and run `cmake ../` (you can attach IDE specific instructions if you want). This will get you the platform specific build files. After that just run `make` or open the created project with your IDE of choice to generate the `svn-all-fast-export` binary file. Dependencies are Subversion, Qt and the Apr development files as well as the git binary to compile and the libraries / binaries of the same projects to run the conversion.

KDE
---
there is a repository kde-ruleset which has several example files and one file that should become the final ruleset for the whole of KDE called 'kde-rules-main'.

Write the Rules
---------------
You need to write a rules file that describes how to slice the Subversion history into Git repositories and branches. See https://techbase.kde.org/Projects/MoveToGit/UsingSvn2Git.
The rules are also documented in the 'samples' directory of the svn2git repository. Feel free to add more documentation here as well.

Work flow
---------
Please feel free to fill this section in.

Some SVN tricks
---------------
You can access your newly rsynced SVN repo with commands like `svn ls file:///path/to/repo/trunk/KDE`.
A common issue is tracking when an item left playground for kdereview and then went from kdereview to its final destination. There is no straightforward way to do this. So the following command comes in handy: `svn log -v file:///path/to/repo/kde-svn/kde/trunk/kdereview | grep /trunk/kdereview/mplayerthumbs -A 5 -B 5` This will print all commits relevant to the package you are trying to track. You can also pipe the above command to head or tail to see the the first and last commit it was in that directory.
