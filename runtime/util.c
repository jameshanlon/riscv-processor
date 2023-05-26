// Copyright 2023 lowRISC contributors.
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include "util.h"
#include "htif.h"

void *memset(void *s, int c, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        ((char*)s)[i] = 0;
    }
    return s;
}

void *memcpy(void *dest, const void *src, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        ((char*)dest)[i] = ((char*)src)[i];
    }
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    for (; *s1 && *s1 == *s2; ++s1, ++s2) {
    }
    return *(const unsigned char *)s1 - *(const unsigned char *)s2;
}

static unsigned strlen(const char* str) {
    long i = 0;
    while(str[i] != 0)
        ++i;
    return i;
}

void print(const char *s) {
    syscall(SYS_write, 1/*stdout*/, (uintptr_t)s, (uintptr_t)strlen(s), 0, 0, 0, 0);
}
