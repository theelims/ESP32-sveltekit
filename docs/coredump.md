# Core dump analysis

This page describes what ESP32 core dumps are, how to obtain and analyse them, and how to use the `analyze_dump.py` script. It also includes an example of investigating a stack overflow using the TCB (Task Control Block) table.

---

## Dependencies

The script `tools/analyze_dump.py` requires Python 3.7+ and the following packages:

| Package | Purpose |
|---------|--------|
| **esp-coredump** | Load and decode ESP32 core dumps, run GDB. |
| *construct* | Parsing binary structures (installed with esp-coredump). |
| *esptool* | Optional; used by esp-coredump for some operations. |

### Install with pip

From the project root:

Install the main package only (dependencies will be pulled in):

```bash
pip install esp-coredump
```

Use a virtual environment to avoid conflicts with other projects:

```bash
python -m venv .venv
# Windows:
.venv\Scripts\activate
# Linux/macOS:
source .venv/bin/activate
pip install esp-coredump
```

### GDB (xtensa toolchain)

The script also needs a GDB for your chip (e.g. `xtensa-esp32s3-elf-gdb`). It is usually provided by the **ESP-IDF toolchain**. If you use PlatformIO or ESP-IDF, the toolchain is often already installed; the script will try to find GDB automatically. Otherwise install the [ESP-IDF tools](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/) or pass GDB explicitly with `--gdb /path/to/xtensa-esp32s3-elf-gdb`.

---

## What are core dumps?

A **core dump** is a snapshot of the device state at the moment of a crash or fatal exception. It includes:

- **CPU register state** (e.g. PC, SP, A0–A15 on Xtensa)
- **Memory regions** (RAM, parts of flash as configured)
- **Task Control Blocks (TCB)** and their **backtraces** for each task

ESP-IDF can write this snapshot to flash (coredump partition), to a host over UART, or to a file. You then analyse it on the host with the same **ELF** binary that was running on the device so that addresses can be resolved to symbols (functions, line numbers).

### Why use them?

- **Post-mortem debugging**: Understand why the device crashed without needing a debugger attached.
- **Field diagnostics**: Get a dump from a device in the field (e.g. via UART or stored in flash) and analyse it later with the matching ELF.
- **Stack overflows**: See which task overflowed (e.g. TCB6) and what the call stack was.

### How they are generated

1. **Enable core dumps** in menuconfig:  
   **Component config → ESP System Settings → Core dump destination** (e.g. Flash, UART, or Disable).
2. When a fatal exception occurs (crash, abort, panic), the **core dump component** runs and writes the snapshot to the chosen destination.
3. You **retrieve the dump** (read from flash, capture from UART, or use a file) and save it as a binary file (e.g. `coredump.bin`).

---

## Working with core dumps

### 1. Get the dump file

- **Flash**: Use `espcoredump.py` or your IDF/PlatformIO workflow to read the coredump partition and save to a file.
- **UART**: Configure “Core dump destination → UART” and capture the binary stream to a file when the device panics.
- **File**: If you already have a `.bin` file from flash or UART, use it as the `core-dump-file` argument for the script.

### 2. Have the matching ELF

The dump was produced by a specific firmware build. You **must** use the **same** ELF that was flashed (or one built from the same commit). The script can search for it by:

- **SHA256 hash** stored in the dump (recommended), or  
- **Standard names** (e.g. `firmware.elf`, `ssvc_open_connect.elf`) or “latest” ELF in the project.

If the ELF does not match, addresses may resolve to wrong symbols or “???”.

### 3. Run the analysis script

The project script `tools/analyze_dump.py`:

1. Loads the core dump and reads its metadata (e.g. SHA256).
2. Finds a suitable GDB (e.g. `xtensa-esp32s3-elf-gdb`) and, on Windows, can set up the ESP-IDF environment.
3. Locates the matching ELF (by hash, name, or fallback).
4. Runs the ESP-IDF coredump analysis (backtraces, panic reason, etc.).
5. Prints a **GDB command** so you can open the core file in GDB for interactive inspection.

---

## Script usage and configuration

### Basic usage

```bash
python tools/analyze_dump.py <core-dump-file>
```

Example:

```bash
python tools/analyze_dump.py coredump.bin
```

The script will:

- On **Windows**: Try to initialize the ESP-IDF environment using the default PowerShell profile (if present).
- Search for **GDB** in standard ESP-IDF locations and in `PATH`.
- Load the dump, extract the SHA256, and search for an ELF with that hash in the project (e.g. `build`, `build/elf`, etc.).
- Run the core dump decoder and print the summary plus the GDB command.

### Arguments

