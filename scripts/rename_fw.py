""" 
    EMS-ESP - https://github.com/emsesp/EMS-ESP
    Copyright 2020-2023  Paul Derbyshire

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>. 

"""

import shutil
import re
import os
Import("env")
import hashlib


OUTPUT_DIR = "build{}".format(os.path.sep)

def readFlag(flag):
    buildFlags = env.ParseFlags(env["BUILD_FLAGS"])
    # print(buildFlags.get("CPPDEFINES"))
    for define in buildFlags.get("CPPDEFINES"):
        if (define == flag or (isinstance(define, list) and define[0] == flag)):
            # print("Found "+flag+" = "+define[1])
            # strip quotes ("") from define[1]
            cleanedFlag = re.sub(r'^"|"$', '', define[1])
            return cleanedFlag
    return None


def bin_copy(source, target, env):

    # get the build info
    app_version = readFlag("APP_VERSION")
    app_name = readFlag("APP_NAME")
    build_target = env.get('PIOENV')

    # print information's
    print("App Version: " + app_version)
    print("App Name: " + app_name)
    print("Build Target: " + build_target)

    # convert . to - so Windows doesn't complain
    variant = app_name + "_" +  build_target + "_" + app_version.replace(".", "-")

    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in ['firmware']:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

    # create string with location and file names based on variant
    bin_file = "{}firmware{}{}.bin".format(OUTPUT_DIR, os.path.sep, variant)
    md5_file = "{}firmware{}{}.md5".format(OUTPUT_DIR, os.path.sep, variant)

    # check if new target files exist and remove if necessary
    for f in [bin_file]:
        if os.path.isfile(f):
            os.remove(f)

    # check if new target files exist and remove if necessary
    for f in [md5_file]:
        if os.path.isfile(f):
            os.remove(f)

    print("Renaming file to "+bin_file)

    # copy firmware.bin to firmware/<variant>.bin
    shutil.copy(str(target[0]), bin_file)

    with open(bin_file,"rb") as f:
        result = hashlib.md5(f.read())
        print("Calculating MD5: "+result.hexdigest())
        file1 = open(md5_file, 'w')
        file1.write(result.hexdigest())
        file1.close()

env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_copy])
env.AddPostAction("$BUILD_DIR/${PROGNAME}.md5", [bin_copy])
