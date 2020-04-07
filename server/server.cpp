#include "server.hpp"
#include "util.hpp"
#include <sstream>
#include <cstring>
#include "message.pb.h"
#include <dirent.h>
#include <sys/types.h>
#include <fstream>
#include <cstdio>

//#define __cplusplus




void Account::print(std::ostream &os) const
{
	os << "id: " << id_ << std::endl;
	os << "login: " << login_ << std::endl;
	os << "name: " << displayName_ << std::endl;

}

unsigned int Server::createAccount(const std::string &login, const std::string &pwdHash, const std::string &name)
{
	for (auto &acc : database_) {
		if (acc.second.getLogin() == login) {
			return 0;
		}
	}

	database_.emplace(id_, Account(id_, login, pwdHash, name));
	++id_;

	return id_ - 1;
}

bool Server::deleteAccount(unsigned int id)
{
	auto it = database_.find(id);

	if (it == database_.end()) {
		return false;
	}

	database_.erase(it);

	return true;
}

bool Server::signIn(const std::string &login, const std::string &pwdHash, const unsigned int challenge)
{
	unsigned int id = 0;
	std::string dbHash;

	for (auto &acc : database_) {
		if (acc.second.getLogin() == login) {
			id = acc.first;
			dbHash = acc.second.getPwdHash();
			break;
		}
	}

	dbHash = dbHash + std::to_string(challenge);

	if (id == 0 || online_.find(id) != online_.end()) {
		return false;
	}

	std::string hash;

	Util::hash512(dbHash, dbHash.size(), hash);

	if (hash != pwdHash) {
		return false;
	}

	online_.insert(id);
	return true;
}

bool Server::signOut(unsigned int id)
{
	auto it = online_.find(id);

	if (it == online_.end()) {
		return false;
	}

	online_.erase(it);

	return true;
}

void Server::listUsers(std::ostream &os) const
{
	os << "Registered users:" << std::endl;

	for (auto &acc : database_) {
		acc.second.print(os);
	}

	os << std::endl;
}

void Server::listOnline(std::ostream &os) const
{
	os << "Online users" << std::endl;

	for (auto id: online_) {
		database_.find(id)->second.print(os);
	}

	os << std::endl;
}

