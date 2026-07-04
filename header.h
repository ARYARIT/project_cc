#ifndef HEADER_H
#define HEADER_H

#include <fstream>

#define DB_VER 3
#define MAX_S 32
#define DB_F "cluster_data.db"
#define LOG_F "db_events.log"

struct ServerRecord {
    int id;
    char name[MAX_S];
    char ip[MAX_S];
    int online;
    bool active; 
};

struct DbHeader {
    int ver;
    int count;
};

void logEv(const char* msg);
bool initDb();
bool addRec(const ServerRecord& r);
ServerRecord* readAll(int& cnt);
bool delRec(int id);
bool upOnline(int id, int new_on);
void sName(const char* sStr);
bool vacDb();
void showStat();
void printSorted();

#endif
