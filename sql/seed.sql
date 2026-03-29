-- Тестовые данные (стоимость в копейках/усл. ед.)
INSERT INTO AIR_CONFIG (id, REGULAR_FLIGHT_PCT, SPECIAL_FLIGHT_PCT) VALUES (1, 5, 10);

INSERT INTO AIR_HELICOPTERS (number, mark, date_creation, lifting_capacity, date_last_fix, flight_resource)
VALUES
 (101, 'Mi-8', '2015-04-01', 4000, '2024-01-15', 500),
 (102, 'Mi-26', '2018-09-10', 20000, '2023-06-01', 300);

INSERT INTO AIR_CREW (number, surname, grade, experience, address, date_of_birth, AIR_HELICOPTERS_number)
VALUES
 (1001, 'Иванов', 'Командир', 12, 'г. Алматы, ул. Летная 1', '1985-03-20', 101),
 (1002, 'Петров', 'Второй пилот', 8, 'г. Алматы, ул. Летная 2', '1990-11-02', 101),
 (1003, 'Сидоров', 'Бортмеханик', 10, 'г. Алматы, ул. Летная 3', '1988-07-15', 101),
 (2001, 'Козлов', 'Командир', 15, 'г. Алматы, пр. Воздушный 5', '1982-01-30', 102),
 (2002, 'Новиков', 'Второй пилот', 6, 'г. Алматы, пр. Воздушный 6', '1993-05-22', 102),
 (2003, 'Волков', 'Бортмеханик', 9, 'г. Алматы, пр. Воздушный 7', '1989-09-09', 102);

INSERT INTO AIR_FLIGHTS (flight_code, date, cargo_weight, amount_people, flight_duration, flight_cost, AIR_HELICOPTERS_number, is_special)
VALUES
 (5001, '2025-02-01', 1200, 8, 3, 100000, 101, 0),
 (5002, '2025-02-10', 800, 4, 2, 80000, 101, 1),
 (5003, '2025-03-01', 5000, 12, 5, 250000, 102, 0),
 (5004, '2025-03-15', 3000, 6, 4, 180000, 102, 1);

-- Командир: без привязки к экипажу; летчик 1001
INSERT INTO AIR_USERS (login, password, AIR_CREW_number) VALUES
 ('commander', 'cmd_secret', NULL),
 ('pilot_ivanov', 'pilot1', 1001);