void Server::test()
{
	//Message msg;
	//Message msg2;
	//msg.set_recid(42);
	//msg.set_senid(32);
	//msg.clear_textcontent();
	//msg.set_textcontent("this is some text");
	//msg.add_kdfkeys("key0");
	//msg.add_kdfkeys("key1");
	//msg.add_kdfkeys("key2");
	//msg.add_kdfkeys("key3");

	//std::cout << msg.DebugString() << std::endl;

	//std::string str;

	//msg.SerializeToString(&str);

	//std::cout << "serialized message = " << str << std::endl;

	//msg2.ParseFromString(str);

	//std::cout << std::endl;

	//std::cout << msg2.DebugString() << std::endl;
	//std::cout << msg2.textcontent() << std::endl;


	/* mbedtls static include tests */

	//unsigned char hash[64];

	//unsigned char toHash[6] = { 'a', 'a', 'k', 'k', 'a', 'a' };

	//mbedtls_sha512_ret(toHash, 6, hash, 0);

	//std::stringstream buff;

	//buff << std::hex;

	//for (size_t i = 0; i < 64; ++i) {
	//        buff << std::setfill('0') << std::setw(2) << static_cast<unsigned>(hash[i]);
	//}

	//buff << std::dec;

	//std::cout << buff.str() << std::endl;
	std::cout << "It works!" << std::endl;

	//mbedtls_ecp_group grp;
	//mbedtls_ecp_group_init(&grp);
	//mbedtls_ecp_group_load(&grp, mbedtls_ecp_group_id::MBEDTLS_ECP_DP_CURVE25519);

	//mbedtls_mpi priv;
	//mbedtls_ecp_point point;
	//mbedtls_ecp_point_init(&point);
	//mbedtls_mpi_init(&priv);

	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_entropy_context entropy;


	mbedtls_ctr_drbg_init(&ctr_drbg);
	mbedtls_entropy_init(&entropy);

	mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned char *) "RANDOM_GEN", 10);


	//mbedtls_ecp_gen_keypair(&grp, &priv, &point, mbedtls_ctr_drbg_random, &ctr_drbg);

	//size_t len;
	//char private_buff[512];
	//char public_x_buff[512];
	//char public_y_buff[512];
	//char public_z_buff[512];

	//mbedtls_mpi_write_string(&priv, 10, private_buff, 512, &len);
	//mbedtls_mpi_write_string(&point.X, 10, public_x_buff, 512, &len);
	//mbedtls_mpi_write_string(&point.Y, 10, public_y_buff, 512, &len);
	//mbedtls_mpi_write_string(&point.Z, 10, public_z_buff, 512, &len);

	//std::cout << "some private key:  " << private_buff << std::endl;
	//std::cout << "some public key x: " << public_x_buff << std::endl;
	//std::cout << "some public key y: " << public_y_buff << std::endl;
	//std::cout << "some public key z: " << public_z_buff << std::endl;

	//mbedtls_ecp_keypair pair0;
	//pair0.d = priv;
	//pair0.Q = point;
	//pair0.grp = grp;

	//Util::printKeyPair(&pair0);

	//mbedtls_ecp_keypair_free(&pair0);


	/* cleanup */
	//mbedtls_ecp_group_free(&grp);
	//mbedtls_ecp_point_free(&point);
	//mbedtls_mpi_free(&priv);
	//mbedtls_ctr_drbg_free(&ctr_drbg);
	//mbedtls_entropy_free(&entropy);


	mbedtls_ecp_keypair pair1;
	mbedtls_ecp_keypair pair2;

	mbedtls_ecp_keypair_init(&pair1);
	mbedtls_ecp_keypair_init(&pair2);


	Util::generateKeyPair(&pair1);
	Util::generateKeyPair(&pair2);

	Util::printKeyPair(&pair1);
	Util::printKeyPair(&pair2);


	mbedtls_mpi shared1;
	mbedtls_mpi shared2;

	mbedtls_mpi_init(&shared1);
	mbedtls_mpi_init(&shared2);

	/* dh exchange */



	int r;
	r = mbedtls_ecdh_compute_shared(&pair1.grp, &shared1, &pair2.Q, &pair1.d, mbedtls_ctr_drbg_random, &ctr_drbg);
	std::cout << "retval = " << r << std::endl;
	r = mbedtls_ecdh_compute_shared(&pair2.grp, &shared2, &pair1.Q, &pair2.d, mbedtls_ctr_drbg_random, &ctr_drbg);
	std::cout << "retval = " << r << std::endl;


	size_t len;
	char shared1_buf[256];
	char shared2_buf[256];

	mbedtls_mpi_write_string(&shared1, 10, shared1_buf, 256, &len);
	mbedtls_mpi_write_string(&shared2, 10, shared2_buf, 256, &len);

	std::cout << "shared1: " << shared1_buf << std::endl;
	std::cout << "shared2: " << shared2_buf << std::endl;

	/* signing */
	mbedtls_ecp_keypair sig;
	mbedtls_ecp_keypair_init(&sig);

	Util::signPubKey(&pair1, &sig);


	/* cleanup */
	mbedtls_ctr_drbg_free(&ctr_drbg);
	mbedtls_entropy_free(&entropy);

	mbedtls_mpi_free(&shared1);
	mbedtls_mpi_free(&shared2);

	mbedtls_ecp_keypair_free(&pair1);
	mbedtls_ecp_keypair_free(&pair2);
	mbedtls_ecp_keypair_free(&sig);
}

