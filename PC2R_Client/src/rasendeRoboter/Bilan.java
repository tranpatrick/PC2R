package rasendeRoboter;

import java.util.HashMap;
import java.util.Map.Entry;

import javafx.application.Platform;
import javafx.beans.property.SimpleIntegerProperty;
import javafx.beans.property.SimpleStringProperty;
import javafx.collections.FXCollections;
import javafx.collections.ObservableList;

/**
 * @author Ladislas Halifa Cette classe permet de representer l'etat courant 
 * d'une session de jeu donne par le numero du tour courant et les scores des
 * joueurs ayant participe a la session de jeu
 */
public class Bilan {
	/**
	 * le numero du tour courant
	 */
	private int tour;
	/**
	 * Liste associant un nom d'utilisateur a un score
	 */
	private HashMap<String,Integer> scoreSheet;
	/**
	 * Liste de Score pour l'interface graphique du client
	 */
	private final ObservableList<Score> data = FXCollections.observableArrayList();

	/**
	 * Initialise le nombre de tour a 0, et instancie un tableau de score vide
	 */
	public Bilan() {
		tour = 0;
		scoreSheet = new HashMap<>();		
	}

	/**
	 * Getter pour le numero du tour courant
	 * @return le numero du tour courant
	 */
	public int getTour() {
		return tour;
	}

	/**
	 * Permet de remplir la liste des scores a transmettre a l'interface
	 * graphique du client
	 * @return une ObservableList contenant les scores de la session
	 * courante
	 */
	public ObservableList<Score> getScoreSheet() {
		data.clear();
		for (Entry<String, Integer> e : scoreSheet.entrySet()) {
			data.add(new Score(e.getKey(), e.getValue()));
		}
		return data;
	}

	/**
	 * Permet de parser un bilan a partir de sa description textuelle
	 * @param bilan chaine de caractere representant l'etat courant 
	 * d'une session envoyee par le serveur
	 */
	public void decoderBilan(String bilan) {
		String tmp = bilan.replaceAll("\\(", ";").replaceAll("\\)", "");
		String bilanSplit[] = tmp.split(";");

		boolean tourRecupere = false;
		for (String s : bilanSplit) {
			if (!tourRecupere) {
				tour = Integer.parseInt(s);
				tourRecupere = true;
			}
			else {
				String aux[] = s.split(",");
				String user = aux[0];
				int score = Integer.parseInt(aux[1]);
				updateScore(user, score);
			}
		}
	}

	/**
	 * Met a jour le score d'un joueur
	 * @param user le nom d'utilisateur d'un joueur
	 * @param score son score
	 */
	private void updateScore(String user, int score) {
		Platform.runLater(new Runnable() {
			@Override
			public void run() {
				scoreSheet.put(user, score);
			}
		});
	}
	
	/**
	 * Permet de remettre a zero les scores d'une partie lors du lancement
	 * d'une nouvelle session
	 */
	public void reset() {
		scoreSheet.clear();
	}

	/**
	 * @author Ladislas HALIFA
	 * Classe interne permet de representer le score d'un joueur en 
	 * utilisant des SimpleProperty
	 */
	public static class Score {
	
		private final SimpleStringProperty user;
		private final SimpleIntegerProperty score;
	
		private Score(String user, Integer score){
			this.user = new SimpleStringProperty(user);
			this.score = new SimpleIntegerProperty(score);
		}
	
		public String getUser() {
			return user.get();
		}
	
		public void setUser(String u) {
			this.user.set(u);
		}
	
		public Integer getScore() {
			return score.get();
		}
	
		public void setScore(Integer sc) {
			score.set(sc);
		}
	}

}
