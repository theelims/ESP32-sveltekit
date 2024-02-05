#   ESP32 SvelteKit --
#
#   A simple, secure and extensible framework for IoT projects for ESP32 platforms
#   with responsive Sveltekit front-end built with TailwindCSS and DaisyUI.
#   https://github.com/theelims/ESP32-sveltekit
#
#   Copyright (C) 2018 - 2023 rjwats
#   Copyright (C) 2023 theelims
#   Copyright (C) 2023 Maxtrium B.V. [ code available under dual license ]
#
#   All Rights Reserved. This software may be modified and distributed under
#   the terms of the LGPL v3 license. See the LICENSE file for details.

from pathlib import Path
from shutil import copytree, rmtree, copyfileobj
from subprocess import check_output, Popen, PIPE, STDOUT, CalledProcessError
from os.path import exists
from typing import Final
import os
import gzip
import mimetypes
import glob
from datetime import datetime

Import("env")

# print("Current build environment:")
# print(env.ParseFlags(env["BUILD_FLAGS"]).get("CPPDEFINES"))

# print("Current CLI targets", COMMAND_LINE_TARGETS)
# print("Current Build targets", BUILD_TARGETS)

OUTPUTFILE: Final[str] = env["PROJECT_DIR"] + "/lib/framework/WWWData.h"
SOURCEWWWDIR: Final[str] = env["PROJECT_DIR"] + "/interface/src"

def OutputFileExits():
    return os.path.exists(OUTPUTFILE)

def findLastestTimeStampWWWInterface():
  list_of_files = glob.glob(SOURCEWWWDIR+'/**/*', recursive=True) 
  # print(list_of_files)
  latest_file = max(list_of_files, key=os.path.getmtime)
  # print(latest_file)

  return os.path.getmtime(latest_file)

def findTimestampOutputFile():
     return os.path.getmtime(OUTPUTFILE)

def needtoRegenerateOutputFile():
    if not flagExists("EMBED_WWW"):
        return True
    else:
        if (OutputFileExits()):
            latestWWWInterface = findLastestTimeStampWWWInterface()
            timestampOutputFile = findTimestampOutputFile()

            # print timestamp of newest file in interface directory and timestamp of outputfile nicely formatted as time
            print(f'Newest interface file: {datetime.fromtimestamp(latestWWWInterface):%Y-%m-%d %H:%M:%S}, WWW Outputfile: {datetime.fromtimestamp(timestampOutputFile):%Y-%m-%d %H:%M:%S}')
            # print(f'Newest interface file: {latestWWWInterface:.2f}, WWW Outputfile: {timestampOutputFile:.2f}')
            
            sourceEdited=( timestampOutputFile < latestWWWInterface )
            if (sourceEdited):
                print("Svelte source files are updated. Need to regenerate.")
                return True
            else:
                print("Current outputfile is O.K. No need to regenerate.")
                return False

        else:
            print("WWW outputfile does not exists. Need to regenerate.")
            return True

def gzipFile(file):
    with open(file, 'rb') as f_in:
        with gzip.open(file + '.gz', 'wb') as f_out:
            copyfileobj(f_in, f_out)
    os.remove(file)

def flagExists(flag):
    buildFlags = env.ParseFlags(env["BUILD_FLAGS"])
    for define in buildFlags.get("CPPDEFINES"):
        if (define == flag or (isinstance(define, list) and define[0] == flag)):
            return True

def buildProgMem():
    mimetypes.init()
    progmem = open(OUTPUTFILE,"w")

    progmem.write("#include <functional>\n")
    progmem.write("#include <Arduino.h>\n")


    progmemCounter = 0

    assetMap = {}

    # Iterate over all files in the build directory
    for path in Path("build").rglob("*.*"):
        asset_path = path.relative_to("build").as_posix()
        print("Converting " + str(asset_path))

        asset_var = 'ESP_SVELTEKIT_DATA_' + str(progmemCounter)
        asset_mime = mimetypes.types_map['.' + asset_path.split('.')[-1]]

        progmem.write('// ' + str(asset_path) + '\n')
        progmem.write('const uint8_t ' + asset_var + '[] = {\n  ')
        progmemCounter += 1
        
        # Open path as binary file, compress and read into byte array
        size = 0
        with open(path, "rb") as f:
            zipBuffer = gzip.compress(f.read()) 
            for byte in zipBuffer:
                if not (size % 16):
                    progmem.write('\n  ')
                
                progmem.write(f"0x{byte:02X}" + ',')
                size += 1

        progmem.write('\n};\n\n')
        assetMap[asset_path] = { "name": asset_var, "mime": asset_mime, "size": size }
    
    progmem.write('typedef std::function<void(const String& uri, const String& contentType, const uint8_t * content, size_t len)> RouteRegistrationHandler;\n\n')
    progmem.write('class WWWData {\n')
    progmem.write('  public:\n')
    progmem.write('    static void registerRoutes(RouteRegistrationHandler handler) {\n')

    for asset_path, asset in assetMap.items():
        progmem.write('      handler("/' + str(asset_path) + '", "' + asset['mime'] + '", ' + asset['name'] + ', ' + str(asset['size']) + ');\n')

    progmem.write('    }\n')
    progmem.write('};\n')
    
    progmem.write('\n')


def buildWeb():
    os.chdir("interface")
    print("Building interface with npm")
    try:
        env.Execute("npm install")
        env.Execute("npm run build")
        buildPath = Path("build")
        wwwPath = Path("../data/www")        
        if not flagExists("EMBED_WWW"):
            if wwwPath.exists() and wwwPath.is_dir():
                rmtree(wwwPath)
            print("Copying and compress interface to data directory")
            copytree(buildPath, wwwPath)
            for currentpath, folders, files in os.walk(wwwPath):
                for file in files:
                    gzipFile(os.path.join(currentpath, file))
        else:
            print("Converting interface to PROGMEM")
            buildProgMem()

    finally:
        os.chdir("..")
        if not flagExists("EMBED_WWW"):
            print("Build LittleFS file system image and upload to ESP32")
            env.Execute("pio run --target uploadfs")

print("running: build_interface.py")

# Dump global construction environment (for debug purpose)
#print(env.Dump())

# Dump project construction environment (for debug purpose)
#print(projenv.Dump())

if (needtoRegenerateOutputFile()):
     buildWeb()

#env.AddPreAction("${BUILD_DIR}/src/HTTPServer.o", buildWebInterface)

# if ("upload" in BUILD_TARGETS):
#     print(BUILD_TARGETS)
#     if (needtoRegenerateOutputFile()):
#         buildWeb()
# else:
#     print("Skipping build interface step for target(s): " + ", ".join(BUILD_TARGETS))


