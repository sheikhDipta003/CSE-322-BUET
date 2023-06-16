import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.util.Scanner;
import java.util.StringTokenizer;
import java.util.Vector;

public class ClientWorker extends Thread{
    DataInputStream dis;
    DataOutputStream dos;
    DataInputStream disFile;
    DataOutputStream dosFile;

    ClientWorker(DataInputStream dis, DataOutputStream dos, DataInputStream disFile, DataOutputStream dosFile){
        this.dis=dis; this.dos=dos; this.disFile=disFile; this.dosFile=dosFile;
    }

    @Override
    public void run(){
        // first
        String fromServer = null;
        try {
            fromServer = dis.readUTF();
        } catch (IOException e) {
            e.printStackTrace();
        }
        System.out.println("Server: " + fromServer);

        while (true)
        {
            Scanner sc = new Scanner(System.in);
            String reply = sc.nextLine();
            StringTokenizer stringTokenizer = new StringTokenizer(reply," ");
            Vector<String> tokens = new Vector<>();

            while (stringTokenizer.hasMoreTokens())
            {
                tokens.add(stringTokenizer.nextToken());
            }

            try {

                if(tokens.elementAt(0).equals("up") || tokens.elementAt(0).equals("upreq") ||
                        tokens.elementAt(0).equals("down") || tokens.elementAt(0).equals("downreq"))
                {
                    dosFile.writeUTF(reply);
                    dosFile.flush();
                }
                else{
                    dos.writeUTF(reply);
                    dos.flush();
                }

                if(reply.equals("exit"))
                {
                    System.exit(1);
                }

            } catch (IOException e) {
                e.printStackTrace();
            }

            try {
                fromServer = dis.readUTF();
                System.out.println("Server: " + fromServer);

            } catch (IOException e) {
                e.printStackTrace();
            }
        }
    }
}