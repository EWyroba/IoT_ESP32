package org.example;
import com.google.gson.Gson;
import org.eclipse.paho.client.mqttv3.*;

import javax.xml.crypto.Data;
import java.sql.*;
import java.time.Instant;

public class RfidLocker {

    public static void main(String[] args) {
        String broker = "tcp://test.mosquitto.org"; // możesz użyć swojego brokera
        String topic = "rfidlocksystem_WM";                    // zamień na swój temat
        String clientId = "RFID_Locker_Server";
        DatabaseConnection.initialize();
        try {
            new WebServer();
        } catch (Exception e) {
            e.printStackTrace();
        }


        try {
            MqttClient client = new MqttClient(broker, clientId, null);

            MqttConnectOptions options = new MqttConnectOptions();
            options.setCleanSession(true);

            client.setCallback(new MqttCallback() {
                @Override
                public void connectionLost(Throwable cause) {
                    System.out.println("Połączenie utracone: " + cause.getMessage());
                }

                @Override
                public void messageArrived(String topic, MqttMessage message) throws Exception {
                    System.out.println("Odebrano wiadomość:");
                    String payload = new String(message.getPayload());
                    System.out.println(payload);
                    Gson gson = new Gson();
                    CardMessage card = gson.fromJson(payload, CardMessage.class);
                    if(card.card_ok) {
                        checkUserPrivilege(card.lock_id, card.hashUID, card.message_id, client);
                    } else {
                        insertEntry(card.lock_id, card.hashUID, false, false);
                        System.out.println("Karta nie jest sformatowana!");
                    }
                }

                @Override
                public void deliveryComplete(IMqttDeliveryToken token) {
                    // nie używane — tylko przy publish
                }
            });

            client.connect(options);
            client.subscribe(topic);
            System.out.println("Subskrybowano temat: " + topic);
            System.out.println("Czekam na wiadomości...");

        } catch (MqttException e) {
            e.printStackTrace();
        }
    }

    public static boolean checkUserPrivilege(String lock, String UID, int message_id, MqttClient client) {
        try {
            Connection conn = DatabaseConnection.getConnection();
            String query = "SELECT id FROM privileges WHERE user_id LIKE '" + UID + "' AND lock_id LIKE'" + lock + "'";
            PreparedStatement ps = conn.prepareStatement(query);
            ResultSet rs = ps.executeQuery();
            ResponseMessage response;
            if(rs.next()) {
                System.out.println("Drzwi otwarte");
                insertEntry(lock, UID, true, true);
                response = new ResponseMessage(message_id, true);
            } else {
                System.out.println("Zła karta!");
                insertEntry(lock, UID, true, false);
                response = new ResponseMessage(message_id, false);
            }

            String jsonResponse = new Gson().toJson(response);

            MqttMessage responseMsg = new MqttMessage(jsonResponse.getBytes());
            System.out.println(responseMsg);
            responseMsg.setQos(1);

            // Możesz też dodać "_response" do tematu
            client.publish("rfidlocksystem_WM/response", responseMsg);
        } catch (Exception e) {
            e.printStackTrace();
        }
        return false;
    }

    public static void insertEntry(String lock, String UID, boolean cardOk, boolean accessGranted) {
        try {
            Connection conn = DatabaseConnection.getConnection();
            String insertQuery = "INSERT INTO access_logs (timestamp, user_id, lock_id, card_ok, access_granted) VALUES (?, ?, ?, ?, ?)";
            PreparedStatement ps = conn.prepareStatement(insertQuery);
            ps.setTimestamp(1, Timestamp.from(Instant.now()));
            ps.setString(2, UID);
            ps.setString(3, lock);
            ps.setBoolean(4, cardOk);
            ps.setBoolean(5, accessGranted);
            ps.executeUpdate();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
