#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "MapParse.h"
#include "ghash.h"

MapParse::MapParse()
    : m_mapList(NULL)
{
}

void MapParse::parseLine(const char* line)
{
    int len = strlen(line);
    const char* lineEnd = line + len;
    unsigned long start, end;

    start = strtoul(line, NULL, 16);
    const char* checkPoint = strstr(line, "-");
    if (!checkPoint) {
        return;
    }
    checkPoint++;
    if (checkPoint == lineEnd) {
        return;
    }

    end = strtoul(checkPoint, NULL, 16);
    const char* pathStart = line + 25 + sizeof(void*) * 6;
    if (pathStart >= lineEnd || pathStart[0] != '/') {
        pathStart = NULL;
    }
    const char* protectStart = strstr(line, " ");
    if (!protectStart && ((protectStart + 5) >= lineEnd)) {
        return;
    }
    protectStart++;
    unsigned protect = 0;

    if (protectStart[0] == 'r') {
        protect |= MapElement::READ;
    }
    if (protectStart[1] == 'w') {
        protect |= MapElement::WRITE;
    }
    if (protectStart[2] == 'x') {
        protect |= MapElement::EXECUTE;
    }
    if (protectStart[3] == 's') {
        protect |= MapElement::SHARED;
    }
    int blockSize = sizeof(MapElement);
    if (pathStart)
        blockSize += strlen(pathStart) + 1;
    MapElement* e = static_cast<MapElement*>(g_malloc_n(1, blockSize));
    e->m_start = start;
    e->m_end = end;
    e->m_protect = protect;
    if (pathStart) {
        strcpy(reinterpret_cast<char*>(e + 1), pathStart);
        e->m_path = reinterpret_cast<const char*>(e + 1);
    } else {
        e->m_path = NULL;
    }
    insertElement(e);
}

MapElement* MapParse::parseFile(const char* fileName)
{
    FILE* file = fopen(fileName, "r");
    MapParse p;
    while (true) {
        char buf[256];
        const char* line = fgets(buf, 256, file);
        if (feof(file)) {
            break;
        }
        p.parseLine(line);
    }
    fclose(file);
    return p.getMapList();
}

void MapParse::insertElement(MapElement* e)
{
    if (m_mapList) {
        MapElement* lastElement = m_mapList->m_prev;
        lastElement->m_next = e;
        m_mapList->m_prev = e;

        e->m_next = m_mapList;
        e->m_prev = lastElement;
    } else {
        m_mapList = e;
        e->m_next = e;
        e->m_prev = e;
    }
}

void MapParse::freeMapList(MapElement* list)
{
    MapElement* head = list;
    if (head == NULL)
        return;
    MapElement* next = head->m_next;
    while (next != head) {
        MapElement* toFree = next;
        next = next->m_next;
        g_free(toFree);
    }
    g_free(head);
}

const MapElement* lowerBound(const MapElement* list, unsigned long start, int (*compare)(const MapElement*, unsigned long))
{
    bool _start = true;
    for (const MapElement* i = list; (i != list || _start) ; i = i->m_next) {
        int now;

        _start = false;
        now = compare(i, start);
        if (now == 0) {
            return i;
        }
    }
    return NULL;
}
