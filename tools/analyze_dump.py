#!/usr/bin/python3
import re
import sys
import os
import argparse
import glob
import subprocess
from construct import (Bytes, Int32ul, Struct)
from esp_coredump.corefile.elf import ESPCoreDumpElfFile
from esp_coredump.corefile.loader import ESPCoreDumpFileLoader
from esp_coredump.corefile import SUPPORTED_TARGETS
from esp_coredump.corefile.gdb import DEFAULT_GDB_TIMEOUT_SEC
from esp_coredump import CoreDump,__version__

try:
    from esptool.loader import ESPLoader
except (AttributeError, ModuleNotFoundError):
    from esptool import ESPLoader

def arg_auto_int(x):
    return int(x, 0)

def setup_esp_idf_environment(ps_profile_path):
    """Configures ESP-IDF environment using PowerShell profile (Windows only)."""
    if sys.platform != 'win32':
        return False
    if not ps_profile_path or not os.path.exists(ps_profile_path):
        print(f"WARNING: PowerShell profile file not found: {ps_profile_path}")
        return False

    print(f"Initializing ESP-IDF using profile: {ps_profile_path}")

    try:
        cmd = [
            "powershell.exe",
            "-NoProfile",
            "-ExecutionPolicy", "Bypass",
            "-Command",
            f". '{ps_profile_path}'; $env:PATH"
        ]

        result = subprocess.run(cmd, capture_output=True, text=True, timeout=15)

        if result.returncode == 0:
            print("✓ ESP-IDF environment initialized successfully")

            path_lines = result.stdout.strip().split('\n')
            if path_lines:
                new_path = path_lines[-1]
                os.environ['PATH'] = new_path
                print(f"✓ PATH updated ({len(new_path.split(';'))} entries)")

            return True
        else:
            print(f"✗ Error initializing ESP-IDF: {result.stderr}")
            return False

    except subprocess.TimeoutExpired:
        print("✗ Timeout while initializing ESP-IDF")
        return False
    except Exception as e:
        print(f"✗ Error running PowerShell: {e}")
        return False

def find_elf_by_partial_hash(partial_hash, search_dirs=None):
    """Finds an ELF file by partial SHA256 hash in the filename."""
    if search_dirs is None:
        search_dirs = ['.', 'build', 'build/elf', 'build/elf/esp32-s3-devkitc-1-16m']

    all_elf_files = []

    for search_dir in search_dirs:
        if os.path.isdir(search_dir):
            pattern = os.path.join(search_dir, "**", "*.elf")
            elf_files = glob.glob(pattern, recursive=True)
            all_elf_files.extend(elf_files)

            pattern = os.path.join(search_dir, "*.elf")
            elf_files = glob.glob(pattern, recursive=False)
            all_elf_files.extend(elf_files)

    all_elf_files = list(set(all_elf_files))

    print(f"Found {len(all_elf_files)} ELF files")

    matching_files = []
    for elf_file in all_elf_files:
        filename = os.path.basename(elf_file)
        filename_no_ext = os.path.splitext(filename)[0]

        if partial_hash.lower() in filename_no_ext.lower():
            matching_files.append(elf_file)

    return matching_files

def find_gdb_in_esp_idf():
    """Finds GDB in typical ESP-IDF installation paths (Windows and Linux)."""
    if sys.platform == 'win32':
        possible_paths = [
            "C:\\Espressif\\tools\\xtensa-esp32s3-elf\\esp-*\\xtensa-esp32s3-elf\\bin\\xtensa-esp32s3-elf-gdb.exe",
            "C:\\Espressif\\tools\\riscv32-esp-elf\\esp-*\\riscv32-esp-elf\\bin\\riscv32-esp-elf-gdb.exe",
            os.path.expanduser("~/.espressif/tools/**/*gdb*.exe"),
            "gdb", "xtensa-esp32s3-elf-gdb", "riscv32-esp-elf-gdb"
        ]
    else:
        possible_paths = [
            os.path.expanduser("~/.espressif/tools/xtensa-esp-elf/*/xtensa-esp-elf/bin/xtensa-esp-elf-gdb"),
            os.path.expanduser("~/.espressif/tools/xtensa-esp32s3-elf/*/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gdb"),
            os.path.expanduser("~/.espressif/tools/riscv32-esp-elf/*/riscv32-esp-elf/bin/riscv32-esp-elf-gdb"),
            "/opt/esp/tools/xtensa-esp32s3-elf/*/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gdb",
            "xtensa-esp32s3-elf-gdb", "riscv32-esp-elf-gdb", "gdb"
        ]

    for pattern in possible_paths:
        try:
            files = glob.glob(pattern, recursive=True)
            for file in files:
                if 'gdb' in os.path.basename(file).lower() and os.path.isfile(file):
                    print(f"Found GDB: {file}")
                    return file
        except Exception:
            continue

    return None

