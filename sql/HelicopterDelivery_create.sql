-- Created by Redgate Data Modeler (https://datamodeler.redgate-platform.com)
-- Last modification date: 2026-03-28 07:27:37.946

-- tables
-- Table: AIR_CONFIG
CREATE TABLE AIR_CONFIG (
    id integer NOT NULL CONSTRAINT AIR_CONFIG_pk PRIMARY KEY,
    REGULAR_FLIGHT_PCT integer NOT NULL,
    SPECIAL_FLIGHT_PCT integer NOT NULL
);

-- Table: AIR_CREW
CREATE TABLE AIR_CREW (
    number integer NOT NULL CONSTRAINT AIR_CREW_pk PRIMARY KEY,
    surname text NOT NULL,
    grade text NOT NULL,
    experience integer NOT NULL,
    address text NOT NULL,
    date_of_birth text NOT NULL,
    AIR_HELICOPTERS_number integer NOT NULL,
    CONSTRAINT AIR_CREW_AIR_HELICOPTERS FOREIGN KEY (AIR_HELICOPTERS_number)
    REFERENCES AIR_HELICOPTERS (number)
);

-- Table: AIR_FLIGHTS
CREATE TABLE AIR_FLIGHTS (
    flight_code integer NOT NULL CONSTRAINT AIR_FLIGHTS_pk PRIMARY KEY,
    date text NOT NULL,
    cargo_weight integer NOT NULL,
    amount_people integer NOT NULL,
    flight_duration integer NOT NULL,
    flight_cost integer NOT NULL,
    AIR_HELICOPTERS_number integer NOT NULL,
    is_special boolean NOT NULL,
    CONSTRAINT AIR_FLIGHTS_AIR_HELICOPTERS FOREIGN KEY (AIR_HELICOPTERS_number)
    REFERENCES AIR_HELICOPTERS (number)
);

-- Table: AIR_HELICOPTERS
CREATE TABLE AIR_HELICOPTERS (
    number integer NOT NULL CONSTRAINT AIR_HELICOPTERS_pk PRIMARY KEY,
    mark text NOT NULL,
    date_creation text NOT NULL,
    lifting_capacity integer NOT NULL,
    date_last_fix text NOT NULL,
    flight_resource integer NOT NULL
);

-- Table: AIR_USERS
CREATE TABLE AIR_USERS (
    login text NOT NULL CONSTRAINT AIR_USERS_pk PRIMARY KEY,
    password text NOT NULL,
    AIR_CREW_number integer,
    CONSTRAINT AIR_USERS_AIR_CREW FOREIGN KEY (AIR_CREW_number)
    REFERENCES AIR_CREW (number)
);

-- End of file.
