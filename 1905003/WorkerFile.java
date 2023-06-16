import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.File;
import java.net.Socket;
import java.util.Random;
import java.util.StringTokenizer;
import java.util.Vector;

public class WorkerFile extends Thread{
    User currUser;
    DataInputStream dis;
    DataOutputStream dos;
    Socket connectionSocketFile;
    DataInputStream disFile;
    DataOutputStream dosFile;
    WorkerFile(DataInputStream dis, DataOutputStream dos, Socket connectionSocketFile, DataInputStream disFile,
               DataOutputStream dosFile, User currUser){
        this.dis=dis; this.dos=dos;
        this.connectionSocketFile=connectionSocketFile;
        this.disFile=disFile; this.dosFile=dosFile;
        this.currUser = currUser;
    }

    @Override
    public void run(){
        while (true)
        {
            try {
                String fromClient = disFile.readUTF();

                StringTokenizer stringTokenizer = new StringTokenizer(fromClient," ");
                Vector<String> tokens = new Vector<>();

                while (stringTokenizer.hasMoreTokens())
                {
                    tokens.add(stringTokenizer.nextToken());
                }

                if(tokens.elementAt(0).equals("down"))
                {
                    // download own file request from user :  down file_name file_type

                    String fileName = tokens.elementAt(1);
                    String fileType = tokens.elementAt(2);

                    currUser.writeToStream("starting download",'c');
                    Server2.sendFile(fileName,fileType,currUser.uid,Server2.MAX_CHUNK_SIZE * 1024,
                            currUser.dosFile,getFileId(currUser));

                }
                else if(tokens.elementAt(0).equals("downreq"))
                {
                    // download file request from user : downreq student_id file_name

                    int uID = Integer.parseInt(tokens.elementAt(1));
                    String fileType = "public";
                    String fileName = tokens.elementAt(2);

                    currUser.writeToStream("starting to download requested file",'c');
                    Server2.sendFile(fileName,fileType,uID,50, currUser.dosFile,getFileId(currUser));

                }
                else if(tokens.elementAt(0).equals("up"))
                {
                    // from user : up file_name file_length file_type

                    String fileName = tokens.elementAt(1);
                    int fileLength = Integer.parseInt(tokens.elementAt(2)); //bytes
                    String fileType = tokens.elementAt(3);

                    if(Server2.CUR_BUFFER_SIZE + fileLength <= Server2.MAX_BUFFER_SIZE)
                    {
                        int CHUNK_SIZE = getRandom(Server2.MIN_CHUNK_SIZE, Server2.MAX_CHUNK_SIZE);
                        currUser.writeToStream("ok",'c');
                        currUser.writeToStream("sf " + CHUNK_SIZE + " "+ getFileId(currUser) + " " +
                                fileName + " " + fileType,'f');
                    }
                    else
                    {
                        System.out.println("Buffer limit exceeded");
                        currUser.writeToStream("not ok",'c');

                        File file =
                                new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                                        currUser.uid+"/"+fileType+"/"+fileName);
                        file.delete();
                        currUser.writeToStream("File deleted",'c');
                    }
                }
                else if(tokens.elementAt(0).equals("upreq"))
                {
                    // from user : upreq file_name file_length requestId
                    // Upload file corresponding to a request id

                    String fileName = tokens.elementAt(1);
                    int fileLength = Integer.parseInt(tokens.elementAt(2));  //bytes
                    String fileType = "public";
                    int requestId = Integer.parseInt(tokens.elementAt(2));

                    if(requestId < Server2.requestUserId.size())
                    {
                        if(Server2.CUR_BUFFER_SIZE + fileLength <= Server2.MAX_BUFFER_SIZE)
                        {
                            int CHUNK_SIZE = getRandom(Server2.MIN_CHUNK_SIZE, Server2.MAX_CHUNK_SIZE);
                            currUser.writeToStream("ok",'c');
                            currUser.writeToStream("sf " + CHUNK_SIZE + " "+ getFileId(currUser) + " "
                                    + fileName + " " + fileType,'f');
                        }
                        else
                        {
                            System.out.println("Buffer limit exceeded");
                            currUser.writeToStream("not ok",'c');

                            File file =
                                    new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                                            currUser.uid+"/"+fileType+"/"+fileName);
                            file.delete();
                            currUser.writeToStream("File Deleted",'c');
                        }

                        int userWhoRequested = Server2.requestUserId.get(requestId);

                        String message = "requested file uploaded with reqid : "+requestId;

                        User.sendMessage(currUser.uid,userWhoRequested,message);
                    }
                    else{
                        currUser.writeToStream("Invalid request id",'c');
                    }

                }
                else if(tokens.elementAt(0).equals("processfile"))
                {
                    int fileLength = Integer.parseInt(tokens.elementAt(1));
                    String fileName = tokens.elementAt(2);
                    String fileType = tokens.elementAt(3);
                    int CHUNK_SIZE = Integer.parseInt(tokens.elementAt(4));

                    try
                    {
                        connectionSocketFile.setSoTimeout(30000);
                        boolean ok = Server2.receiveFile(fileName,fileType,fileLength,currUser.uid,disFile,dosFile,
                                CHUNK_SIZE);
                        connectionSocketFile.setSoTimeout(0);

                        if(ok)
                        {
                            currUser.writeToStream("ACK",'f');
                            System.out.println("upload complete");
                        }
                        else
                        {
                            File file = new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                                    currUser.uid+"/"+fileType+"/"+fileName);
                            System.out.println(file.delete());
                            currUser.writeToStream("NOT_ACK",'f');
                            System.out.println("upload failed");
                        }

                    }
                    catch(Exception e)
                    {
                        File file = new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                                currUser.uid+"/"+fileType+"/"+fileName);
                        System.out.println(file.delete());
                        currUser.writeToStream("File deleted due to exception",'f');
                        System.err.println("Could not transfer file");
                    }
                }
                else if(tokens.elementAt(0).equals("timeout"))
                {
                    String fileType = tokens.elementAt(1);
                    String fileName = tokens.elementAt(2);

                    File file =
                            new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                                    currUser.uid+"/"+fileType+"/"+fileName);
                    System.out.println(file.delete());
                    currUser.writeToStream("File deleted due to timeout",'f');
                }

            } catch (Exception e) {
                currUser.isOnline=false;
                Thread.currentThread().interrupt(); // preserve the message
                return;
            }
        }
    }

    public int getRandom(int min,int max) // get a random int in [min,max]
    {
        Random random = new Random();
        return (random.nextInt(max - min + 1) + min) * 1024;
    }

    public String getFileId(User u)
    {
        int x;
        Random rd = new Random();
        do{ x = rd.nextInt(); }
        while(Server2.rands.contains(x));

        Server2.rands.add(x);

        return u.uid + "_"+ x;
    }
}