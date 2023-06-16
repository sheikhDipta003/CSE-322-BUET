import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.util.Vector;

public class Server2 {

    static long MAX_BUFFER_SIZE = 100000 * 1024; // byte
    static int MIN_CHUNK_SIZE = 20; // kb
    static int MAX_CHUNK_SIZE = 60; // kb

    static volatile long CUR_BUFFER_SIZE = 0;

    static int reqId = 0;
    static Vector<Integer> requestUserId = new Vector<>();
    static Vector<Integer> rands = new Vector<>();

    public static boolean receiveFile(String fileName, String fileType,int fileLength,int userID,DataInputStream disFile,
                                      DataOutputStream dosFile,int CHUNK_SIZE) throws IOException {

        int bytes = 0;
        FileOutputStream fos =
                new FileOutputStream("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                        userID+"/"+fileType+"/"+fileName);

        try{
            int size = fileLength;     // read file size
            byte[] buffer = new byte[CHUNK_SIZE];
            int CHUNK = 0;
            // extra
            CUR_BUFFER_SIZE += CHUNK_SIZE;
            while (size > 0) {

                boolean ok;
                try {
                    ok = (bytes = disFile.read(buffer, 0, Math.min(buffer.length, size))) != -1;
                }catch (SocketTimeoutException socketTimeoutException){
                    CUR_BUFFER_SIZE -= CHUNK_SIZE;
                    fos.close();
                    System.out.println("FileOutputStream Closed due to socket timeout");
                    return false;
                }

                if(!ok) break;

                CHUNK++;

                fos.write(buffer,0,bytes);

                size -= bytes;      // read upto file size

                dosFile.writeUTF("ACK");
                dosFile.flush();
            }

            CUR_BUFFER_SIZE -= CHUNK_SIZE;
            fos.close();
            System.out.println("FileOutputStream Closed");

        }catch (Exception e)
        {
            CUR_BUFFER_SIZE -= CHUNK_SIZE;
            fos.close();
            System.out.println("FileOutputStream Closed due to an exception");
        }

        // check confirmation and validate file length
        String msg = disFile.readUTF();
        File file =
                new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                        userID+"/"+fileType+"/"+fileName);
        if(msg.equals("ACK")){
            if(file.length() != fileLength)
            {
                System.out.println("File length mismatch");
                file.delete();
                return false;
            }
        }
        else
        {
            file.delete();
            return false;
        }

        return true;
    }

    public static void sendFile(String fileName, String fileType,int uID,int CHUNK_SIZE,DataOutputStream dataOutputStream,
                                String fileID) throws IOException {

        File file =
                new File("C:/Users/HP/Desktop/3-2-Lab/cse322-Networks/socket-programming/mycodes/files/"+
                        uID+"/"+fileType+"/"+fileName);
        FileInputStream fis = new FileInputStream(file);

        try{
            long fileLength = file.length();

            dataOutputStream.writeUTF("file "+ fileLength +" "+fileName+" "+fileType+" "+CHUNK_SIZE+" "+fileID);
            dataOutputStream.flush();

            // break file into chunks
            int bytes = 0;
            byte[] buffer = new byte[CHUNK_SIZE];
            int CHUNK = 0;
            while ((bytes=fis.read(buffer))!=-1){
                if(CHUNK % 1000 == 0) System.out.println("Chunk #"+CHUNK);
                CHUNK++;
                dataOutputStream.write(buffer,0,bytes);
                dataOutputStream.flush();
            }
            fis.close();

        }catch (Exception e)
        {
            fis.close();
        }
    }

    Server2 () {
        try{
            ServerSocket welcomeSocket = new ServerSocket(7777);
            ServerSocket welcomeSocketFile = new ServerSocket(5555);

            while(true) {

                Socket connectionSocket = welcomeSocket.accept();
                DataInputStream dis = new DataInputStream(connectionSocket.getInputStream());
                DataOutputStream dos = new DataOutputStream(connectionSocket.getOutputStream());

                Socket connectionSocketFile = welcomeSocketFile.accept();
                DataInputStream disFile = new DataInputStream(connectionSocketFile.getInputStream());
                DataOutputStream dosFile = new DataOutputStream(connectionSocketFile.getOutputStream());

                User currUser = new User(connectionSocket.getPort(),dis,dos,disFile,dosFile);
                System.out.println("Just connected to client with port -> " + connectionSocket.getPort() + ", file : "+
                        connectionSocketFile.getPort());

                new Worker(dis, dos, disFile, dosFile, currUser).start();
                new WorkerFile(dis, dos, connectionSocketFile, disFile, dosFile, currUser).start();
            }
        }catch(Exception e) {
            System.out.println("Cannot connect to server");
            System.exit(1);
        }
    }

    public static void main(String[] args) {
        new Server2();
    }
}