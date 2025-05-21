#ifndef FILEREAD
#define FILEREAD
#include "../GeneralItems.h"

namespace Foldirs{ //usable
    Database* InitializeDatabase();
    void ReadDirectory(Database*&, const char*);
    void PrintSize(Database*&);
    void PrintDatabase(Database*&);
}

namespace { //internal
    int AppendPath(const char* dir, const char* filename, char* out);
    void ClearChunk(Database*&, int);
    void IncreaseDB(Database*&);
    char const* GetExtension(const char*);
    // void EntryAllocate(Database*&, int, int);
    void DeepEntryCopy(Database*&, Database*&);
    void AddToDB(Database*&, const char*, const char*);
    }
#endif