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

#include <stdint.h>

#define SYS_exit 93
#define SYS_read 63
#define SYS_write 64

uintptr_t syscall(uintptr_t n, uintptr_t a0, uintptr_t a1, uintptr_t a2,
                  uintptr_t a3, uintptr_t a4, uintptr_t a5, uintptr_t a6);

void shutdown(int code);