# ====== ESP-IDF ENVIRONMENT INITIALIZATION (Windows only) ======
PS_PROFILE_PATH = "C:\\Espressif\\tools\\Microsoft.v5.5.2.PowerShell_profile.ps1" if sys.platform == 'win32' else ""

print("=" * 60)
print("ESP-IDF ENVIRONMENT INITIALIZATION")
print("=" * 60)

if sys.platform == 'win32':
    if not os.path.exists(PS_PROFILE_PATH):
        print(f"WARNING: Profile file not found: {PS_PROFILE_PATH}")
        print("  Ensure ESP-IDF is installed correctly.")
        print("  You can specify another path with --ps-profile")
    else:
        setup_success = setup_esp_idf_environment(PS_PROFILE_PATH)
        if not setup_success:
            print("\nWARNING: Continuing without full ESP-IDF initialization...")
            print("  Some features may be unavailable.")
else:
    print("Linux/macOS: ESP-IDF PowerShell setup skipped (use 'source export.sh' in your shell if needed).")

print("\n" + "=" * 60)

parser = argparse.ArgumentParser(description=f'coredump.py - ESP32 Core Dump Utility')
parser.add_argument('--chip', default='auto', choices=['auto'] + SUPPORTED_TARGETS, help='Target chip type')
parser.add_argument('--port', '-p', help='Serial port device')
parser.add_argument('--baud', '-b', type=int, default=115200, help='Serial port baud rate')
parser.add_argument('--prog', help='Path to application ELF file')
parser.add_argument('--gdb', '-g', help='Path to GDB executable')
parser.add_argument('--elf-dir', '-e', action='append', help='Additional directories to search for ELF files')
parser.add_argument('--ps-profile', default=PS_PROFILE_PATH or None,
                    help='Path to ESP-IDF PowerShell profile (Windows only)')
parser.add_argument('--skip-esp-setup', action='store_true',
                    help='Skip ESP-IDF environment setup')
parser.add_argument('core-dump-file', help='Path to core dump file')

args = parser.parse_args()
kwargs = {k: v for k, v in vars(args).items() if v is not None}
kwargs["core"] = kwargs.pop('core-dump-file')

# ====== 1. RE-INITIALIZE ESP-IDF (if user specified another profile) ======
if not kwargs.get('skip_esp_setup') and sys.platform == 'win32':
    ps_profile_path = kwargs.get('ps_profile') or PS_PROFILE_PATH
    if ps_profile_path != PS_PROFILE_PATH or 'Espressif' not in os.environ.get('PATH', ''):
        if ps_profile_path and os.path.exists(ps_profile_path):
            print(f"\nUsing custom profile: {ps_profile_path}")
            setup_esp_idf_environment(ps_profile_path)
        elif not os.path.exists(PS_PROFILE_PATH or ''):
            print(f"WARNING: Profile file not found: {PS_PROFILE_PATH}")
            print("  Trying to find GDB without environment setup...")

