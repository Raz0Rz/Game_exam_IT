#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <windows.h>
#include <algorithm>
#include <chrono>
#include <fstream>
#include <cstdlib>
#include <filesystem>

#include "C:\Users\user\Desktop\Kurs\Pr/Renderer.h"
#include "C:\Users\user\Desktop\Kurs\Pr/QuestWindow.h"

using namespace std;
namespace fs = std::filesystem;

class Compiler {
public:
    struct Result {
        bool success;
        bool passed;
        string output;
        string error;
    };

    Result compileAndRun(const string& userCode, const string& expected) {
        Result res;
        res.success = false;
        res.passed = false;

        // Сохраняем код пользователя КАК ЕСТЬ
        ofstream file("temp.cpp");
        file << userCode;  // Ничего не добавляем!
        file.close();

        // Компилируем
        system("g++ temp.cpp -o temp.exe 2> err.txt");

        // Проверяем ошибки
        ifstream err("err.txt");
        getline(err, res.error, '\0');
        err.close();

        if (!res.error.empty()) {
            return res;
        }

        // Запускаем
        system("temp.exe > out.txt");

        // Читаем вывод
        ifstream out("out.txt");
        getline(out, res.output, '\0');
        out.close();

        // Чистим
        system("del temp.cpp temp.exe err.txt out.txt >nul 2>nul");

        res.success = true;
        res.passed = (res.output == expected);

        return res;
    }
};

class player {
private:
    int time = 10;
    int score = 0;
    int status = 0;

public:
    player(int t, int stat) { time = t; status = stat; }

    int get_time() { return time; }
    int get_score() { return score; }
    int get_status() { return status; }

    void score_plus(int t) { score += t; }
    void score_minus(int t) { score -= t; }

    void time_reduce(int t) { time -= t; }
    void time_buff(int t) { time += t; }

    void addstatus(int s = 3) { status += s; }
    void reducestatus(int s = 1) { status -= s; }

    bool isAlive() {
        if (time <= 0) {
            return 0;
        }
        return 1;
    }
};

class effect {
public:
    virtual ~effect() = default;
    virtual string getName() const = 0;
};

class debuf : public effect {
private:
    int sec;
    player& p;
public:
    debuf(player& playerRef, int t = 2) : p(playerRef), sec(t) {}
    string getName() const override { return "Time reduction"; }

    void water() {
        p.addstatus(sec);
    }
    void pain() {
        p.addstatus(sec);
    }
    void scary() {
        p.addstatus(sec);
    }
};

class buff : public effect {
private:
    int sec;
    player& p;
public:
    buff(player& playerRef, int t = 1) : p(playerRef), sec(t) {}
    string getName() const override { return "Time increase"; }

    void moral() {
        p.reducestatus();
        p.time_buff(sec);
    }
};

class questions {
private:
    string txtQuest;
    int idOtv;
    vector<string> varOtv;
    string expected;
public:
    questions(string txt, int id, vector<string> var)
        : txtQuest(txt), idOtv(id), varOtv(var) {
    }
    questions(string txt, string code)
        : txtQuest(txt), expected(code) {
    }

    void addQuest(string name_quest) {
        varOtv.push_back(name_quest);
    }

    void changeAnswer(int id, string new_txt) {
        varOtv[idOtv] = new_txt;
        idOtv = id;
    }

    // ИСПОЛЬЗОВАТЬ ТОЛЬКО ЕСЛИ В ПАРАМЕТРЕ ПО УМОЛЧАНИЮ 3!! ЗНАЧЕНИЯ (т.е. проверка вопросов)
    bool askQuestSFML(QuestWindow& qw, int& elapsed) {
        auto start = chrono::steady_clock::now();
        bool correct = qw.show(txtQuest, varOtv, idOtv);
        auto end = chrono::steady_clock::now();
        elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
        return correct;
    }

    // ИСПОЛЬЗОВАТЬ ТОЛЬКО ЕСЛИ В ПАРАМЕТРЕ ПО УМОЛЧАНИЮ 2!! ЗНАЧЕНИЯ (т.е. проверка кода)
    bool ask_code_QuestSFML(QuestWindow& qw, int& elapsed) {
        auto start = chrono::steady_clock::now();

        // Получаем код от фронтенда
        string userCode = qw.getCode(txtQuest);

        // Компилируем и проверяем
        Compiler compiler;
        auto result = compiler.compileAndRun(userCode, expected);

        auto end = chrono::steady_clock::now();
        elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();

        return result.passed;  // возвращаем true/false
    }
};

