package application;

import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.Label;
import javafx.scene.control.TableView;
import javafx.scene.control.TextArea;
import javafx.scene.control.TextField;
import javafx.scene.layout.BorderPane;
import javafx.scene.layout.GridPane;
import javafx.scene.layout.VBox;
import javafx.scene.text.Text;
import rasendeRoboter.Bilan.Score;

/**
 * 
 * @author Ladislas Halifa
 * Classe necessaire pour Game.fxml, mais rien n'est fait ici
 * 
 */
public class GameController {
	/**
	 * Conteneur principal
	 */
	@FXML private BorderPane mainFrame;
	
	/**
	 * zone de saisie du nom d'utilisateur
	 */
    @FXML private TextField userTextField;

    /**
     * bouton de connexion
     */
    @FXML private Button loginButton;

    /**
     * zone de saisie de l'adresse du serveur
     */
    @FXML private TextField hostTextField;

    
    /**
     * zone d'affichage d'un message d'erreur
     */
    @FXML private Text errorMessageText;
	
	/**
	 * zone de saisie du chat
	 */
	@FXML private TextArea sendChatTextArea;
	
	/**
	 * Bouton pour envoyer message
	 */
	@FXML private Button sendChatButton;
	
	/**
	 * zone d'affichage des messages du chat
	 */
	@FXML private TextArea chatTextArea;
	
	/**
	 * zone d'affichage de l'adresse du serveur
	 */
	@FXML private Label hostAdressLabel;
	

	@FXML private Label version;
	
	/**
	 * Bouton de deconnexion
	 */
	@FXML private Button logoutButton;
	
	/**
	 * zone d'affichage du plateau de jeu
	 */
	@FXML private GridPane plateauGrid;
	
	/**
	 * Bouton pour Encherir ou annonce que l'on a trouve une solution
	 */
	@FXML private Button trouveEnchereButton;

	/**
	 * zone de saisie du nombre de coups
	 */
	@FXML private TextField coupTextField;
    
	/**
	 * zone d'affichage d'erreur sur la saisie d'un nombre de coup
	 */
	@FXML private Label errorLabel;
    
	/**
	 * zone d'affichage du numero du tour actuel
	 */
    @FXML private Label tourLabel;
    
    /**
     * zone d'affichage des score
     */
    @FXML private TableView<Score> scoreTableView;
    
    /**
     * zone de saisie et d'affichage d'une solution en cours de saisie
     */
    @FXML private TextArea solutionTextArea;
    
    /**
     * Bouton pour soumettre une solution
     */
    @FXML private Button solutionButton;

    /**
     * Zone d'affichage des messages recus
     */
    @FXML private TextArea serverAnswer;
    
    /**
     * Conteneur de la zone de saisie d'une solution
     */
    @FXML private VBox solutionVBox;
    
    /**
     * zone d'affichage du nombre de coups d'une solution
     */
    @FXML private Label coupsSolutionLabel;
    
    /**
     * zone d'affichage de la phase courante
     */
    @FXML private Label phaseLabel;
}