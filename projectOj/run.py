#!/usr/bin/env python3
import os, sys, glob, platform, subprocess

def run(cmd):
    print(">", " ".join(cmd))
    return subprocess.call(cmd)

cpps = sorted(glob.glob("*.cpp"))
if not cpps:
    print("No .cpp files found in this folder.")
    sys.exit(1)

system = platform.system()  # "Windows", "Darwin" (macOS), "Linux"
exe = "game.exe" if system == "Windows" else "game"

# Base compile command
cmd = ["g++", *cpps, "-o", exe]

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
rc = run(cmd)
if rc != 0:
    sys.exit(rc)

# Run
if system == "Windows":
    sys.exit(run([os.path.join(".", exe)]))
else:
    sys.exit(run(["./" + exe]))
