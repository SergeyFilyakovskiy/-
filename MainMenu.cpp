#include <sstream>
#include <iostream>
#include "sqlite3.h"
#include <Windows.h>
#include <vector>
#include <conio.h>
#include "Header.h"

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
int OutputApp(const std::vector<Applications>& applications)
{
	int i=1;
	for (const auto& application : applications)
	{
		std::cout << "Номер: " << i << std::endl;
		std::cout << "Дата: " << UTF8_to_CP1251(application.m_date) << std::endl;
		std::cout << "Тип: " << UTF8_to_CP1251(application.m_type) << std::endl;
		std::cout << "Цена: " << UTF8_to_CP1251(application.m_cost) << std::endl;
		std::cout << "Статус: " << UTF8_to_CP1251(application.m_status) << std::endl;
		std::cout << "Имя пользователя: " << UTF8_to_CP1251(application.m_login) << std::endl;
		std::cout << "------------------------\n";
		i++;
	}
	return 0;
}
int main()
{
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);

	int choise;
	do
	{
		std::cout << "~~~~~~~~Вход в систему~~~~~~~~\n";
		std::cout << "1. Войти как пользователь\n2. Войти как администратор\n3. Зарегистрироваться\n4. Выход из программы\n";
		std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
		choise = checkInput<int>();
		switch (choise)
		{
		case 1:
		{
			std::string login;
			std::cout << "Enter login: ";
			std::cin >> login;
			std::vector<Applications> appl;
			if (sign_in("1", login))
			{
				do
				{
					std::cout << "=========Меню пользователя=========" << std::endl;
					std::cout << "1. Подать заявку\n2. Просмотреть историю заявок\n3. Выход" << std::endl;
					std::cout << "===================================" << std::endl;
					choise = checkInput<int>();
					switch (choise)
					{
					case 1:
					{
						applicationSubmission(login);
						break;
					}

					case 2:
					{
						appl = getApplications(login);
						OutputApp(appl);
						break;
					}

					default:
						break;
					}
				} while (choise != 3);
			}
			else break;
			break;
		}
		case 2:
		{
			std::string login;
			std::cout << "Enter login: ";
			std::cin >> login;
			int choise;
			if (sign_in("2", login))
			{
				do
				{
					std::cout << "=========Меню администратора=========" << std::endl;
					std::cout << "1. Показать непросмотренные заявки\n2. Показать историю заявок\n3. Просмотр данных пользователей\n4. Изменить данные пользователя\n5. Просмотр остатка бюджета\n6. Управление бюджетом\n7. Выход\n";
					std::cout << "=====================================" << std::endl;
					choise = checkInput<int>();
					switch (choise)
					{
						case 1:
						{
						
							do
							{	
								std::vector<Applications> appl;
								appl = getPendingApplications();
								OutputApp(appl);
								std::cout << "1. Подтвердить заявку\n2. Отклонить заяку\n3. Выход" << std::endl;
								int choise;
								choise = checkInput<int>();
								switch (choise)
								{
								case 1:
								{
									comfirmApplication(appl);
									break;
								}
								case 2:
								{
									deleteApplication(appl);
									break;
								}
								default:
									break;
								}
							} while (choise!= 3);
							break;
						}

						case 2:
						{
							std::vector<Applications> appl;
							std::string login_query;
							std::cout << "Введите логин пользователя, заявки которого хотите увидеть" << std::endl;
							std::cin >> login_query;
							appl = getApplications(login_query);
							OutputApp(appl);
						break;
						}

						case 3:
						{
							std::cout << "Введите логин пользователя: " << std::endl;
							std::string Login;
							std::cin >> Login;
							showUserData(Login);
							break;
						}

						case 4:
						{
							setUserData();
							break;
						}

						case 5:
						{
							int budget = getBudget();
							std::cout << "Бюджет: ";
							std::cout << budget<< std::endl;
							break;
						}

						case 6:
						{
							changeBudget();
							break;
						}
						default:
							break;
					}
				} while (choise!= 7);
			}
			else break;
		}
		case 3:
			registration();
			break;
		default:
			break;
		}
	} while (choise != 4);

	return 0;
}