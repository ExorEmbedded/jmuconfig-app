#!/bin/sh

GITCONFIG=.git

if [ -f .git ] ; then
    GITCONFIG="$(cat .git | awk '{print $2}' )"
    echo "Wer are a git submodule pointing to $GITCONFIG"
fi

if ( ! grep svn $GITCONFIG/config ) ; then
    echo "Patching .git/config"
    cat >> $GITCONFIG/config  <<EOF
[svn-remote "svn"]
    url = http://exorint.unfuddle.com/svn/exorint_qthmi/branches/Production/QTHmi/HMIRuntimeCommon/InputPanel
    fetch = :refs/remotes/git-svn
EOF
fi

git show origin/master | head -n 1 | awk '{print $2}' > $GITCONFIG/refs/remotes/trunk

echo "Fetching from svn. Please wait..."
echo ""
git svn fetch

echo "Done"