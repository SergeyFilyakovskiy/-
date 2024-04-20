#pragma once
#include<string>
#include<vector>
#include <iostream>
#include <conio.h>

struct Applications
{
	std::string m_date;
	std::string m_type;
	std::string m_cost;
	std::string m_status;
	std::string m_login;
};

int sign_in(std::string acLevel, std::string login);

int registration();

int applicationSubmission(std::string login);

std::vector<Applications> getApplications(std::string login);

std::string UTF8_to_CP1251(std::string const& utf8);

std::vector<Applications> getPendingApplications();

int comfirmApplication(const std::vector<Applications>& applications);

int deleteApplication(const std::vector<Applications>& applications);

int showUserData(std::string login);

int setUserData();

int getBudget();

int changeBudget();

int calculateBudget(std::string cost);