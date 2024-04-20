#include "Header.h"
#include <vector>
#include <conio.h>
#include <sstream>
#include <iostream>
#include "sqlite3.h"
#include <fstream>
#include <Windows.h>

template<typename T>
T checkInput()
{
	T val;
	while (true)
	{
		std::string input;
		std::cin >> input;

		std::stringstream ss(input);
		if (ss >> val && ss.eof())
			break;
		else
			std::cout << "Некорректный ввод! Попробуйте еще раз." << std::endl;
	}
	
	return val;
}

struct User 
{
	std::string login;
	std::string password;
	std::string accessLevel;
	
};

static int callbackApplication(void* user, int argc, char** argv, char** azColName)
{
	std::vector<Applications>* applications = static_cast<std::vector<Applications>*>(user);
	Applications app;
	app.m_date = argv[0] ? argv[0] : "";
	app.m_type = argv[1] ? argv[1] : "";
	app.m_cost = argv[2] ? argv[2] : "";
	app.m_status = argv[3] ? argv[3] : "";
	app.m_login = argv[4] ? argv[4] : "";
	applications->push_back(app);
	return 0;
}

std::string UTF8_to_CP1251(std::string const& utf8)
{
	if (!utf8.empty())
	{
		int wchlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), NULL, 0);
		if (wchlen > 0 && wchlen != 0xFFFD)
		{
			std::vector<wchar_t> wbuf(wchlen);
			MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), utf8.size(), &wbuf[0], wchlen);
			std::vector<char> buf(wchlen);
			WideCharToMultiByte(1251, 0, &wbuf[0], wchlen, &buf[0], wchlen, 0, 0);

			return std::string(&buf[0], wchlen);
		}
	}
	return std::string();
}

std::vector<Applications> getApplications(std::string login)
{
	sqlite3* db;
	char* zErrMsg = 0;
	int rc;
	std::vector<Applications> application;

	rc = sqlite3_open("Database.db", &db);

	if (rc)
	{
		std::cout << "Не могу открыть базу данных: " << sqlite3_errmsg(db) << "\n";
		return application;
	}
	std::string sql = "SELECT * FROM APPLICATIONS WHERE USERLOGIN='" + login + "';";
	rc = sqlite3_exec(db, sql.c_str(), callbackApplication, (void*)&application, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		std::cout << "SQL error: " << zErrMsg << "\n";
		sqlite3_free(zErrMsg);
		return application;
	}
	else
	{
		std::cout << "Operation done successfully\n";
	}
	sqlite3_close(db);
	return application;
}

static int callbackUser(void* user, int argc, char** argv, char** azColName) 
{
	User* u = static_cast<User*>(user);
	for (int i = 0; i < argc; i++) 
	{
		std::string column_name = azColName[i];
		if (column_name == "LOGIN") {
			u->login = argv[i] ? argv[i] : "NULL";
		}
		else if (column_name == "PASSWORD") {
			u->password = argv[i] ? argv[i] : "NULL";
		}
		else if (column_name == "ACCESSLEVEL") {
			u->accessLevel = argv[i] ? argv[i] : "NULL";
		}
	}
	return 0;
}

User getUserData(std::string login)
{
	sqlite3* db;
	char* zErrMsg = 0;
	int rc;
	User user;

	rc = sqlite3_open("Database.db", &db);

	if (rc)
	{
		std::cout << "Не могу открыть базу данных: " << sqlite3_errmsg(db) << "\n";
		return user;
	}

	std::string sql = "SELECT * from Users WHERE LOGIN = '" + login + "';";

	rc = sqlite3_exec(db, sql.c_str(), callbackUser, &user, &zErrMsg);

	if (rc != SQLITE_OK)
	{
		std::cout << "SQL ошибка: " << zErrMsg << "\n";
		sqlite3_free(zErrMsg);
	}
	sqlite3_close(db);
	return user;
}
 
int sign_in(std::string acLevel, std::string login)
{
	std::string password;
	char c;
	std::cout << "Enter password: ";
	while ((c = _getch()) != '\r')
	{
		if (c == '\b' && !password.empty())
		{
			password.pop_back();
			std::cout << "\b \b";
		}
		else if (c != '\b')
		{
			password.push_back(c);
			_putch('*');
		}
	}
	User user = getUserData(login);
	if (strcmp(login.c_str(), user.login.c_str())==0 && strcmp(user.accessLevel.c_str(), acLevel.c_str())==0 && strcmp(password.c_str(), user.password.c_str())==0)
	{
		std::cout << "Вход выполнен успешно" << std::endl;	
		return 1;
	}
	else
	{
		std::cout << "Пароль введен неверно" << std::endl;
		return 0;
	}
}

