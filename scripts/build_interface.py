from pathlib import Path
from subprocess import check_output, Popen, PIPE, STDOUT, CalledProcessError
import os
import zlib

Import("env")

def flagExists(flag):
    buildFlags = env.ParseFlags(env["BUILD_FLAGS"])
    for define in buildFlags.get("CPPDEFINES"):
        if (define == flag or (isinstance(define, list) and define[0] == flag)):
            return True

def buildWeb():
    os.chdir("interface")
    print("Building interface with npm")
    try:
        env.Execute("npm install")
        env.Execute("npm run build")
    finally:
        os.chdir("..")
    if not flagExists("PROGMEM_WWW"):
        env.Execute("pio run --target uploadfs")
    #else:
        # How to create the header-file for PROGMEM?
    
if ("upload" in BUILD_TARGETS):
    print(BUILD_TARGETS)
    buildWeb()
else:
    print("Skipping build interface step for target(s): " + ", ".join(BUILD_TARGETS))