void Server::checkRequests()
{
	std::string path = "tmp_files/req";
	std::vector<std::string> requestFiles;
	std::vector<std::string> locks;

	struct dirent *entry;
    DIR *dir = opendir("tmp_files/req");
    if (dir == NULL) {
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
		std::string fname = entry->d_name;

		if (fname == "." || fname == ".." || fname == "_test") {
			continue;
		}
		if (fname.size() < 5) {
			requestFiles.push_back(fname);
			continue;
		}
		if (fname.substr(fname.size() - 4, 4) == "lock") {
			locks.push_back(fname);
		} else {
			requestFiles.push_back(fname);
		}
    }
    closedir(dir);

	for (int i = 0; i < requestFiles.size(); ++i) {
		std::string req = requestFiles[i];
		bool locked = false;
		for (const auto & lock : locks) {
			if (lock == req) {
				locked = true;
				break;
			}
		}
		if (locked) {
			continue;
		}
		std::ofstream lockFile(path + "/" + req + ".lock"); // lock the file
		lockFile.close();

		std::ifstream reqFile(path + "/" + req); // open requests
		std::string line;
		while (std::getline(reqFile, line)) {
			int delim = line.find_first_of(';');
			std::string command = line.substr(0, delim);

			std::string tmp = line.substr(delim + 1, line.size() - 1 - delim);

			delim = tmp.find_first_of(';');

			std::string recieverAndMessage = tmp.substr(delim + 1, tmp.size() - 1 - delim);

			delim = recieverAndMessage.find_first_of(';');
			std::string reciever = recieverAndMessage.substr(0, delim);

			requests_.emplace_back(reciever, line); // store request
		}
		reqFile.close();
		std::ofstream eraseReqFile(path + "/" + req, std::ofstream::out | std::ofstream::trunc);
		eraseReqFile.close();

		std::string a = path + "/" + req + ".lock";
		std::remove(a.c_str()); // unlock the file
	}
}

void Server::processRequests()
{
	std::string path = "tmp_files/res";
	std::vector<std::string> responseFiles;
	std::vector<std::string> locks;

	struct dirent *entry;
    DIR *dir = opendir("tmp_files/res");
    if (dir == NULL) {
        return;
    }
    while ((entry = readdir(dir)) != NULL) {
		std::string fname = entry->d_name;

		if (fname == "." || fname == ".." || fname == "_test") {
			continue;
		}
		if (fname.size() < 5) {
			responseFiles.push_back(fname);
			continue;
		}
		if (fname.substr(fname.size() - 4, 4) == "lock") {
			locks.push_back(fname);
		} else {
			responseFiles.push_back(fname);
		}
    }
    closedir(dir);

	std::vector<int> solvedIndex;
	for (int i = 0; i < responseFiles.size(); ++i) {
		std::string res = responseFiles[i];
		bool locked = false;
		for (const auto & lock : locks) {
			if (lock == res) {
				locked = true;
				break;
			}
		}
		if (locked) {
			continue;
		}

		std::ofstream LockFile(path + "/" + res + ".lock"); // lock the file
		LockFile.close();

		std::ofstream ResFile(path + "/" + res, std::ofstream::out | std::ofstream::app); // open the response file
		for (int i = 0; i < requests_.size(); ++i) {
			if (requests_[i].first == res) {
				int delim = requests_[i].second.find_first_of(';');
				std::string command = requests_[i].second.substr(0, delim);

				std::string tmp = requests_[i].second.substr(delim + 1);

				delim = tmp.find_first_of(';');
				std::string sender = tmp.substr(0, delim);
				tmp = tmp.substr(delim + 1);
				delim = tmp.find_first_of(';');
				std::string message = tmp.substr(delim + 1);

				if (command == "sendMessage") {
					ResFile << "recieveMessage;" << sender << ";" << message << std::endl;
				}

				solvedIndex.push_back(i);
			}
		}

		ResFile.close();

		std::string a = path + "/" + res + ".lock";
		std::remove(a.c_str()); // unlock the file
	}

	for (int i = solvedIndex.size() - 1; i > -1; --i) { // remove solved requests from requests_
		requests_.erase(requests_.begin() + i);
	}
}
