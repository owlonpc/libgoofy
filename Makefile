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

CPPFLAGS = -D_DEFAULT_SOURCE -D_POSIX_C_SOURCE=200809L
CFLAGS   = -std=c99 -pedantic -Wall -Wextra -Oz ${CPPFLAGS}
LDFLAGS  = -fPIC -shared
CC       = cc

libgoofy.so: goofy.c
	${CC} -o $@ ${CFLAGS} ${LDFLAGS} $<

libgoofy.so: Makefile

install: libgoofy.so
	install $< ~/.local/lib
	./shell-install.py

uninstall: libgoofy.so
	rm -rf ~/.local/lib/$<
	./shell-install.py -u

clean:
	rm -f libgoofy.so

.PHONY: install uninstall clean
