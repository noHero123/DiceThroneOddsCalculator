#pragma once
#include <vector>
#include "RandomGenerator.h"
#include "Types.h"

class ORP
{
public:
	typedef size_t Move;
    static const Move no_move = 1048576; // 2^20
	ORP()
	{
	}

	void getCombinations(const std::vector<size_t>& numbers, unsigned size, std::vector<size_t>& line, size_t& counter, size_t target, std::vector<size_t>& erg) {
		for (unsigned i = 0; i < numbers.size(); i++) {
			line.push_back(numbers[i]);
			if (size <= 1) { // Condition that prevents infinite loop in recursion
				//std::cout<< "combinations " << counter << " ";
				//for (auto ziff : line)
				//{
				//	std::cout << ziff;
				//}
				//std::cout << std::endl;
				if (counter == target)
				{
					erg = line;
					counter++;
					return;
				}
				counter++;
				line.erase(line.end() - 1);
			}
			else {
				getCombinations(numbers, size - 1, line, counter, target, erg); // Recursion happens here
				
				line.erase(line.end() - 1);
			}
			if (counter > target)
			{
				return;
			}
		}
		if (counter > target)
		{
			return;
		}
	}

	template<typename RandomEngine>
	void do_move(Move move, RandomEngine* engine)
	{
		if (is_chance_node)
		{ 
			if (roll_atempts > 0)
			{
				/*if (roll_atempts == 2 && last_move == 15)
				{
					std::cout <<"start" << dice[0] << dice[1] << dice[2] << dice[3] << dice[4] << std::endl;
				}*/
				roll_atempts--;
				is_chance_node = false;
				std::vector<bool>& bits = (*allowed_rerolls_vecs)[last_move];
				size_t number_bits = count_bits(last_move);
				const auto& db = (*all_combs)[number_bits - 1];
				const std::vector<DiceIdx>& erg = db.combs_sorted[move];
				size_t b = 0;
				for (int i = 0; i < 5; ++i) {
					if (bits[i])
					{
						dice[i] = erg[b];
						b++;
					}
				}
				/*if (roll_atempts == 1 && last_move == 15)
				{
					std::cout << "end" << dice[0] << dice[1] << dice[2] << dice[3] << dice[4] << std::endl;
				}*/
				size_t idx = get_hash(dice);
				if ((*all_ability_combinations_dt)[idx].success)
				{	
					win = true;
				}
				return;
			}

			if (rerolls > 0)
			{
				rerolls--;
				is_chance_node = false;
				std::vector<bool>& bits = (*allowed_rerolls_vecs)[last_move];
				size_t number_bits = count_bits(last_move);
				const auto& db = (*all_combs)[number_bits - 1];
				const std::vector<DiceIdx>& erg = db.combs_sorted[move];
				size_t b = 0;
				for (int i = 0; i < 5; ++i) {
					if (bits[i])
					{
						dice[i] = erg[b];
						b++;
						break;
					}
				}
				//blow up for dice_anatomy:
				size_t idx = get_hash(dice);
				if ((*all_ability_combinations_dt)[idx].success)
				{
					win = true;
				}
				return;
			}
		}
		else
		{
			is_chance_node = true;
			this->last_move = move;
			if (roll_atempts > 0)
			{
				this->last_reroll_move = move;
			}
		}
		return;
	}

	bool has_moves() const
	{
		if (win)
		{
			return false;
		}
		return rerolls > 0 || roll_atempts > 0;
	}

	template<typename RandomEngine>
	size_t get_random_move_idx(RandomEngine* engine)
	{
		size_t number_bits = count_bits(last_move);
		const auto& db = (*all_combs)[number_bits - 1];
		size_t max_combs = db.combs_size;
		size_t move = (size_t)RandomGenerator::uniform_int_distribution(0, max_combs-1, *engine);
		return db.dicehash_to_combs_sorted[move];
	}

	template<typename RandomEngine>
	void do_random_move(RandomEngine* engine)
	{
		//get aggressive moves;
		
		size_t move = 0;
		if (is_chance_node)
		{
			move = get_random_move_idx(engine);
		}
		else
		{
			std::vector<DiceHash> moves = get_moves();
			size_t idx0 = (size_t)RandomGenerator::uniform_int_distribution(0, moves.size() - 1, *engine);
			move = moves[idx0];
		}
		do_move(move, engine);
	}

	DiceHash get_hash(const std::vector<DiceIdx>& v) const
	{
		DiceHash v4 = (*inverted_dice_anatomy)[v[4]];
		DiceHash v3 = (*inverted_dice_anatomy)[v[3]];
		DiceHash v2 = (*inverted_dice_anatomy)[v[2]];
		DiceHash v1 = (*inverted_dice_anatomy)[v[1]];
		DiceHash v0 = (*inverted_dice_anatomy)[v[0]];


		return v4 + v3 * 6 + v2 * 36 + v1 * 216 + v0 * 1296;
		//return v[4] + v[3] * 6 + v[2] * 36 + v[1] * 216 + v[0] * 1296;
	}

