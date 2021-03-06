/*
 * api documentation
 * will be changed probably
 * #TODO
 * class declarations?
 *
 *
 */
#include <string>
#include <vector>


/**
 * Creates an account
 * @param mail		unique mail
 * @param name		name, does not have to be unique
 * @param password	password
 * @return positive id if successfull, 0 otherwise
 */
size_t createAccount(std::string &mail, std::string &password, std::string &name);

/**
 * deletes an account
 * @param id		id of an account
 * @return 0 if successfull
 */
bool deleteAccount(size_t id);

/**
 * changes password of an user
 * @param id		id of an user
 * @param oldp		old password
 * @param newp		new password
 * @return 0 if successfull
 */
int changePassword(size_t id, std::string &oldp, std::string &newp);

/**
 * lists users into stream
 * @param users		where to write users
 */
void listUsers(std::ostream &os);

/**
 * find an account with name
 * @param name		name of an account
 * @return id if exists, empty 0 otherwise
 */
size_t findAccount(std::string &name);

/**
 * sends message to another user
 * @param id		id of another user
 * @param message	message to be sent
 */
void sendMessage(size_t &id, std::string &message);

/**
 * sends a contact request to another user
 * @param mail		mail of another user
 */
void sendContactRequest(size_t &id);

/**
 * accepts a contact request from another user
 * @param mail		mail of another user
 */
void acceptContactRequest(size_t &id);

/**
 * denies a contact request from another user
 * @param mail		mail of another user
 */
void denyContactRequest(size_t &id);

/**
 * pulls messages from server
 * @param id		id of user, whose messages to pull
 */
void pullMessages(size_t id);

/**
 * pulls notifications from server
 * @param id		id of user, whose notifications to pull
 */
void pullNotifications(size_t id);

/**
 * signs in an user that is not online
 * @param mail		mail of an user
 * @param password	password of the user
 * @return true if successfull
 */
bool signIn(std::string &mail, std::string &password);

/**
 * signs out an user
 * @param mail		mail of an user
 * @return true if successfull
 */
bool signOut(std::string &mail);
