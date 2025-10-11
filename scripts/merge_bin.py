import os
import re
Import("env")

APP_BIN = "$BUILD_DIR/${PROGNAME}.bin"
OUTPUT_DIR = "build{}merged{}".format(os.path.sep, os.path.sep)

BOARD_CONFIG = env.BoardConfig()

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


def merge_bin(source, target, env):

    # check if output directories exist and create if necessary
    if not os.path.isdir("build"):
        os.mkdir("build")

    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    MERGED_BIN = "$PROJECT_DIR{}{}{}_{}_{}_webflash.bin".format(os.path.sep, OUTPUT_DIR, readFlag("APP_NAME"), env.get('PIOENV'), readFlag("APP_VERSION").replace(".", "-"))

    # The list contains all extra images (bootloader, partitions, eboot) and
    # the final application binary
    flash_images = env.Flatten(env.get("FLASH_EXTRA_IMAGES", [])) + ["$ESP32_APP_OFFSET", APP_BIN]
    flash_size = env.BoardConfig().get("upload.flash_size", "4MB")
    flash_freq = env.BoardConfig().get("build.f_flash", "40000000L")
    flash_freq = str(flash_freq).replace("L", "")
    flash_freq = str(int(int(flash_freq) / 1000000)) + "m"
    flash_mode = env.BoardConfig().get("build.flash_mode", "dio")
    memory_type = env.BoardConfig().get("build.arduino.memory_type", "qio_qspi")

    if flash_mode == "qio" or flash_mode == "qout":
        flash_mode = "dio"
    if memory_type == "opi_opi" or memory_type == "opi_qspi":
        flash_mode = "dout"

    # Run esptool to merge images into a single binary
    env.Execute(
        " ".join(
            [
                "$PYTHONEXE",
                "$OBJCOPY",
                "--chip",
                BOARD_CONFIG.get("build.mcu", "esp32"),
                "merge-bin",
                "-o",
                MERGED_BIN,
                "--flash-mode",
                flash_mode,
                "--flash-freq",
                flash_freq,
                "--flash-size",
                flash_size    
            ]
            + flash_images
        )
    )

# Add a post action that runs esptoolpy to merge available flash images
env.AddPostAction(APP_BIN , merge_bin)
