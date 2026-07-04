#include <iostream>
#include <locale.h>
#include "header.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "Ukr");

    if (!initDb()) return -1;
    int chs;
    do {
        cout << "\nМЕНЮ\n";
        cout << "1. Вивести всі сервери (Сортування)\n";
        cout << "2. Додати новий сервер\n";
        cout << "3. Оновити онлайн сервера\n";
        cout << "4. Видалити сервер (Мяке)\n";
        cout << "5. Пошук сервера за назвою\n";
        cout << "6. Показати зведену статистику\n";
        cout << "7. Запустити очищення\n";
        cout << "0. Вихід\n";
        cout << "... ";
        cin >> chs;

        if (chs == 1) {
            printSorted();
        }
        else if (chs == 2) {
            ServerRecord r;
            cout << "ID: "; cin >> r.id;
            cin.ignore(); 
            cout << "Назва сервера: ";
            cin.getline(r.name, MAX_S);
            cout << "IP адрес: ";
            cin.getline(r.ip, MAX_S);
            cout << "Поточний онлайн: ";
            cin >> r.online;

            r.active = true;
            if (addRec(r)) cout << "Записано в БД\n";
        } 
        else if (chs == 3) {
            int id, online;
            cout << "ID сервера: "; cin >> id;
            cout << "Новий онлайн: "; cin >> online;
            if (upOnline(id, online)) cout << "Дані змінені\n";
            else cout << "Сервер не знайдено\n";
        }
        else if (chs == 4) {
            int id;
            cout << "ID для видалення: "; cin >> id;
            if (delRec(id)) cout << "Сервер прихований\n";
            else cout << "Сервер не знайдено\n";
        }
        else if (chs == 5) {
            char sStr[MAX_S];
            cout << "Частина назви для пошуку: ";
            cin.ignore();
            cin.getline(sStr, MAX_S);
            sName(sStr);
        }
        else if (chs == 6) {
            showStat();
        }
        else if (chs == 7) {
            if (vacDb()) cout << "БД оптимізован\n";
            else cout << "Помилка оптимізації\n";
        }
    } while (chs != 0);

    return 0;
}
