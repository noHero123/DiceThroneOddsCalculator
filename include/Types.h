#pragma once
#include <string>
#include <sstream> 
#include <vector>
#include <iomanip> //setprecision

typedef unsigned char DiceIdx;
typedef unsigned short DiceHash;

//typedef size_t DiceIdx;
//typedef size_t DiceHash;

struct OddsResult
{
	std::string ability="";
	std::vector<bool> rerolls{};
	float odd{0.0F};

	std::pair<std::string, std::string> get_data() const
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << odd;
		return { ability, stream.str() };
	}

	std::string get_odds() const
	{
		std::stringstream stream;
		stream << std::fixed << std::setprecision(2) << odd;
		return stream.str();
	}

	std::string get_reroll() const
	{
		std::stringstream stream;
		for(const auto & b : rerolls)
		{
			stream << (b ? "1" : "0");
		}
		return stream.str();
	}
};

struct CardData
{
	int cp = 10;
	size_t use_max_cards = 30;

	size_t lvlsixit = 0;
	size_t lvlsamesis = 0;
	size_t lvltip_it = 0;
	size_t lvlwild = 0;
	size_t lvltwiceWild = 0;
	size_t lvlslightlyWild = 0;
	size_t hasCheer = 0;
	size_t numberProbabilityManipulation = 0;
};

struct Card
{
	size_t card_id{ 0 }; // 0 = sixit
	bool can_use{ true };
	size_t lvl{ 0 };
	size_t cp_cost{ 1 };
	size_t function_to_call{0};
	size_t dice_manipulateable{ 0 };
};

struct Combinations
{
	size_t lenght{ 0 };
	size_t combs_size{ 0 };
	std::vector<std::vector<DiceIdx>> combs{};
	std::vector<std::vector<DiceIdx>> combs_sorted{};
	std::vector<DiceHash> combs_sorted_amount{};
	std::vector<DiceHash> dicehash_to_combs_sorted{}; // map for hash -> combs_sorted
};

struct BattleResult
{
	size_t move = 0;
	size_t playedGames = 0;
	double proz = 0.0;
	int iterations = 0;
};

struct DiceThrowDice
{
	DiceIdx die{ 0U };
	bool reroll{ false };
};

struct DiceManipulation
{
	DiceIdx card{ 0 }; //1: samesis, 2: tipit, 3:sixit, 4: wild
	DiceIdx dice1val{ 9 };
	DiceIdx dice1change{ 9 };
	DiceIdx dice2val{ 9 };
	DiceIdx dice2change{ 9 };

	bool isEqual(const DiceManipulation& dm) const
	{
		if (card != dm.card)
		{
			return false;
		}
		if (dice1change != dm.dice1change)
		{
			return false;
		}
		if (dice2change != dm.dice2change)
		{
			return false;
		}
		if (dice1val != dm.dice1val)
		{
			return false;
		}
		if (dice2val != dm.dice2val)
		{
			return false;
		}

		return true;
	}

	void load_from_string(std::string data)
	{
		for (size_t i = 0; i < data.size(); i++)
		{
			if (i == 0)
			{
				card = data[i] - '0';
			}
			if (i == 1)
			{
				dice1val = data[i] - '0';
			}
			if (i == 2)
			{
				dice1change = data[i] - '0';
			}
			if (i == 3)
			{
				dice2val = data[i] - '0';
			}
			if (i == 4)
			{
				dice2change = data[i] - '0';
			}
		}
	}

	std::string get_save_string() const
	{
		//std::stringstream ss{};
		//ss << ;
		if (dice2val < 9)
		{
			return std::to_string(card * 10000 + dice1val * 1000 + dice1change * 100 + dice2val * 10 + dice2change);
		}
		return std::to_string(card * 100 + dice1val * 10 + dice1change);
	}

};

struct DiceThrow
{
	bool success{ false };
	DiceIdx dice[5]{ 0U };
	bool rerollers[5]{ false };
	std::vector<DiceManipulation> manipula{{},{},{},{},{},{}};
	std::vector<int> reroll_manipulation{ 0,0,0,0,0 };
	DiceHash hash{65535};

