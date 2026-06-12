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
        bool cheatDetected;
    };

    Result compileAndRun(const string& userCode, const string& expected) {
        Result res;
        res.success = false;
        res.passed = false;
        res.cheatDetected = false;

        // АНТИ-ЧИТ
        vector<string> forbidden = {
            "system(",
            "goto",
            "#define"
        };

        for (const auto& pattern : forbidden) {
            if (userCode.find(pattern) != string::npos) {
                res.cheatDetected = true;
                res.error = "Запрещенная конструкция: " + pattern;
                return res;
            }
        }

        // Подставляем библиотеки и using namespace std
        string fullCode =
            "#include <iostream>\n"
            "#include <string>\n"
            "#include <vector>\n"
            "#include <algorithm>\n"
            "using namespace std;\n\n"
            + userCode;

        ofstream file("temp.cpp");
        file << fullCode;
        file.close();

        system("g++ temp.cpp -o temp.exe 2> err.txt");

        ifstream err("err.txt");
        getline(err, res.error, '\0');
        err.close();

        if (!res.error.empty()) {
            return res;
        }

        system("timeout /t 5 /nobreak >nul & temp.exe > out.txt 2>&1");

        ifstream out("out.txt");
        getline(out, res.output, '\0');
        out.close();

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
    vector<string> requiredCode;
public:
    questions(string txt, int id, vector<string> var)
        : txtQuest(txt), idOtv(id), varOtv(var) {
    }
    questions(string txt, string code, vector<string> req = {})
        : txtQuest(txt), expected(code), requiredCode(req) {
    }

    void addQuest(string name_quest) {
        varOtv.push_back(name_quest);
    }

    void changeAnswer(int id, string new_txt) {
        varOtv[idOtv] = new_txt;
        idOtv = id;
    }

    bool askQuestSFML(QuestWindow& qw, int& elapsed) {
        auto start = chrono::steady_clock::now();
        bool correct = qw.show(txtQuest, varOtv, idOtv);
        auto end = chrono::steady_clock::now();
        elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
        return correct;
    }

    bool ask_code_QuestSFML(QuestWindow& qw, int& elapsed) {
        auto start = chrono::steady_clock::now();
        string userCode = qw.getCode(txtQuest);

        for (const auto& req : requiredCode) {
            if (userCode.find(req) == string::npos) {
                return false;
            }
        }

        Compiler compiler;
        auto result = compiler.compileAndRun(userCode, expected);
        auto end = chrono::steady_clock::now();
        elapsed = chrono::duration_cast<chrono::seconds>(end - start).count();
        return result.passed;
    }
};

// ==================== БАЗОВЫЙ КЛАСС ====================
class lvls {
protected:
    player& p;

public:
    lvls(player& playerRef) : p(playerRef) {}
    virtual ~lvls() = default;

    void reduceTime(int t) { p.time_reduce(t); }
    void addScore(int s) { p.score_plus(s); }
    void reduceScore(int s) { p.score_minus(s); }
    void reduceStatus(int s = 1) { p.reducestatus(s); }
    bool isAlive() { return p.isAlive(); }

    void applyBuff(int t = 1) { buff b(p, t); b.moral(); }
    void applyDebuff(int t = 2, string type = "pain") {
        debuf db(p, t);
        if (type == "water") db.water();
        else if (type == "pain") db.pain();
        else if (type == "scary") db.scary();
    }
};

// ==================== LOC1 ====================
class loc1 : public lvls {
public:
    loc1(player& playerRef) : lvls(playerRef) {}

