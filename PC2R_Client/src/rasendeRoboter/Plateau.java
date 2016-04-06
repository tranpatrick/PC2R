package rasendeRoboter;

import java.awt.Point;
import java.util.HashMap;
import java.util.Map.Entry;

import application.Outils;

/**
 * @author Ladislas Halifa 
 * Cette classe permet de representer l'etat d'un plateau ainsi qu'une enigme.
 */
public class Plateau {
	private Case[][] plateau;
	/**
	 * L'enigme actuelle du plateau
	 */
	private Enigme enigme;
	/**
	 * Liste associant la couleur d'un robot representee par un String,
	 *  avec sa position courante dans le plateau representee par un Point
	 */
	private HashMap<String, Point> robots;

	/**
	 * Initialise une instance de Plateau, sans enigme
	 * @param description decrit l'etat du plateau pour une session, compose
	 * d'une suite de murs
	 */
	public Plateau(String description) {
		plateau = new Case[16][16];
		for (int i=0; i < plateau.length; i++) {
			for (int j=0; j<plateau.length; j++) {
				plateau[i][j] = new Case();
			}
		}
		enigme = null;
		robots = new HashMap<String,Point>();
		parserPlateau(description);
	}

	/**
	 * Getter pour obtenir la case (i,j) du plateau
	 * @param i la ligne de la case
	 * @param j la colonne de la case
	 * @return une instance de Case
	 */
	public Case getCase(int i, int j) {
		return plateau[i][j];
	}

	/**
	 * Permet d'obtenir la position d'un robot
	 * @param color la couleur du robot (R, B, J, V)
	 * @return un Point contenant la ligne et la colonne de la position du robot
	 */
	public Point getPositionRobot(String color) {
		return robots.get(color);
	}

	/**
	 * Getter pour la ligne de la position d'un robot 
	 * @param color la couleur du robot (R, B, J, V)
	 * @return un entier compris entre 0 et 15
	 */
	public int getRobotX(String color) {
		return robots.get(color).x;
	}

	/**
	 * Getter pour la colonne de la position d'un robot 
	 * @param color la couleur du robot (R, B, J, V)
	 * @return un entier compris entre 0 et 15
	 */
	public int getRobotY(String color) {
		return robots.get(color).y;
	}

	/**
	 * Getter de l'enigme
	 * @return l'instance de l'enigme courante
	 */
	public Enigme getEnigme() {
		return enigme;
	}

	/**
	 * Prepare le plateau avec les positions des robots et de la cible d'une
	 * enigme donne
	 * @param enigme une instance d'enigme
	 */
	public void setEnigme(Enigme enigme) {
		this.enigme = enigme;
		Point cible = enigme.getCiblePosition();
		plateau[cible.x][cible.y].setCible(enigme.getCibleColor());
		placerRobot();
	}

	/**
	 * Permet de placer les robots a leurs positions indiquees par l'enigme
	 */
	private void placerRobot() {
		robots.put("R", new Point(enigme.getRouge()));
		robots.put("B", new Point(enigme.getBleu()));
		robots.put("J", new Point(enigme.getJaune()));
		robots.put("V", new Point(enigme.getVert()));
		plateau[getRobotX("R")][getRobotY("R")].setRobot("R");
		plateau[getRobotX("B")][getRobotY("B")].setRobot("B");
		plateau[getRobotX("J")][getRobotY("J")].setRobot("J");
		plateau[getRobotX("V")][getRobotY("V")].setRobot("V");
	}

	/**
	 * Permet de reinitialiser l'etat du plateau en remettant les robots a
	 * leurs position initiales
	 */
	public void initPositionsRobots() {
		for(Entry<String, Point> e : robots.entrySet()) {
			plateau[e.getValue().x][e.getValue().y].enleverRobot();
		}
		if (enigme != null)
			placerRobot();
	}

	/**
	 * Permet de retirer du plateau les robots ainsi que la cible si elle
	 * existe 
	 */
	public void enleverRobots() {
		if (enigme != null) {
			Point cible = enigme.getCiblePosition();
			plateau[cible.x][cible.y].enleverCible();
		}
		for(Entry<String, Point> e : robots.entrySet()) {
			plateau[e.getValue().x][e.getValue().y].enleverRobot();
		}
	}



