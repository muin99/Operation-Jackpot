#!/usr/bin/env python3
"""
Build and run script for Operation Jackpot multiplayer game.
Run from project root: python run.py
"""
import os, sys, glob, platform, subprocess

def run(cmd):
    print(">", " ".join(cmd))
    return subprocess.call(cmd)

# Get project root directory (where this script is located)
project_root = os.path.dirname(os.path.abspath(__file__))
os.chdir(project_root)

# Find all source files in src/
src_dir = os.path.join(project_root, "src")
cpps = sorted(glob.glob(os.path.join(src_dir, "*.cpp")))
if not cpps:
    print("No .cpp files found in src/ directory.")
    sys.exit(1)

system = platform.system()  # "Windows", "Darwin" (macOS), "Linux"
exe_name = "projectOj.exe" if system == "Windows" else "projectOj"

# Create bin/Debug directory if it doesn't exist
bin_dir = os.path.join(project_root, "bin", "Debug")
os.makedirs(bin_dir, exist_ok=True)
exe_path = os.path.join(bin_dir, exe_name)

# Base compile command with include directory
include_dir = os.path.join(project_root, "include")
cmd = ["g++", "-I" + include_dir, *cpps, "-o", exe_path]

# OS-specific link flags
if system == "Windows":
    cmd += ["-lfreeglut", "-lopengl32", "-lglu32", "-lgdi32"]
elif system == "Darwin":  # macOS
    # Try freeglut first (Homebrew), then fallback to system GLUT
    # You can swap -lfreeglut -> -lglut if you prefer system GLUT
    cmd += ["-framework", "OpenGL", "-lfreeglut"]
else:  # Linux
    cmd += ["-lfreeglut", "-lGL", "-lGLU"]

# Build
print(f"Building from {src_dir}...")
rc = run(cmd)
if rc != 0:
    sys.exit(rc)

# Run
print(f"Running {exe_path}...")
sys.exit(run([exe_path]))

