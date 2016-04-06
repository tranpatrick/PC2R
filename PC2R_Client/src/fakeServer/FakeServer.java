package fakeServer;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * 
 * @author Ladislas Halifa
 * Classe permettant de simuler un serveur a l'ecoute de demande de connexion
 * d'un client
 */
public class FakeServer {
	public static void main(String[] args) {
		int port = 2016;
		ServerSocket s;
		try {
			s = new ServerSocket(port);
			while (true) {
				Socket client = s.accept();
				Thread t = new ClientThread(client);
				t.start();		
			}
		} catch (Exception e) {

			e.printStackTrace();
		}
	}
}
