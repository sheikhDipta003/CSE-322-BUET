import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.StringTokenizer;
import java.util.Vector;

public class Worker extends Thread{

    User currUser;
    DataInputStream dis;
    DataOutputStream dos;
    DataInputStream disFile;
    DataOutputStream dosFile;

    Worker(DataInputStream dis, DataOutputStream dos, DataInputStream disFile, DataOutputStream dosFile, User currUser){
        this.dis=dis; this.dos=dos; this.disFile=disFile; this.dosFile=dosFile; this.currUser = currUser;
    }

    @Override
    public void run(){
        System.out.println("<S>: Server started");

        try {
            currUser.getCredentials();
            String fromClient = dis.readUTF();
            int id = Integer.parseInt((fromClient));
            if(User.isLoggedIn(id))
            {
                currUser.writeToStream("You are already logged in",'c');
                Thread.currentThread().interrupt();
                return;
            }
            else if(User.hasLoggedInBefore(id))
            {
                for(User u:User.users)
                {
                    if(u.uid == id)
                    {
                        currUser.restore(u.userMessages,u.fileCount);
                        break;
                    }
                }

                User.remove(id);
                currUser.setId(id);
                User.users.add(currUser);
            }
            else
            {
                currUser.setId(id);
                User.users.add(currUser);
            }

        } catch (IOException e) {
            e.printStackTrace();
        }

        while (true) {
            String fromClient;
            try {

                fromClient = dis.readUTF();
                StringTokenizer stringTokenizer = new StringTokenizer(fromClient," ");
                Vector<String> tokens = new Vector<>();

                while (stringTokenizer.hasMoreTokens())
                {
                    tokens.add(stringTokenizer.nextToken());
                }

                if(tokens.elementAt(0).equals("alluser"))
                {
                    // lookup all students list
                    currUser.writeToStream(User.getUsers(),'c');
                }
                else if(tokens.elementAt(0).equals("showfiles"))
                {
                    //lookup all public and private files of this user
                    String[] publicFiles = User.lookupFiles(currUser.uid,"public");
                    StringBuilder ret = new StringBuilder("Public Files : \n");
                    for(String s:publicFiles)
                    {
                        ret.append(s).append("\n");
                    }

                    String[] privateFiles = User.lookupFiles(currUser.uid,"private");
                    ret.append("\nPrivate Files : \n");
                    for(String s:privateFiles)
                    {
                        ret.append(s).append("\n");
                    }
                    currUser.writeToStream(ret.toString(),'c');
                }
                else if(tokens.elementAt(0).equals("showpublicfiles"))
                {
                    // lookup only the public files of a specific student: showpublicfiles <user-id>
                    int userID = Integer.parseInt(tokens.elementAt(1));

                    if(User.isAuthUser(userID))
                    {
                        String[] publicFiles = User.lookupFiles(userID,"public");
                        StringBuilder ret = new StringBuilder("Public Files of " + userID + " : \n");
                        for(String s:publicFiles)
                        {
                            ret.append(s).append("\n");
                        }
                        currUser.writeToStream(ret.toString(),'c');
                    }
                    else
                    {
                        currUser.writeToStream("User does not exist",'c');
                    }

                }
                else if(tokens.elementAt(0).equals("reqfile"))
                {
                    // request for a file : reqfile <space separated description of file>
                    StringBuilder description = new StringBuilder();
                    for(int i=1;i<tokens.size();i++) description.append(tokens.elementAt(i));

                    String message = "File request ID : "+Server2.reqId+" , file description : "+description;
                    Server2.reqId++;

                    for(User other:User.users)
                    {
                        User.sendMessage(currUser.uid,other.uid,message);
                    }

                    currUser.writeToStream("File request broadcast",'c');
                    Server2.requestUserId.add(currUser.uid);
                }
                else if(tokens.elementAt(0).equals("msg"))
                {
                    // send message to other user : msg <user-id> <message>
                    int to = Integer.parseInt(tokens.elementAt(1));

                    StringBuilder message = new StringBuilder();
                    for(int i=2;i<tokens.size();i++) message.append(tokens.elementAt(i));

                    if(User.isAuthUser(to))
                    {
                        User.sendMessage(currUser.uid,to, message.toString());
                        currUser.writeToStream("Message sent",'c');
                    }
                    else
                    {
                        currUser.writeToStream("Invalid User",'c');
                    }
                }
                else if(tokens.elementAt(0).equals("unread"))
                {
                    //show all unread messages of this user
                    Vector<String>stringVector =  currUser.showUnreadMessage();

                    StringBuilder reply = new StringBuilder("Unread Messages\n");
                    System.out.println(reply);

                    for(String s:stringVector)
                    {
                        reply.append(s).append("\n");
                    }

                    currUser.writeToStream(reply.toString(),'c');
                }
                else if(tokens.elementAt(0).equals("exit"))
                {
                    currUser.isOnline=false;
                    Thread.currentThread().interrupt(); // preserve the message
                    return;
                }
            } catch (Exception e) {
                currUser.isOnline=false;
                Thread.currentThread().interrupt(); // preserve the message
                return;
            }
        }
    }


}