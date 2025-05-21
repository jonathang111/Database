#include "FileRead.h"
//Main functions
namespace Foldirs{

    Database* InitializeDatabase(){
        Database* db = new Database[MAX_DBCHUNK];
        ClearChunk(db, 0);
        // EntryAllocate(db, 0, MAX_DBCHUNK);
        db->currentSize = 0;
        db->maxSize = 1024;
        return db;
    }

    void ReadDirectory(Database*& db, const char* dirname){
        DIR* Directory = opendir(dirname);
        if(Directory == nullptr){
            std::cout << "Directory Error: " << strerror(errno) << std::endl;
            return;
        }
        //std::cout << "successful directory open...\n";
        struct dirent* file = readdir(Directory);
        while(file){
            if(REDUNFILES(file->d_name)){
                file = readdir(Directory);
                continue;
            }
            AddToDB(db, dirname, file->d_name);
            file = readdir(Directory);
        }
        closedir(Directory); 
    }

    void PrintSize(Database*& db){
        std::cout << db->currentSize << "\n";
    }

    void PrintDatabase(Database*& db){
        for(int i = 0; i < db->currentSize; i++){
            std::cout << "File Type: " << db[i].type << ' ' << "Extension: " << db[i].filedir.FileExtension << ' ';
            std::cout << db[i].filedir.Directory << '/' << db[i].filedir.FileName << '\n';
        }
    }

}

namespace { //internal, only visible in this file
    int AppendPath(const char* dir, const char* filename, char* out){ //need to add error handling
        size_t len = 0;
        if(dir && *dir){ 
            /*dir, pointers returns false if null, 
            *dir dereferenced, if empty, i.e. no char in first position, return false*/
            len = std::snprintf(out, MAX_PATHSIZE, "%s", dir); // copy to buff
            if(out[len-1] != '/' && len + 1 < MAX_PATHSIZE){
                out[len++] = '/';
                out[len] = '\0';
            }
    
            if(filename && *filename && len + strlen(filename) < MAX_PATHSIZE){
                std::strncat(out, filename, MAX_PATHSIZE - len -1); //cat to buff,
            }
            else return 1;
        }
        else return 1;
    
        return 0;
    }

    void ClearChunk(Database*& input, int size){
        for(int i = 0; i < size + MAX_DBCHUNK; i++){
            input[i].currentSize = 0;
            input[i].maxSize = 0;
            input[i].type = unknown;
        }
    }

    void IncreaseDB(Database* &db){ //not RAII standards, i know ):
        Database* temp = new Database[db->currentSize + MAX_DBCHUNK];;
        Database* temp2 = db;
    
        ClearChunk(temp, db->currentSize);
        // EntryAllocate(temp, db->currentSize, MAX_DBCHUNK + db->currentSize);
        // memcpy(temp, db, (sizeof(Database) * db->currentSize));
        //this will be better managed later, i promsie!
        DeepEntryCopy(temp, db);
    
        temp->currentSize = db->currentSize;
        temp->maxSize = db->maxSize + MAX_DBCHUNK;
        db = temp;
        delete[] temp2; //does NOT delete old char* pointers in entry, so keep using memcpy for now
    }

    const char* GetExtension(const char* filename){
        const char* temp = strrchr(filename, '.');
        if(!temp || temp == filename || strlen(temp) >= MAX_EXTENSION) return "NULL";
        return temp + 1;
    }

    // void EntryAllocate(Database*& db, int start, int end){
    //     for(int i = start; i < end; i++){
    //         db[i].filedir.Directory = new char[MAX_DIRECTORY];
    //         db[i].filedir.FileName = new char[MAX_FILENAME];
    //         db[i].filedir.FileExtension = new char[MAX_EXTENSION];
    //     }
    // }

    void AddToDB(Database*& db, const char* dirname, const char* filename){
        int currentsize = db->currentSize;
        if(currentsize == db->maxSize)
            IncreaseDB(db);
        struct stat info;
        int error;
        char* path = new char[MAX_PATHSIZE];
    
        error = AppendPath(dirname, filename, path);
        if(error){
            std::cout << "Append error\n";
            return;
        }
        //std::cout << "Opening: " << path << std::endl;
        if(lstat(path, &info) == 0){ //lstat instead of stat for symblink prevention
            if(S_ISREG(info.st_mode)){ //only copy files
                db[currentsize].type = file;
                strcpy(db[currentsize].filedir.Directory, dirname);
                strcpy(db[currentsize].filedir.FileName, filename);
                db[currentsize].filedir.FileName[MAX_FILENAME-1] = '\0';
                const char* temp = GetExtension(filename);
                strncpy(db[currentsize].filedir.FileExtension, temp, MAX_EXTENSION);//NULL of ext too long
                db->currentSize++;
            }
            else if(S_ISDIR(info.st_mode))Foldirs::ReadDirectory(db, path);
            else db[currentsize].type = unknown;
        }
        else{
            std::cout << "Stat Error: " << strerror(errno) << std::endl;
            std::cout << "Path Name: " << path << std::endl;
        }
        delete[] path;
    }

    void DeepEntryCopy(Database*& newone, Database*& oldone){
        for(int i = 0; i < oldone->currentSize; i++){
            newone[i].type = oldone[i].type;
            strcpy(newone[i].filedir.Directory, oldone[i].filedir.Directory);
            strcpy(newone[i].filedir.FileExtension, oldone[i].filedir.FileExtension);
            strcpy(newone[i].filedir.FileName, oldone[i].filedir.FileName);
        }
    }
}