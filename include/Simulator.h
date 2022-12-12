#pragma once
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>      // std::stringstream

#include <Eigen/Dense>

//#include "RandomGenerator.h"
#include "Types.h"
//#include "MontecarloDuct.h"
#include "ORP.h"
#include "FasterRandomGenerator.h"
#include "Helpers.h"
#include "Logger.h"

class Simulator
{
	typedef  ORP ORPX;
public:

	static const size_t number_dice_ = 5;//4;
	static const size_t possible_combs_blow_up_size_ = 7776;// 1296; // == 6^number_dice
	static const size_t sixit_anz_ = number_dice_;
	static const size_t samesis_anz_ = 20;//anz = 12;  == 2*(number_dice_ choose 2)
	static const size_t samesis2_anz_ = 50; //anz = 24; == 2*(number_dice_ choose 2) + 3*(number_dice_ choose 3)
	static const size_t tipit_anz_ = number_dice_ * 2;
	static const size_t tipit2_anz_ = number_dice_ * 4;
	static const size_t wilds_anz_ = number_dice_ * 6;
	static const size_t twiceaswild_anz_ = 390;//anz = 240; == (number_dice_ choose 2)*36 + wilds_anz_
	static const size_t slightlywild_anz_ = number_dice_ * 5;
	static const size_t max_combo_store_size = twiceaswild_anz_;


	Simulator(DiceRoller& rg);

	std::vector<OddsResult> get_probability(std::string hero_name, const std::vector<std::string>& abilities, std::string diceanatomy, bool default_sim, bool chase, std::string chased_ability, std::vector<DiceIdx> org_dice, const CardData& cardData, size_t roll_atemps, size_t rerolls);

	DiceHash get_hash(const std::vector<DiceIdx>& v)
	{
		return v[4] + v[3] * 6 + v[2] * 36 + v[1] * 216 + v[0] * 1296;
	}

	DiceHash get_hash(const std::vector<DiceThrowDice>& v)
	{
		DiceIdx v1 = v[4].die;
		DiceIdx v2 = v[3].die;
		DiceIdx v3 = v[2].die;
		DiceIdx v4 = v[1].die;
		DiceIdx v5 = v[0].die;
		return v1 + v2 * 6 + v3 * 36 + v4 * 216 + v5 * 1296;
	}

	DiceHash get_hash(const DiceThrow& v)
	{
		return v.dice[4] + v.dice[3] * 6 + v.dice[2] * 36 + v.dice[1] * 216 + v.dice[0] * 1296;
	}

	bool hit_target_ability(const std::vector<DiceIdx>& target, DiceThrow& dice, const std::vector<DiceIdx>& mydiceanatomy);

	void get_samessis(const DiceThrow& dice, std::vector<DiceThrow>& samesis, size_t save_idx);
	void get_tipits2(const DiceThrow& dice, std::vector<DiceThrow>& tips, size_t save_idx);
	void get_sixits(const DiceThrow& dice, std::vector<DiceThrow>& sixeds, size_t save_idx);
	void get_wilds(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx);
	void get_slightlywilds(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx);
	void get_tipits(const DiceThrow& dice, std::vector<DiceThrow>& tips, size_t save_idx);
	void get_samessis2(const DiceThrow& dice, std::vector<DiceThrow>& samesis, size_t save_idx);
	void get_wilds2(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx);
	void get_probabilitymanipulations(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx);


	DiceHash get_hash_smaller(const std::vector<DiceIdx>& v, size_t sides)
	{
		DiceHash sum{ 0 };
		DiceIdx siz = (DiceIdx)v.size();
		DiceHash power = 1;
		for (size_t i = v.size(); i-- > 0; )
		{
			sum += v[i] * power;//std::pow does not return size_t!
			power *= (DiceHash)sides;
		}
		return sum;
	}

	DiceHash get_hash_smaller(const std::vector<DiceIdx>& v)
	{
		DiceHash sum{ 0 };
		DiceIdx siz = (DiceIdx)v.size();
		DiceHash power = 1;
		for (size_t i = v.size(); i-- > 0; )
		{
			sum += v[i] * power;
			power *= 6;
		}
		return sum;
	}
	
