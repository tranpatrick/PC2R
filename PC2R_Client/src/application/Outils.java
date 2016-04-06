package application;

import java.util.ArrayList;

import javafx.scene.paint.Color;
import javafx.scene.text.Text;

/**
 * 
 * @author Ladislas Halifa
 * Cette classe contient des methodes statiques utilisees pour verifier
 * que certains parametres sont bien formes
 */
public class Outils {

	/**
	 * Permet d'extraire la commande d'une requete
	 * @param reponse une requete
	 * @return la commande d'une requete
	 */
	public static String getCommandeName(String reponse) {
		return reponse.split("/")[0];
	}

	/**
	 * Permet d'extraire le premier parametre d'une requete
	 * @param reponse une requete
	 * @return le premier parametre d'une requete s'il existe, null sinon
	 */
	public static String getFirstArg(String reponse) {
		String[] cmd = reponse.split("/");
		if (cmd.length > 1)
			return cmd[1];
		else
			return null;
	}

	/**
	 * Permet d'extraire le second parametre d'une requete
	 * @param reponse une requete
	 * @return le second parametre d'une requete s'il existe, null sinon
	 */
	public static String getSecondArg(String reponse) {
		String[] cmd = reponse.split("/");
		if (cmd.length > 2)
			return cmd[2];
		else
			return null;
	}

	/**
	 * Verifie qu'un nom d'utilisateur est valide (compose uniquement de
	 * caracteres alpha-numerique)
	 * @param username le nom d'utilisateur a tester
	 * @return true si le pseudo est valide, false sinon
	 */
	private static boolean checkUsername(String username) {
		if (username == null || username.equals(""))
			return false;
		return username.matches("\\w+");
	}

	/**
	 * Verifie que l'adresse IPv4 du serveur est valide (composee de quatre
	 * nombres compris entre 0 et 255)
	 * @param host l'adresse a tester
	 * @return true si l'adresse est valide, false sinon 
	 */
	public static boolean checkHost(String host) {
		if (host == null)
			return false;
		if (host.equals("localhost")) {
			return true;
		}
		if (host.matches("^\\d+.\\d+.\\d+.\\d+$")) {
			String[] ip = host.split("\\.");
			for (String s : ip) {
				System.out.println(s);
				int number = Integer.parseInt(s);
				if (!(number < 256 && number >= 0)) {
					return false;
				}
			}
			return true;
		}
		return false;
	}

	/**
	 * Verifie a la fois si le nom d'utilisateur et l'adresse du serveur sont
	 * valides
	 * @param username le nom d'utilisateur
	 * @param host l'adresse du serveur
	 * @param actionTarget Node sur lequel afficher le message d'erreur
	 * @return true si l'username et le serveur sont valides, false sinon
	 */
	public static boolean checkHostAndCheckUsername(
			String username, String host, Text actionTarget) {
		if (!checkUsername(username)) {
			actionTarget.setText("Pseudo non valide");
			actionTarget.setFill(Color.FIREBRICK);
			return false;
		}
		if (!checkHost(host)) {
			actionTarget.setText("Serveur non valide");
			actionTarget.setFill(Color.FIREBRICK);
			return false;
		}
		return true;
	}

	/**
	 * Verifie qu'une solution d'un joueur est bien composee d'une serie de 
	 * deplacements legaux
	 * @param deplacements la solution du joueur
	 * @return true si la solution est valide, false sinon
	 */
	public static boolean isValidSolution(String deplacements) {
		if (deplacements == null || deplacements.equals(""))
			return false;
		return deplacements.matches("([RBJV][HBGD])+");
	}

	/**
	 * Verifie que la description d'un plateau est bien formee
	 * @param plateau la description textuelle du plateau
	 * @return true si le plateau est valide, false sinon
	 */
	public static boolean isValidPlateau(String plateau) {
		if (plateau == null || plateau.equals(""))
			return false;
		return plateau.matches("(\\(\\d+,\\d+,[HDGB]\\))+");
	}

	/**
	 * Permet d'obtenir une serie de deplacements a partir d'une solution
	 * @param deplacements la solution d'un joueur
	 * @return une liste contenant la serie de deplacements
	 */
	public static ArrayList<String> getCoups(String deplacements) {
		ArrayList<String> coups = new ArrayList<>();
		while (deplacements.length() > 1) {
			coups.add(deplacements.substring(0, 2));
			deplacements = deplacements.substring(2);
		}
		return coups;
	}

}

