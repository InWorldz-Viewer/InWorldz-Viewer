#!/usr/bin/env python
""" Update version numbers across multiple files.

The script needs to be called with the new 4 part version number
as the first argument with the numbers separated by periods.
In addition, an optional channel argument can be included second.
If the channel text contains any spaces it should be enclosed
within quotation marks.

The script should be placed within the "scripts" dir found in the
top level of the viewer's directory structure.

Example usage:
python version_update.py 1.2.3.4 "New version"

"""

from sys import argv
from os.path import *
import fileinput
import re

version = argv[1]
if len(argv) >= 3:
    channel = argv[2]
(major, minor, patch, build) = version.split('.')
version_commas = version.replace('.', ',')

def change(file_path, searchExp, replaceExp):
    for line in fileinput.input(file_path, inplace=1):
        index = line.find(searchExp)
        if index >= 0:
            index += len(searchExp)
            line = line[0: index] + replaceExp
        print(line),
    fileinput.close()


# ../indra/llcommon/llversionviewer.h
file_path = abspath(join(dirname(__file__), '..', 'indra',
                         'llcommon', 'llversionviewer.h'))
change(file_path, 'const S32 LL_VERSION_MAJOR = ', major+';\n')
change(file_path, 'const S32 LL_VERSION_MINOR = ', minor+';\n')
change(file_path, 'const S32 LL_VERSION_PATCH = ', patch+';\n')
change(file_path, 'const S32 LL_VERSION_BUILD = ', build+';\n')
try:
    change(file_path, 'const char * const LL_CHANNEL = "', channel+'";\n')
except NameError:
    pass

# ../indra/newview/res/viewerRes.rc
file_path = abspath(join(dirname(__file__), '..', 'indra',
                         'newview', 'res', 'viewerRes.rc'))
change(file_path, 'FILEVERSION ', version_commas+'\n')
change(file_path, 'PRODUCTVERSION ', version_commas+'\n')
change(file_path, 'VALUE "FileVersion", "', version+'"\n')
change(file_path, 'VALUE "ProductVersion", "', version+'"\n')

# ../indra/newview/Info-InWorldz.plist
file_path = abspath(join(dirname(__file__), '..', 'indra',
                         'newview', 'Info-InWorldz.plist'))
exp = re.compile('[0-9]+\.[0-9]+\.[0-9]+\.[0-9]+')
for line in fileinput.input(file_path, inplace=1):
    if exp.search(line):
        line = '    <string>' + version + '</string>\n'
    print(line),
fileinput.close()
