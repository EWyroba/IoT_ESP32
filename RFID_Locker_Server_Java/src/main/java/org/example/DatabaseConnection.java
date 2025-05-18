package org.example;

import java.sql.*;

public class DatabaseConnection {

    private static final String DB_NAME = "rfid_locker";
    private static final String BASE_URL = "jdbc:mysql://localhost:3306/";
    private static final String USER = "root";
    private static final String PASSWORD = "";

    private static Connection connection = null;

    // Metoda inicjalizująca bazę przy starcie programu
    public static void initialize() {
        try {
            // Krok 1: Połączenie bez konkretnej bazy
            try (Connection conn = DriverManager.getConnection(BASE_URL, USER, PASSWORD);
                 Statement stmt = conn.createStatement()) {

                // Krok 2: Sprawdzenie czy baza istnieje
                ResultSet rs = stmt.executeQuery("SHOW DATABASES LIKE '" + DB_NAME + "'");
                if (!rs.next()) {
                    stmt.executeUpdate("CREATE DATABASE " + DB_NAME);
                    System.out.println("Utworzono bazę danych: " + DB_NAME);
                }
            }

            // Krok 3: Połączenie z konkretną bazą
            connection = DriverManager.getConnection(BASE_URL + DB_NAME, USER, PASSWORD);

            // Krok 4: Sprawdzenie tabel
            checkAndCreateTables(connection);

        } catch (SQLException e) {
            e.printStackTrace();
            System.err.println("Nie udało się zainicjalizować połączenia z bazą danych.");
        }
    }

    private static void checkAndCreateTables(Connection conn) throws SQLException {
        Statement stmt = conn.createStatement();

        // Tabela users
        ResultSet rs = conn.getMetaData().getTables(null, null, "user_data", new String[] {"TABLE"});
        if (!rs.next()) {
            stmt.executeUpdate("""
                CREATE TABLE user_data (
                    user_id VARCHAR(64) PRIMARY KEY,
                    name VARCHAR(50),
                    user_type VARCHAR(50)
                )
            """);
            System.out.println("Utworzono tabelę 'user_data'.");
        }

        rs = conn.getMetaData().getTables(null, null, "lock_data", new String[] {"TABLE"});
        if (!rs.next()) {
            stmt.executeUpdate("""
                CREATE TABLE lock_data (
                    lock_id varchar(5) PRIMARY KEY,
                    room_desc varchar(50),
                    floor int(2)
                )
            """);
            System.out.println("Utworzono tabelę 'lock_data'.");
        }

        // Tabela access_logs
        rs = conn.getMetaData().getTables(null, null, "access_logs", new String[] {"TABLE"});
        if (!rs.next()) {
            stmt.executeUpdate("""
                CREATE TABLE access_logs (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
                    user_id VARCHAR(64),
                    lock_id varchar(5),
                    card_ok BOOLEAN,
                    access_granted BOOLEAN,
                    FOREIGN KEY (lock_id) REFERENCES lock_data(lock_id)
                )
            """);
            System.out.println("Utworzono tabelę 'access_logs'.");
        }

        // Tabela access_logs
        rs = conn.getMetaData().getTables(null, null, "privileges", new String[] {"TABLE"});
        if (!rs.next()) {
            stmt.executeUpdate("""
                CREATE TABLE privileges (
                    id INT AUTO_INCREMENT PRIMARY KEY,
                    user_id VARCHAR(64),
                    lock_id varchar(5),
                    FOREIGN KEY (user_id) REFERENCES user_data(user_id),
                    FOREIGN KEY (lock_id) REFERENCES lock_data(lock_id)
                )
            """);
            System.out.println("Utworzono tabelę 'privileges'.");
        }


    }

    // Zwraca aktywne połączenie
    public static Connection getConnection() {
        try {
            connection = DriverManager.getConnection(BASE_URL + DB_NAME, USER, PASSWORD);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return connection;
    }
}