	double get_result() const
	{
		if (win)
		{
			return 1.0;
		}
		return 0.0;
	}

	size_t count_bits(size_t x) const
	{
		size_t out = 0;
		for (size_t i = 0; i < 5; ++i) {
			if ((x >> i) & 1)
			{
				out++;
			}
		}
		return out;
	}

	std::vector<DiceHash> get_moves() const
	{
		if (is_chance_node)
		{
			size_t number_bits = count_bits(last_move);
			DiceHash max_size = (DiceHash)((*all_combs)[number_bits - 1].combs_sorted.size());
			size_t max_combs = (*all_combs)[number_bits - 1].combs_size;
			return {max_size , 123};//in case we have a chance-node we only return the maximum number, instead of a vec with numbers from 0 - max-1
		}
		if (win)
		{
			return {};
		}
		if(last_move>= 0 && last_move <= 31)
		{
			return (roll_atempts>0) ? (*allowed_rerolls_idx)[last_move] : (*allowed_single_rerolls_idx)[last_reroll_move];
		}
		return {};
	}

	//####################################################################################################################################
	//SOLVER##############################################################################################################################
	//####################################################################################################################################

	bool is_finished()
	{
		return win || (roll_atempts == 0 && rerolls == 0);
	}

	void do_roll(size_t move)
	{
		if (roll_atempts > 0)
		{
			roll_atempts--;
			const std::vector<bool>& bits = (*allowed_rerolls_vecs)[last_move];
			size_t number_bits = (*counted_bits)[last_move];
			const auto& db = (*all_combs)[number_bits - 1];
			const std::vector<DiceIdx>& erg = db.combs_sorted[move];
			size_t b = 0;
			for (int i = 0; i < 5; ++i) {
				if (bits[i])
				{
					dice[i] = erg[b];
					b++;
				}
			}/*
			dice[0] = bits[0] ? erg[b++] : dice[0];
			dice[1] = bits[1] ? erg[b++] : dice[1];
			dice[2] = bits[2] ? erg[b++] : dice[2];
			dice[3] = bits[3] ? erg[b++] : dice[3];
			dice[4] = bits[4] ? erg[b++] : dice[4];*/
			win = (*all_ability_combinations_dt)[get_hash(dice)].success;
			return;
		}

		if (rerolls > 0)
		{
			std::vector<bool>& bits = (*allowed_rerolls_vecs)[last_move];
			size_t number_bits = (*counted_bits)[last_move];
			rerolls -= number_bits;
			const auto& db = (*all_combs)[number_bits - 1];
			const std::vector<DiceIdx>& erg = db.combs_sorted[move];
			size_t b = 0;
			for (int i = 0; i < 5; ++i) {
				if (bits[i])
				{
					dice[i] = erg[b];
					b++;
				}
			}/*
			dice[0] = bits[0] ? erg[b++] : dice[0];
			dice[1] = bits[1] ? erg[b++] : dice[1];
			dice[2] = bits[2] ? erg[b++] : dice[2];
			dice[3] = bits[3] ? erg[b++] : dice[3];
			dice[4] = bits[4] ? erg[b++] : dice[4];*/
			//blow up for dice_anatomy:
			//size_t idx = get_hash(dice);
			win = (*all_ability_combinations_dt)[get_hash(dice)].success;
			return;
		}
		return;
	}

	size_t get_amount_of_throw(size_t throw_idx)
	{
		size_t number_bits = (*counted_bits)[last_move];
		const auto& db = (*all_combs)[number_bits - 1];
		return  db.combs_sorted_amount[throw_idx];
	}

	size_t get_total_of_throws()
	{
		size_t number_bits = (*counted_bits)[last_move];
		const auto& db = (*all_combs)[number_bits - 1];
		return db.combs_size;
	}

	size_t get_number_avail_dice_trows()
	{
		size_t number_bits = (*counted_bits)[last_move];
		size_t max_size = (*all_combs)[number_bits - 1].combs_sorted.size();
		return max_size;
	}

	const std::vector<DiceHash>& get_rerolls()
	{
		return (roll_atempts > 0) ? (*allowed_rerolls_idx)[last_move] : (*allowed_single_rerolls_idx)[last_reroll_move];
	}

