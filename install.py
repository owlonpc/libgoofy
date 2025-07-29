#!/usr/bin/env python3

# Copyright 2025 owl
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import sys
from pathlib import Path

line = "export LD_PRELOAD=/usr/lib/libmimalloc.so"
fish_line = "set -x LD_PRELOAD /usr/lib/libmimalloc.so"

configs = [
    "~/.bashrc", "~/.bash_profile", "~/.profile",
    "~/.zshrc", "~/.zsh_profile", "~/.zprofile",
    "~/.config/fish/config.fish"
]

for env_var in ["BASH_ENV", "ENV"]:
    if env_var in os.environ:
        configs.append(os.environ[env_var])

def install():
    for config in configs:
        path = Path(config).expanduser()
        if not path.exists():
            continue
        
        content = path.read_text()
        target_line = fish_line if "fish" in str(path) else line
        
        if target_line not in content:
            with path.open("a") as f:
                f.write(f"\n{target_line}\n")
            print(f"added to {config}")
        else:
            print(f"already in {config}")

def uninstall():
    for config in configs:
        path = Path(config).expanduser()
        if not path.exists():
            continue
        
        content = path.read_text()
        target_line = fish_line if "fish" in str(path) else line
        
        if target_line in content:
            lines = content.splitlines()
            lines = [l for l in lines if l.strip() != target_line]
            path.write_text("\n".join(lines) + "\n")
            print(f"removed from {config}")
        else:
            print(f"not found in {config}")

if len(sys.argv) > 1 and sys.argv[1] == "-u":
    uninstall()
else:
    install()