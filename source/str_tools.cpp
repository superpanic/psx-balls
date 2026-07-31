#include "str_tools.hh"

const char* stringstring(const char* haystack, const char* needle) {
    if(!haystack || !needle) {
        return nullptr; // Invalid input
    }

    size_t needle_len = stringlength(needle);
    if(needle_len == 0) {
        return haystack; // Empty needle matches at the start
    }

    for(const char* p = haystack; *p; ++p) {
        if(stringncompare(p, needle, needle_len)) {
            return p; // Found the needle
        }
    }

    return nullptr; // Needle not found
}

size_t stringlength(const char* str) {
    const char* s = str;
    while(*s) {
        ++s;
    }
    return s - str; // Return the length of the string
}

bool stringncompare(const char* s1, const char* s2, size_t n) {
    for(size_t i = 0; i < n; ++i) {
        if(s1[i] != s2[i]) {
            return false; // characters differ
        }
        if(s1[i] == '\0') {
            return true; // reached the end of both strings
        }
    }
    return true; // First n characters are equal
}

long stringtolong(const char *str, char **endptr, int base) {
    // Simple implementation of string to long conversion
    // This is a placeholder; a full implementation would handle errors and different bases
    long result = 0;
    while(*str) {
        if(*str >= '0' && *str <= '9') {
            result = result * base + (*str - '0');
        } else {
            break; // Non-digit character encountered
        }
        ++str;
    }
    if(endptr) {
        *endptr = (char*)str; // Set endptr to the character after the last processed digit
    }
    return result;
}

bool isnumber(char c) {
	return c >= '0' && c <= '9'; // Check if character is a digit
}

bool isspace(char c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
