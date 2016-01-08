#ifndef MAPPARSE_H
#define MAPPARSE_H
#pragma once
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
    const char* m_path;
    struct MapElement* m_next;
    struct MapElement* m_prev;
};

const MapElement* lowerBound(const MapElement* list, unsigned long start, int (*compare)(const MapElement*, unsigned long));

class MapParse {
public:
    MapParse();
    void parseLine(const char* line);
    inline MapElement* getMapList()
    {
        return m_mapList;
    }
    static MapElement* parseFile(const char*);
    static void freeMapList(MapElement* list);

private:
    void insertElement(MapElement* e);
    MapElement* m_mapList;
};

#endif /* MAPPARSE_H */
