package rasendeRoboter;

import javafx.scene.control.Label;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.Pane;
import javafx.scene.paint.Color;

/**
 * 
 * @author Ladislas Halifa
 * Classe representant une Case du plateau, contient les informations sur les 
 * murs de cette Case, la presence d'un robot ou non sur cette case ainsi
 * que le fait que cette case soit une cible ou non
 */
public class Case {
	/**
	 * le rendu graphique de la case
	 */
	BorderPane caseGUI;
	
	/**
	 * la couleur (R, B, J, V) de la cible si cette case est une cible
	 */
	String cible;
	
	/**
	 * la couleur  (R, B, J, V) du robot present sur la case si elle en contient un
	 */
	String robot;
	
	/**
	 * Presence d'un mur sur le bord haut de la case
	 */
	private boolean haut;
	/**
	 * Presence d'un mur sur le bord bas de la case
	 */
	private boolean bas;
	/**
	 * Presence d'un mur sur le bord gauche  de la case
	 */
	private boolean gauche;
	/**
	 * Presence d'un mur sur le bord droit de la case
	 */
	private boolean droit;

	/**
	 * Initialise une case vide ne contenant pas de murs, et n'etant pas 
	 * marquee comme  une cible
	 */
	public Case() {
		caseGUI = new BorderPane();
		cible = "";
		robot = "";
		haut = false;
		bas= false;
		gauche = false;
		droit = false;
	}

	/**
	 * Verifie si la case a un mur en haut
	 * @return true si la case a un mur en haut, false sinon
	 */
	public boolean isHaut() {
		return haut;
	}

	/**
	 * Verifie si la case a un mur en bas
	 * @return true si la case a un mur en bas, false sinon
	 */
	public boolean isBas() {
		return bas;
	}

	/**
	 * Verifie si la case a un mur a gauche
	 * @return true si la case a un mur a gauche, false sinon
	 */
	public boolean isGauche() {
		return gauche;
	}

	/**
	 * Verifie si la case a un mur a droite
	 * @return true si la case a un mur a droite, false sinon
	 */
	public boolean isDroit() {
		return droit;
	}

	/**
	 * Verifie si la case est vide, i.e ne contient pas de robot
	 * @return true si la case est vide, false sinon
	 */
	public boolean isVide() {
		return robot.equals("");
	}

	/**
	 * Modifie le bord haut de la case
	 * @param haut le boolean representant le mur, true pour un mur
	 */
	private void setHaut(boolean haut) {
		this.haut = haut;
	}

	/**
	 * Modifie le bord bas de la case
	 * @param bas le boolean representant le mur, true pour un mur
	 */
	private void setBas(boolean bas) {
		this.bas = bas;
	}

	/**
	 * Modifie le bord gauche de la case
	 * @param gauche le boolean representant le mur, true pour un mur
	 */
	private void setGauche(boolean gauche) {
		this.gauche = gauche;
	}

	/**
	 * Modifie le bord droit de la case
	 * @param droit le boolean representant le mur, true pour un mur
	 */
	private void setDroit(boolean droit) {
		this.droit = droit;
	}

	/**
	 * Permet de marquer la case courante comme une cible
	 * @param cible la couleur de la cible (R, B, J, V)
	 */
	public void setCible(String cible) {
		this.cible = cible;
	}

	/**
	 * Permet de positionner un robot sur la case
	 * @param robot la couleur du robot (R, B, J, V)
	 */
	public void setRobot(String robot) {
		this.robot = robot;
	}

	/**
	 * Permet de retirer une cible de la case
	 */
	public void enleverCible() {
		cible = "";
	}

	/**
	 * Retire le robot de la case
	 */
	public void enleverRobot() {
		robot = "";
	}

	/**
	 * Detruit tout les murs de la case, la demarque si elle etait une cible,
	 * et la vide de son robot si elle en contenait un
	 */
	public void reset() {
		robot = "";
		cible = "";
		haut = false;
		bas= false;
		gauche = false;
		droit = false;
	}

	/**
	 * Permet de placer un mur sur la case 
	 * @param mur lettre representant le mur a placer
	 */
	protected void buildWall(String mur) {
		if (mur.equals("H")) {
			setHaut(true);
		}
		else if (mur.equals("B")) {
			setBas(true);
		}
		else if (mur.equals("G")) {
			setGauche(true);
		}
		else if (mur.equals("D")) {
			setDroit(true);
		}	
		else {
			System.err.println("[Case.buildWall] - je ne dois pas passer ici");
		}
	}

