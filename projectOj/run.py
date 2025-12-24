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

# Base compile command
cmd = ["g++", "-std=c++17", "-I" + project_include]

# 🔹 WINDOWS (MinGW + freeglut)
if system == "Windows":
    # CHANGE THIS PATH if freeglut is installed elsewhere
    FREEGLUT_ROOT = r"C:\freeglut"

    cmd += [
        "-I" + os.path.join(FREEGLUT_ROOT, "include"),
        "-L" + os.path.join(FREEGLUT_ROOT, "lib"),
        *cpps,
        "-o", exe_path,
        "-lfreeglut",
        "-lopengl32",
        "-lglu32",
        "-lgdi32",
        "-lwinmm",
    ]

# 🔹 macOS
elif system == "Darwin":
    cmd += [
        *cpps,
        "-o", exe_path,
        "-framework", "OpenGL",
        "-lfreeglut",
    ]

# 🔹 Linux
else:
    cmd += [
        *cpps,
        "-o", exe_path,
        "-lfreeglut",
        "-lGL",
        "-lGLU",
    ]

# Build
print(f"Building from {src_dir}...")
rc = run(cmd)
if rc != 0:
    sys.exit(rc)

# Run
print(f"Running {exe_path}...")
sys.exit(run([exe_path]))