	std::vector<DiceHash> get_rerolls(const std::vector<DiceIdx>& dont_reroll, const std::vector<DiceIdx> allways_reroll, const std::vector<DiceIdx>& target, const std::vector<DiceIdx>& anatomy)
	{

		if (roll_atempts == 0)
		{
			const std::vector<DiceHash>& single_rerolls = (*allowed_single_rerolls_idx)[last_reroll_move];
			size_t idx = get_hash(dice);
			const auto& dicethrow = (*all_ability_combinations_dt)[idx];
			DiceHash reroll_sum = 0;
			size_t temp_rr = rerolls;
			for (const auto& roll_idx : single_rerolls)
			{
				const std::vector<bool>& bits = (*allowed_rerolls_vecs)[roll_idx];
				for (size_t i = 0; i < bits.size(); i++)
				{
					if (bits[i] == true && dicethrow.rerollers[i] == true)
					{
						reroll_sum += roll_idx;
						temp_rr--;
						if (temp_rr == 0)
						{
							return { reroll_sum };
						}
						break;
					}
				}
			}
			return { reroll_sum };
		}

		const std::vector<DiceHash>& rerolls = (*allowed_rerolls_idx)[last_move];
		//dont reroll dice with lowes prob:
		
		std::vector<DiceHash> allowed_rerolls_2{};
		allowed_rerolls_2.reserve(rerolls.size());
		for (const auto& roll_idx : rerolls)
		{
			const std::vector<bool>& bits = (*allowed_rerolls_vecs)[roll_idx];
			bool allowed = true;
			for (size_t i = 0; i < 5; i++)
			{
				if (!bits[i])
				{
					size_t diceval = (*inverted_dice_anatomy)[dice[i]];
					for (const auto& v : allways_reroll)
					{
						if (v == diceval)
						{
							allowed = false;
							break;
						}
					}
					if (!allowed)
					{
						break;
					}
				}
			}
			if (allowed)
			{
				allowed_rerolls_2.push_back(roll_idx);
			}
		}

		std::vector<DiceHash> allowed_rerolls{};
		allowed_rerolls.reserve(allowed_rerolls_2.size());
		std::vector<DiceIdx> temp_target{ 0,0,0,0,0,0 };
		for (const auto & roll_idx : allowed_rerolls_2)
		{
			const std::vector<bool>& bits = (*allowed_rerolls_vecs)[roll_idx];
			bool allowed = true;
			temp_target[0] = target[0];
			temp_target[1] = target[1];
			temp_target[2] = target[2];
			temp_target[3] = target[3];
			temp_target[4] = target[4];
			temp_target[5] = target[5];

			for (size_t i = 0; i < 5; i++)
			{
				if (!bits[i])
				{
					size_t idx = anatomy[(*inverted_dice_anatomy)[dice[i]]];
					if (temp_target[idx] > 0)
					{
						temp_target[idx]--;
					}
				}
			}

			for (size_t i = 0; i < 5; i++)
			{
				if(bits[i])
				{ 

					size_t diceval = (*inverted_dice_anatomy)[dice[i]];
					size_t idx = anatomy[diceval];
					if (temp_target[idx] > 0)
					{
						temp_target[idx]--;
					}
					else
					{
						continue;
					}
					for (const auto& dr : dont_reroll)
					{
						if (diceval == dr)
						{
							allowed = false;
							break;
						}
					}
					if (!allowed)
					{
						break;
					}
				}
			}
			if (allowed)
			{
				allowed_rerolls.push_back(roll_idx);
			}
		}

		// dont reroll double occourance: example AAAC dont reroll first and second AND second and third -> both would reroll AA
		std::vector<size_t> rolled_values{};
		std::vector<DiceHash> allowed_rerolls3{};
		allowed_rerolls3.reserve(allowed_rerolls.size());
		rolled_values.reserve(allowed_rerolls.size());
		std::vector<size_t> factors{ 0,7,49,343,2401,16807};
		for (const auto& roll_idx : allowed_rerolls)
		{
			const std::vector<bool>& bits = (*allowed_rerolls_vecs)[roll_idx];
			size_t value = 0;
			for (size_t i = 0; i < 5; i++)
			{
				if (bits[i])
				{
					size_t diceval = (*inverted_dice_anatomy)[dice[i]];
					value += factors[diceval]+1;
				}
			}
			bool found = false;
			for (const auto& v : rolled_values)
			{
				if (v == value)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				rolled_values.push_back(value);
				allowed_rerolls3.push_back(roll_idx);
			}
			
		}

		return allowed_rerolls3;
	}

	std::vector<DiceIdx> dice{ 0,0,0,0,0 };
	std::vector<DiceThrow>* all_ability_combinations_dt{nullptr};
	std::vector<Combinations>* all_combs{ nullptr };
	std::vector<std::vector<DiceHash>>* allowed_rerolls_idx{ nullptr };
	std::vector<std::vector<DiceHash>>* allowed_single_rerolls_idx{ nullptr };
	std::vector<std::vector<bool>>* allowed_rerolls_vecs{ nullptr };
	std::vector<DiceIdx>* inverted_dice_anatomy{ nullptr };
	std::vector<DiceIdx>* counted_bits{ nullptr };

	size_t rerolls{ 0 };
	size_t roll_atempts{ 0 };
	size_t last_move{ 31 };
	size_t last_reroll_move{ 31 };
	bool is_chance_node{ false };
	bool win{ false };
};