	/**
	 * Construit l'affichage du mur gauche de la case
	 */
	private void buildLeftWall() {
		Pane left = new Pane();
		left.setStyle("-fx-background-color: #000000;");
		left.setPrefSize(3.0, 15.0);
		caseGUI.setLeft(left);
	
	}

	/**
	 * Construit l'affichage du mur droit de la case
	 */
	private void buildRightWall() {
		Pane right = new Pane();
		right.setStyle("-fx-background-color: #000000;");
		right.setPrefSize(3.0, 15.0);
		caseGUI.setRight(right);
	}

	/**
	 * Construit l'affichage du mur haut de la case
	 */
	private void buildTopWall() {
		Pane top = new Pane();
		top.setStyle("-fx-background-color: #000000;");
		top.setPrefSize(15.0, 3.0);
		caseGUI.setTop(top);
	}

	/**
	 * Construit l'affichage du mur bas de la case
	 */
	private void buildBotWall() {
		Pane bot = new Pane();
		bot.setStyle("-fx-background-color: #000000;");
		bot.setPrefSize(15.0, 3.0);
		caseGUI.setBottom(bot);
	}

	/**
	 * Genere le code permettant d'afficher le robot sur la case, si la case
	 * contient aussi une cible, genere a la place le code permettant d'afficher
	 * a la fois le robot et la cible
	 */
	public void displayRobot() {
		Pane l = new Pane();
		Label ll;
		if (cible.equals("")) {
			switch (robot) {
			case "R":
				l.setStyle("-fx-background-color: #DC143C;");
				break;
			case "B":
				l.setStyle("-fx-background-color: #00BFFF;");
				break;
			case "J":
				l.setStyle("-fx-background-color: #FFFF00;");
				break;
			case "V":
				l.setStyle("-fx-background-color: #00FF00;");
				break;
			default:
				System.err.println("displayRobot : je ne dois pas passer ici");
				break;
			}
			caseGUI.setCenter(l);
		}
		else {
			 ll = new Label("X"+cible+"");
			switch (robot) {
			case "R":
				ll.setStyle("-fx-background-color: #DC143C; -fx-font-weight: bold; -fx-font-size: 17px;");
				break;
			case "B":
				ll.setStyle("-fx-background-color: #00BFFF; -fx-font-weight: bold; -fx-font-size: 17px;");
				break;
			case "J":
				ll.setStyle("-fx-background-color: #FFFF00; -fx-font-weight: bold; -fx-font-size: 17px;");
				break;
			case "V":
				ll.setStyle("-fx-background-color: #00FF00; -fx-font-weight: bold; -fx-font-size: 17px;");
				break;
			default:
				System.err.println("displayRobot : je ne dois pas passer ici");
				break;
			}
			caseGUI.setCenter(ll);
		}
	
	}

	/**
	 * Genere le code permettant d'afficher une cible sur la case, si la case
	 * contient aussi un robot, la methode ne fait rien du tout
	 */
	public void displayCible() {
		Label l = new Label(" X ");
		if (robot.equals("")) {
			switch (cible) {
			case "R":
				l.setTextFill(Color.CRIMSON);
				break;
			case "B":
				l.setTextFill(Color.DEEPSKYBLUE);
				break;
			case "J":
				l.setTextFill(Color.YELLOW);
				break;
			case "V":
				l.setTextFill(Color.LIME);
				break;
			default:
				System.err.println("displayCible : je ne dois pas passer ici");
				break;
			}
			l.setStyle("-fx-background-color: #FFFFFF; -fx-font-weight: bold; -fx-font-size: 20px;");
			caseGUI.setCenter(l);
		}
	}

	/**
	 * Permet de construire l'affichage de la case
	 * @return le BorderPane de la case a afficher sur le plateau
	 */
	public BorderPane render() {
		caseGUI.getChildren().clear();
		if (!cible.equals("")) 
			displayCible();
		if (!robot.equals(""))
			displayRobot();
		if (isBas())
			buildBotWall();
		if (isHaut())
			buildTopWall();
		if (isDroit())
			buildRightWall();
		if (isGauche())
			buildLeftWall();
		return caseGUI;
	}

}
