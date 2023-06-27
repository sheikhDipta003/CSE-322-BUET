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

    public static boolean receiveFile(String filePath, String fileName, String fileType,int fileLength,int userID,DataInputStream disFile,
                                      DataOutputStream dosFile,int CHUNK_SIZE) throws IOException {

        int bytes = 0;
        FileOutputStream fos = new FileOutputStream(filePath + userID+"/"+fileType+"/"+fileName);

        try{
            int size = fileLength;     // read file size
            byte[] buffer = new byte[CHUNK_SIZE];
            CUR_BUFFER_SIZE += CHUNK_SIZE;
            while (size > 0) {

                boolean ok;
                try {
                    ok = (bytes = disFile.read(buffer, 0, Math.min(buffer.length, size))) != -1;
                }catch (SocketTimeoutException socketTimeoutException){
                    CUR_BUFFER_SIZE -= CHUNK_SIZE;
                    fos.close();
                    System.out.println("<S>: FileOutputStream closed due to socket timeout");
                    return false;
                }

                if(!ok) break;

                fos.write(buffer,0,bytes);

                size -= bytes;      // read upto file size

                dosFile.writeUTF("ACK");
                dosFile.flush();
            }

            CUR_BUFFER_SIZE -= CHUNK_SIZE;
            fos.close();
            System.out.println("<S>: FileOutputStream closed");

        }catch (Exception e)
        {
            CUR_BUFFER_SIZE -= CHUNK_SIZE;
            fos.close();
            System.out.println("<S>: FileOutputStream closed due to an exception");
        }

        // check confirmation and validate file length
        String msg = disFile.readUTF();
        File file =
                new File(filePath + userID+"/"+fileType+"/"+fileName);
        if(msg.equals("ACK")){
            if(file.length() != fileLength)
            {
                System.out.println("<S>: File length mismatch");
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

    public static void sendFile(String filePath, String fileName, String fileType,int uID,int CHUNK_SIZE,DataOutputStream dos,
                                String fileID) throws IOException {

        File file = new File(filePath + uID+"/"+fileType+"/"+fileName);
        FileInputStream fis = new FileInputStream(file);

        try{
            long fileLength = file.length();

            dos.writeUTF("file "+ fileLength +" "+fileName+" "+fileType+" "+CHUNK_SIZE+" "+fileID);
            dos.flush();

            // break file into chunks
            int bytes = 0;
            byte[] buffer = new byte[CHUNK_SIZE];
            int CHUNK = 0;
            while ((bytes=fis.read(buffer))!=-1){
                if(CHUNK % 1000 == 0) System.out.println("Chunk No. " + CHUNK);
                CHUNK++;
                dos.write(buffer,0,bytes);
                dos.flush();
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
                System.out.println("<S>: Connected to a client to port - " + connectionSocket.getPort() + ", FileStream port - "+
                        connectionSocketFile.getPort());

                new Worker(dis, dos, disFile, dosFile, currUser).start();
                new WorkerFile(dis, dos, connectionSocketFile, disFile, dosFile, currUser).start();
            }
        }catch(Exception e) {
            System.out.println("<S>: Cannot connect to the specified ports");
            System.exit(1);
        }
    }

    public static void main(String[] args) {
        new Server2();
    }
}