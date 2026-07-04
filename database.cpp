#include <iostream>
#include <fstream>
#include <ctime>
#include <string.h>
#include "header.h"

using namespace std;

void logEv(const char* msg) {
    ofstream f(LOG_F, ios::app);
    if (f.is_open()) {
        time_t t = time(0);
        char* dt = ctime(&t);
        if (strlen(dt) > 0) dt[strlen(dt) - 1] = '\0';
        f << "[" << dt << "] " << msg << "\n";
        f.close();
    }
}

bool initDb() {
    ifstream chk(DB_F, ios::binary);
    if (chk.is_open()) {
        chk.close();
        return true;
    }

    ofstream f(DB_F, ios::binary);
    if (!f.is_open()) {
        cout << "[ERROR]\n";
        return false;
    }

    DbHeader h = { DB_VER, 0 };
    f.write((char*)&h, sizeof(DbHeader));
    f.close();
    
    logEv("БД створена");
    return true;
}

bool addRec(const ServerRecord& r) {
    fstream f(DB_F, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return false;

    DbHeader h;
    f.read((char*)&h, sizeof(DbHeader));

    f.clear();
    streampos writePos = sizeof(DbHeader) + (h.count * sizeof(ServerRecord));

    f.seekp(writePos);
    f.write((char*)&r, sizeof(ServerRecord));

    f.clear();

    h.count++;
    f.seekp(0, ios::beg);
    f.write((char*)&h, sizeof(DbHeader));

    f.close();
    logEv("Додана запись");
    return true;
}

ServerRecord* readAll(int& cnt) {
    ifstream f(DB_F, ios::binary);
    if (!f.is_open()) return nullptr;

    DbHeader h;
    f.read((char*)&h, sizeof(DbHeader));
    cnt = h.count;

    if (cnt == 0) {
        f.close();
        return nullptr;
    }

    ServerRecord* lst = new ServerRecord[cnt];
    f.read((char*)lst, sizeof(ServerRecord) * cnt);

    f.close();
    return lst;
}

bool delRec(int id) {
    fstream f(DB_F, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return false;

    DbHeader h;
    f.read((char*)&h, sizeof(DbHeader));

    ServerRecord tmp;
    for (int i = 0; i < h.count; i++) {
        streampos pos = f.tellg();
        f.read((char*)&tmp, sizeof(ServerRecord));

        if (tmp.id == id && tmp.active) {
            tmp.active = false;
            f.seekp(pos);
            f.write((char*)&tmp, sizeof(ServerRecord));
            f.close();
            logEv("Запись видалена");
            return true;
        }
    }
    f.close();
    return false;
}

bool upOnline(int id, int new_on) {
    fstream f(DB_F, ios::in | ios::out | ios::binary);
    if (!f.is_open()) return false;

    DbHeader h;
    f.read((char*)&h, sizeof(DbHeader));

    ServerRecord tmp;
    for (int i = 0; i < h.count; i++) {
        streampos pos = f.tellg();
        f.read((char*)&tmp, sizeof(ServerRecord));

        if (tmp.id == id && tmp.active) {
            tmp.online = new_on;
            f.seekp(pos);
            f.write((char*)&tmp, sizeof(ServerRecord));
            f.close();
            logEv("Оновлено онлайн");
            return true;
        }
    }
    f.close();
    return false;
}

void sName(const char* sStr) {
    int cnt = 0;
    ServerRecord* db = readAll(cnt);
    if (!db) { cout << "БД порожня\n"; return; }
    
    cout << "\nРезультати пошуку по \"" << sStr << "\": ---\n";
    bool fnd = false;
    for (int i = 0; i < cnt; i++) {
        if (db[i].active && strstr(db[i].name, sStr) != nullptr) {
            cout << "ID: " << db[i].id << " | Назва: " << db[i].name 
                 << " | IP: " << db[i].ip << " | Онлайн: " << db[i].online << endl;
            fnd = true;
        }
    }
    if (!fnd) cout << "Нічого не знайдено\n";
    delete[] db;
}

bool vacDb() {
    int cnt = 0;
    ServerRecord* db = readAll(cnt);
    if (!db) return false;

    ofstream tFile("temp.db", ios::binary);
    if (!tFile.is_open()) { delete[] db; return false; }

    DbHeader h = { DB_VER, 0 };
    tFile.write((char*)&h, sizeof(DbHeader)); 

    int a_cnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (db[i].active) {
            tFile.write((char*)&db[i], sizeof(ServerRecord));
            a_cnt++;
        }
    }

    h.count = a_cnt;
    tFile.seekp(0, ios::beg);
    tFile.write((char*)&h, sizeof(DbHeader));
    tFile.close();
    delete[] db;

    ifstream src("temp.db", ios::binary);
    ofstream dst(DB_F, ios::binary | ios::trunc);
    if (src.is_open() && dst.is_open()) {
        dst << src.rdbuf();
    }
    src.close();
    dst.close();
    
    logEv("Стиск виконано");
    return true;
}

void showStat() {
    int cnt = 0;
    ServerRecord* db = readAll(cnt);
    if (!db) { cout << "Немає даних\n"; return; }

    int t_srv = 0, t_on = 0, m_on = -1;
    char m_name[MAX_S] = "";

    for (int i = 0; i < cnt; i++) {
        if (db[i].active) {
            t_srv++;
            t_on += db[i].online;
            if (db[i].online > m_on) {
                m_on = db[i].online;
                strcpy(m_name, db[i].name);
            }
        }
    }

    cout << "\nСТАТИСТИКА БД\n";
    cout << "Активних серверів: " << t_srv << endl;
    cout << "Загальний онлайн: " << t_on << endl;
    if (t_srv > 0) {
        cout << "Середній онлайн: " << (float)t_on / t_srv << endl;
        cout << "Топ сервер: " << m_name << " (" << m_on << " гравців)\n";
    }
    delete[] db;
}

void printSorted() {
    int cnt = 0;
    ServerRecord* db = readAll(cnt); 
    if (!db) { cout << "\n[БД порожня]\n"; return; }

    for (int i = 0; i < cnt - 1; i++) {
        for (int j = 0; j < cnt - i - 1; j++) {
            if (db[j].online < db[j + 1].online) {
                ServerRecord tmp = db[j];
                db[j] = db[j + 1];
                db[j + 1] = tmp;
            }
        }
    }

    cout << "\nСЕРВЕРА (ВІДСОРТОВАННІ ПО ОНЛАЙНУ)\n";
    int v_cnt = 0;
    for (int i = 0; i < cnt; i++) {
        if (db[i].active) {
            cout << "ID: " << db[i].id 
                 << " | Назва: " << db[i].name 
                 << " | IP: " << db[i].ip 
                 << " | Онлайн: " << db[i].online << endl;
            v_cnt++;
        }
    }
    if (v_cnt == 0) cout << "[Всі сервера видалені]\n";
    delete[] db; 
}
