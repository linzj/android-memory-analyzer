#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "MapParse.h"

MapParse::MapParse()
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
    MapElement e;
    e.m_start = start;
    e.m_end = end;
    e.m_protect = protect;
    if (pathStart)
        e.m_path.assign(pathStart);
    m_mapList.push_back(e);
}

MapParse::MapList MapParse::parseFile(const char* fileName)
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
