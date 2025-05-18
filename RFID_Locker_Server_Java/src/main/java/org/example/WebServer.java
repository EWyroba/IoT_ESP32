package org.example;

import com.google.gson.Gson;
import fi.iki.elonen.NanoHTTPD;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.sql.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;


import org.json.JSONArray;



public class WebServer extends NanoHTTPD {

    public WebServer() throws Exception {
        super(8080);
        start(SOCKET_READ_TIMEOUT, false);
        System.out.println("Web frontend działa na http://localhost:8080");
    }

    @Override
    public Response serve(IHTTPSession session) {
        Method method = session.getMethod();
        String uri = session.getUri();
        Gson gson = new Gson();
        try {
            if (uri.endsWith(".js") || uri.endsWith(".css") || uri.endsWith(".html")) {
                String mime = uri.endsWith(".js") ? "application/javascript" :
                        uri.endsWith(".css") ? "text/css" : "text/html";

                InputStream stream = getClass().getResourceAsStream("/public" + uri);
                if (stream != null) {
                    return newFixedLengthResponse(Response.Status.OK, mime, stream, stream.available());
                } else {
                    return newFixedLengthResponse(Response.Status.NOT_FOUND, MIME_PLAINTEXT, "404 Not Found");
                }
            }

            switch (uri) {
                case "/api/users":
                    return jsonResponse(gson.toJson(getUsers()));
                case "/api/locks":
                    return jsonResponse(gson.toJson(getLocks()));
                case "/api/logs":
                    JSONArray array = new JSONArray();
                    try {
                        Connection conn = DatabaseConnection.getConnection();
                        String sql = """
                            SELECT a.timestamp, a.user_id, u.name, a.lock_id, a.card_ok, a.access_granted
                            FROM access_logs a
                            LEFT JOIN user_data u ON a.user_id = u.user_id
                            ORDER BY a.timestamp DESC
                        """;
                        Statement stmt = conn.createStatement();
                        ResultSet rs = stmt.executeQuery(sql);
                        while (rs.next()) {
                            JSONArray row = new JSONArray();
                            row.put(rs.getString("timestamp"));
                            row.put(rs.getString("user_id"));
                            row.put(rs.getString("name") != null ? rs.getString("name") : "-");
                            row.put(rs.getString("lock_id"));
                            row.put(rs.getBoolean("card_ok") ? "TAK" : "NIE");
                            row.put(rs.getBoolean("access_granted") ? "TAK" : "NIE");
                            array.put(row);
                        }
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                    return jsonResponse(array.toString());
                case "/api/privileges":
                    JSONArray privileges = new JSONArray();
                    try (Connection conn = DatabaseConnection.getConnection();
                         Statement stmt = conn.createStatement();
                         ResultSet rs = stmt.executeQuery("""
                            SELECT p.user_id, COALESCE(u.name, '-') AS user_name,
                                   p.lock_id, COALESCE(l.room_desc, '-') AS room_name
                            FROM privileges p
                            LEFT JOIN user_data u ON p.user_id = u.user_id
                            LEFT JOIN lock_data l ON p.lock_id = l.lock_id
                         """)) {
                        while (rs.next()) {
                            JSONArray row = new JSONArray();
                            row.put(rs.getString("user_id"));
                            row.put(rs.getString("user_name"));   // Imię i nazwisko
                            row.put(rs.getString("lock_id"));
                            row.put(rs.getString("room_name"));   // Opis pokoju
                            privileges.put(row);
                        }
                    }
                    return jsonResponse(privileges.toString());
                case "/api/addUser":
                    if(Method.POST.equals(method)) {
                        Map<String, String> bodyParams = new HashMap<>();
                        session.parseBody(bodyParams);
                        Map<String, String> parsed = session.getParms(); // ← działa, jeśli dane są przesłane jako application/x-www-form-urlencoded


                        String userId = parsed.get("user_id");
                        String name = parsed.get("name");
                        String userType = parsed.get("user_type");

                        try (Connection conn = DatabaseConnection.getConnection()) {
                            PreparedStatement stmt = conn.prepareStatement("INSERT INTO user_data (user_id, name, user_type) VALUES (?, ?, ?)");
                            stmt.setString(1, userId);
                            stmt.setString(2, name);
                            stmt.setString(3, userType);
                            stmt.executeUpdate();
                            return newFixedLengthResponse("OK");
                        } catch (SQLException e) {
                            e.printStackTrace();
                            return newFixedLengthResponse(Response.Status.INTERNAL_ERROR, MIME_PLAINTEXT, "Błąd bazy danych");
                        }
                    }
                case "/api/addRoom":
                    if (Method.POST.equals(session.getMethod())) {
                        Map<String, String> bodyParams = new HashMap<>();
                        try {
                            session.parseBody(bodyParams);
                        } catch (IOException | ResponseException e) {
                            e.printStackTrace();
                            return newFixedLengthResponse(Response.Status.INTERNAL_ERROR, MIME_PLAINTEXT, "Błąd parsowania");
                        }

                        Map<String, String> parsed = session.getParms(); // ← działa, jeśli dane są przesłane jako application/x-www-form-urlencoded


                        String lockId = parsed.get("lock_id");
                        String desc = parsed.get("desc");
                        String floorStr = parsed.get("floor");

                        try (Connection conn = DatabaseConnection.getConnection()) {
                            PreparedStatement stmt = conn.prepareStatement("INSERT INTO lock_data (lock_id, room_desc, floor) VALUES (?, ?, ?)");
                            stmt.setString(1, lockId);
                            stmt.setString(2, desc);
                            stmt.setInt(3, Integer.parseInt(floorStr));
                            stmt.executeUpdate();
                            return newFixedLengthResponse("OK");
                        } catch (SQLException e) {
                            e.printStackTrace();
                            return newFixedLengthResponse(Response.Status.INTERNAL_ERROR, MIME_PLAINTEXT, "Błąd bazy danych");
                        }
                    } else {
                        return newFixedLengthResponse(Response.Status.METHOD_NOT_ALLOWED, MIME_PLAINTEXT, "Tylko POST");
                    }
                case "/api/addPrivilege":
                    if (Method.POST.equals(session.getMethod())) {
                        Map<String, String> bodyParams = new HashMap<>();
                        try {
                            session.parseBody(bodyParams);
                        } catch (IOException | ResponseException e) {
                            e.printStackTrace();
                            return newFixedLengthResponse(Response.Status.INTERNAL_ERROR, MIME_PLAINTEXT, "Błąd parsowania");
                        }

                        Map<String, String> parsed = session.getParms(); // ← działa, jeśli dane są przesłane jako application/x-www-form-urlencoded

                        String userId = parsed.get("user_id");
                        String lockId = parsed.get("lock_id");

                        try (Connection conn = DatabaseConnection.getConnection()) {
                            PreparedStatement stmt = conn.prepareStatement("INSERT INTO privileges (user_id, lock_id) VALUES (?, ?)");
                            stmt.setString(1, userId);
                            stmt.setString(2, lockId);
                            stmt.executeUpdate();
                            return newFixedLengthResponse("OK");
                        } catch (SQLException e) {
                            e.printStackTrace();
                            return newFixedLengthResponse(Response.Status.INTERNAL_ERROR, MIME_PLAINTEXT, "Błąd bazy danych");
                        }
                    } else {
                        return newFixedLengthResponse(Response.Status.METHOD_NOT_ALLOWED, MIME_PLAINTEXT, "Tylko POST");
                    }

                case "/":
                default:
                    return newFixedLengthResponse(Response.Status.OK, "text/html", loadHtml());
            }
        } catch (Exception e) {
            e.printStackTrace();
            return newFixedLengthResponse(Response.Status.INTERNAL_ERROR, "text/plain", "Błąd serwera");
        }
    }

    private Response jsonResponse(String json) {
        return newFixedLengthResponse(Response.Status.OK, "application/json", json);
    }

    private List<String[]> getUsers() throws SQLException {
        Connection conn = DatabaseConnection.getConnection();
        ResultSet rs = conn.createStatement().executeQuery("SELECT * FROM user_data");
        List<String[]> users = new ArrayList<>();
        while (rs.next()) {
            users.add(new String[]{
                    rs.getString("user_id"),
                    rs.getString("name"),
                    rs.getString("user_type")
            });
        }
        return users;
    }

    private List<String[]> getLocks() throws SQLException {
        Connection conn = DatabaseConnection.getConnection();
        ResultSet rs = conn.createStatement().executeQuery("SELECT * FROM lock_data");
        List<String[]> locks = new ArrayList<>();
        while (rs.next()) {
            locks.add(new String[]{
                    rs.getString("lock_id"),
                    rs.getString("room_desc"),
                    String.valueOf(rs.getInt("floor"))
            });
        }
        return locks;
    }

    private List<String[]> getLogs() throws SQLException {
        Connection conn = DatabaseConnection.getConnection();
        ResultSet rs = conn.createStatement().executeQuery("SELECT * FROM access_logs ORDER BY timestamp DESC LIMIT 100");
        List<String[]> logs = new ArrayList<>();
        while (rs.next()) {
            logs.add(new String[]{
                    rs.getTimestamp("timestamp").toString(),
                    rs.getString("user_id"),
                    rs.getString("lock_id"),
                    rs.getBoolean("card_ok") ? "TAK" : "NIE",
                    rs.getBoolean("access_granted") ? "TAK" : "NIE"
            });
        }
        return logs;
    }

    private String loadHtml() {
        try (InputStream is = getClass().getClassLoader().getResourceAsStream("public/panel.html")) {
            if (is == null) return "Nie znaleziono pliku panel.html";
            return new String(is.readAllBytes(), StandardCharsets.UTF_8);
        } catch (IOException e) {
            e.printStackTrace();
            return "Błąd ładowania panel.html";
        }
    }
}
