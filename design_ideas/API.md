#YOLO crypto
/**
 * Tohle rozhodne neni nic, co by se dalo povazovat za skutecny API.
 * Zejmena proto, ze vlastne nejsme dohodnuti, jak to cely pojmout.
 * Je to teda spis takovy seznam veci, ktery by ta appka mela umet
 * z hlediska funkcnosti ven. Co se tyce interni funkcnosti (sifrovaci
 * funkce, hashovaci funkce, requesty etc), je potreba to jeste promyslet
 * a na necem se dohodnout, tenhle dokument to neresi.
 */

// USER MANAGEMENT
createAccount();
deleteAccount();
changePassword();
resetPassword();


// USER ACTIVITY
// manual
signIn();
signOut();
sendMessage();
findAccount();
sendContactRequest();

// automatic
pullNotifications();
pullMessages();
