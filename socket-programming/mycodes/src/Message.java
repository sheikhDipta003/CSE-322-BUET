public class Message {
    int from;
    int to;
    String message;
    boolean isRead;

    public Message(int from, int to, String message) {
        this.from = from;
        this.to = to;
        this.message = message;
        this.isRead = false;
    }
}