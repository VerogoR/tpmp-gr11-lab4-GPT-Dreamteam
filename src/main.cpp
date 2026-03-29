#include "auth.h"
#include "db.h"
#include "mutations.h"
#include "payroll.h"
#include "queries.h"
#include "storage.h"

#include <sqlite3.h>

#include <cstdlib>
#include <iostream>
#include <string>

static std::string read_line(const std::string& prompt) {
    std::cout << prompt;
    std::string s;
    if (!std::getline(std::cin, s))
        return {};
    return s;
}

static int read_int(const std::string& prompt, int def = 0) {
    std::string s = read_line(prompt);
    if (s.empty())
        return def;
    try {
        return std::stoi(s);
    } catch (...) {
        return def;
    }
}

static void print_payroll_table(sqlite3* db) {
    const char* q = "SELECT id, period_start, period_end, computed_at, crew_number, amount_cents FROM AIR_PAYROLL_PERIOD ORDER BY id DESC LIMIT 20";
    sqlite3_stmt* st = nullptr;
    if (sqlite3_prepare_v2(db, q, -1, &st, nullptr) != SQLITE_OK)
        return;
    while (sqlite3_step(st) == SQLITE_ROW) {
        std::cout << sqlite3_column_int(st, 0) << " | "
                  << sqlite3_column_text(st, 1) << " .. " << sqlite3_column_text(st, 2) << " | "
                  << "crew " << sqlite3_column_int(st, 4) << " | "
                  << sqlite3_column_int64(st, 5) << "\n";
    }
    sqlite3_finalize(st);
}

