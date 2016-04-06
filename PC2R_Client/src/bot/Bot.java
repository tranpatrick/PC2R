package bot;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintStream;
import java.net.Socket;
import java.net.UnknownHostException;
import java.util.ArrayList;
import java.util.Random;

import application.Outils;
import application.Protocole;
import rasendeRoboter.Phase;

/**
 * 
 * @author Ladislas Halifa
 * Client autonome idiot essayant de se faire passer pour un humain
 */
public class Bot {
	public static String myName;
	@SuppressWarnings("unused")
	private String userName, host;
	private Socket socket;
	private BufferedReader in;
	private PrintStream out;
	private Receive receiver;
	private Random rand;

	private int lastEnchere = Integer.MAX_VALUE;
	private int monEnchere = 0;
	private String lastActif;


	private Phase phase = null;
	private boolean tuEnchere = false;
	private boolean attenteStatutSolution = false;


	public Bot(String host) {
		rand = new Random(System.currentTimeMillis()); 
		userName = BotData.getName();
		myName = userName;
		this.host = host;
		System.out.println("Nom du bot : "+userName);
		System.out.println("Tentative de connexion sur : "+host);
		socket = connexion(userName, host);
	}



	public static void main(String[] args) {
		if (args.length != 1) {
			System.err.println("usage: java -jar bot.jar <host>");
		}
		else {
			new Bot(args[0]);
		}
	}