	std::vector<OddsResult> oddsCalculator(std::vector<std::string> abilities, std::string diceanatomy, const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, const CardData cardData);
	std::vector<OddsResult> oddsCalculatorChase(std::string ability, std::vector<std::string> abilities, std::string diceanatomy, const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, const CardData & cardData);

	DiceIdx get_random_fast();

	void test_random_gens();
	void test_odds_calc();
	void test_odds_calc_chase();
    void combo_test();

	bool use_solver(const std::string& diceanatomy, const std::string& rollTarget, size_t roll_atemps, size_t rerolls, const std::vector<Card>& cards, int cp, size_t anz_cards);
	
	void blowUpData();
	void choose(size_t offset, size_t k, const std::vector<DiceIdx>& combogen, std::vector<DiceIdx>& combination, size_t& counter);
	void get_all_possible_combinations(size_t number_dice);
	void get_all_positive_combs(const std::vector<DiceIdx>& mydiceanatomy, const std::vector<DiceIdx>& target, const std::vector<Card>& cards, size_t cp, size_t use_max_cards);
	void get_all_positive_combs_storage_lower_bound(const std::vector<DiceIdx>& mydiceanatomy, const std::vector<DiceIdx>& target, const std::vector<Card>& cards, size_t cp, size_t use_max_cards);
	bool fast_test(const std::vector<DiceIdx>& target, const DiceThrow& dice, const std::vector<DiceIdx>& mydiceanatomy, const std::vector<Card>& cards, size_t cp, size_t use_max_cards);

	int global_check_unsorted(const DiceThrow& dice);
	int test_all_combis(const DiceThrow& dice, size_t cp, size_t useMaxCards, size_t combo_store_idx);
	
	bool read_ability(std::string ability_name, std::string diceanatomy, const std::vector<Card>& cards, size_t cp, size_t numbercards);
	void precalc_ability(std::string ability_name, const std::vector<DiceIdx>& target_ability, std::vector<DiceIdx>& mydiceanatomy, bool isDTA);
	void precalc_ability(std::string ability, std::string diceanatomy);


	//default one
	bool find_keepers_comb_ultra_fast(std::vector<DiceIdx>& dice, std::vector<bool>& rerollers, bool randomize, bool just_one_reroll, const std::vector<DiceThrow>& all_combis);

	const DiceThrow* find_keepers_comb_ultra_faster21(  const DiceThrow* rerollers);
	const DiceThrow* find_keepers_comb_ultra_faster_one_reroll21(const DiceThrow* rerollers);
	bool go_for_comb_ultra_faster2(size_t atmp, size_t reroll, const DiceThrow* rerollers);
	size_t combo_sim_ultra_faster_reroll_DT(const std::vector<DiceIdx>& org_dice, std::vector<bool>& org_rerollers, size_t roll_atemps, size_t rerolls, bool is_default_sim);
	
	const DiceThrow* go_for_comb_ultra_faster2_chase(size_t atmp, size_t reroll, const DiceThrow* rerollers);
	size_t combo_sim_ultra_faster_reroll_DT_chase(const std::vector<DiceIdx>& org_dice, std::vector<bool>& org_rerollers, size_t roll_atemps, size_t rerolls, bool is_default_sim);

	bool is_straight(DiceThrow& dt, bool is_big);
	bool is_big_straight(const DiceThrow& dt);
	void update_rerollers(DiceThrow& dt);

	std::vector<bool> test_all_rolls(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, float & erg_odd);
	std::vector<bool> test_all_rolls_chase(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, float& erg_odd);
	
	
	//std::vector<bool> test_montecarlo(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls);
	//std::vector<MCTSDUCTSmallFast::Node<ORPX>> ductNodes_{};

	void get_all_possible_combinations(size_t number_dice, size_t number_sides);

	void test_loaded_and_gen();

	void generate_data_for_MCTS(const std::vector<DiceIdx>& dice_anatomy);
	size_t get_amount_of_throw(ORPX& ORPX, size_t throw_idx);
	std::vector<bool> test_solver(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, std::string dice_anatomy, std::string target, const std::vector<Card>& cards, bool default_odd, bool get_rerolls, float & erg_odd);
	float solve_throw(size_t lvl, size_t& best_idx);
	float solve_default_throw(size_t lvl, size_t& best_idx);
	float solve_throw_reroller(size_t lvl, size_t& best_idx, bool is_default_sim);