int registration()
{
	
	int choise;
	do
	{
		std::cout << "~~~~~~~~~~Меню регистрации~~~~~~~~~~\n";
		std::cout << "1. Зарегистрироваться как пользователь\n2. Зарегистрироваться как администратор\n3. Выход"<<std::endl;
		std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
		choise = checkInput<int>();
		switch (choise)
		{
			case 1:
			{
			std::string login, query;
			std::string password;
			sqlite3_stmt* stmt;
			char c;

			std::cout << "Enter login: ";
			std::cin >> login;
			std::cout << "Enter password: ";
			while ((c = _getch()) != '\r')
			{
				if (c == '\b' && !password.empty())
				{
					password.pop_back();
					std::cout << "\b \b";
				}
				else if (c != '\b')
				{
					password.push_back(c);
					_putch('*');
				}
			}

			sqlite3* db;
			int rc = sqlite3_open("Database.db", &db);

			if (rc) {
				std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
				return 1;
			}
			else {
				std::cout << "Opened database successfully" << std::endl;
			}
			query = "INSERT INTO USERS (LOGIN, PASSWORD) VALUES(?, ?);";
			rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Prepare" << std::endl;
				return (-1);
			}

			rc = sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}

			rc = sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}

			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				std::cerr << "Error Execute" << std::endl;
				std::cout << "Пользователь с таким логином уже существует!" << std::endl;
				return (-1);
			}

			std::cout << "Records created Successfully!" << std::endl;

			sqlite3_finalize(stmt);
			sqlite3_close(db);

			break;
			}
			case 2:
			{
				std::string login, query;
				std::string password;
				sqlite3_stmt* stmt;
				char c;
				std::fstream file;
				std::string line, temp;
				file.open("AdminRegistrationPassword.txt", std::ios::in);

				if (file.is_open())
				{
					getline(file, line);
					file.close();
				}
				else
				{
					std::cout << "Не удалось открыть файл" << std::endl;
				}
				std::cout << "Enter login: ";
				std::cin >> login;
				std::cout << "Enter password: ";
				while ((c = _getch()) != '\r')
				{
					if (c == '\b' && !password.empty())
					{
						password.pop_back();
						std::cout << "\b \b";
					}
					else if (c != '\b')
					{
						password.push_back(c);
						_putch('*');
					}
				}
				std::cout << std::endl;
				std::cout << "Введите ключевое слово для регистрации администратора" << std::endl;
				std::cin >> temp;
				if (!strcmp(line.c_str(), temp.c_str()))
				{
					sqlite3* db;
					int rc = sqlite3_open("Database.db", &db);

					if (rc) {
						std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
						return 1;
					}
					else {
						std::cout << "Opened database successfully" << std::endl;
					}
					query = "INSERT INTO USERS (LOGIN, PASSWORD, ACCESSLEVEL) VALUES(?, ?, ?);";
					rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
					if (rc != SQLITE_OK)
					{
						std::cerr << "Error Prepare" << std::endl;
						return (-1);
					}

					rc = sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
					if (rc != SQLITE_OK)
					{
						std::cerr << "Error Bind" << std::endl;
						return (-1);
					}

					rc = sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_STATIC);
					if (rc != SQLITE_OK)
					{
						std::cerr << "Error Bind" << std::endl;
						return (-1);
					}

					rc = sqlite3_bind_int(stmt, 3, 2);
					if (rc != SQLITE_OK)
					{
						std::cerr << "Error Bind" << std::endl;
						return (-1);
					}
					rc = sqlite3_step(stmt);
					if (rc != SQLITE_DONE)
					{
						std::cerr << "Error Execute" << std::endl;
						std::cout << "Пользователь с таким логином уже существует!" << std::endl;
						return (-1);
					}

					std::cout << "Records created Successfully!" << std::endl;

					sqlite3_finalize(stmt);
					sqlite3_close(db);
				}
				else
				{
					std::cout << "Ключевые слова не совпадают" << std::endl;
				};
				break;
			}
			default:
				break;
		}
	} while (choise != 3);
	return 0;
}

std::string cp1251_to_utf8(const char* str) 
{
	std::string res;
	int result_u, result_c;
	result_u = MultiByteToWideChar(1251, 0, str, -1, 0, 0);
	if (!result_u) { return 0; }
	wchar_t* ures = new wchar_t[result_u];
	if (!MultiByteToWideChar(1251, 0, str, -1, ures, result_u)) {
		delete[] ures;
		return 0;
	}
	result_c = WideCharToMultiByte(65001, 0, ures, -1, 0, 0, 0, 0);
	if (!result_c) {
		delete[] ures;
		return 0;
	}
	char* cres = new char[result_c];
	if (!WideCharToMultiByte(65001, 0, ures, -1, cres, result_c, 0, 0)) {
		delete[] cres;
		return 0;
	}
	delete[] ures;
	res.append(cres);
	delete[] cres;
	return res;
}

