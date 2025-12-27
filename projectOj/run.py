#!/usr/bin/env python3
"""
Build and run script for Operation Jackpot multiplayer game.
Run from project root: python run.py
"""
import os, sys, glob, platform, subprocess

def run(cmd):
    print(">", " ".join(cmd))
    return subprocess.call(cmd)

# Get project root directory
project_root = os.path.dirname(os.path.abspath(__file__))
os.chdir(project_root)

# Find all source files
src_dir = os.path.join(project_root, "src")
cpps = sorted(glob.glob(os.path.join(src_dir, "*.cpp")))
if not cpps:
    print("No .cpp files found in src/ directory.")
    sys.exit(1)

system = platform.system()  # Windows, Darwin, Linux
exe_name = "projectOj.exe" if system == "Windows" else "projectOj"

# Create bin/Debug directory
bin_dir = os.path.join(project_root, "bin", "Debug")
os.makedirs(bin_dir, exist_ok=True)
exe_path = os.path.join(bin_dir, exe_name)

# Project include directory
project_include = os.path.join(project_root, "include")

# ENet paths (in project root)
enet_include = os.path.join(project_root, "enet", "include")
enet_lib_dir = os.path.join(project_root, "enet")

# Base compile command
cmd = ["g++", "-std=c++17", "-I" + project_include, "-I" + enet_include]

# 🔹 WINDOWS (MinGW + freeglut)
if system == "Windows":
    # CHANGE THIS PATH if freeglut is installed elsewhere
    FREEGLUT_ROOT = r"C:\freeglut"

    # Use MinGW-built ENet library (libenet.a) from build directory
    enet_build_lib = os.path.join(project_root, "enet", "build", "libenet.a")
    
    cmd += [
        "-I" + os.path.join(FREEGLUT_ROOT, "include"),
        "-L" + os.path.join(FREEGLUT_ROOT, "lib"),
        "-D__WSAFDIsSet=__WSAFDIsSet",  # Fix for MinGW __WSAFDIsSet issue
        *cpps,
        "-o", exe_path,
        "-Wl,--enable-auto-import",  # Enable auto-import for __imp_ symbols (MinGW)
        "-lfreeglut",
        "-lopengl32",
        "-lglu32",
        "-lgdi32",
        enet_build_lib,  # Link MinGW-built ENet library
        "-lws2_32",  # Windows Sockets API (required for ENet on Windows)
        "-lwinmm",  # Windows Multimedia API (for timeGetTime) - link after ENet
    ]

# 🔹 macOS
elif system == "Darwin":
    cmd += [
        "-L" + enet_lib_dir,
        *cpps,
        "-o", exe_path,
        "-framework", "OpenGL",
        "-lfreeglut",
        "-lenet64",
    ]

# 🔹 Linux
else:
    cmd += [
        "-L" + enet_lib_dir,
        *cpps,
        "-o", exe_path,
        "-lfreeglut",
        "-lGL",
        "-lGLU",
        "-lenet64",
    ]

# Check if we need to rebuild (compare source file timestamps with executable)
need_rebuild = True
if os.path.exists(exe_path):
    exe_mtime = os.path.getmtime(exe_path)
    # Check if any source file is newer than the executable
    src_newer = any(os.path.getmtime(cpp) > exe_mtime for cpp in cpps)
    if not src_newer:
        need_rebuild = False
        print("Executable is up-to-date, skipping rebuild...")

# Check if executable file is locked before building
# Only kill processes if we need to rebuild AND the file is locked
file_locked = False
if need_rebuild and os.path.exists(exe_path):
    try:
        # Try to open the file for writing to check if it's locked
        with open(exe_path, 'r+b') as f:
            pass
    except (IOError, OSError, PermissionError):
        # File is locked by a running process
        file_locked = True

# Only kill processes if we need to rebuild AND the file is locked
if need_rebuild and file_locked:
    if system == "Windows":
        try:
            import time
            # Try to find and kill running instances by executable name
            exe_name_no_ext = os.path.splitext(exe_name)[0]  # "projectOj" without .exe
            result = subprocess.run(
                ["powershell", "-Command", 
                 f"$procs = Get-Process -Name '{exe_name_no_ext}' -ErrorAction SilentlyContinue; if ($procs) {{ $procs | Stop-Process -Force; Write-Host 'Killed running game instances (file was locked)...' }}"],
                capture_output=True,
                timeout=2
            )
            # Also try to kill by window title (catches processes that might have different names)
            subprocess.run(
                ["powershell", "-Command", 
                 "$procs = Get-Process | Where-Object {$_.MainWindowTitle -like '*Operation Jackpot*'}; if ($procs) { $procs | Stop-Process -Force }"],
                capture_output=True,
                timeout=2
            )
            time.sleep(0.5)  # Give Windows time to release the file lock
        except:
            pass  # Ignore errors - if process doesn't exist, that's fine
    else:
        # Linux/macOS: try to kill by process name
        try:
            subprocess.run(["pkill", "-f", exe_name], capture_output=True, timeout=2)
            import time
            time.sleep(0.5)
        except:
            pass

# Build (only if needed)
if need_rebuild:
    print(f"Building from {src_dir}...")
    rc = run(cmd)
    if rc != 0:
        sys.exit(rc)
else:
    print(f"Using existing executable: {exe_path}")

# Run
print(f"Running {exe_path}...")
sys.exit(run([exe_path]))