	std::vector<std::vector<float>> createMarkovAttemptSub(const std::vector<DiceIdx>& org_dice, bool default_sim, const std::vector<bool> & rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	std::vector<std::vector<float>> createMarkovAttempt(bool default_sim, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	std::vector<std::vector<float>> createMarkovReroll(bool default_sim, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	float createMarkovChain(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls);
	std::vector<bool> createMarkovChainSolver(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card>& cards, float & erg_odd);

	//deprecated
	void createMarkovAttemptSubEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceIdx>& org_dice, const std::vector<bool>& rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	//recommended
	void createMarkovAttemptSubEigenRow(Eigen::MatrixXf& markovMatrix, const std::vector<DiceIdx>& org_dice, const std::vector<bool>& org_rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	void createMarkovAttemptEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	void createMarkovRerollEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	//deprecated O(n^3)
	std::vector<bool> createMarkovChainSolverEigen(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card>& cards, float& erg_odd);
	//recommended O(n^2)
	std::vector<bool> createMarkovChainSolverEigenRow(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card>& cards, float& erg_odd);
	
	size_t simcount_obj_ = 100000;

	//helper for generator
	std::vector<std::vector<DiceIdx>> samesis2_helper{};
	std::vector<std::vector<DiceIdx>> wild2_helper{};

	
	std::vector<std::vector<DiceThrow>> combo_store_;
	std::vector<std::vector<Card>> card_store_;

	std::vector<DiceThrow> possible_combs_;
	std::vector<DiceThrow> possible_list_with_cheat_dt; // containts all throws and its success (or not :D)
	std::vector<DiceThrow> possible_list_with_cheat_dt_succ_only;//helper for generation, only contains throws that are successfull with and without cards
	std::vector<DiceThrow> possible_combs_blow_up_;//helper for generation
	std::vector<DiceHash> possible_combs_blow_up_to_sorted_idx;//helper for generation

	// FINAL LIST FOR COMPARISON###############################################
	std::vector<DiceThrow> all_ability_combinations_dt;//final list
	std::vector<DiceThrow> all_ability_combinations_no_cards_dt;//final list

	std::vector<DiceThrow> loaded_all_ability_combinations_dt;//only for testing

	DiceThrow target_reroll_dice;
	DiceThrow target_reroller_start;

	//helper for MCTS and solver
	std::vector<DiceIdx> counted_bits_reroll{};
	std::vector<Combinations> all_combs{};//combinations with dice-data
	std::vector<std::vector<DiceHash>> allowed_rerolls_idx{};
	std::vector<std::vector<DiceHash>> allowed_single_rerolls_idx{};
	std::vector<std::vector<bool>> allowed_rerolls_vecs{};
	std::vector<std::vector<DiceHash>> allowed_rerolls_vecs_idx{};

	std::vector<ORPX> strat_ORPXs_{};
	std::vector<ORPX> throw_ORPXs_{};
	std::vector<DiceIdx> dont_reroll_{};
	std::vector<DiceIdx> allways_reroll_{};
	std::vector<DiceIdx> ability_target_{};
	std::vector<DiceIdx> solver_anatomy_{};

	std::vector<Combinations> all_combs2{};//combinations with symbol-data (is smaller (3 for AAABBC) as dice(6))

	//for faster generation:
	std::vector<DiceThrow> possible_combs_save;
	std::vector<DiceThrow> possible_combs_blow_up_save;//helper for generation
	std::vector<DiceHash> possible_combs_blow_up_to_sorted_idx_save;
	std::vector<DiceThrow> possible_list_with_cheat_dt_save;
	std::vector<DiceThrow>* possible_list_with_cheat_dt_save_last;
	std::vector<DiceThrow>* possible_list_with_cheat_dt_save_last2;
	std::vector<DiceIdx> generator_anatomy;
	std::vector<DiceIdx> generator_target;
	bool possible_calced_{ false };

	std::vector<ChaseData> chase_data_{};
	DiceRoller diceRoller_; // DiceRoller &

	// helper for getting best reroll
	std::vector<DiceThrow> temp_rerolls;
	std::vector<DiceThrow> best_rerolls;
	Helpers helper{};
	
};