int applicationSubmission(std::string login)
{
		std::cout << "Введите дату (дд.мм.гг), тип расходов, стоимость" << std::endl;
		std::string date, type;
		int cost;
		date = checkInput<std::string>();
		type = checkInput<std::string>();
		cost = checkInput<int>();
		type = cp1251_to_utf8(type.c_str());
		std::cout << "Вы уверены в выборе?\n1. Да\n2. Нет\n" << std::endl;
		int choise;
		choise = checkInput<int>();
		if (choise == 1)
		{
			std::string query;
			sqlite3_stmt* stmt;
			sqlite3* db;
			int rc = sqlite3_open("Database.db", &db);
			if (rc)
			{
				std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
				return 1;
			}
			else
			{
				std::cout << "Opened database successfully" << std::endl;
			}
			query = "INSERT INTO APPLICATIONS (USERLOGIN, DATE, COSTNAME, COST) VALUES(?, ?, ?, ?);";
			rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Prepare" << std::endl;
				return (-1);
			}
			rc = sqlite3_bind_text(stmt, 1, login.c_str(), -1, SQLITE_STATIC);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}
			rc = sqlite3_bind_text(stmt, 2, date.c_str(), -1, SQLITE_STATIC);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}
			rc = sqlite3_bind_text(stmt, 3, type.c_str(), -1, SQLITE_STATIC);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}
			rc = sqlite3_bind_int(stmt, 4, cost);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}
			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				std::cerr << "Error Execute" << std::endl;
				return (-1);
			}

			std::cout << "Records created Successfully!" << std::endl;

			sqlite3_finalize(stmt);
			sqlite3_close(db);
		}
	return 0;
}

std::vector<Applications> getPendingApplications()
{
	sqlite3* db;
	char* zErrMsg = 0;
	int rc;
	std::vector<Applications> application;

	rc = sqlite3_open("Database.db", &db);

	if (rc)
	{
		std::cout << "Не могу открыть базу данных: " << sqlite3_errmsg(db) << "\n";
		return application;
	}
	std::string sql = "SELECT * FROM APPLICATIONS WHERE STATUS=1;";
	rc = sqlite3_exec(db, sql.c_str(), callbackApplication, (void*)&application, &zErrMsg);
	if (rc != SQLITE_OK)
	{
		std::cout << "SQL error: " << zErrMsg << "\n";
		sqlite3_free(zErrMsg);
		return application;
	}
	else
	{
		std::cout << "Operation done successfully\n";
	}
	sqlite3_close(db);
	return application;
}

int comfirmApplication(const std::vector<Applications>& applications)
{
	std::cout << "Введите номер заяки для подтверждения" << std::endl;
	int appNum;
	appNum = checkInput<int>() - 1;
	std::cout << "\nВы уверены в своем выборе?\n1. Да\n2. Нет" << std::endl;
	int choise;
	choise = checkInput<int>();
	
	if (choise == 1)
	{
		if (calculateBudget(applications[appNum].m_cost) == 0)
		{
			std::string query;
			sqlite3_stmt* stmt;
			sqlite3* db;
			int rc = sqlite3_open("Database.db", &db);
			if (rc)
			{
				std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
				return 1;
			}
			else
			{
				std::cout << "Opened database successfully" << std::endl;
			}
			query = "UPDATE APPLICATIONS SET STATUS = ? WHERE DATE='" + cp1251_to_utf8(applications[appNum].m_date.c_str()) + "' AND COST='" + cp1251_to_utf8(applications[appNum].m_cost.c_str()) + "' AND COSTNAME= '" + cp1251_to_utf8(applications[appNum].m_type.c_str()) + "' AND USERLOGIN= '" + cp1251_to_utf8(applications[appNum].m_login.c_str()) + "';";
			rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Prepare" << std::endl;
				return (-1);
			}
			rc = sqlite3_bind_int(stmt, 1, 2);
			if (rc != SQLITE_OK)
			{
				std::cerr << "Error Bind" << std::endl;
				return (-1);
			}
			rc = sqlite3_step(stmt);
			if (rc != SQLITE_DONE)
			{
				std::cerr << "Error Execute" << std::endl;
				return (-1);
			}

			std::cout << "Records created Successfully!" << std::endl;

			sqlite3_finalize(stmt);
			sqlite3_close(db);
		}
	}
	return 0;
}

