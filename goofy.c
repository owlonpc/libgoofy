/* Copyright 2025 owl

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#define PROBABILITY 0.05

typedef struct {
	uint32_t keys[64];
	size_t len;
	float x;
	float y;
} Row;

static Row rows[4];
static struct wl_keyboard_listener *old_listener = NULL, new_listener;
struct xkb_keymap *xkb_keymap;
struct xkb_state *xkb_state;

uint32_t
adjacentkey(uint32_t c)
{
	float x = -1, y = -1;

	for (size_t row = 0; row < 4; row++) {
		for (size_t col = 0; rows[row].keys[col]; col++) {
			if (rows[row].keys[col] == c) {
				x = rows[row].x + col;
				y = rows[row].y;
				goto found;
			}
		}
	}
	return c;

found:;
	uint32_t adjacent[15];
	size_t weights[15], len = 0, weight = 0;

	for (size_t row = 0; row < 4; row++) {
		for (size_t col = 0; rows[row].keys[col]; col++) {
			uint32_t key = rows[row].keys[col];
			if (key == c)
				continue;

			float dx = x - (rows[row].x + col);
			float dy = y - rows[row].y;
			float distsqr = dx * dx + dy * dy;

			if (distsqr <= 3.f) {
				adjacent[len] = key;
				weights[len] = (int)(1000.f / (distsqr + .1f));
				weight += weights[len++];
			}
		}
	}

	if (!len)
		return c;

	int r = rand() % weight;
	for (size_t i = 0; i < len; i++) {
		r -= weights[i];
		if (r < 0)
			return adjacent[i];
	}

	return c;
}

static int
rowindex(const char *name)
{
	if (name[0] != 'A')
		return -1;

	switch (name[1]) {
	case 'E': return 0;
	case 'D': return 1;
	case 'C': return 2;
	case 'B': return 3;
	}
	return -1;
}

void
buildrowlayout(struct xkb_keymap *km)
{
	memset(rows, 0, sizeof rows);

	for (xkb_keycode_t kc = 8; kc < 255; kc++) {
		const char *name = xkb_keymap_key_get_name(km, kc);
		if (!name)
			continue;

		int r = rowindex(name);
		if (r < 0)
			continue;

		Row *row = &rows[r];
		if (row->len < 64)
			row->keys[row->len++] = kc;
	}

	rows[0].x = 0.0f, rows[0].y = 0.0f;
	rows[1].x = 0.5f, rows[1].y = 1.0f;
	rows[2].x = 0.75f, rows[2].y = 2.0f;
	rows[3].x = 1.25f, rows[3].y = 3.0f;
}

void
keymap(void *data, struct wl_keyboard *kbd, uint32_t format, int fd, uint32_t size)
{
	char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

	struct xkb_context *ctx = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
	xkb_keymap = xkb_keymap_new_from_string(ctx, map, XKB_KEYMAP_FORMAT_TEXT_V1, 0);
	xkb_state = xkb_state_new(xkb_keymap);

	munmap(map, size);

	buildrowlayout(xkb_keymap);
	srand(time(NULL));

	old_listener->keymap(data, kbd, format, fd, size);
}

void
key(void *data, struct wl_keyboard *wl_keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
	static uint32_t map[65536];

	uint32_t outkey;
	if (state == WL_KEYBOARD_KEY_STATE_PRESSED) {
		outkey = (float)rand() / RAND_MAX < PROBABILITY ? adjacentkey(key + 8) : key + 8;
		map[key] = outkey;
	} else {
		outkey = map[key];
	}

	old_listener->key(data, wl_keyboard, serial, time, outkey - 8, state);
}

int
wl_proxy_add_listener(struct wl_proxy *proxy, void (**impl)(void), void *data)
{
	static int (*fn)(struct wl_proxy *, void (**)(void), void *) = NULL;
	if (!fn)
		*(void **)&fn = dlsym(RTLD_NEXT, "wl_proxy_add_listener");

	if (!old_listener) {
		const char *iface = wl_proxy_get_class(proxy);
		if (iface && !strcmp(iface, "wl_keyboard")) {
			old_listener = (struct wl_keyboard_listener *)impl;
			new_listener = *old_listener;
			impl = (void (**)(void))&new_listener;

			new_listener.key = key;
			new_listener.keymap = keymap;
		}
	}

	return fn(proxy, impl, data);
}