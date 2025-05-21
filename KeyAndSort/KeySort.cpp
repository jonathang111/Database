#include "KeySort.h"
//possibly introduce the use of an ordered map for lexical?

namespace KeySort::Support{

    void SortkeybyLexical(std::unordered_map<char, std::vector<Entry>>& map){ //work in progress

    }

    void SortvalbyLexical(std::unordered_map<char, std::vector<Entry>>& map){
        for(auto& [ch, vec] : map){
            std::sort(vec.begin(), vec.end(), [](const Entry& a, const Entry& b){
                std::string fileA(a.FileName);
                std::string fileB(b.FileName);

                auto found = fileA.find('.');
                auto found2 = fileB.find('.');
                fileA.substr(0, found);
                fileB.substr(0,found2);
                return fileA < fileB;
            });
        }
    }

}

namespace KeySort::Getter{

std::string FileExtension(Database db){
    return db.filedir.FileExtension;
}

char Lexical(Database db){
    if(db.filedir.FileName[0] >= 65 && db.filedir.FileName[0] <= 90)
        return db.filedir.FileName[0] + 32;
    return db.filedir.FileName[0];
}

}