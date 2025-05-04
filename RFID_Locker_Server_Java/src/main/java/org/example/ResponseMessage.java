package org.example;

class ResponseMessage {
    int message_id;
    boolean access_granted;

    public ResponseMessage(int message_id, boolean access_granted) {
        this.message_id = message_id;
        this.access_granted = access_granted;
    }
}
