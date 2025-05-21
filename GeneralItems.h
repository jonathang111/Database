 //this file should store structs, enums, etc that will be common
#ifndef GENERALITEMS
#define GENERALITEMS

#include <chrono>
#include <cstring>
//#include <filesystem> //maybe for winodws?
#include <dirent.h>
#include <iostream>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <optional>
#include <algorithm>

#define MAX_PATHSIZE 512
#define MAX_DIRECTORY 256
#define MAX_FILENAME 128
#define MAX_DBCHUNK 4096
#define MAX_EXTENSION 32

#define REDUNFILES(x) strcmp(x, ".") == 0 || strcmp(x, "..") == 0

typedef enum{
    unknown = 0,
    file = 1,
    folder = 2
} FileType;

struct Entry{
    char Directory[MAX_DIRECTORY];
    char FileName[MAX_FILENAME];
    char FileExtension[MAX_EXTENSION];
    bool isHidden;
};

struct Database{
    FileType type; //type
    Entry filedir; //directory
    int maxSize;
    int currentSize;
};

class CacheDB{
    private:
    int keyamt, entryamt, date;
    float version;
};
#endif