static int run_menu(sqlite3* db, heli::Session& session) {
    for (;;) {
        std::cout << "\n--- Меню ---\n";
        std::cout << "1. Налёт после капремонта и ресурс (*)\n";
        std::cout << "2. Рейсы за период (*)\n";
        std::cout << "3. Спецрейсы: сводка\n";
        std::cout << "4. Обычные рейсы: сводка\n";
        std::cout << "5. Вертолёт с макс. числом рейсов — экипаж и деньги\n";
        std::cout << "6. Экипаж с макс. начислениями — все рейсы\n";
        std::cout << "7. Мои рейсы (экипаж *) / по вертолёту (командир)\n";
        std::cout << "8. Начисления за период (все) — таблица AIR_PAYROLL_PERIOD\n";
        std::cout << "9. Начисления летчику за период (*)\n";
        std::cout << "10. Начисления летчику за период: только спец / только обычные (*)\n";
        std::cout << "11. Экспорт: CSV рейсов, JSON вертолётов\n";
        std::cout << "12. Добавить тестовое изображение (BLOB) для вертолёта\n";
        if (session.role == heli::Role::Commander) {
            std::cout << "13. CRUD вертолёт (командир)\n";
            std::cout << "14. CRUD экипаж (командир)\n";
            std::cout << "15. CRUD рейс\n";
            std::cout << "16. Добавить пользователя\n";
        }
        std::cout << "0. Выход\n";

        int c = read_int("> ", -1);
        std::string err;

        if (c == 0)
            return 0;
        if (c == 1) {
            std::cout << heli::q_hours_and_resource(db, session, err) << (err.empty() ? "" : err + "\n");
        } else if (c == 2) {
            std::string d1 = read_line("дата с (YYYY-MM-DD): ");
            std::string d2 = read_line("дата по: ");
            std::cout << heli::q_flights_in_period(db, session, d1, d2, err) << (err.empty() ? "" : err + "\n");
        } else if (c == 3) {
            if (session.role != heli::Role::Commander) {
                std::cout << "только командир\n";
                continue;
            }
            std::cout << heli::q_special_totals(db, err) << (err.empty() ? "" : err + "\n");
        } else if (c == 4) {
            if (session.role != heli::Role::Commander) {
                std::cout << "только командир\n";
                continue;
            }
            std::cout << heli::q_regular_totals(db, err) << (err.empty() ? "" : err + "\n");
        } else if (c == 5) {
            if (session.role != heli::Role::Commander) {
                std::cout << "только командир\n";
                continue;
            }
            std::cout << heli::q_heli_max_flights_crew(db, err) << (err.empty() ? "" : err + "\n");
        } else if (c == 6) {
            if (session.role != heli::Role::Commander) {
                std::cout << "только командир\n";
                continue;
            }
            std::cout << heli::q_crew_max_money_flights(db, err) << (err.empty() ? "" : err + "\n");
        } else if (c == 7) {
            if (session.role == heli::Role::Commander) {
                int hn = read_int("номер вертолёта: ");
                std::cout << heli::q_crew_flights_by_helicopter(db, hn, err) << (err.empty() ? "" : err + "\n");
            } else {
                std::cout << heli::q_crew_flights(db, session, err) << (err.empty() ? "" : err + "\n");
            }
        } else if (c == 8) {
            if (session.role != heli::Role::Commander) {
                std::cout << "только командир\n";
                continue;
            }
            std::string d1 = read_line("период с: ");
            std::string d2 = read_line("период по: ");
            if (!heli::payroll_compute_squad_period(db, d1, d2, err))
                std::cout << err << "\n";
            else {
                std::cout << "записано в AIR_PAYROLL_PERIOD:\n";
                print_payroll_table(db);
            }
        } else if (c == 9) {
            int cn = session.role == heli::Role::Commander ? read_int("табельный номер летчика: ") : session.crew_number;
            std::string d1 = read_line("период с: ");
            std::string d2 = read_line("период по: ");
            long long amt = 0;
            if (heli::payroll_pilot_period_amount(db, cn, d1, d2, amt, err))
                std::cout << "начислено: " << amt << "\n";
            else
                std::cout << err << "\n";
        } else if (c == 10) {
            int cn = session.role == heli::Role::Commander ? read_int("табельный номер летчика: ") : session.crew_number;
            std::string d1 = read_line("период с: ");
            std::string d2 = read_line("период по: ");
            int t = read_int("1=спец 0=обычные: ");
            long long amt = 0;
            if (heli::payroll_pilot_period_by_type(db, cn, d1, d2, t != 0, amt, err))
                std::cout << "начислено: " << amt << "\n";
            else
                std::cout << err << "\n";
        } else if (c == 11) {
            if (session.role != heli::Role::Commander) {
                std::cout << "только командир\n";
                continue;
            }
            std::string p1 = read_line("путь CSV: ");
            std::string p2 = read_line("путь JSON: ");
            if (heli::storage_export_flights_csv(db, p1, err) && heli::storage_export_helicopters_json(db, p2, err))
                std::cout << "ok\n";
            else
                std::cout << err << "\n";
        } else if (c == 12) {
            int hn = read_int("номер вертолёта: ");
            unsigned char png1x1[] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};
            std::vector<unsigned char> blob(png1x1, png1x1 + sizeof png1x1);
            if (heli::storage_save_image(db, "AIR_HELICOPTERS", hn, "png", blob, err))
                std::cout << "изображение сохранено в AIR_IMAGES\n";
            else
                std::cout << err << "\n";
        } else if (c == 13 && session.role == heli::Role::Commander) {
            int op = read_int("1 add 2 upd 3 del: ");
            int num = read_int("number: ");
            if (op == 1) {
                std::string mark = read_line("mark: ");
                std::string dc = read_line("date_creation: ");
                int lift = read_int("lifting: ");
                std::string dlf = read_line("date_last_fix: ");
                int res = read_int("flight_resource: ");
                heli::mut_insert_helicopter(db, session, num, mark, dc, lift, dlf, res, err);
            } else if (op == 2) {
                std::string mark = read_line("mark: ");
                std::string dc = read_line("date_creation: ");
                int lift = read_int("lifting: ");
                std::string dlf = read_line("date_last_fix: ");
                int res = read_int("flight_resource: ");
                heli::mut_update_helicopter(db, session, num, mark, dc, lift, dlf, res, err);
            } else if (op == 3) {
                heli::mut_delete_helicopter(db, session, num, err);
            }
            std::cout << (err.empty() ? "ok\n" : err + "\n");
        } else if (c == 14 && session.role == heli::Role::Commander) {
            int op = read_int("1 add 2 upd 3 del: ");
            int num = read_int("таб. номер: ");
            if (op == 1) {
                std::string sn = read_line("фамилия: ");
                std::string gr = read_line("должность: ");
                int ex = read_int("стаж: ");
                std::string ad = read_line("адрес: ");
                std::string dob = read_line("год рожд / дата: ");
                int hn = read_int("вертолёт: ");
                heli::mut_insert_crew(db, session, num, sn, gr, ex, ad, dob, hn, err);
            } else if (op == 2) {
                std::string sn = read_line("фамилия: ");
                std::string gr = read_line("должность: ");
                int ex = read_int("стаж: ");
                std::string ad = read_line("адрес: ");
                std::string dob = read_line("дата рожд: ");
                int hn = read_int("вертолёт: ");
                heli::mut_update_crew(db, session, num, sn, gr, ex, ad, dob, hn, err);
            } else if (op == 3) {
                heli::mut_delete_crew(db, session, num, err);
            }
            std::cout << (err.empty() ? "ok\n" : err + "\n");
        } else if (c == 15) {
            int op = read_int("1 add 2 upd 3 del: ");
            int fc = read_int("код рейса: ");
            if (op == 1) {
                std::string dt = read_line("дата: ");
                int cg = read_int("масса: ");
                int pe = read_int("люди: ");
                int du = read_int("длительность: ");
                int co = read_int("стоимость: ");
                int hn = read_int("вертолёт: ");
                int sp = read_int("спец 1/0: ");
                heli::mut_insert_flight(db, session, fc, dt, cg, pe, du, co, hn, sp != 0, err);
            } else if (op == 2) {
                std::string dt = read_line("дата: ");
                int cg = read_int("масса: ");
                int pe = read_int("люди: ");
                int du = read_int("длительность: ");
                int co = read_int("стоимость: ");
                int hn = read_int("вертолёт: ");
                int sp = read_int("спец 1/0: ");
                heli::mut_update_flight(db, session, fc, dt, cg, pe, du, co, hn, sp != 0, err);
            } else if (op == 3) {
                heli::mut_delete_flight(db, session, fc, err);
            }
            std::cout << (err.empty() ? "ok\n" : err + "\n");
        } else if (c == 16 && session.role == heli::Role::Commander) {
            std::string lg = read_line("login: ");
            std::string pw = read_line("password: ");
            int cr = read_int("crew number или -1 для командира: ");
            heli::mut_insert_user(db, session, lg, pw, cr, err);
            std::cout << (err.empty() ? "ok\n" : err + "\n");
        } else {
            std::cout << "неизвестная команда\n";
        }
    }
}

int main(int argc, char** argv) {
    if (argc > 1) {
        std::string a = argv[1];
        if (a == "--help" || a == "-h") {
            std::cout << "helicopter_app [db_path]\n  переменные: HELI_SQL_DIR, HELI_SEED=1\n";
            return 0;
        }
    }

    std::string dbpath = "build/helicopter.db";
    if (argc > 1)
        dbpath = argv[1];

    std::string err;
    std::string sqldir = heli::resolve_sql_dir();
    sqlite3* db = heli::db_open_or_create(dbpath, sqldir, err);
    if (!db) {
        std::cerr << err << "\n";
        return 1;
    }

    std::cout << "Вход (commander / pilot_ivanov при HELI_SEED=1)\n";
    std::string login = read_line("логин: ");
    std::string pass = read_line("пароль: ");
    heli::Session session{};
    if (!heli::auth_login(db, login, pass, session, err)) {
        std::cerr << err << "\n";
        heli::db_close(db);
        return 2;
    }
    std::cout << "Добро пожаловать, " << session.login
              << (session.role == heli::Role::Commander ? " (командир)\n" : " (экипаж)\n");

    int r = run_menu(db, session);
    heli::db_close(db);
    return r;
}
