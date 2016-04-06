package application;

import java.io.PrintStream;

/**
 * 
 * @author Ladislas Halifa
 * Cette classe contient les commandes du Protocole ainsi que des methodes pour
 * envoyer des requetes bien formees au serveur
 */
public class Protocole {

	public static final int PORT = 2016;

	public static final String BIENVENUE = "BIENVENUE";
	public static final String CONNECTE = "CONNECTE";
	public static final String DECONNEXION = "DECONNEXION";
	
	public static final String RECEIVE_CHAT = "CHAT";
	
	public static final String SESSION = "SESSION";
	public static final String VAINQUEUR = "VAINQUEUR";
	
	public static final String TOUR = "TOUR";
	public static final String TU_AS_TROUVE = "TUASTROUVE";
	public static final String IL_A_TROUVE = "ILATROUVE";
	public static final String FIN_REFLEXION = "FINREFLEXION";
	
	public static final String VALIDATION = "VALIDATION";
	public static final String ECHEC = "ECHEC";
	public static final String NOUVELLE_ENCHERE = "NOUVELLEENCHERE";
	public static final String FIN_ENCHERE = "FINENCHERE";
	
	public static final String SA_SOLUTION = "SASOLUTION";
	public static final String BONNE = "BONNE";
	public static final String MAUVAISE = "MAUVAISE";
	public static final String FIN_RESOLUTION = "FINRESO";
	public static final String TROP_LONG = "TROPLONG";
	
	public static final String CONNEXION = "CONNEXION";
	public static final String SORT = "SORT";
	public static final String SEND_CHAT = "SEND";
	
	public static final String TROUVE = "SOLUTION";
	
	public static final String ENCHERE = "ENCHERE";
	
	public static final String SOLUTION = "SOLUTION";

	/**
	 * Envoie une requete de deconnexion au serveur
	 * @param username l'utilisateur qui se deconnecte
	 * @param out PrintStream du serveur
	 */
	public static void disconnect(String username, PrintStream out) {
		send(SORT, username, out);
	}

	/**
	 * Envoie une requete de connexion au serveur
	 * @param username l'utilisateur qui se connecte
	 * @param out PrintStream du serveur
	 */
	public static void connect(String username, PrintStream out) {
		send(CONNEXION, username, out);
	}

	/**
	 * Envoie une requete de message instantanne au serveur
	 * @param username l'utilisateur envoyant un message
	 * @param message le message a envoyer
	 * @param out PrintStream du serveur
	 */
	public static void sendChat(String username, String message, PrintStream out) {
		send(SEND_CHAT, username, message, out);
	}
	
	/**
	 * Envoie une requete de TROUVE au serveur
	 * @param username l'utilisateur annoncant qu'il a une solution
	 * @param coups le nombre de coups de la solution
	 * @param out PrintStream du serveur
	 */
	public static void sendTrouve(String username, String coups, PrintStream out) {
		send(TROUVE, username, coups, out);
	}

	/**
	 * Envoie une requete d'ENCHERE au serveur
	 * @param userName l'utilisateur qui enchere
	 * @param coups le nombre de coups de l'enchere
	 * @param out PrintStream du serveur
	 */
	public static void sendEnchere(String userName, String coups, PrintStream out) {
		send(ENCHERE, userName, coups, out);
	}

	/**
	 * Envoie une requete de SOLUTION au serveur
	 * @param userName l'utilisateur qui envoie sa solution
	 * @param deplacements la solution du joueur
	 * @param out PrintStream du serveur
	 */
	public static void sendSolution(String userName, String deplacements, PrintStream out) {
		send(SOLUTION, userName, deplacements, out);
	}


	/**
	 * Genere une requete a partir d'une commande et d'un seul parametre
	 * @param commande la commande de la requete
	 * @param param1 le parametre de la requete
	 * @param out PrintStream du serveur
	 */
	private static void send(String commande, String param1, PrintStream out){
		out.print(commande+"/"+param1+"/\n");
	}

	/**
	 * Genere une requete a partir d'une commande et de deux parametres
	 * @param commande la commande de la requete
	 * @param param1 le premier parametre
	 * @param param2 le second parametre
	 * @param out PrintStream du serveur
	 */
	private static void send(String commande, String param1, String param2, PrintStream out){
		out.print(commande+"/"+param1+"/"+param2+"/\n");
	}

}