# ====== 2. FIND GDB ======
if 'gdb' not in kwargs or not kwargs['gdb']:
    print("\nSearching for GDB...")

    if sys.platform == 'win32':
        espressif_paths = [
            "C:\\Espressif\\tools\\xtensa-esp32s3-elf\\esp-*\\xtensa-esp32s3-elf\\bin\\xtensa-esp32s3-elf-gdb.exe",
            "C:\\Espressif\\tools\\riscv32-esp-elf\\esp-*\\riscv32-esp-elf\\bin\\riscv32-esp-elf-gdb.exe",
            "C:\\Espressif\\frameworks\\esp-idf-v*\\tools\\**\\*gdb*.exe"
        ]
    else:
        espressif_paths = [
            os.path.expanduser("~/.espressif/tools/xtensa-esp32s3-elf/*/xtensa-esp32s3-elf/bin/xtensa-esp32s3-elf-gdb"),
            os.path.expanduser("~/.espressif/tools/riscv32-esp-elf/*/riscv32-esp-elf/bin/riscv32-esp-elf-gdb"),
        ]

    for pattern in espressif_paths:
        try:
            files = glob.glob(pattern, recursive=True)
            for file in files:
                if os.path.isfile(file):
                    kwargs['gdb'] = file
                    print(f"Found GDB in ESP-IDF: {file}")
                    break
            if kwargs.get('gdb'):
                break
        except Exception:
            continue

    if not kwargs.get('gdb'):
        found_gdb = find_gdb_in_esp_idf()
        if found_gdb:
            kwargs['gdb'] = found_gdb

    if not kwargs.get('gdb'):
        gdb_names = ['xtensa-esp32s3-elf-gdb', 'riscv32-esp-elf-gdb', 'gdb']
        which_cmd = 'where' if sys.platform == 'win32' else 'which'
        for gdb_name in gdb_names:
            try:
                result = subprocess.run([which_cmd, gdb_name], capture_output=True, text=True)
                if result.returncode == 0 and result.stdout.strip():
                    kwargs['gdb'] = result.stdout.strip().split('\n')[0].strip()
                    print(f"Found GDB in PATH: {kwargs['gdb']}")
                    break
            except Exception:
                continue

if kwargs.get('gdb'):
    if not os.path.exists(kwargs['gdb']):
        print(f"ERROR: GDB not found at: {kwargs['gdb']}")
        print("\nPlease specify GDB path in one of these ways:")
        print("1. Install ESP-IDF toolchain")
        print("2. Specify GDB explicitly: --gdb /path/to/xtensa-esp32s3-elf-gdb")
        sys.exit(1)
    print(f"Using GDB: {kwargs['gdb']}")
else:
    print("WARNING: GDB not found!")
    print("Analysis may fail.")

# ====== 3. LOAD CORE DUMP ======
print(f"\nLoading core dump: {kwargs['core']}")
loader = ESPCoreDumpFileLoader(kwargs["core"])
loader._load_core_src()

loader.core_elf_file = loader._create_temp_file()
with open(loader.core_elf_file, 'wb') as fw:
    fw.write(loader.core_src.data)

core_elf = ESPCoreDumpElfFile(loader.core_elf_file, e_machine=ESPCoreDumpElfFile.EM_XTENSA)

core_sha_trimmed = None
for seg in core_elf.note_segments:
    for note_sec in seg.note_secs:
        if note_sec.name == b'ESP_CORE_DUMP_INFO' and note_sec.type == ESPCoreDumpElfFile.PT_ESP_INFO:
            coredump_sha256_struct = Struct('ver' / Int32ul, 'sha256' / Bytes(64))
            coredump_sha256 = coredump_sha256_struct.parse(note_sec.desc[:coredump_sha256_struct.sizeof()])
            core_sha_trimmed = coredump_sha256.sha256.rstrip(b'\x00').decode()
            print(f'Core dump SHA256: {core_sha_trimmed}')