	/**
	 * Effectue un deplacement d'une case a partir d'un coup
	 * @param coup le deplacement a effectue constitue de deux lettres, la 
	 * premiere represente la couleur du robot (R, B, J, V), 
	 * la deuxieme la direction du deplacement (H, B, G, D)
	 * @return true si le coup a fait deplacer le robot d'une case, false sinon
	 */
	public boolean move(String coup) {
		boolean hasMove = false;
		String color = coup.substring(0, 1);
		String direction = coup.substring(1, 2);
		Point p = getPositionRobot(color);
		if (p != null) {
			switch (direction) {
			case "G":
				if (p.y != 0 && !plateau[p.x][p.y].isGauche() && !plateau[p.x][p.y-1].isDroit() && 
				plateau[p.x][p.y-1].isVide()) {
					plateau[p.x][p.y].enleverRobot();
					p.setLocation(p.x, p.y-1);
					plateau[p.x][p.y].setRobot(color);
					robots.put(color, p);
					hasMove = true;
				}
				break;
			case "D":
				if (p.y != 15 && !plateau[p.x][p.y].isDroit() && !plateau[p.x][p.y+1].isGauche() && 
				plateau[p.x][p.y+1].isVide()) {
					plateau[p.x][p.y].enleverRobot();
					p.setLocation(p.x, p.y+1);
					plateau[p.x][p.y].setRobot(color);
					robots.put(color, p);
					hasMove = true;
				}
				break;
			case "H":
				if (p.x != 0 && !plateau[p.x][p.y].isHaut() && !plateau[p.x-1][p.y].isBas() && 
				plateau[p.x-1][p.y].isVide()) {
					plateau[p.x][p.y].enleverRobot();
					p.setLocation(p.x-1, p.y);
					plateau[p.x][p.y].setRobot(color);
					robots.put(color, p);
					hasMove = true;
				}
				break;
			case "B":
				if (p.x != 15 && !plateau[p.x][p.y].isBas() && !plateau[p.x+1][p.y].isHaut() && 
				plateau[p.x+1][p.y].isVide()) {
					plateau[p.x][p.y].enleverRobot();
					p.setLocation(p.x+1, p.y);
					plateau[p.x][p.y].setRobot(color);
					robots.put(color, p);
					hasMove = true;
				}
				break;
			default:
				System.err.println("[Plateau::move] : je ne dois pas passer ici");
				break;
			}
		}
		else {
			System.err.println("move : je ne dois pas passer ici");
		}
		return hasMove;
	}

	/**
	 * Parse une chaine de caractere decrivant un plateau pour construire
	 * les murs du plateau de jeu 
	 * @param description decrit l'etat du plateau pour une session, compose
	 * d'une suite de mur
	 */
	public void parserPlateau(String description) {
		if (Outils.isValidPlateau(description)) {
			String tmp = description.replaceAll("\\)\\(", "\\);\\(");
			String tmpTab[] = tmp.split(";");
			for (String s : tmpTab) {
				if (s.length() > 1) {
					String aux = s.substring(1, s.length()-1);
					String auxTab[] = aux.split(",");
					int i = Integer.parseInt(auxTab[0]);
					int j = Integer.parseInt(auxTab[1]);
					String mur = auxTab[2];
					plateau[i][j].buildWall(mur);
				}
			}
		}
		else {
			System.err.println("[Plateau::decoderString] plateau non valide");
		}
	}

	/**
	 * Permet de "detruire" tout les murs du plateau
	 */
	public void reset() {
		for (int i=0; i < plateau.length; i++) {
			for (int j=0; j<plateau.length; j++) {
				plateau[j][i].reset();
			}
		}
	}

	/**
	 * Redefinition de la methode toString
	 * @return la chaine de caracteres d'un plateau
	 */
	public String toString() {
		String ret = "";
		for (int i=0; i < plateau.length; i++) {
			for (int j=0; j<plateau.length; j++) {
				Case c = plateau[i][j];
				if (c.isBas())
					ret += "("+i+","+j+",B)";
				if (c.isHaut())
					ret += "("+i+","+j+",H)";
				if (c.isGauche())
					ret += "("+i+","+j+",G)";
				if (c.isDroit())
					ret += "("+i+","+j+",D)";
			}
		}
		return ret;
	}

}
