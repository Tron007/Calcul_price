//============================================================================
// Name        : service_cost_calculation.cpp
// Author      : First
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <mysql.h>
#include <mysql++.h>
#include <math.h>
#include <boost/lexical_cast.hpp>
using namespace std;
#include <unistd.h>



// mysql variables for connection, store result
MYSQL *conn, mysql;
MYSQL_RES *res;
MYSQL_ROW row;
MYSQL_RES *add_res;
MYSQL_ROW add_row;

////// mysql connections settings
//const char *server="localhost";
//const char *user="root";
//const char *password="admin";
//const char *database="account_database";


// mysql connections settings
// mysql connections settings
const char *server="52.8.83.48";
const char *user="007";
const char *password="Yeshimbetov^Karakalpak8";
const char *database="account_database";

int query_state;

std::string mysql_get_subscriber_fullName = "";
std::string mysql_query_str = "";
std::string call_id = "";
std::string numberA = "";
std::string numberB = "";
size_t numberA_size = 0;
size_t numberB_size = 0;
std::string international_call_number_prefix = "810";
std::string current_number_prefix = "";
std::string country_code = "";
std::string city_code = "";
size_t country_code_length = 0;
double time_round = 0;
size_t price_round = 0;
double call_price = 0.0; // 1 minute = 100$
double call_cost = 0.0; // cost of one call example 3 minute = 300$
std::string call_cost_str = "";
std::string mysql_get_call_price = "";
std::string call_type_str = "";
double call_duration = 0.0;
double millisecond_to_second = 0.0;
double round_with_step = 0.0;
double fract_part = 0.0;


std::string temp_Name_country;
int temp_number_size;
std::string Kazakhstan_code="87";


size_t get_country_code_length(std::string phone_number)
{
	size_t country_code_length = 0;

	if (phone_number[3] == '1') // if country code start with '1'
	{
		country_code_length = 4;
	} else if (phone_number[3] == '2')
	{
		if (phone_number[4] == '0') // if country code second digit is o then we can determine length  of country code
		{
			country_code_length = 2;
		} else
		{
			country_code_length = 3;
		}
	} else if (phone_number[3] == '3')
	{
		if (phone_number[4] == '5' || phone_number[4] == '7' || phone_number[4] == '8')
		{
			country_code_length = 3;
		} else
		{
			country_code_length = 2;
		}
	} else if (phone_number[3] == '4')
	{
		if (phone_number[4] == '2')
		{
			country_code_length = 3;
		} else
		{
			country_code_length = 2;
		}
	} else if (phone_number[3] == '5')
	{
		if (phone_number[4] == '0' || phone_number[4] == '9')
		{
			country_code_length = 3;
		} else
		{
			country_code_length = 2;
		}
	} else if (phone_number[3] == '6')
	{
		if (phone_number[4] == '7' || phone_number[4] == '8' || phone_number[4] == '9')
		{
			country_code_length = 3;
		} else
		{
			country_code_length = 2;
		}
	} else if (phone_number[3] == '7')
	{
		country_code_length = 3;
	} else if (phone_number[3] == '8')
	{
		if (phone_number[4] == '0' || phone_number[4] == '5' || phone_number[4] == '7' || phone_number[4] == '8' || phone_number[4] == '9')
		{
			country_code_length = 3;
		} else
		{
			country_code_length = 2;
		}
	} else if (phone_number[3] == '9')
	{
		if (phone_number[4] == '6' || phone_number[4] == '7' || phone_number[4] == '9')
		{
			country_code_length = 3;
		} else
		{
			country_code_length = 2;
		}
	}

	return country_code_length;
}



