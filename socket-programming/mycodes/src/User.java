import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.io.IOException;
import java.util.Vector;

public class User {

    int uid;
    int clientSocketID;
    DataInputStream dis;
    DataOutputStream dos;
    DataInputStream disFile;
    DataOutputStream dosFile;
    boolean isOnline;
    int fileCount;
    static Vector<User> users = new Vector<>();
    Vector<Message> userMessages;

    public User(int clientSocketID,DataInputStream dis, DataOutputStream dos,DataInputStream disFile, DataOutputStream dosFile) {
        this.clientSocketID = clientSocketID;
        this.dis = dis;
        this.dos = dos;
        this.disFile = disFile;
        this.dosFile = dosFile;
        this.isOnline = true;
        this.fileCount = 0;
        this.userMessages = new Vector<Message>();
    }

    public void getCredentials() throws IOException {
        dos.writeUTF("ID: ");
        dos.flush();
    }

    public static String getUsers()
    {
        StringBuilder toPrint = new StringBuilder("List of all users : \n");

        for(User u:users)
        {
            toPrint.append(u.uid).append(" - ").append(u.getStatus()).append("\n");
        }

        return toPrint.toString();
    }

    public static User find(int id)
    {
        for(User u:users)
        {
            if(u.uid == id)
                return u;
        }

        return null;
    }

    public static void remove(int id)
    {
        users.removeIf(u -> u.uid == id);
    }

    public String getStatus() {
        if(isOnline) return "Online";
        else return "Offline";
    }

    public static String[] lookupFiles(int uID, String type){
        File dir = null;
        if(type.equalsIgnoreCase("private")){
            dir = new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                    uID+"/private");
        }
        else if(type.equalsIgnoreCase("public")){
            dir = new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                    uID+"/public");
        }
        assert dir != null;
        return dir.list();
    }

    public void setId(int id) throws IOException {
        this.uid = id;
        System.out.println("Client ID : " + id);

        dos.writeUTF("Login successful");
        dos.flush();

        new File("files/"+ id +"/public").mkdirs();
        new File("files/"+ id +"/private").mkdirs();
    }

    public void writeToStream(String text, char ch) throws IOException {
        if(ch=='c') {
            dos.writeUTF(text);
            dos.flush();
        }
        else if(ch=='f'){
            dosFile.writeUTF(text);
            dosFile.flush();
        }
    }

    public static void sendMessage(int from,int to,String msg){
        User toUser = find(to);
        assert toUser != null;
        System.out.println(toUser.uid);
        toUser.addMessageToQueue(new Message(from,to,msg));
    }

    public void addMessageToQueue(Message m) {
        userMessages.add(m);
    }

    public Vector<String> showUnreadMessage()
    {
        System.out.println("Unread messages");
        Vector<String>stringVector = new Vector<String>();
        for(Message m:userMessages)
        {
            if(!m.isRead)
            {
                stringVector.add("From "+m.from+" : "+m.message);
                m.isRead=true;
            }
        }
        return stringVector;
    }

    public static boolean isLoggedIn(int id)
    {
        for(User u:users)
        {
            if(u.uid == id && u.isOnline) return true;
        }

        return false;
    }

    public static boolean isAuthUser(int id) {
        for (User u : users) {
            if (u.uid == id) return true;
        }
        return false;
    }

    public static boolean hasLoggedInBefore(int id) {
        for(User u:users) {
            if(u.uid == id && !u.isOnline) return true;
        }
        return false;
    }

    public void restore(Vector<Message> userMessages,int fileCount)
    {
        this.userMessages = userMessages;
        this.fileCount = fileCount;
    }
}