-- Расширения схемы: выплаты за период, медиа, триггер ресурса

CREATE TABLE IF NOT EXISTS AIR_PAYROLL_PERIOD (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    period_start TEXT NOT NULL,
    period_end TEXT NOT NULL,
    computed_at TEXT NOT NULL,
    crew_number INTEGER NOT NULL,
    amount_cents INTEGER NOT NULL,
    FOREIGN KEY (crew_number) REFERENCES AIR_CREW(number)
);

CREATE TABLE IF NOT EXISTS AIR_IMAGES (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ref_table TEXT NOT NULL,
    ref_id INTEGER NOT NULL,
    format TEXT NOT NULL,
    image_data BLOB NOT NULL
);

CREATE TRIGGER IF NOT EXISTS trg_air_flights_resource_check
BEFORE INSERT ON AIR_FLIGHTS
FOR EACH ROW
WHEN NEW.flight_duration > 0
BEGIN
    SELECT CASE
        WHEN (
            COALESCE((
                SELECT SUM(flight_duration) FROM AIR_FLIGHTS
                WHERE AIR_HELICOPTERS_number = NEW.AIR_HELICOPTERS_number
                  AND date >= (SELECT date_last_fix FROM AIR_HELICOPTERS WHERE number = NEW.AIR_HELICOPTERS_number)
            ), 0) + NEW.flight_duration
        ) > (SELECT flight_resource FROM AIR_HELICOPTERS WHERE number = NEW.AIR_HELICOPTERS_number)
        THEN RAISE(ABORT, 'Превышен ресурс летного времени до следующего капремонта')
    END;
END;