    int forest(QuestWindow& qw) {
        questions q("Name the main principles of OOP", 2, {
            "Indexing, abstraction, arraying",
            "Objectness, polymorphism, abstraction",
            "Polymorphism, inheritance, encapsulation",
            "Objectness, repeatability, classiness"
            });

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            applyBuff(1);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            applyDebuff(2, "pain");
            reduceTime(10);
            reduceScore(10);
            if (!isAlive()) return -1;
            return 0;
        }
    }

    int river(QuestWindow& qw) {
        questions q("What is a class in OOP?", 1, {
            "Data structure",
            "User-defined data type",
            "Template for function definition",
            "User container"
            });

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            applyDebuff(1, "water");
            reduceTime(10);
            reduceScore(10);
            if (!isAlive()) return -1;
            return 0;
        }
    }

    int canion(QuestWindow& qw) {
        questions q("What are virtual functions in OOP?", 3, {
            "1", "2", "3", "4"
            });

        if (p.get_status() > 0) {
            q.addQuest("5");
            reduceTime(5);
            reduceStatus();

            int elapsed;
            if (q.askQuestSFML(qw, elapsed)) {
                reduceTime(elapsed);
                addScore(30);
                applyBuff(1);
                if (!isAlive()) return -1;
                return 1;
            }
            else {
                reduceTime(elapsed);
                reduceScore(10);
                reduceTime(10);
                if (!isAlive()) return -1;
                return 0;
            }
        }

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            reduceScore(10);
            reduceTime(10);
            applyDebuff(1, "scary");
            if (!isAlive()) return -1;
            return 0;
        }
    }

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
            reduceTime(5);
            reduceStatus();
        }

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            applyBuff(1);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            applyDebuff(1, "pain");
            reduceTime(10);
            reduceScore(20);
            if (!isAlive()) return -1;
            return 0;
        }
    }

    int village(QuestWindow& qw) {
        if (p.get_status() > 0) {
            applyBuff(1);
        }
        return 1;
    }

    int market(QuestWindow& qw) {
        if (p.get_status() > 0) {
            reduceStatus();
            reduceTime(5);
        }

        questions q("What is encapsulation?", 2, {
            "1", "2", "3", "4"
            });

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            reduceScore(10);
            applyDebuff(1, "pain");
            if (!isAlive()) return -1;
            return 0;
        }
    }

    int barn(QuestWindow& qw) {
        questions q("OOP question", 0, {
            "1", "2", "3", "4"
            });

        if (p.get_status() > 0) {
            reduceStatus();
            reduceTime(5);
            q.addQuest("5");
        }

        int elapsed;
        if (q.askQuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            reduceScore(10);
            applyDebuff(1, "pain");
            if (!isAlive()) return -1;
            return 0;
        }
    }

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
            reduceTime(5);
            reduceStatus();

            questions q4("Complex OOP question", 0, {
                "1", "2", "3", "4"
                });

            int elapsed4;
            if (q4.askQuestSFML(qw, elapsed4)) {
                reduceTime(elapsed4);
                addScore(20);
                fl += 1;
            }
            else {
                reduceTime(elapsed4);
                db.pain();
                reduceScore(10);
            }
        }
        else {
            b.moral();
        }

        int elapsed1;
        if (q1.askQuestSFML(qw, elapsed1)) {
            reduceTime(elapsed1);
            fl += 1;
            addScore(20);
        }
        else {
            reduceTime(elapsed1);
            if (fl4 && fl == 0) {
                return 666;
            }
            reduceScore(10);
            db.pain();
        }

        int elapsed2;
        if (q2.askQuestSFML(qw, elapsed2)) {
            reduceTime(elapsed2);
            addScore(20);
            fl += 1;
        }
        else {
            reduceTime(elapsed2);
            if (fl == 1 && fl4) {
                return 666;
            }
            else if (fl == 0 && !fl4) {
                return 666;
            }
            reduceScore(10);
            db.pain();
        }

        int elapsed3;
        if (q3.askQuestSFML(qw, elapsed3)) {
            reduceTime(elapsed3);
            addScore(20);
            fl += 1;
        }
        else {
            reduceTime(elapsed3);
            if (fl4 && fl == 2) {
                questions q5("Complex OOP question", 1, {
                    "1$", "2", "3@", "4#", "5"
                    });
                int elapsed4;
                if (q5.askQuestSFML(qw, elapsed4)) {
                    reduceTime(elapsed4);
                    db.pain();
                    reduceScore(10);
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
                    reduceTime(elapsed5);
                    db.pain();
                    reduceScore(10);
                    fl = -1;
                }
                else {
                    return 666;
                }
            }
            else {
                reduceScore(10);
                db.pain();
            }
        }

        if (!isAlive()) {
            return -1;
        }

        if (fl == -1) {
            b.moral();
            addScore(50);
            return 13;
        }
        else if (fl >= 2) {
            b.moral();
            addScore(100);
            return 1;
        }
        else {
            reduceScore(50);
            return 0;
        }
    }
};

// ==================== LOC3 ====================
class loc3 : public lvls {
public:
    loc3(player& playerRef) : lvls(playerRef) {}