	DiceThrow()
	{
		return;
	}
	DiceThrow(const DiceThrow& dt)
	{
		this->success = dt.success;
		this->dice[0] = dt.dice[0];
		this->dice[1] = dt.dice[1];
		this->dice[2] = dt.dice[2];
		this->dice[3] = dt.dice[3];
		this->dice[4] = dt.dice[4];
		this->rerollers[0] = dt.rerollers[0];
		this->rerollers[1] = dt.rerollers[1];
		this->rerollers[2] = dt.rerollers[2];
		this->rerollers[3] = dt.rerollers[3];
		this->rerollers[4] = dt.rerollers[4];
		this->reroll_manipulation[0] = dt.reroll_manipulation[0];
		this->reroll_manipulation[1] = dt.reroll_manipulation[1];
		this->reroll_manipulation[2] = dt.reroll_manipulation[2];
		this->reroll_manipulation[3] = dt.reroll_manipulation[3];
		this->reroll_manipulation[4] = dt.reroll_manipulation[4];
		this->manipula[0] = dt.manipula[0];
		this->manipula[1] = dt.manipula[1];
		this->manipula[2] = dt.manipula[2];
		this->manipula[3] = dt.manipula[3];
		this->manipula[4] = dt.manipula[4];
		this->manipula[5] = dt.manipula[5];
		this->hash = dt.hash;
		return;
	}

	DiceThrow& operator= (const DiceThrow& dt)
	{
		this->success = dt.success;
		this->dice[0] = dt.dice[0];
		this->dice[1] = dt.dice[1];
		this->dice[2] = dt.dice[2];
		this->dice[3] = dt.dice[3];
		this->dice[4] = dt.dice[4];
		this->rerollers[0] = dt.rerollers[0];
		this->rerollers[1] = dt.rerollers[1];
		this->rerollers[2] = dt.rerollers[2];
		this->rerollers[3] = dt.rerollers[3];
		this->rerollers[4] = dt.rerollers[4];
		this->reroll_manipulation[0] = dt.reroll_manipulation[0];
		this->reroll_manipulation[1] = dt.reroll_manipulation[1];
		this->reroll_manipulation[2] = dt.reroll_manipulation[2];
		this->reroll_manipulation[3] = dt.reroll_manipulation[3];
		this->reroll_manipulation[4] = dt.reroll_manipulation[4];
		this->manipula[0] = dt.manipula[0];
		this->manipula[1] = dt.manipula[1];
		this->manipula[2] = dt.manipula[2];
		this->manipula[3] = dt.manipula[3];
		this->manipula[4] = dt.manipula[4];
		this->manipula[5] = dt.manipula[5];
		this->hash = dt.hash;
		return *this;
	}

	void reset()
	{
		this->success = false;
		this->rerollers[0] = false;
		this->rerollers[1] = false;
		this->rerollers[2] = false;
		this->rerollers[3] = false;
		this->rerollers[4] = false;
		this->reroll_manipulation[0] = 0;
		this->reroll_manipulation[1] = 0;
		this->reroll_manipulation[2] = 0;
		this->reroll_manipulation[3] = 0;
		this->reroll_manipulation[4] = 0;

		for (size_t i = 0; i < 6; i++)
		{
			manipula[i].card = 0;
			manipula[i].dice1change = 9;
			manipula[i].dice2change = 9;
			manipula[i].dice1val = 9;
			manipula[i].dice2val = 9;
		}
	}


	bool isEqual(const DiceThrow& dt) const
	{
		for (size_t i = 0; i < 5; i++)
		{
			if (dice[i] != dt.dice[i])
			{
				return false;
			}
			if (rerollers[i] != dt.rerollers[i])
			{
				return false;
			}
			if (success != dt.success)
			{
				return false;
			}
		}
		for (size_t i = 0; i < 6; i++)
		{
			if (!manipula[i].isEqual(dt.manipula[i]))
			{
				return false;
			}
		}
		return true;
	}

	bool isEqual3(const DiceThrow& dt) const
	{
		if (hash != dt.hash)
		{
			return false;
		}
		if (success != dt.success)
		{
			return false;
		}
		std::vector<DiceIdx> keep1{ 0,0,0,0,0,0 };
		std::vector<DiceIdx> keep2{ 0,0,0,0,0,0 };
		std::vector<DiceIdx> rr1{ 0,0,0,0,0,0 };
		std::vector<DiceIdx> rr2{ 0,0,0,0,0,0 };
		for (size_t i = 0; i < 5; i++)
		{
			keep1[dice[i]]++;
			keep2[dt.dice[i]]++;
			rr1[rerollers[i]]++;
			rr2[dt.rerollers[i]]++;
			
		}
		for (size_t i = 0; i < 6; i++)
		{
			if (keep1[i] != keep2[i] || rr1[i] != rr2[i])
			{
				return false;
			}
		}
		return true;
	}

