package bot;

import java.util.Random;

/**
 * 
 * @author Ladislas Halifa
 * Cette classe contient des constantes pour le Bot, ainsi que quelques 
 * methodes pour choisir une constante aleatoirement
 */
public class BotData {

	private static final Random rand = new Random(System.currentTimeMillis());

	private static final String[] NAMES = 
		{
				"akali",
				"annie",
				"ashe",
				"braum",
				"caitlyn",
				"diana",
				"draven",
				"mundo",
				"ekko",
				"elise",
				"evelynn",
				"fiora",
				"graves",
				"heimer",
				"jayce",
				"kennen",
				"katarina",
				"leona",
				"morgana",
				"poppy",
				"quinn",
				"riven",
				"sivir",
				"shen",
				"ryze",
				"sona",
				"soraka",
				"tahm",
				"miss fortune",
				"teemo",
				"twisted fate",
				"vayne",
				"final boss veigar",
				"xin",
				"yasuo",
				"ziggs",
				"zac",
				"garen",
				"jax",
				"lux",
				"darius",
				"lucian",
		};

	public static String getName() {
		return NAMES[rand.nextInt(NAMES.length)];	
	}

	public static final String MESSAGE_BIENVENUE = "Salut";
	public static final String MESSAGE_BIENVENUE_2 = "Bonjour";
	public static final String MESSAGE_BIENVENUE_3 = "Yo";
	public static final String MESSAGE_BIENVENUE_4 = "Hello";
	public static final String MESSAGE_HI = "Hi i'm "+Bot.myName;


	public static final String[] BIENVENUE = 
		{
				MESSAGE_BIENVENUE, 
				MESSAGE_BIENVENUE_2, 
				MESSAGE_BIENVENUE_3,
				MESSAGE_BIENVENUE_4,
		};

	public static final String MESSAGE_OK = "ok";
	public static final String MESSAGE_OK_1= "d'accord";
	public static final String MESSAGE_OK_2= "dac";
	public static final String MESSAGE_OK_3= "c'est vrai";
	public static final String MESSAGE_OK_4= "j'avoue";
	public static final String MESSAGE_OK_5= "oui";
	public static final String MESSAGE_OK_6= "non";

	public static final String[] OK = 
		{
				MESSAGE_OK,
				MESSAGE_OK_1,
				MESSAGE_OK_2, 
				MESSAGE_OK_3,
				MESSAGE_OK_4,
				MESSAGE_OK_5,
				MESSAGE_OK_6
		};

	public static final String MESSAGE_PROVOC = "Trop facile";
	public static final String MESSAGE_PROVOC_2 = "beaucoup trop simple pour moi";
	public static final String MESSAGE_PROVOC_3 = "Hmm interessant";
	public static final String MESSAGE_PROVOC_4 = "celle la elle est pour moi";
	public static final String MESSAGE_PROVOC_5 = "facile";
	public static final String MESSAGE_PROVOC_6 = "pas facile cette enigme";
	public static final String[] PROVOC = 
		{
				MESSAGE_PROVOC,
				MESSAGE_PROVOC_6,
				MESSAGE_PROVOC_2, 
				MESSAGE_PROVOC_3,
				MESSAGE_PROVOC_4,
				MESSAGE_PROVOC_5
		};

	public static final String MESSAGE_GG = "Bien joue";
	public static final String MESSAGE_GG_1 = "Bravo";
	public static final String MESSAGE_GG_2 = "GG";
	public static final String MESSAGE_GG_3 = "Pas mal du tout";
	public static final String MESSAGE_GG_4 = "Pff";

	public static final String[] GG = 
		{
				MESSAGE_GG,
				MESSAGE_GG_1,
				MESSAGE_GG_2, 
				MESSAGE_GG_3,
				MESSAGE_GG_4
		};

	public static final String MESSAGE_DMG = "Bien essaye";
	public static final String MESSAGE_DMG_1 = "Dommage";
	public static final String MESSAGE_DMG_2 = "Bien tente";
	public static final String MESSAGE_DMG_3 = "T'etais pas loin";


	public static final String[] DMG = 
		{
				MESSAGE_DMG,
				MESSAGE_DMG_1,
				MESSAGE_DMG_2, 
				MESSAGE_DMG_3,
		};

	public static final String MESSAGE_BYE = "A plus";
	public static final String MESSAGE_BYE_1 = "Bye";
	public static final String MESSAGE_BYE_2 = "Au revoir";
	public static final String MESSAGE_BYE_3 = "++";
	public static final String MESSAGE_BYE_4 = "J'y vais";

	public static final String[] BYE = 
		{
				MESSAGE_BYE,
				MESSAGE_BYE_1,
				MESSAGE_BYE_2, 
				MESSAGE_BYE_3,
				MESSAGE_BYE_4,
		};

	public static String getMessageBienvenue(){
		return BIENVENUE[rand.nextInt(BIENVENUE.length)];
	}


	public static String getMessageOK(){
		return OK[rand.nextInt(OK.length)];	
	}

	public static String getMessageProvoc(){
		return PROVOC[rand.nextInt(PROVOC.length)];	
	}

	public static String getMessageFelicitation(){
		return GG[rand.nextInt(GG.length)];	
	}

	public static String getMessageBienTente() {
		return DMG[rand.nextInt(DMG.length)];	
	}

	public static String getMessageBye(){
		return BYE[rand.nextInt(BYE.length)];	
	}

}
