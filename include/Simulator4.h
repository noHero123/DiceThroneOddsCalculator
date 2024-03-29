#pragma once
#include <iostream>
#include <chrono>
#include <random>
#include <sstream>      // std::stringstream

#include <Eigen/Dense>
#include <unordered_map>

//#include "RandomGenerator.h"
#include "Types.h"
//#include "MontecarloDuct.h"
#include "ORP4.h"
#include "FasterRandomGenerator.h"
#include "Helpers.h"
#include "Logger.h"

class Simulator4
{
	typedef  ORP4 ORPX;
public:

	size_t number_dice_ = 4;//4;
	size_t possible_combs_blow_up_size_ = 1296;// 1296; // == 6^number_dice
	size_t sixit_anz_ = number_dice_;
	size_t samesis_anz_ = 12;//anz = 12;  == 2*(number_dice_ choose 2)
	size_t samesis2_anz_ = 24; //anz = 24; == 2*(number_dice_ choose 2) + 3*(number_dice_ choose 3)
	size_t tipit_anz_ = number_dice_ * 2;
	size_t tipit2_anz_ = number_dice_ * 4;
	size_t wilds_anz_ = number_dice_ * 6;
	size_t twiceaswild_anz_ = 240;//anz = 240; == (number_dice_ choose 2)*36 + wilds_anz_
	size_t slightlywild_anz_ = number_dice_ * 5;
	size_t max_combo_store_size = twiceaswild_anz_;

	Simulator4(DiceRoller& rg);

	std::vector<OddsResult> get_probability(std::string hero_name, const std::vector<std::string>& abilities, std::string diceanatomy, bool default_sim, bool chase, std::string chased_ability, std::vector<DiceIdx> org_dice, const CardData& cardData, size_t roll_atemps, size_t rerolls);

	DiceHash get_hash(const std::vector<DiceIdx>& v)
	{
		if (number_dice_ == 5)
		{
			return v[4] + v[3] * 6 + v[2] * 36 + v[1] * 216 + v[0] * 1296;
		}
		return v[3] + v[2] * 6 + v[1] * 36 + v[0] * 216;
		
	}

	DiceHash get_hash(const std::vector<DiceThrowDice>& v)
	{
		if (number_dice_ == 5)
		{
			return v[4].die + v[3].die * 6 + v[2].die * 36 + v[1].die * 216 + v[0].die * 1296;
		}
		return v[3].die + v[2].die * 6 + v[1].die * 36 + v[0].die * 216;
	}

	DiceHash get_hash(const DiceThrow& v)
	{
		if (number_dice_ == 5)
		{
			return v.dice[4] + v.dice[3] * 6 + v.dice[2] * 36 + v.dice[1] * 216 + v.dice[0] * 1296;
		}
		return v.dice[3] + v.dice[2] * 6 + v.dice[1] * 36 + v.dice[0] * 216;
	}

	DiceHash get_hash5(const std::vector<DiceThrowDice>& v)
	{
		return v[4].die + v[3].die * 6 + v[2].die * 36 + v[1].die * 216 + v[0].die * 1296;
	}

	DiceHash get_hash5(const std::vector<DiceIdx>& v)
	{
		return v[4] + v[3] * 6 + v[2] * 36 + v[1] * 216 + v[0] * 1296;
	}