	private void attendre(long secondes) {
		try {
			Thread.sleep(secondes*1000);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	public Socket connexion(String username, String host) {
		if (Outils.checkHost(host)) {
			this.host= host;
			this.userName = username;
			try {
				if (socket == null) {
					this.socket = new Socket(host, Protocole.PORT);
					this.in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
					this.out = new PrintStream(socket.getOutputStream(), true);
				}
				Protocole.connect(userName, out);
				String serverAnswer = in.readLine();
				if (serverAnswer.equals(Protocole.BIENVENUE+"/"+username+"/")) {
					this.receiver = new Receive(in);
					receiver.start();
				}
				else {
					System.out.println(username+" deja utilise");
					return null;
				}
			} catch (UnknownHostException e) {
				System.out.println("Connexion refusee");
				return null;
			} catch (IOException e) {
				System.out.println("Connexion refusee");
				return null;
			}
		}
		System.out.println("Bot connecte sur "+host);
		return socket;
	}

	public synchronized void decoderReponseServer(String reponse) {
		String commande = Outils.getCommandeName(reponse);
		String user, message, data;
		switch (commande) {
		case Protocole.BIENVENUE:
			System.out.println("BIENVENUE");
			break;
		case Protocole.CONNECTE:
			user = Outils.getFirstArg(reponse);
			if (Math.random() < 0.7) {
				attendre(2);
				String msg = BotData.getMessageBienvenue()+" "+user;
				Protocole.sendChat(userName, msg, out);
			}
			if (Math.random() > 0.95) {
				attendre(2);
				Protocole.sendChat(userName, BotData.getMessageBye(), out);
				attendre (1);
				Protocole.disconnect(userName, out);
				try {
					socket.close();
					System.exit(0);
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
			break;
		case Protocole.DECONNEXION:
			user = Outils.getFirstArg(reponse);
			break;
		case Protocole.RECEIVE_CHAT:
			user = Outils.getFirstArg(reponse);
			message = Outils.getSecondArg(reponse);
			System.out.println("recu : "+message+" de la part de "+user);
			if (!user.equals(userName)) {

				if (Math.random() < 0.4) {
					attendre(2);
					String msg = BotData.getMessageOK();
					Protocole.sendChat(userName, msg, out);
				}

				if (Math.random() > 0.95) {
					attendre(2);
					Protocole.sendChat(userName, BotData.getMessageBye(), out);
					attendre (1);
					Protocole.disconnect(userName, out);
					try {
						socket.close();
						System.exit(0);
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			break;
		case Protocole.SESSION:
			data = Outils.getFirstArg(reponse);
			phase = Phase.ATTENTE_TOUR;
			System.out.println("Attente tour");
			attendre(2);
			Protocole.sendChat(userName, "Hi I'm "+userName, out);
			break;
		case Protocole.VAINQUEUR:
			phase = Phase.ATTENTE_TOUR;
			data = Outils.getFirstArg(reponse);
			System.out.println("Fin de la session");
			break;
		case Protocole.TOUR:
			data= Outils.getSecondArg(reponse);
			if (phase == Phase.ATTENTE_TOUR) {
				phase = Phase.REFLEXION;
				System.out.println("Phase de reflexion");
				attendre(4);
				if (Math.random() < 0.4) {
					message = BotData.getMessageProvoc();
					Protocole.sendChat(userName, BotData.getMessageProvoc(), out);
				}
			}
			else {
				System.err.println("[SESSION] Je ne dois pas passer ici");
			}
			break;
		case Protocole.IL_A_TROUVE:
			if (phase == Phase.REFLEXION) {
				System.out.println(phase);
				phase = Phase.ENCHERE;
				user = Outils.getFirstArg(reponse);
				data = Outils.getSecondArg(reponse);
				System.out.println(user+" a trouve une solution en "+data+" coups");
				System.out.println("Fin de la phase de reflexion");
				lastEnchere = Integer.parseInt(data);
				if (rand.nextDouble() < 0.15) {
					attendre(1);
					Protocole.sendChat(userName, "autant de coups "+user+" ?", out);
				}
				double alea = rand.nextDouble();
				System.out.println("proba enchere "+alea);
				if (alea < 0.3) {
					attendre(3);
					int enchere = lastEnchere - rand.nextInt(5) - 2;
					if (enchere > 0) {
						tuEnchere = true;
						monEnchere = enchere;
						Protocole.sendEnchere(userName, ""+enchere, out);
					}
				}
			}
			else {
				System.err.println("[IL_A_TROUVE] Je ne dois pas passer ici");
			}
			break;
		case Protocole.FIN_REFLEXION:
			if (phase == Phase.REFLEXION) {
				phase = Phase.ENCHERE;		
				lastEnchere = Integer.MAX_VALUE;
				double alea = rand.nextDouble();
				System.out.println("proba enchere "+alea+" ref = 95%");
				if (alea < 0.95) {
					attendre(2);
					tuEnchere = true;
					Protocole.sendEnchere(userName, ""+53, out);
					monEnchere = 53;
					lastEnchere = 53;
				}
				if (Math.random() > 0.95) {
					attendre(2);
					Protocole.sendChat(userName, BotData.getMessageBye(), out);
					attendre (1);
					Protocole.disconnect(userName, out);
					try {
						socket.close();
						System.exit(0);
					} catch (IOException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}
			else {
				System.err.println("[FIN_REFLEXION] Je ne dois pas passer ici");
			}
			break;
		case Protocole.VALIDATION:
			if (phase == Phase.ENCHERE && tuEnchere) {
				System.out.println("Enchere validee");
				tuEnchere = false;				
			}
			else {
				System.err.println("Je ne dois pas passer ici");
			}
			break;
		case Protocole.ECHEC:
			if (phase == Phase.ENCHERE && tuEnchere) {
				user = Outils.getFirstArg(reponse);
				System.out.println("Enchere annulee car incoherente avec celle de "+user);
				tuEnchere = false;				
			}
			else {
				System.err.println("[VALIDATION] Je ne dois pas passer ici");
			}
			break;
		case Protocole.NOUVELLE_ENCHERE:
			if (phase == Phase.ENCHERE) {
				user = Outils.getFirstArg(reponse);
				data = Outils.getSecondArg(reponse);
				System.out.println(user+" a encheri avec "+data+" coups");
				lastEnchere = Integer.parseInt(data);
				double alea = rand.nextDouble();
				System.out.println("proba : "+alea+"  50%");
				if (alea < 0.5) {
					attendre(3);
					int enchere = lastEnchere - rand.nextInt(5)-2;
					System.out.println("lastEnchere = "+lastEnchere);
					if (enchere > 0) {
						tuEnchere = true;
						monEnchere = enchere;
						Protocole.sendEnchere(userName, ""+enchere, out);
					}
				}
				else {
					if (rand.nextDouble() < 0.5)
						Protocole.sendChat(userName, "pas mal "+user, out);
					else
						Protocole.sendChat(userName, "serieux "+user+" ?", out);
				}
			}
			else {
				System.err.println("[NOUVELLE_ENCHERE] Je ne dois pas passer ici");
			}
			break;
		case Protocole.FIN_ENCHERE:
			if (phase == Phase.ENCHERE) {
				phase = Phase.RESOLUTION;
				System.out.println("debut phase de resolution");
				user = Outils.getFirstArg(reponse);
				data = Outils.getSecondArg(reponse);
				System.out.println("Fin des encheres");
				if (!user.equals(userName)) {
					if (!user.equals("")) {
						System.out.println("Le joueur actif est "+user);
						lastActif = user;
					}
				}
				else {
					String sol = genererSolution(monEnchere);
					lastActif = userName;
					System.out.println("solution a envoye = "+sol);
					int alea = rand.nextInt(8) + 7;
					System.out.println("Envoie de la solution dans "+alea+"s");
					attendre(alea);
					Protocole.sendSolution(userName, sol, out);
				}
			}
			else {
				System.err.println("Je ne dois pas passer ici");
			}
			break;
		case Protocole.SA_SOLUTION:
			if (phase == Phase.RESOLUTION && attenteStatutSolution == false) {
				user = Outils.getFirstArg(reponse);
				data = Outils.getSecondArg(reponse);
				if (!user.equals(userName)) {
					System.out.println(user+" a propose la solution suivante : "+data);
				}
				attenteStatutSolution = true;
			}
			else {
				System.err.println("Je ne dois pas passer ici");
			}
			break;
		case Protocole.BONNE:
			if (phase == Phase.RESOLUTION && attenteStatutSolution) {
				if (lastActif.equals(userName)) {
					Protocole.sendChat(userName, "je ferai mieux la prochaine fois", out);
				}
				else {
					attendre(1);
					String msg = BotData.getMessageFelicitation()+" "+lastActif;
					Protocole.sendChat(userName, msg, out);
				}
				System.out.println("Solution correcte");
				System.out.println("Fin du tour");
				lastActif = "";
				attenteStatutSolution = false;	
				phase = Phase.ATTENTE_TOUR;
				System.out.println("attente tour");
			}
			else {
				System.err.println("Je ne dois pas passer ici");
			}
			break;
		case Protocole.MAUVAISE:
			if (phase == Phase.RESOLUTION && attenteStatutSolution) {
				user = Outils.getFirstArg(reponse);
				System.out.println("Solution refusee");
				if (!lastActif.equals(userName)) {
					attendre(1);
					String msg = BotData.getMessageBienTente()+" "+lastActif;
					Protocole.sendChat(userName, msg, out);
				}
				if (!user.equals(userName)) {
					System.out.println("Le joueur actif est "+user);
					lastActif = user;
				}
				else {
					String sol = genererSolution(monEnchere);
					lastActif = userName;
					System.out.println("solution a envoye = "+sol);
					int alea = rand.nextInt(8) + 7;
					System.out.println("Envoie de la solution dans "+alea+"s");
					attendre(alea);
					Protocole.sendSolution(userName, sol, out);
				}
				attenteStatutSolution = false;
			}
			else {
				System.err.println("Je ne dois pas passer ici");
			}
			break;
		case Protocole.FIN_RESOLUTION:
			if (phase == Phase.RESOLUTION || phase == Phase.ENCHERE) {
				System.out.println("Plus de joueurs restants");
				System.out.println("Fin du tour");
				phase = Phase.ATTENTE_TOUR;
				System.out.println("attente tour");
			}
			else {
				System.err.println("je ne dois pas passer ici");
			}
			break;
		case Protocole.TROP_LONG:
			if (phase == Phase.RESOLUTION) {
				user = Outils.getFirstArg(reponse);
				System.out.println("Temps depasse");
				if (!lastActif.equals(userName)) {
					Protocole.sendChat(userName, "il faut aller plus vite "+lastActif, out);
				}
				if (!user.equals(userName)) {
					System.out.println("Le joueur actif est "+user);
				}
				else {
					String sol = genererSolution(monEnchere);
					lastActif = userName;
					System.out.println("solution a envoye = "+sol);
					int alea = rand.nextInt(8) + 7;
					System.out.println("Envoie de la solution dans "+alea+"s");
					attendre(alea);
					Protocole.sendSolution(userName, sol, out);
				}
			}
			else {
				System.err.println(Protocole.TROP_LONG+" - je ne dois pas passer ici");
			}
			break;
		default:
			System.out.println("default "+reponse);
			break;
		}
	}


	private String genererSolution(int monEnchere) {
		Random rand = new Random(System.currentTimeMillis());
		String tmp = "";
		double alea; 
		for (int i = 0; i < monEnchere; i++) {
			alea = rand.nextDouble();
			if (alea < 0.25) {
				tmp += "R";
			}
			else if (alea < 0.5) {
				tmp += "B";
			}
			else if (alea < 0.75) {
				tmp += "J";
			}
			else {
				tmp += "V";
			}
			alea = rand.nextDouble();
			if (alea < 0.25) {
				tmp += "H";
			}
			else if (alea < 0.5) {
				tmp += "B";
			}
			else if (alea < 0.75) {
				tmp += "G";
			}
			else {
				tmp += "D";
			}
		}
		return tmp;
	}

	String recu;

	class Receive extends Thread {
		private BufferedReader in;
		private ArrayList<String> cmds = new ArrayList<>();
		public Receive(BufferedReader in) {
			this.in = in;
		}

		@Override
		public void run() {
			try {
				while ((recu = in.readLine()) != null) {
					synchronized(recu) {
						System.out.println("recu : "+recu);
						cmds.add(recu);
						Thread t = new Thread(new Runnable() {

							@Override
							public void run() {
								decoderReponseServer(cmds.remove(0));							
							}
						});
						t.start();
					}
				}
			} catch (IOException e) {
				e.printStackTrace();
			}
		}
	}
}
