#!/bin/bash

git diff 8780dc83dd6a06ed76a5dfd215b054012c2a4577 -- '*.c' '*.h' '*.S' > srcdiff
checkpatch.pl --ignore FILE_PATH_CHANGES -terse --no-signoff -no-tree --summary-file srcdiff
rm srcdiff
