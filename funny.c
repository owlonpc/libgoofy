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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define LEN(x) (sizeof x / sizeof *x)

typedef struct {
	const char *keys;
	float       x, y;
} Row;

Row qwerty[] = { { "1234567890-+", 0.0f, 0.0f },
	             { "qwertyuiop[]", 0.5f, 1.0f },
	             { "asdfghjkl;'", 0.75f, 2.0f },
	             { "zxcvbnm,./", 1.25f, 3.0f } };

char
adjacentchar(char c)
{
	float x = -1, y = -1;

	for (size_t row = 0; row < LEN(qwerty); row++) {
		for (size_t col = 0; qwerty[row].keys[col]; col++) {
			if (qwerty[row].keys[col] == c) {
				x = qwerty[row].x + col;
				y = qwerty[row].y;
				goto found;
			}
		}
	}
	return c;

found:;
	char   adjacent[15];
	size_t weights[15], len = 0, weight = 0;

	for (size_t row = 0; row < LEN(qwerty); row++) {
		for (size_t col = 0; qwerty[row].keys[col]; col++) {
			char key = qwerty[row].keys[col];
			if (key == c)
				continue;

			float dx      = x - (qwerty[row].x + col);
			float dy      = y - qwerty[row].y;
			float distsqr = dx * dx + dy * dy;

			if (distsqr <= 3.f) {
				adjacent[len] = key;
				weights[len]  = (int)(1000.f / (distsqr + .1f));
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

int
getchar(void)
{
	static int (*ogetchar)(void) = NULL;
	if (!ogetchar)
		*(void **)&ogetchar = dlsym(RTLD_NEXT, "getchar");

	static int seeded = 0;
	if (!seeded) {
		srand(time(NULL));
		seeded = 1;
	}

	int c = ogetchar();
	return (rand() % 10 != 0) ? c : adjacentchar(c);
}