    int l1(QuestWindow& qw) {
        applyDebuff(2, "scarry");
        string task = "Необходимо написать код, который считает 2+2. Вы должны создать хотя бы 1 переменную (result) и вывести её с помощью cout.";
        string expected = "4";
        vector<string> req = { "result" };
        // Если статус негативный - задача усложняется
        if (p.get_status() > 0) {
            reduceTime(5);
            reduceStatus();
            task = "Напишите программу, которая выводит результат 15 * 3 - 8 / 2. Вы должны создать хотя бы 1 переменную (result) и вывести её с помощью cout.";
            expected = "41";
        }

        questions q(task, expected, req);

        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            applyBuff(1);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            reduceScore(10);
            reduceTime(10);
            applyDebuff(1, "pain");
            if (!isAlive()) return -1;
            return 0;
        }
    }
    int l21(QuestWindow& qw) {
        applyDebuff(1);
        bool fl = 0;
        string task = "Напишите функцию int square(int x), которая возвращает квадрат числа. Вызовите её для числа 5 и выведите результат";
        string expected = "25";
        vector<string> req = { "int square(int x)" };
        if (p.get_status() > 0) {
            reduceTime(5);
            reduceStatus();
            task = "Вам нужно создать вектор [5, 8, 1, 2, 4, 8, 2, 0, 10, 3], после этого написать ф-цию сортировки массива по возрастанию с использованием встроенных методов STL\n";
            task += "Использовать while или for запрещено";
            expected = "0 1 2 2 3 4 5 8 8 10";
            req = { "sort" };
            fl = 1;
        }

        questions q(task, expected, req);
        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            if (fl) {
                applyBuff(1);
            }
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            applyDebuff(1);
            reduceScore(10);
            reduceTime(10);
            if (!isAlive()) return -1;
            return 0;
        }
    }
    int l22(QuestWindow& qw) {
        applyDebuff(1);
        bool fl = 0;
        string task = "Напишите функцию int sqr(int x), которая возвращает корень числа. Вызовите её для числа 64 и выведите результат";
        string expected = "8";
        vector<string> req = { "int sqr(int x)" };
        if (p.get_status() > 0) {
            reduceTime(5);
            reduceStatus();
            task = "Вам нужно создать вектор [5, 8, 1, 2, 4, 8, 2, 0, 10, 3], после этого написать ф-цию сортировки массива по убыванию с использованием встроенных методов STL\n";
            expected = "10 8 8 5 4 3 2 2 1 0";
            req = { "sort" };
            fl = 1;
        }
        questions q(task, expected, req);
        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            if (fl) {
                applyBuff(1);
            }
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            applyDebuff(1);
            reduceScore(10);
            reduceTime(10);
            if (!isAlive()) return -1;
            return 0;
        }
    }
    int r1(QuestWindow& qw) {
        applyBuff(2);
        string task = "Необходимо написать код, который считает 3+3. Вы должны создать хотя бы 1 переменную (result) и вывести её с помощью cout.";
        string expected = "6";
        vector<string> req = { "result" };
        if (p.get_status() > 0) {
            reduceTime(5);
            reduceStatus();
            task = "Напишите программу, которая выводит результат 24 * 5 - 12 / 4. Вы должны создать хотя бы 1 переменную (result) и вывести её с помощью cout.";
            expected = "117";
        }
        questions q(task, expected, req);

        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            reduceTime(elapsed);
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            reduceTime(elapsed);
            reduceScore(10);
            reduceTime(10);
            applyDebuff(2, "pain");
            if (!isAlive()) return -1;
            return 0;
        }
    }
    int r21() {
        applyBuff(1);
        bool fl = 0;
        string task = "Напишите функцию int fac(int x), которая возвращает факториал числа. Вызовите её для числа 12 и выведите результат";
        string expected = "479001600";
        vector<string> req = { "int fac(int x)" };
        if (p.get_status() > 0) {
            reduceTime(5);
            reduceStatus();
            task = "Вам нужно создать вектор [5, 8, 1, 2, 4, 8, 2, 0, 10, 3], после этого написать ф-цию сортировки массива по возрастанию с использованием встроенных методов STL\n";
            expected = "0 1 2 2 3 4 5 8 8 10";
            req = { "sort" };
            fl = 1;
        }

        questions q(task, expected, req);
        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            if (fl) {
                applyBuff(1);
            }
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            applyDebuff(2);
            reduceScore(10);
            reduceTime(10);
            if (!isAlive()) return -1;
            return 0;
        }
    }
    int r22() {
        applyBuff(1);
        bool fl = 0;
        string task = "Напишите функцию int sqr(int x), которая возвращает корень числа. Вызовите её для числа 64 и выведите результат";
        string expected = "8";
        vector<string> req = { "int sqr(int x)" };
        if (p.get_status() > 0) {
            reduceTime(5);
            reduceStatus();
            task = "Вам нужно создать вектор [5, 8, 1, 2, 4, 8, 2, 0, 10, 3], после этого написать ф-цию сортировки массива по убыванию с использованием встроенных методов STL\n";
            expected = "10 8 8 5 4 3 2 2 1 0";
            req = { "sort" };
            fl = 1;
        }
        questions q(task, expected, req);
        int elapsed;
        if (q.ask_code_QuestSFML(qw, elapsed)) {
            if (fl) {
                applyBuff(1);
            }
            addScore(20);
            if (!isAlive()) return -1;
            return 1;
        }
        else {
            applyDebuff(2);
            reduceScore(10);
            reduceTime(10);
            if (!isAlive()) return -1;
            return 0;
        }
    }
    int boss3() {

    }
};
