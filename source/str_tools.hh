#pragma once

#include <stdarg.h>
#include <stddef.h>

const char* stringstring(const char* haystack, const char* needle);
size_t stringlength(const char* str);
bool stringncompare(const char* s1, const char* s2, size_t n);
long stringtolong(const char *str, char **endptr, int base);
bool isnumber(char c);
bool isspace(char c);