int countD=0;
int main() {
	while (true) {
	usleep(3000);

	mysql_init(&mysql);
	conn=mysql_real_connect(&mysql, server, user, password, database, 0, 0, 0);
	if(conn==NULL)
	{
		std::cout<<mysql_error(&mysql)<<std::endl<<std::endl;
	}
	mysql_query(&mysql,"SET NAMES 'UTF8'");



	// view all A-B numbers
	query_state=mysql_query(conn, "SELECT call_id, numberA, numberB, milli_second FROM ama_data WHERE call_type=''");
	if(query_state!=0)
	{
		std::cout<<mysql_error(conn)<<std::endl<<std::endl;
	}
	res=mysql_store_result(conn);


	while((row=mysql_fetch_row(res))!=NULL)
	{
		countD++;
		if (countD>1000) {std:cout<<"1000"<<endl;countD=0;}
		call_id = boost::lexical_cast<std::string>(row[0]);
		numberA = boost::lexical_cast<std::string>(row[1]);
		numberB = boost::lexical_cast<std::string>(row[2]);
		call_duration = boost::lexical_cast<double>(row[3]);
		numberA_size = numberA.size();
		numberB_size = numberB.size();


		//std::cout << numberB << std::endl;
		call_type_str = "";
		if (numberA_size == 6) // we calculate call cost if we call from 6 digit number
		{
			if (numberB_size >= 10) // Intercity, International, Hot Line calls
			{
				// we not interested in numberA because it's our number, otherwise in numberB
				// we need country code for calculate cost of call
				current_number_prefix = numberB.substr(0, 3);
				// ################################################################################################
				if (current_number_prefix == international_call_number_prefix) // International call
				{
					country_code_length = get_country_code_length(numberB);
					country_code = international_call_number_prefix + numberB.substr(3, country_code_length);

					mysql_get_call_price = "SELECT tariff, round, round_price FROM call_tariff WHERE call_tariff.code = '"+country_code+"'";
					query_state=mysql_query(conn, mysql_get_call_price.c_str());

					add_res=mysql_store_result(conn);
					add_row=mysql_fetch_row(add_res);


					if (add_row != NULL)
					{
						call_price = boost::lexical_cast<double>(add_row[0]);
						time_round = boost::lexical_cast<double>(add_row[1]);
						price_round = boost::lexical_cast<size_t>(add_row[2]);

						/*
						 *  if call seconds after dividing to round_step have fraction part then we do this procedure
						 *  1) divide seconds to round_step (2, 3, 10 etc)
						 *  2) take result from 1 step then get int (whole) part (use floor() c++ function)
						 *  3) then we this whole part increase by 1
						 *  4) and multiply by round_step and we get the rounded value with given round_step
						 *  else just do nothing
						 */


						millisecond_to_second = round(call_duration / 1000);
						call_duration = millisecond_to_second;
						fract_part = modf(millisecond_to_second / time_round, &round_with_step);
						if (fract_part != 0)
						{
							round_with_step = floor(millisecond_to_second / time_round);
							round_with_step++;
							call_duration = round_with_step * time_round;
						}
					//std::cout << call_duration << std::endl;
						call_cost = call_duration * (call_price/60); // this price we need write in ama_data in call_cost column in current call

						call_type_str = "international call";

					}

					mysql_free_result(add_res);

					// ################################################################################################
				} else if ((numberB_size == 10 || numberB_size == 11) && numberB[0] == '8') // Intercity call and Mobile call
				{


					city_code = numberB.substr(0, 2);
					temp_number_size = numberB_size-2;
					add_row=NULL;

					if (city_code==Kazakhstan_code){//if Kazakhstan
                  //  std::cout<<"find KAZ"<<std::endl;
						while (add_row == NULL && temp_number_size>3){
							city_code = numberB.substr(0, temp_number_size);

							 //std::cout<<city_code<<std::endl;


							mysql_get_call_price = "SELECT tariff, round, round_price, add_text "
									"FROM call_tariff WHERE left(code,2)='"+Kazakhstan_code+"' AND LOCATE('"+city_code+"',code)";
							query_state=mysql_query(conn, mysql_get_call_price.c_str());

							add_res=mysql_store_result(conn);
							add_row=mysql_fetch_row(add_res);
							temp_number_size--;
						}


					}else {//if Russia
						//std::cout<<"find RUS"<<std::endl;

						while (add_row == NULL && temp_number_size>3){
							city_code = numberB.substr(0, temp_number_size);


							//std::cout<<city_code<<std::endl;
							std::string Russia_STR="Россия";
							mysql_get_call_price = "SELECT tariff, round, round_price, add_text "
									"FROM call_tariff WHERE country='"+Russia_STR+"' AND LOCATE('"+city_code+"',code)";
							query_state=mysql_query(conn, mysql_get_call_price.c_str());

							add_res=mysql_store_result(conn);
							add_row=mysql_fetch_row(add_res);
							temp_number_size--;
							//std::cout<<"find RUS2"<<std::endl;
						}
						}






					if (add_row != NULL) // if result not NULL we find city code in mysql table then we can calculate cost of call
					{
						call_price = boost::lexical_cast<double>(add_row[0]);
						time_round = boost::lexical_cast<double>(add_row[1]);
						price_round = boost::lexical_cast<size_t>(add_row[2]);




						/*
						 *  if call seconds after dividing to round_step have fraction part then we do this procedure
						 *  1) divide seconds to round_step (2, 3, 10 etc)
						 *  2) take result from 1 step then get int (whole) part (use floor() c++ function)
						 *  3) then we this whole part increase by 1
						 *  4) and multiply by round_step and we get the rounded value with given round_step
						 *  else just do nothing
						 */

						millisecond_to_second = round(call_duration / 1000);
						call_duration = millisecond_to_second;
						fract_part = modf(millisecond_to_second / time_round, &round_with_step);
						if (fract_part != 0)
						{
							round_with_step = floor(millisecond_to_second / time_round);
							round_with_step++;
							call_duration = round_with_step * time_round;
						}
						call_cost = call_duration * (call_price/60); // this price we need write in ama_data in call_cost column in current call
						//std::cout<<call_cost<<"  ->"<<numberB<<std::endl;
						// ################################################################################################


						std::string add_text_str = boost::lexical_cast<std::string>(add_row[3]);
						if (add_text_str == "Мобильная связь")
						{
							call_type_str = "mobile call";
						} else
						{
							call_type_str = "intercity call";
						}

					} else if (add_row == NULL) // Hot line call
					{
						mysql_get_call_price = "SELECT tariff, round, round_price FROM call_tariff WHERE call_tariff.code = '"+numberB+"'";
						query_state=mysql_query(conn, mysql_get_call_price.c_str());

						add_res=mysql_store_result(conn);
						add_row=mysql_fetch_row(add_res);

						if (add_row != NULL)
						{
							call_price = boost::lexical_cast<double>(add_row[0]);
							time_round = boost::lexical_cast<double>(add_row[1]);
							price_round = boost::lexical_cast<size_t>(add_row[2]);

							/*
							 *  if call seconds after dividing to round_step have fraction part then we do this procedure
							 *  1) divide seconds to round_step (2, 3, 10 etc)
							 *  2) take result from 1 step then get int (whole) part (use floor() c++ function)
							 *  3) then we this whole part increase by 1
							 *  4) and multiply by round_step and we get the rounded value with given round_step
							 *  else just do nothing
							 */

							millisecond_to_second = round(call_duration / 1000);
							call_duration = millisecond_to_second;
							fract_part = modf(millisecond_to_second / time_round, &round_with_step);
							if (fract_part != 0)
							{
								round_with_step = floor(millisecond_to_second / time_round);
								round_with_step++;
								call_duration = round_with_step * time_round;
							}
							call_cost = call_duration * (call_price/60); // this price we need write in ama_data in call_cost column in current call


							call_type_str = "hot line";
						}

						// if there no particular properties like certain length of digit or start with '1' then we just
						// try find whole numberB in code
					}

					mysql_free_result(add_res);
				}
				// ################################################################################################
			} else if (numberA_size == 6 && numberB_size == 3 && numberB[0] == '1') // Fixed service
			{
				// just find number and if we find then just cost of call equal to price of call

				mysql_get_call_price = "SELECT tariff FROM call_tariff WHERE call_tariff.code = '"+numberB+"'";
				query_state=mysql_query(conn, mysql_get_call_price.c_str());

				add_res=mysql_store_result(conn);
				add_row=mysql_fetch_row(add_res);

				if (add_row != NULL)
				{
					call_cost = boost::lexical_cast<double>(add_row[0]);
					call_type_str = "fixed service";
				}
				mysql_free_result(add_res);
			} else if (numberB_size == 6) {
				call_cost = 0;
				call_type_str = "inner";

			}


			// std::cout << "call cost: " << call_cost << std::endl;
			call_cost_str = boost::lexical_cast<std::string>(call_cost);
			mysql_query_str = "UPDATE ama_data SET call_cost = '"+call_cost_str+"', "
					"call_type = '"+call_type_str+"', call_direction = 'outgoing' "
					"WHERE call_id = '"+call_id+"'";
			query_state=mysql_query(conn, mysql_query_str.c_str());
			call_cost = 0.0;
		} else
		{
			call_cost = 0.0;

			mysql_query_str = "UPDATE ama_data SET call_direction = 'incoming',call_type = 'incoming' WHERE call_id = '"+call_id+"'";
			query_state=mysql_query(conn, mysql_query_str.c_str());
		}



	}
	mysql_free_result(res);
	//mysql_free_result(add_res); //double free error
	mysql_close(conn);
	std::cout<<"END"<<std::endl;
}
	return 0;
}
