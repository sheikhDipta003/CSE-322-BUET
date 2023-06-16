import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.net.Socket;
import java.util.StringTokenizer;
import java.util.Vector;

public class ClientWorkerFile extends Thread{
    DataInputStream dis;
    DataOutputStream dos;
    Socket clientSocketFile;
    DataInputStream disFile;
    DataOutputStream dosFile;

    ClientWorkerFile(DataInputStream dis, DataOutputStream dos, Socket clientSocketFile, DataInputStream disFile, DataOutputStream dosFile){
        this.dis=dis; this.dos=dos;
        this.clientSocketFile=clientSocketFile;
        this.disFile=disFile; this.dosFile=dosFile;
    }
    @Override
    public void run(){
        while (true) {

            System.out.println("File Stream : ");

            // ping reply
            String fromServer;
            try {
                fromServer = disFile.readUTF();
                System.out.println("Server (filestream): " + fromServer);

                StringTokenizer stringTokenizer = new StringTokenizer(fromServer," ");
                Vector<String> tokens = new Vector<>();

                while (stringTokenizer.hasMoreTokens())
                {
                    tokens.add(stringTokenizer.nextToken());
                }

                if(tokens.elementAt(0).equals("sf"))
                {

                    int CHUNK_SIZE = Integer.parseInt(tokens.elementAt(1));
                    String fileId = tokens.elementAt(2);
                    String fileName = tokens.elementAt(3);
                    String fileType = tokens.elementAt(4);

                    clientSocketFile.setSoTimeout(30000);
                    Client2.sendFile(fileName,fileType,CHUNK_SIZE,disFile,dosFile);
                    clientSocketFile.setSoTimeout(0);
                }
                else if(tokens.elementAt(0).equals("rf"))
                {
                    int filesize = Integer.parseInt(tokens.elementAt(1));
                    String fileName = tokens.elementAt(2);
                    String fileType = tokens.elementAt(3);
                    int CHUNK_SIZE = Integer.parseInt(tokens.elementAt(4));
                    String fileID = tokens.elementAt(5);

                    Client2.receiveFile(fileName,fileType,filesize,disFile,CHUNK_SIZE,fileID);
                    System.out.println("File downloaded");
                }

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}