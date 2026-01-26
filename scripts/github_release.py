#!/usr/bin/env python3
"""Script to create and push a git tag from the VERSION file."""

import os
import subprocess
import sys

def find_version_file():
    """Find VERSION file by looking in current dir and parent dirs."""
    current = os.path.abspath(os.getcwd())
    while current != os.path.dirname(current):  # Stop at filesystem root
        version_path = os.path.join(current, 'VERSION')
        if os.path.isfile(version_path):
            return version_path
        current = os.path.dirname(current)
    return None

def main():
    # Read version from VERSION file
    version_path = find_version_file()
    if not version_path:
        print("Error: VERSION file not found", file=sys.stderr)
        sys.exit(1)
    
    try:
        with open(version_path, 'r') as f:
            version = f.read().strip()
    except OSError as e:
        print(f"Error reading VERSION file: {e}", file=sys.stderr)
        sys.exit(1)
    
    if not version:
        print("Error: VERSION file is empty", file=sys.stderr)
        sys.exit(1)
    
    tag = f"v{version}"
    
    # Create annotated tag
    cmd_tag = ['git', 'tag', '-a', tag, '-m', f'Release {tag}']
    print(f"Creating tag: {' '.join(cmd_tag)}")
    try:
        subprocess.run(cmd_tag, check=True)
        print(f"Created tag: {tag}")
    except subprocess.CalledProcessError as e:
        print(f"Error creating tag: {e}", file=sys.stderr)
        sys.exit(1)
    
    # Push tag to origin
    cmd_push = ['git', 'push', 'origin', tag]
    print(f"Pushing tag: {' '.join(cmd_push)}")
    try:
        subprocess.run(cmd_push, check=True)
        print(f"Pushed tag {tag} to origin")
    except subprocess.CalledProcessError as e:
        print(f"Error pushing tag: {e}", file=sys.stderr)
        sys.exit(1)

if __name__ == '__main__':
    main()