| Argument | Short | Description |
|----------|--------|-------------|
| `core-dump-file` | — | **Required.** Path to the core dump file (e.g. `coredump.bin`). |
| `--prog` | — | Path to the application ELF file. If omitted, the script searches by hash or standard names. |
| `--gdb` | `-g` | Path to the GDB executable (e.g. `xtensa-esp32s3-elf-gdb`). If omitted, the script searches automatically. |
| `--elf-dir` | `-e` | Additional directory to search for ELF files. Can be repeated. |
| `--chip` | — | Target chip (`auto`, or a supported target). Default: `auto`. |
| `--port` | `-p` | Serial port (for workflows that need it). |
| `--baud` | `-b` | Baud rate. Default: 115200. |
| `--ps-profile` | — | **(Windows)** Path to ESP-IDF PowerShell profile. Default: standard Espressif path. |
| `--skip-esp-setup` | — | Skip ESP-IDF environment setup (e.g. if PATH is already set). |

### Configuration tips

- **ELF not found**: Use `--prog path/to/firmware.elf` or add search dirs with `--elf-dir build --elf-dir ../other/build`.
- **GDB not found**: Install the ESP-IDF toolchain or pass GDB explicitly:  
  `--gdb /path/to/xtensa-esp32s3-elf-gdb` (Linux/macOS) or  
  `--gdb "C:\Espressif\tools\...\xtensa-esp32s3-elf-gdb.exe"` (Windows).
- **Linux/macOS**: The script does not run the Windows PowerShell profile. Source your ESP-IDF environment in the shell if needed, e.g. `source $IDF_PATH/export.sh`, so that `xtensa-esp32s3-elf-gdb` is in `PATH` if you do not pass `--gdb`.

### Example commands

```bash
# Automatic ELF and GDB detection
python tools/analyze_dump.py coredump.bin

# Specify ELF and GDB
python tools/analyze_dump.py --prog build/firmware.elf --gdb /opt/esp/tools/xtensa-esp32s3-elf-gdb coredump.bin

# Add extra directories for ELF search
python tools/analyze_dump.py --elf-dir build --elf-dir ../firmware/build coredump.bin

# Skip ESP-IDF setup (e.g. already in correct environment)
python tools/analyze_dump.py --skip-esp-setup coredump.bin
```

---

## Example: investigating a stack overflow using the TCB table

When a task’s stack overflows, the panic output and the core dump often point to a specific **TCB** (Task Control Block). The decoder prints backtraces **per task**; the one that overflowed is usually the crashing task (e.g. **TCB6** in this example).

### 1. Typical panic output

You might see something like:

```text
CORRUPT HEAP: Bad head at 0x3fcxxxxx. Expected 0xabba1234 got 0x...
...
Task with corrupted stack: TCB6
```

or a **Stack canary watchpoint** or **LoadProhibited** in the context of a task name that corresponds to one of the TCBs.

### 2. Run the analysis script

```bash
python tools/analyze_dump.py coredump.bin
```

The script will print a summary that includes **backtraces for each task**, often labelled by TCB index (e.g. “Backtrace for TCB0”, “Backtrace for TCB6”, …).

### 3. Find the faulty task (e.g. TCB6)

- In the script output, locate the backtrace for **TCB6** (or the TCB number mentioned in the panic).
- If the panic reason says “Stack canary” or “corrupted stack” and points to TCB6, that task’s stack overflowed.

Example of what you might see:

```text
Backtrace for TCB6: 0x4008abcd:0x3fc9f000 0x40081234:0x3fc9f020 ...
```

### 4. Resolve addresses to symbols

The script uses the ELF to resolve these addresses. In the summary you should see function names (and possibly files) for each frame. If not, run GDB as suggested at the end of the script output:

```bash
python -m esp_coredump dbg_corefile --gdb "/path/to/xtensa-esp32s3-elf-gdb" --core coredump.bin build/firmware.elf
```

In GDB:

- **Backtrace for the crashing task**:  
  `thread apply 6 bt full`  
  (if TCB6 is thread 6; adjust the thread number to match your decoder output).
- **Frame and locals**:  
  `f 0` then `info locals`.
- **All threads**:  
  `info threads` then `thread N` and `bt full` for the overflowing task.

### 5. Interpret the result for TCB6

- **Cause**: The stack of the task corresponding to TCB6 grew beyond its allocated size (e.g. large local buffers, deep recursion, or big structures on the stack).
- **Fix**:  
  - Increase this task’s stack size in the code (e.g. `xTaskCreate(..., stack_size, ...)` or `CONFIG_..._TASK_STACK_SIZE` in menuconfig), or  
  - Reduce stack usage: move large buffers to heap, reduce recursion depth, or split work.

### 6. Summary table (example)

