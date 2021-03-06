#include <windows.h>
#include <stdio.h>
#include <assert.h>

#define NUM_GOOD_ARGS 2

int main(int argc, char *argv[])
{
    if (argc != NUM_GOOD_ARGS) {
        fprintf(stderr, "usage: %s filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    HANDLE                file;
    HANDLE                fileMap;
    PIMAGE_DOS_HEADER     dosHeader;
    PIMAGE_NT_HEADERS     ntHeaders;
    PIMAGE_SECTION_HEADER section;
    PIMAGE_SECTION_HEADER sectionHeader;
    HMODULE               user32;
    LPBYTE                map;
    DWORD                 fileSize;

    file = CreateFile(argv[1], GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ |
        FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Error opening %s!\n", argv[1]);
        return EXIT_FAILURE;
    }

    fileSize = GetFileSize(file, 0);
    if (!fileSize) {
        fprintf(stderr, "File size is 0 bytes!\n");
        CloseHandle(file);
        return EXIT_FAILURE;
    }

    fileMap = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, fileSize, NULL);
    if (!fileMap) {
        fprintf(stderr, "Error mapping file to memory!\n");
        CloseHandle(file);
        return EXIT_FAILURE;
    }

    map = MapViewOfFile(fileMap, FILE_MAP_ALL_ACCESS, 0, 0, fileSize);
    if (!fileMap) {
        fprintf(stderr, "Error creating map view of file!\n");
        CloseHandle(fileMap);
        CloseHandle(file);
        return EXIT_FAILURE;
    }

    dosHeader = (PIMAGE_DOS_HEADER) map;
    if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        fprintf(stderr, "MZ header not found! This file is not a valid PE.\n");
        free(dosHeader);
        CloseHandle(map);
        CloseHandle(fileMap);
        CloseHandle(file);
        return EXIT_FAILURE;
    }

    ntHeaders = (PIMAGE_NT_HEADERS) ((DWORD) map + dosHeader->e_lfanew);
    if (ntHeaders->Signature != IMAGE_NT_SIGNATURE) {
        fprintf(stderr, "NT signature not found! File is not a valid PE.\n");
        free(ntHeaders);
        free(dosHeader);
        CloseHandle(map);
        CloseHandle(fileMap);
        CloseHandle(file);
        return EXIT_FAILURE;
    }

    printf("MZ header: %X\n", dosHeader->e_magic);
    printf("NT signature: %X\n", ntHeaders->Signature);

    free(ntHeaders);
    free(dosHeader);
    CloseHandle(map);
    CloseHandle(fileMap);
    CloseHandle(file);

    return EXIT_SUCCESS;
}
