#ifndef KEYSORT
#define KEYSORT
#include "../GeneralItems.h"
#include "../CacheReadWrite/CacheRW.h"

namespace KeySort::Support{

    template<typename KeyFunc>
    auto GroupEntries(Database*& db, KeyFunc keyFn) 
        -> std::unordered_map<decltype(keyFn(db[0])), std::vector<Entry>> 
    {   
        using KeyType = decltype(keyFn(db[0]));
        std::unordered_map<KeyType, std::vector<Entry>> temp;

        for (int i = 0; i < db->currentSize; ++i)
            temp[keyFn(db[i])].push_back(db[i].filedir);
        

        return temp;
    }

        //Support functions
        void SortvalbyLexical(std::unordered_map<char, std::vector<Entry>>& map);

}

namespace KeySort::Getter{
    //type get
    std::string FileExtension(Database);
    char Lexical(Database);
}

namespace KeySort{

    typedef enum{
        path = 0,
        lexical = 1,
        fileExtension = 2,
        fileSize = 3
    } SortType;

    template<typename keyType, typename valueType>
    auto mapDB(SortType type, Database*& db)
        -> std::unordered_map<keyType, std::vector<valueType>>{
        std::unordered_map<keyType, std::vector<valueType>> map2;
        switch(type){
            case lexical: 
            if constexpr (std::is_same<keyType, char>::value && std::is_same<valueType, Entry>::value){
                map2 = Support::GroupEntries(db, Getter::Lexical);
                if(!map2.empty())
                Support::SortvalbyLexical(map2);
            }
            else std::cout << "Incorrect map types for lexical\n";
            return map2;

            case fileExtension: 
            if constexpr (std::is_same<keyType, std::string>::value && std::is_same<valueType, Entry>::value)
                map2 = Support::GroupEntries(db, Getter::FileExtension);
            else std::cout << "Incorrect map types for file extension\n";
            return map2;

            case fileSize: return map2;
            case path:  return map2;
        };
        return map2;
    }


}

#endif