| Step | Action |
|------|--------|
| 1 | Reproduce crash and capture core dump (flash/UART/file). |
| 2 | Run `python tools/analyze_dump.py coredump.bin`. |
| 3 | In the output, find the TCB mentioned in the panic (e.g. TCB6) and its backtrace. |
| 4 | Resolve symbols (script output or GDB: `bt full`, `thread apply N bt full`). |
| 5 | Identify the task and the functions that use most stack (large locals, recursion). |
| 6 | Increase that task’s stack size or reduce stack usage. |

---

## Example: Task Watchdog timeout

When a task does not yield for too long (no `vTaskDelay`, blocking calls, or heavy computation in a tight loop), the **Task Watchdog** can fire and reset the chip.

### 1. Typical panic output

You may see something similar to:

```text
Task watchdog got triggered. The following tasks did not reset the watchdog in time:
 - idle (TCB3)
 - comm-task (TCB6)
...
abort() was called at PC 0x400e1234 on core 0
```

### 2. Run the analysis script

```bash
python tools/analyze_dump.py coredump.bin
```

Look for the **backtrace of the task mentioned in the panic** (e.g. `comm-task` / `TCB6`). The decoder output will usually contain a section with backtraces per TCB.

### 3. Inspect the blocking code in GDB

Open the core dump in GDB using the command printed by the script, then:

```gdb
info threads           # list all tasks
thread N               # select the thread corresponding to comm-task / TCB6
bt full                # see the full backtrace and local variables
```

Check where the task is stuck:

- Tight loop without `vTaskDelay` or yielding.
- Long-running computation on the main / communication task.
- Waiting on a mutex/semaphore from ISR-unsafe context.

### 4. Fix

- Insert **periodic delays** or yields inside long loops (e.g. `vTaskDelay(1)`).
- Offload heavy computation to a **dedicated worker task** with its own stack and lower priority.
- Ensure blocking calls have **timeouts** and that you handle them properly.

---

## Example: Heap corruption / use-after-free

Heap corruption is harder to debug because the crash often happens **later** than the actual bug. Core dumps help by showing the state of the heap and backtraces of all tasks at the moment of failure.

### 1. Typical panic output

You might see one of:

```text
CORRUPT HEAP: Bad head at 0x3fcxxxxx. Expected 0xabba1234 got 0x...
assertion "heap != NULL && "free() target pointer is outside heap areas"" failed
```

### 2. Analyse the dump

```bash
python tools/analyze_dump.py coredump.bin
```

Then, open GDB:

```bash
python -m esp_coredump dbg_corefile --gdb /path/to/xtensa-esp32s3-elf-gdb --core coredump.bin build/firmware.elf
```

In GDB:

```gdb
bt full            # if crash is in malloc/free/heap functions
info threads       # inspect other threads that might be freeing the same memory
```

Look for:

- Double `free()` on the same pointer.
- Free from one task and later use (dereference) from another (**use-after-free**).
- Writing past the end of a dynamically allocated buffer.

### 3. Fix

- Use **ownership rules**: one task “owns” a buffer and is responsible for freeing it.
- Replace raw pointers with safer patterns (e.g. reference counting, ownership flags).
- Add **bounds checks** for all buffer writes, especially when parsing external data.

---

## Example: ASSERT / abort (ESP_ERROR_CHECK)

ESP-IDF often calls `abort()` when an `assert()` or `ESP_ERROR_CHECK()` fails. Core dumps make it easy to jump **exactly** to the source line that triggered the abort.

### 1. Typical panic output

```text
abort() was called at PC 0x400d5678 on core 1

ELF file SHA256: ...
Backtrace: 0x400d5678:0x3ffb1f10 0x4008abcd:0x3ffb1f30 ...
```

### 2. Find the abort location

Run:

```bash
python tools/analyze_dump.py coredump.bin
```

Then open GDB (using the script’s suggested command) and run:

```gdb
bt full          # see the full stack
f 0              # go to the top frame where abort() was called
list             # show the source code around the current line
```

You will typically land inside `ESP_ERROR_CHECK`/`assert` or just above it, seeing:

- The **function** that failed.
- The **error code** or condition that was not met.

### 3. Fix

- If it is an `ESP_ERROR_CHECK`, handle the error instead of unconditionally aborting (log, retry, fail gracefully).
- If it is an `assert`, either:
  - Fix the logic so the invariant holds, or
  - Replace `assert` with a **runtime check** that reports a controlled error instead of aborting in production.

---

## References

- **ESP-IDF Core dump documentation**: [Core Dump](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/core_dump.html) (Espressif).
- **Project script**: `tools/analyze_dump.py` (English, supports Windows and Linux/macOS).
