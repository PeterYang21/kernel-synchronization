#!/bin/sh

#
# Date modified: 10/29/2017 12:48PM -0400
#

#
# Get ID of the commit that the patch should be based on.
#
# The following git-log command outputs ID of the latest commit that modifies
# mkpatch.sh. Every time we'd like to include a commit onto the patch base,
# please update the date above and include this file in the commit.
#
commit_id=`git log -1 --pretty=format:"%H" --follow mkpatch.sh`

# Generate the diff
git diff $commit_id linux-3.18.77