int deleteApplication(const std::vector<Applications>& applications)
{
	std::cout << "Введите номер заяки для отклонения" << std::endl;
	int appNum;
	appNum = checkInput<int>() - 1;
	std::cout << "\nВы уверены в своем выборе?\n1. Да\n2. Нет" << std::endl;
	int choise;
	choise = checkInput<int>();
	if (choise == 1)
	{
		std::string query;
		sqlite3_stmt* stmt;
		sqlite3* db;
		int rc = sqlite3_open("Database.db", &db);
		if (rc)
		{
			std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
			return 1;
		}
		else
		{
			std::cout << "Opened database successfully" << std::endl;
		}
		query = "DELETE FROM APPLICATIONS WHERE DATE='" + cp1251_to_utf8(applications[appNum].m_date.c_str()) + "' AND COST='" + cp1251_to_utf8(applications[appNum].m_cost.c_str()) + "' AND COSTNAME= '" + cp1251_to_utf8(applications[appNum].m_type.c_str()) + "' AND USERLOGIN= '" + cp1251_to_utf8(applications[appNum].m_login.c_str()) + "';";
		rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Error Prepare" << std::endl;
			return (-1);
		}
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			std::cerr << "Error Execute" << std::endl;
			return (-1);
		}

		std::cout << "Records created Successfully!" << std::endl;

		sqlite3_finalize(stmt);
		sqlite3_close(db);
	}
	return 0;
}

int showUserData(std::string login)
{
	User user;
	user = getUserData(login);
	std::cout << "Логин пользователя: " << user.login << std::endl;
	std::cout << "Пароль пользователя: " << user.password << std::endl;
	std::cout << "Уровень доступа пользователя: " << user.accessLevel << std::endl;
	return 0;
}

int setUserData()	
{
	std::string query, login;
	sqlite3_stmt* stmt;
	sqlite3* db;
	std::cout << "Введите логин пользователя" << std::endl;
	std::cin >> login;
	User user, newUserData;
	user = getUserData(login);
	std::cout << "Введите новый логин и пароль пользователя" << std::endl;
	std::cin >> newUserData.login >> newUserData.password;
	int choise;
	std::cout << "Вы уверены в выборе?\n1. Да\n2. Нет" << std::endl;
	choise = checkInput<int>();
	if (choise == 1)
	{
		int rc = sqlite3_open("Database.db", &db);
		if (rc)
		{
			std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
			return 1;
		}
		else
		{
			std::cout << "Opened database successfully" << std::endl;
		}
		query = "UPDATE Users SET PASSWORD= ?, LOGIN= ? WHERE LOGIN= '" + user.login + "'";
		rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, 0);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Error Prepare" << std::endl;
			return (-1);
		}
		rc = sqlite3_bind_text(stmt, 1, newUserData.password.c_str(), -1, SQLITE_STATIC);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Error Bind" << std::endl;
			return (-1);
		}
		rc = sqlite3_bind_text(stmt, 2, newUserData.login.c_str(), -1, SQLITE_STATIC);
		if (rc != SQLITE_OK)
		{
			std::cerr << "Error Bind" << std::endl;
			return (-1);
		}
		rc = sqlite3_step(stmt);
		if (rc != SQLITE_DONE)
		{
			std::cerr << "Error Execute" << std::endl;
			return (-1);
		}

		std::cout << "Records created Successfully!" << std::endl;

		sqlite3_finalize(stmt);
		sqlite3_close(db);
	}
	return 0;
}

int getBudget()
{
	std::string line;
	std::fstream file;
	file.open("Budget.txt", std::ios::in);

	if (file.is_open())
	{
		getline(file, line);
		file.close();
	}
	else
	{
		std::cout << "Не удалось открыть файл" << std::endl;
	}
	int Budget = std::stoi(line);
	return Budget;
}

int changeBudget()
{
	int choise, budget;
	std::cout << "Введите новый бюджет" << std::endl;
	budget = checkInput<int>();
	std::cout << "Вы уверены в выборе?\n1. Да\n2. Нет" << std::endl;
	choise = checkInput<int>();
	if (choise == 1)
	{
		std::fstream file;
		file.open("Budget.txt", std::ofstream::out | std::ofstream::trunc);

		if (file.is_open())
		{
			file << budget;
			file.close();
		}
		else
		{
			std::cout << "Не удалось открыть файл" << std::endl;
		}
	}
	return 0;
}
int calculateBudget(std::string cost)
{
	int budget = getBudget();
	int Cost = std::stoi(cost);
	if (budget > Cost)
	{
		budget = budget - Cost;
		std::fstream file;
		file.open("Budget.txt", std::ofstream::out | std::ofstream::trunc);

		if (file.is_open())
		{
			file << budget;
			file.close();
		}
		else
		{
			std::cout << "Не удалось открыть файл" << std::endl;
		}
		return 0;
	}
	else
	{
		std::cout << "Недостаточно бюджета" << std::endl;
		return 1;
	}
}