	std::vector<DiceThrowDice> getvec() const
	{
		return { {dice[0], rerollers[0]}, {dice[1], rerollers[1]}, {dice[2], rerollers[2]}, {dice[3], rerollers[3]}, {dice[4], rerollers[4]} };
	}
	std::vector<DiceThrowDice> getvec(size_t anz) const
	{
		std::vector<DiceThrowDice> vdt{};
		for (size_t i = 0; i < anz; i++)
		{
			vdt.push_back({ dice[i], rerollers[i] });
		}
		return vdt;
	}

	std::vector<std::string> my_string_split(std::string message, std::string delimiter) const
	{
		std::vector<std::string> result{};
		size_t pos = 0;
		std::string s = message;
		std::string token = "";
		while ((pos = s.find(delimiter)) != std::string::npos) {
			token = s.substr(0, pos);
			result.push_back(token);
			s.erase(0, pos + delimiter.length());
		}
		if (s != "")
		{
			result.push_back(s);
		}

		return result;
	}

	void loadRerollersFromInt(int x)
	{
		for (size_t i = 0; i < 5; ++i) {
			rerollers[i] = ((x >> i) & 1) ? true : false;
		}
		return;
	}

	void load_from_string(std::string data)
	{
		std::vector<std::string> strings = my_string_split(data, ",");
		success = (data[0] == 'x') ? true : false;
		int val = data[1] - 'A';
		loadRerollersFromInt(val);
		for (size_t i = 1; i < strings.size(); i++)
		{
			manipula[i - 1].load_from_string(strings[i]);
		}
	}

	char getChar(int number) const
	{
		switch (number)
		{
		case 9:
			return '9';
		case 8:
			return '8';
		case 7:
			return '7';
		case 6:
			return '6';
		case 5:
			return '5';
		case 4:
			return '4';
		case 3:
			return '3';
		case 2:
			return '2';
		case 1:
			return '1';
		default: 
			return '0';
		}
	}

	std::string getChar(bool number) const
	{
		if (number)
		{
			return "1";
		}
		return "0";
	}

	std::string get_rerollers_string() const
	{
		std::string d = "";
		std::string s = "";
		for (size_t i = 0; i < 5; i++)
		{
			d += std::to_string(dice[i]);
		}
		for (size_t i = 0; i < 5; i++)
		{
			if (rerollers[i])
			{
				s += std::to_string(dice[i]);
			}
		}
		std::sort(s.begin(), s.end());
		std::sort(d.begin(), d.end());
		return d+":"+s;
	}

	std::string get_reroll_txt() const
	{
		int val = (rerollers[4] ? 16 : 0) + (rerollers[3] ? 8 : 0) + (rerollers[2] ? 4 : 0) + (rerollers[1] ? 2 : 0) + (rerollers[0] ? 1 : 0);
		const char b = 'A' + val;
		return std::string{b};
	}

	std::string get_save_string() const
	{
		//std::string s = getChar(success) + ',' + getChar(rerollers[0]) + getChar(rerollers[1]) + getChar(rerollers[2]) + getChar(rerollers[3]) + getChar(rerollers[4]);
		std::string s = ((success)?"x":"y") + get_reroll_txt();
		
		for (size_t i = 0; i < 6; i++)
		{
			if (manipula[i].card > 0)
			{
				s+= "," + manipula[i].get_save_string();
			}
		}
		return s;
	}

	std::string get_save_string_big() const
	{
		std::string s = getChar(success) + ',' + getChar(rerollers[0]) + getChar(rerollers[1]) + getChar(rerollers[2]) + getChar(rerollers[3]) + getChar(rerollers[4]);

		for (size_t i = 0; i < 6; i++)
		{
			if (manipula[i].card > 0)
			{
				s += "," + manipula[i].get_save_string();
			}
		}
		return s;
	}

	std::string get_string() const
	{
		std::stringstream ss{};
		ss << (success) ? 1 : 0;
		ss << ",";
		for (size_t i = 0; i < 5; i++)
		{
			ss << dice[i] + 1;
		}
		ss << ",";
		for (size_t i = 0; i < 5; i++)
		{
			ss << rerollers[i];
		}
		for (size_t i = 0; i < 6; i++)
		{
			if (manipula[i].card > 0)
			{
				ss << "," << manipula[i].get_save_string();
			}
		}
		ss << ";";
		return ss.str();
	}
};

struct ChaseData
{
	std::string ability = "";
	size_t amount = 0U;
	float erg = 0.0F;
	std::vector<DiceThrow> possible_list_with_cheat_dt{};
	std::vector<DiceThrow> all_ability_combinations_dt{};
	std::vector<DiceThrow> all_ability_combinations_no_cards_dt{};
};