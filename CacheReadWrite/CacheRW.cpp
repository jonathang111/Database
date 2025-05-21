#include "CacheRW.h"

namespace CacheRW::Internal{

size_t ComputeOffset(size_t EntryEnd){
size_t roughOffset = static_cast<size_t>(EntryEnd * GROWTH_FACTOR);
size_t allignedOffset = static_cast<size_t>(roughOffset + (BLOCK_SIZE-1)) & ~ (BLOCK_SIZE-1);

return allignedOffset;
}

void StoreKeyIndex(int keyAmount, KeyIndexEntry*& keyArray ,StorageInterface& storage){ //specific to dynamic key?
    for(int i = 0; i < keyAmount; i++){
        //write keySize so we can read it
        storage.write(reinterpret_cast<char*>(&keyArray[i].keySize), sizeof(int));
        //AGAIN!!!!!!  WRITE THE CONTENTS OF POINTER!!! NOT POINTER ITSELF!!!!!!!!!!!!!!!!!!!!!!!!!!
        storage.write(reinterpret_cast<char*>(keyArray[i].key), keyArray[i].keySize * sizeof(char));
        //write offset
        storage.write(reinterpret_cast<char*>(&keyArray[i].offset), sizeof(int));
        //write count
        storage.write(reinterpret_cast<char*>(&keyArray[i].count), sizeof(int));
    }
}

}

namespace CacheRW{

std::ostream& operator<<(std::ostream& os, const KeyIndexEntry& entry){ //quick print key entry
    os << "[Key: " << entry.key
        << ", Offset: " << entry.offset
        << ", Count: " << entry.count << "]";
    return os;
}

std::optional<CacheMeta> ReadCacheMetaData(StorageInterface& storage){ //optional returns pointer to cacheDB or nothing.
    CacheHeader header;

    if(!storage.isOpen()){ //maybe remove or somehow add name of file.
        std::cout << "Could not open file\n";
         return {};
    }
    storage.seek(0);
    storage.read(&header, sizeof(CacheHeader));
    std::string name = storage.getLabel();
    CacheMeta output(header, name.c_str());
    return output;
}

auto GetKeyArray(CacheMeta cacheMetaData) //need to transfer to storage interface logic
    -> std::optional<KeyIndexEntry*>{
    std::ifstream file(cacheMetaData.cacheName, std::ios::binary);
    KeyIndexEntry* output;
    if(!file.is_open()){
        std::cout << "Could not open " << cacheMetaData.cacheName << std::endl;
        return {};
    }

    output = new KeyIndexEntry[cacheMetaData.keyAmount];
    file.seekg(cacheMetaData.footerStart);
    for(int i = 0; i < cacheMetaData.keyAmount; i++){ //loop for dynamic tags only
        file.read(reinterpret_cast<char*>(&output[i].keySize), sizeof(int));

        output[i].key = new char[output[i].keySize + 1];
        file.read(output[i].key, output[i].keySize);
        output[i].key[output[i].keySize] = '\0';

        file.read(reinterpret_cast<char*>(&output[i].offset), sizeof(int));

        file.read(reinterpret_cast<char*>(&output[i].count), sizeof(int));
    }
    //file.read(reinterpret_cast<char*>(output), sizeof(KeyIndexEntry)*cacheMetaData.keyAmount);
    return output;

}

}