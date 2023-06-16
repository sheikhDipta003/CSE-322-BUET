// Duplicate of Client2 to simulate multiple clients

import java.io.*;
import java.net.Socket;
import java.net.SocketTimeoutException;

public class Client3 {

    public static void sendFile(String fileName, String fileType, int CHUNK_SIZE,DataInputStream disFile,DataOutputStream dosFile) throws IOException {

        File file = new File("C:/Users/DELL/Documents/1905003/clientfile/"+fileName);
        FileInputStream fis = new FileInputStream(file);

        long fileLength = file.length();

        dosFile.writeUTF("file "+ fileLength +" "+fileName+" "+fileType+" "+CHUNK_SIZE);
        dosFile.flush();

        int bytes = 0;
        byte[] buffer = new byte[CHUNK_SIZE];
        while ((bytes=fis.read(buffer))!=-1){
            dosFile.write(buffer,0,bytes);
            dosFile.flush();

            try {
                String msg = disFile.readUTF();
                if(!msg.equals("ACK"))
                {
                    System.out.println("No ACK");
                    break;
                }

            }catch (SocketTimeoutException socketTimeoutException){
                System.out.println("timeout");
                dosFile.writeUTF("timeout "+fileName + " " + fileType);
                dosFile.flush();
                fis.close();
                return;
            }
        }
        fis.close();

        dosFile.writeUTF("ACK");
        dosFile.flush();

        String msg = disFile.readUTF();
        if(msg.equals("ACK")) System.out.println("File uploaded");
        else System.out.println("Upload failed");

    }

    public static void receiveFile(String fileName, String fileType,int fileLength,DataInputStream dataInputStream,int CHUNK_SIZE,String fileId) throws IOException {

        int bytes = 0;
        FileOutputStream fos = new FileOutputStream("C:/Users/DELL/Documents/1905003/clientfile/"+fileType+"_"+fileId+"_"+fileName);

        int size = fileLength;     // read file size
        byte[] buffer = new byte[CHUNK_SIZE];
        while (size > 0 && (bytes = dataInputStream.read(buffer, 0, Math.min(buffer.length, size))) != -1) {
            fos.write(buffer,0,bytes);
            size -= bytes;      // read upto file size
        }
        fos.close();
    }

    Client3()
    {
        try{
            Socket clientSocket = new Socket("localhost", 7777);
            DataInputStream dis = new DataInputStream(clientSocket.getInputStream());
            DataOutputStream dos = new DataOutputStream(clientSocket.getOutputStream());

            Socket clientSocketFile = new Socket("localhost", 5555);
            DataInputStream disFile = new DataInputStream(clientSocketFile.getInputStream());
            DataOutputStream dosFile = new DataOutputStream(clientSocketFile.getOutputStream());

            new ClientWorker(dis,dos,disFile,dosFile).start();
            new ClientWorkerFile(dis,dos,clientSocketFile,disFile,dosFile).start();
        }catch(Exception e) {
            System.out.println("Cannot connect to server");
            System.exit(1);
        }
    }

    public static void main(String[] args) {
        new Client3();
    }
}