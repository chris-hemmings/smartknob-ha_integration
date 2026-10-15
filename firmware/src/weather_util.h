#pragma once

#include <stddef.h>
#include <stdint.h>

void weatherFromWmoCode(int code, char* description_out, size_t description_len, char* icon_out, size_t icon_len);
