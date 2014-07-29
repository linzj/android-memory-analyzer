#ifndef MAPPARSE_H
#define MAPPARSE_H
#pragma once
#include <vector>
#include <string>
#include <stdint.h>

struct MapElement {
    enum PROTECT {
        SHARED = 1 << 3,
        READ = 1 << 2,
        WRITE = 1 << 1,
        EXECUTE = 1
    };
    unsigned long m_start;
    unsigned long m_end;
    unsigned m_protect;
    std::string m_path;
};

class MapParse {
public:
    typedef std::vector<MapElement> MapList;
    MapParse();
    void parseLine(const char* line);
    inline MapList& getMapList()
    {
        return m_mapList;
    }
    static MapList parseFile(const char*);

private:
    MapList m_mapList;
};

#endif /* MAPPARSE_H */