	DiceHash get_hash(const std::vector<DiceThrowDice>& v, DiceIdx v4)
	{
		return v4 + v[3].die * 6 + v[2].die * 36 + v[1].die * 216 + v[0].die * 1296;
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
	void update_helper_data(size_t number_dice);


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
	std::vector<OddsResult> oddsCalculatorChase(std::string ability, std::vector<std::string> abilities, std::string diceanatomy, const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, const CardData& cardData);

	DiceIdx get_random_fast();

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
	void sort_combis(std::vector<std::vector<DiceIdx>>& erg);
	void sort_combis2(std::vector<std::vector<DiceIdx>>& erg);
	std::vector<std::vector<DiceIdx>> get_max_combis(const std::vector<std::vector<DiceIdx>>& erg);
	std::vector<std::vector<DiceIdx>> get_all_sub_combs(std::vector<DiceIdx> data, bool isDTA);
	std::vector<std::vector<size_t>> calc_all_permuts(const std::vector<Card>& cards, std::vector<size_t> v);

	//precalc with matrixes
	struct CardMatrixData
	{
		size_t cards_used = 0;
		size_t cp_used = 0;
		std::vector<DiceIdx> cards_combi = { 0,0,0,0,0,0,0,0 };
		size_t number_cards = 0;

		std::vector<Card> get_cards()
		{
			return Helpers::getCards(cards_combi[7], cards_combi[1], cards_combi[2], cards_combi[3], cards_combi[4], cards_combi[0], cards_combi[5], cards_combi[6]);
		}
	};

	void precalc_matrix_ability(bool calc_dta, size_t thread, size_t max_threads);
	void precalc_ability_matrix_part(bool isDTA, size_t thread, size_t max_threads);
	bool has_matrix_data(const std::vector<DiceIdx>& combi, bool isDTA, bool sim4, size_t thread);
	void save_matrix_to_sqlite(const CardMatrixData& mdata, const Eigen::MatrixXi& matrix, bool isDTA, size_t thread);
	void createCardMatrix(const Card& card, Eigen::MatrixXi& markovMatrix);
	void calculateMatrices(const CardMatrixData& mdata, std::vector<CardMatrixData>& matrices_data, std::vector<Eigen::MatrixXi>& matrices, const std::vector<Card>& card_matrices, const std::vector<Eigen::MatrixXi>& basic_matrices);
	void load_combs_from_matrix(std::string ability_name, std::string diceanatomy, const Eigen::MatrixXi& tempmat);
	bool read_ability_matrix(std::string ability_name, std::string diceanatomy, const std::vector<Card>& cards, size_t cp, size_t numbercards);

	//default one
	bool find_keepers_comb_ultra_fast(std::vector<DiceIdx>& dice, std::vector<bool>& rerollers, bool randomize, bool just_one_reroll, const std::vector<DiceThrow>& all_combis);

	const DiceThrow* find_keepers_comb_ultra_faster21(const DiceThrow* rerollers);
	const DiceThrow* find_keepers_comb_ultra_faster_one_reroll21(const DiceThrow* rerollers);
	bool go_for_comb_ultra_faster2(size_t atmp, size_t reroll, const DiceThrow* rerollers);
	size_t combo_sim_ultra_faster_reroll_DT(const std::vector<DiceIdx>& org_dice, std::vector<bool>& org_rerollers, size_t roll_atemps, size_t rerolls, bool is_default_sim);

	const DiceThrow* go_for_comb_ultra_faster2_chase(size_t atmp, size_t reroll, const DiceThrow* rerollers);
	size_t combo_sim_ultra_faster_reroll_DT_chase(const std::vector<DiceIdx>& org_dice, std::vector<bool>& org_rerollers, size_t roll_atemps, size_t rerolls, bool is_default_sim);

	bool is_straight(DiceThrow& dt, bool is_big);
	bool is_big_straight(const DiceThrow& dt);
	void update_rerollers(DiceThrow& dt);

	std::vector<bool> test_all_rolls(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, float& erg_odd);

	//std::vector<bool> test_montecarlo(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls);
	//std::vector<MCTSDUCTSmallFast::Node<ORPX>> ductNodes_{};

	void get_all_possible_combinations(size_t number_dice, size_t number_sides);

	void test_loaded_and_gen();

	size_t get_amount_of_throw(ORPX& ORPX, size_t throw_idx);
	float solve_throw(size_t lvl, size_t& best_idx);
	float solve_throw_reroller(size_t lvl, size_t& best_idx, bool is_default_sim);
	std::vector<bool> test_solver(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, std::string dice_anatomy, std::string target, const std::vector<Card>& cards, bool default_odd, bool get_rerolls, float& erg_odd);
	

	std::vector<size_t> sort_indexes(const std::vector<DiceThrow>& values);
	Eigen::MatrixXf binary_exponentation(const Eigen::MatrixXf& A, unsigned e);
	void createMarkovAttemptSubEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceIdx>& org_dice, const std::vector<bool>& rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	void createMarkovAttemptSubEigenRow(Eigen::MatrixXf& markovMatrix, const std::vector<DiceIdx>& org_dice, const std::vector<bool>& org_rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	void createMarkovAttemptEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	void createMarkovRerollEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list);
	std::vector<bool> createMarkovChainSolverEigen(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card>& cards, float& erg_odd);
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
	std::vector<DiceThrow>* possible_list_with_cheat_dt_save_upper;
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
