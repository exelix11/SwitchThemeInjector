# Tests
The most challenging part of the project is to make sure nothing breaks after changes in the code. This is quite hard as we treat qlaunch as a black-box that may or may not accept our edits to the system files.
So most tests are about ensuring that the output is consistent with previous versions.

Unfortunately it means that we need original szs files to test this, i can't include them in the repo so for the time being you will have to obtain them by yourself.

My reference currently are files from 10.0 qlaunch. The ResidentMenu.szs file SHA256 starts with `199FA9B627EDF`.

Put original files in `Cases/Source `and expected results in `Cases/Expected`.

Synthetic tests are files not actually from the home menu that can be used as a baseline for testing certain features, I plan to expand on them as using non-copyrighted files would mean having tests that can be automated with github actions or equivalent.