# ====== 4. FIND ELF FILE ======
if 'prog' not in kwargs or not kwargs['prog']:
    search_dirs = ['.', 'build', 'build/elf', 'build/elf/esp32-s3-devkitc-1-16m', '../build', '../../build']

    if 'elf_dir' in kwargs:
        if isinstance(kwargs['elf_dir'], list):
            search_dirs.extend(kwargs['elf_dir'])
        else:
            search_dirs.append(kwargs['elf_dir'])

    if core_sha_trimmed:
        print(f"\nSearching for ELF file with partial hash: {core_sha_trimmed}")
        matching_files = find_elf_by_partial_hash(core_sha_trimmed, search_dirs)

        if matching_files:
            if len(matching_files) == 1:
                kwargs["prog"] = matching_files[0]
                print(f"Found ELF file: {matching_files[0]}")
            else:
                print(f"Found multiple matching ELF files:")
                for i, elf_file in enumerate(matching_files):
                    print(f"  {i+1}. {elf_file}")

                kwargs["prog"] = matching_files[0]
                print(f"Using: {matching_files[0]}")
        else:
            print("\nNo ELF files with this hash found, searching for standard names...")

            standard_elf_names = [
                "ssvc_open_connect.elf",
                "firmware.elf",
                "main.elf",
                "app.elf"
            ]

            for search_dir in search_dirs:
                if os.path.isdir(search_dir):
                    for elf_name in standard_elf_names:
                        elf_path = os.path.join(search_dir, elf_name)
                        if os.path.exists(elf_path):
                            kwargs["prog"] = elf_path
                            print(f"Found standard ELF: {elf_path}")
                            break
                    if 'prog' in kwargs:
                        break

            if 'prog' not in kwargs:
                print("\nSearching for any ELF file in project...")
                all_elf_files = []
                for search_dir in search_dirs:
                    if os.path.isdir(search_dir):
                        pattern = os.path.join(search_dir, "**", "*.elf")
                        elf_files = glob.glob(pattern, recursive=True)
                        all_elf_files.extend(elf_files)

                if all_elf_files:
                    all_elf_files.sort(key=lambda x: os.path.getmtime(x) if os.path.exists(x) else 0, reverse=True)

                    print(f"Found {len(all_elf_files)} ELF files")
                    print("Latest 5 files:")
                    from datetime import datetime
                    for i, elf_file in enumerate(all_elf_files[:5]):
                        mtime = os.path.getmtime(elf_file)
                        mtime_str = datetime.fromtimestamp(mtime).strftime('%Y-%m-%d %H:%M:%S')
                        print(f"  {i+1}. {os.path.basename(elf_file)} - {mtime_str}")

                    kwargs["prog"] = all_elf_files[0]
                    print(f"\nUsing latest ELF file: {all_elf_files[0]}")
                else:
                    print("\nERROR: No ELF file found!")
                    print("Please specify the ELF file explicitly:")
                    print("  python analyze_dump.py --prog path/to/file.elf coredump.bin")
                    print("\nOr specify search directories:")
                    print("  python analyze_dump.py --elf-dir build --elf-dir ../build coredump.bin")
                    sys.exit(1)

print(f"\nUsing ELF file: {kwargs['prog']}")

# ====== 5. RUN ANALYSIS ======
print("\nRunning core dump analysis...")
print("=" * 60)

final_elf = kwargs.get('prog')
final_core = kwargs.get('core')

for key in ['elf_dir', 'ps_profile', 'skip_esp_setup']:
    if key in kwargs:
        del kwargs[key]

try:
    espcoredump = CoreDump(**kwargs)
    temp_core_files = espcoredump.info_corefile()

    if temp_core_files:
        for f in temp_core_files:
            try:
                os.remove(f)
            except OSError:
                pass

    print("\n✓ Analysis completed successfully!")

    print("\n" + "=" * 60)
    print("GDB COMMAND (INTERACTIVE DEBUGGING)")
    print("=" * 60)

    gdb_path = kwargs.get('gdb')

    if gdb_path:
        debug_cmd = f"python -m esp_coredump dbg_corefile --gdb \"{gdb_path}\" --core {final_core} {final_elf}"
    else:
        debug_cmd = f"python -m esp_coredump dbg_corefile --core {final_core} {final_elf}"

    print("To inspect variables and stack in GDB, run:")
    print(f"\n{debug_cmd}\n")
    print("Useful GDB commands:")
    print("  bt full       - show backtrace and all variable values")
    print("  f 0           - switch to crash frame")
    print("=" * 60)

except Exception as e:
    print(f"\n✗ Error analyzing core dump: {e}")

    if "GDB executable not found" in str(e):
        print("\nGDB issue. Try:")
        print("1. Specify GDB path: --gdb /path/to/xtensa-esp32s3-elf-gdb")
        if sys.platform == 'win32':
            print("2. Ensure ESP-IDF profile exists: C:\\Espressif\\tools\\Microsoft.v5.5.2.PowerShell_profile.ps1")
            print("3. Run C:\\Espressif\\tools\\idf-env.exe or install via ESP-IDF Tools Installer")

    print("\nTry specifying the ELF file explicitly:")
    print(f"  python analyze_dump.py --prog build/ssvc_open_connect.elf coredump.bin")

    sys.exit(1)

