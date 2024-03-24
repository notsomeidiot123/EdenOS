#pragma once
#define size_t unsigned long
#define NULL 0

void *memchr(const void *str, int c, size_t n);
int memcmp(const void *str1, const void *str2, size_t n);
void *memcpy(void *dest, const void *src, size_t n);
void *memmove(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
char *strcat(char *dest, const char *src);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);