class loc1 {
public:
    player& p;

    loc1(player& playerRef) : p(playerRef) {}

    // ==================== ЛЕС (ВОЛКИ) ====================
    int forest(QuestWindow& qw) {
        questions q("Name the main principles of OOP", 2, {
            "Indexing, abstraction, arraying",
            "Objectness, polymorphism, abstraction",
            "Polymorphism, inheritance, encapsulation",
            "Objectness, repeatability, classiness"
            });

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            p.score_plus(20);
            buff b(p, 1);
            b.moral();
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            debuf db(p, 2);
            p.time_reduce(10);
            db.pain();
            p.score_minus(10);
            if (!p.isAlive()) return -1;
            return 0;
        }
    }

    // ==================== РЕКА ====================
    int river(QuestWindow& qw) {
        questions q("What is a class in OOP?", 1, {
            "Data structure",
            "User-defined data type",
            "Template for function definition",
            "User container"
            });

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            p.score_plus(20);
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            debuf db(p, 1);
            p.time_reduce(10);
            db.water();
            p.score_minus(10);
            if (!p.isAlive()) return -1;
            return 0;
        }
    }

    // ==================== КАНЬОН ====================
    int canion(QuestWindow& qw) {
        questions q("What are virtual functions in OOP?", 3, {
            "1", "2", "3", "4"
            });

        if (p.get_status() > 0) {
            q.addQuest("5");
            p.time_reduce(5);
            p.reducestatus();

            int elapsed;
            if (q.askQuestSFML(qw, elapsed)) {
                p.time_reduce(elapsed);
                buff b(p, 1);
                p.score_plus(30);
                b.moral();
                if (!p.isAlive()) return -1;
                return 1;
            }
            else {
                p.time_reduce(elapsed);
                p.score_minus(10);
                p.time_reduce(10);
                if (!p.isAlive()) return -1;
                return 0;
            }
        }

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            p.score_plus(20);
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            p.score_minus(10);
            p.time_reduce(10);
            debuf db(p, 1);
            db.scary();
            if (!p.isAlive()) return -1;
            return 0;
        }
    }

    // ==================== ГЛУБОКИЙ ЛЕС (МЕДВЕДЬ) ====================
    int deepforest(QuestWindow& qw) {
        questions q("What is an abstract class in OOP?", 0, {
            "A template or base for other classes that inherit from it",
            "A class that serves as a base for one or more derived classes",
            "A class that cannot be inherited from",
            "A class declared inside another class"
            });

        if (p.get_status() > 0) {
            q.addQuest("A template or base for other classes that inherit from it");
            q.changeAnswer(4, "A class that has at least one private member");
            p.time_reduce(5);
            p.reducestatus();
        }

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            buff b(p, 1);
            p.score_plus(20);
            b.moral();
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            debuf db(p, 1);
            p.time_reduce(10);
            p.score_minus(20);
            db.pain();
            if (!p.isAlive()) return -1;
            return 0;
        }
    }

    // ==================== ДЕРЕВНЯ ====================
    int village(QuestWindow& qw) {
        if (p.get_status() > 0) {
            buff b(p, 1);
            b.moral();
        }
        return 1;  // всегда успех
    }

    // ==================== РЫНОК ====================
    int market(QuestWindow& qw) {
        if (p.get_status() > 0) {
            p.reducestatus();
            p.time_reduce(5);
        }

        questions q("What is encapsulation?", 2, {
            "1", "2", "3", "4"
            });

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            p.score_plus(20);
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            p.score_minus(10);
            debuf db(p, 1);
            db.pain();
            if (!p.isAlive()) return -1;
            return 0;
        }
    }

    // ==================== САРАЙ (ГУСЬ) ====================
    int barn(QuestWindow& qw) {
        questions q("OOP question", 0, {
            "1", "2", "3", "4"
            });

        if (p.get_status() > 0) {
            p.reducestatus();
            p.time_reduce(5);
            q.addQuest("5");
        }

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            p.score_plus(20);
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            p.score_minus(10);
            debuf db(p, 1);
            db.pain();
            if (!p.isAlive()) return -1;
            return 0;
        }
    }

    // ==================== ПОЛЕ (ХИМЕРА) ====================
    int pole(QuestWindow& qw) {
        int fl = 0;
        bool fl4 = 0;
        buff b(p, 1);
        debuf db(p, 1);

        questions q1("What advantages does using OOP provide?", 2, {
            "1", "2", "3", "4"
            });

        questions q2("What is polymorphism?", 0, {
            "The ability of objects of different classes to use the same interface",
            "A mechanism that allows combining data and methods...",
            "The principle of highlighting essential characteristics...",
            "A mechanism that allows creating new classes..."
            });

        questions q3("What are the 5 main principles of writing clean OOP code?", 3, {
            "1", "2", "3", "SOLID"
            });

        if (p.get_status() > 0) {
            fl4 = 1;
            db.scary();
            q2.addQuest("The ability of objects of different classes to use the same interface");
            q2.changeAnswer(4, "A mechanism that allows extending system functionality...");
            p.time_reduce(5);
            p.reducestatus();

            questions q4("Complex OOP question", 0, {
                "1", "2", "3", "4"
                });

            int elapsed4;
            if (q4.askQuestSFML(qw, elapsed4)) {
                p.time_reduce(elapsed4);
                p.score_plus(20);
                fl += 1;
            }
            else {
                p.time_reduce(elapsed4);
                db.pain();
                p.score_minus(10);
            }
        }
        else {
            b.moral();
        }

        int elapsed1;
        if (q1.askQuestSFML(qw, elapsed1)) {
            p.time_reduce(elapsed1);
            fl += 1;
            p.score_plus(20);
        }
        else {
            p.time_reduce(elapsed1);
            if (fl4 && fl == 0) {
                return 666;  // смерть
            }
            p.score_minus(10);
            db.pain();
        }

        int elapsed2;
        if (q2.askQuestSFML(qw, elapsed2)) {
            p.time_reduce(elapsed2);
            p.score_plus(20);
            fl += 1;
        }
        else {
            p.time_reduce(elapsed2);
            if (fl == 1 && fl4) {
                return 666;
            }
            else if (fl == 0 && !fl4) {
                return 666;
            }
            p.score_minus(10);
            db.pain();
        }

        int elapsed3;
        if (q3.askQuestSFML(qw, elapsed3)) {
            p.time_reduce(elapsed3);
            p.score_plus(20);
            fl += 1;
        }
        else {
            p.time_reduce(elapsed3);
            if (fl4 && fl == 2) {
                questions q5("Complex OOP question", 1, {
                    "1$", "2", "3@", "4#", "5"
                    });
                int elapsed4;
                if (q5.askQuestSFML(qw, elapsed4)) {
                    p.time_reduce(elapsed4);
                    db.pain();
                    p.score_minus(10);
                    fl = -1;
                }
                else {
                    return 666;
                }
            }
            else if (!fl4 && fl == 1) {
                questions q6("Complex OOP question", 2, {
                    "1$", "2@#", "3", "4#", "5!^"
                    });
                int elapsed5;
                if (q6.askQuestSFML(qw, elapsed5)) {
                    p.time_reduce(elapsed5);
                    db.pain();
                    p.score_minus(10);
                    fl = -1;
                }
                else {
                    return 666;
                }
            }
            else {
                p.score_minus(10);
                db.pain();
            }
        }

        if (!p.isAlive()) {
            return -1;
        }

        if (fl == -1) {
            b.moral();
            p.score_plus(50);
            return 13;  // особая победа
        }
        else if (fl >= 2) {
            b.moral();
            p.score_plus(100);
            return 1;
        }
        else {
            p.score_minus(50);
            return 0;
        }
    }
};

class loc3 {
public:
    player& p;

    loc3(player& playerRef) : p(playerRef) {}

    int l1(QuestWindow& qw) {
        questions q("Необходимо написать код, который считает 2+2 и выводит ответ с помощью cout", "4");
        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            p.time_reduce(elapsed);
            p.score_plus(20);
            buff b(p, 1);
            b.moral();
            if (!p.isAlive()) return -1;
            return 1;
        }
        else {
            p.time_reduce(elapsed);
            debuf db(p, 2);
            p.time_reduce(10);
            db.pain();
            p.score_minus(10);
            if (!p.isAlive()) return -1;
            return 0;
        }
    }
};
