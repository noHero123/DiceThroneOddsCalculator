#include <algorithm>
#include "Simulator.h"

#include <sstream>
#include <iterator>
#include <numeric>




Simulator::Simulator(DiceRoller& rg) : diceRoller_{rg} // diceRoller_{rg}
{
    if (number_dice_ >= 6)
    {
        std::cout << "ERROR doesnt support more than 5 dice\r\n";
    }

    /*std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distr(0, 5);//for number generation 
    gen_ = gen;
    distr_ = distr;*/

    
    combo_store_.clear();
    card_store_.clear();

    for (size_t i = 0; i < 10; i++)
    {
        std::vector<DiceThrow> a1;
        a1.resize(max_combo_store_size);
        combo_store_.push_back(a1);
    }
    for (size_t i = 0; i < 11; i++)
    {
        std::vector<Card> a1;
        a1.resize(10);
        card_store_.push_back(a1);
    }
    //all combs for samesis2:
    samesis2_helper.clear();
    for (DiceIdx i = 0; i < number_dice_; i++)
    {
        for (DiceIdx j = 0; j < number_dice_; j++)
        {
            for (DiceIdx k = j; k < number_dice_; k++)
            {
                if (i != j && i != k)
                {
                    samesis2_helper.push_back({ i,j,k });
                }
            }
        }
    }
    //all combs for wild2:
    wild2_helper.clear();
    for (DiceIdx i = 0; i < number_dice_; i++)
    {
        for (DiceIdx k = 0; k < 6; k++)
        {
            wild2_helper.push_back({ i,k, i,k });
        }
    }
    for (DiceIdx i = 0; i < number_dice_; i++)
    {
        for (DiceIdx j = i+1; j < number_dice_; j++)
        {
            for (DiceIdx k = 0; k < 6; k++)
            {
                for (DiceIdx l = 0; l < 6; l++)
                {
                    wild2_helper.push_back({ i,k, j, l });
                }
            }
        }
    }

    // DATA FOR MCTS ###########################################################################
    all_combs.clear();
    all_combs.resize(number_dice_);
    std::vector<DiceIdx> numbers = {0,1,2,3,4,5};
    std::vector<DiceIdx> line{};
    size_t pow = 6;//because we start with i = 1 and not i = 0
    for (size_t i = 1; i <= number_dice_; i++)
    {
        size_t idx = i - 1;
        line.clear();
        all_combs[idx].lenght = i;
        Helpers::getCombinations(numbers, i, line, all_combs[idx].combs);
        all_combs[idx].combs_size = pow;
        pow *= 6;
        get_all_possible_combinations(i);
        all_combs[idx].combs_sorted.clear();
        all_combs[idx].combs_sorted_amount.clear();
        for (size_t j = 0; j < possible_combs_.size(); j++)
        {
            std::vector<DiceIdx> roll{};
            for (size_t k = 0; k < i; k++)
            {
                roll.push_back(possible_combs_[j].dice[k]);
            }
            all_combs[idx].combs_sorted.push_back(roll);
            all_combs[idx].combs_sorted_amount.push_back(0);
        }
        
        all_combs[idx].dicehash_to_combs_sorted.clear();
        all_combs[idx].dicehash_to_combs_sorted.resize(all_combs[idx].combs_size);
        for (DiceHash j  = 0; j< all_combs[idx].combs_sorted.size(); j++)
        {
            std::vector<DiceIdx> v = all_combs[idx].combs_sorted[j];
            do
            {
                size_t idxhash = get_hash_smaller(v);
                all_combs[idx].dicehash_to_combs_sorted[idxhash] = j;
                all_combs[idx].combs_sorted_amount[j]++;
            } while (std::next_permutation(v.begin(), v.end()));
        }

    }

    allowed_rerolls_idx.clear();
    allowed_single_rerolls_idx.clear();
    allowed_rerolls_vecs.clear();
    allowed_rerolls_idx.push_back({});
    allowed_single_rerolls_idx.push_back({});
    size_t two_power_num_dice = 1 << number_dice_;
    
    for (size_t i = 1; i < two_power_num_dice; i++)
    {
        std::vector<DiceHash> allowed_idx{};
        auto bitsi = Helpers::getBitArray(i, number_dice_);
        for (DiceHash j = 1; j < two_power_num_dice; j++)
        {
            auto bitsj = Helpers::getBitArray(j, number_dice_);
            bool allowed = true;
            for (size_t k = 0; k < number_dice_; k++)
            {
                if (bitsi[k] == false && bitsj[k] == true)
                {
                    allowed = false;
                    break;
                }
            }
            if (allowed)
            {
                allowed_idx.push_back(j);
            }
        }
        allowed_rerolls_idx.push_back(allowed_idx);
        //single bits;
        allowed_idx.clear();
        DiceHash power2 = 1;
        for (size_t j = 0; j < number_dice_; j++)
        {
            DiceHash valj = power2; //std::pow(2, j);
            power2 *= 2;
            auto bitsj = Helpers::getBitArray(valj, number_dice_);
            bool allowed = true;
            for (size_t k = 0; k < number_dice_; k++)
            {
                if (bitsi[k] == false && bitsj[k] == true)
                {
                    allowed = false;
                    break;
                }
            }
            if (allowed)
            {
                allowed_idx.push_back(valj);
            }
        }
        allowed_single_rerolls_idx.push_back(allowed_idx);
    }
    counted_bits_reroll.clear();
    allowed_rerolls_vecs_idx.clear();
    for (size_t i = 0; i < two_power_num_dice; i++)
    {
        allowed_rerolls_vecs.push_back(Helpers::getBitArray(i, 5));
        DiceIdx bits_count = 0;
        std::vector<DiceHash> used_indexes{};
        for (DiceIdx j = 0; j < number_dice_; j++)
        {
            if (allowed_rerolls_vecs.back()[j])
            {
                bits_count++;
                used_indexes.push_back(j);
            }
        }
        counted_bits_reroll.push_back(bits_count);
        allowed_rerolls_vecs_idx.push_back(used_indexes);
    }

    //######################################################################################
    //ductNodes_.resize(1000001);
    //######################################################################################
    //data for database
    get_all_possible_combinations(number_dice_); // gets all diceroll combinations with 5 dice, sorted.
};

void Simulator::generate_data_for_MCTS(const std::vector<DiceIdx>& dice_anatomy)
{
    //TODO only generate new, if dice_anatomy changed
    // DATA FOR MCTS2 ###########################################################################
    all_combs2.clear();
    all_combs2.resize(5);
    std::vector<DiceIdx> numbers = {};
    std::vector<DiceIdx> line{};

    for (size_t i = 0; i < 6; i++)
    {
        DiceIdx val = dice_anatomy[i];
        bool found = false;
        for (const auto& v : numbers)
        {
            if (v == val)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            numbers.push_back(val);
        }
    }

    size_t pow = 6;//because we start with i = 1 and not i = 0
    size_t small_pow = numbers.size();
    for (size_t i = 1; i <= number_dice_; i++)
    {
        size_t idx = i - 1;
        line.clear();
        all_combs2[idx].lenght = i;
        Helpers::getCombinations(numbers, i, line, all_combs2[idx].combs);
        all_combs2[idx].combs_size = pow;
        pow *= 6;
        get_all_possible_combinations(i, numbers.size());
        all_combs2[idx].combs_sorted.clear();
        all_combs2[idx].combs_sorted_amount.clear();
        for (size_t j = 0; j < possible_combs_.size(); j++)
        {
            std::vector<DiceIdx> roll{};
            for (size_t k = 0; k < i; k++)
            {
                roll.push_back(possible_combs_[j].dice[k]);
            }
            all_combs2[idx].combs_sorted.push_back(roll);
            all_combs2[idx].combs_sorted_amount.push_back(0);
        }

        //small mapping:
        std::vector<DiceHash> anatomy_hash_to_combs_sort{};
        anatomy_hash_to_combs_sort.resize(small_pow);
        small_pow *= numbers.size();
        for (DiceHash j = 0; j < possible_combs_.size(); j++)
        {
            std::vector<DiceIdx> v{};
            for (size_t k = 0; k < i; k++)
            {
                v.push_back(possible_combs_[j].dice[k]);
            }
            do
            {
                size_t idxhash = get_hash_smaller(v, numbers.size());
                anatomy_hash_to_combs_sort[idxhash] = j;
            } while (std::next_permutation(v.begin(), v.end()));
        }
        //get all dice combis
        get_all_possible_combinations(i);
        all_combs2[idx].dicehash_to_combs_sorted.clear();
        all_combs2[idx].dicehash_to_combs_sorted.resize(all_combs2[idx].combs_size);
        for (size_t j = 0; j < possible_combs_.size(); j++)
        {
            std::vector<DiceIdx> v{};
            std::vector<DiceIdx> anatomied{};
            for (size_t k = 0; k < i; k++)
            {
                v.push_back(possible_combs_[j].dice[k]);
                anatomied.push_back(dice_anatomy[possible_combs_[j].dice[k]]);
            }
            size_t anatomyhash = get_hash_smaller(anatomied, numbers.size());
            DiceHash combtarget = anatomy_hash_to_combs_sort[anatomyhash];
            do
            {
                size_t idxhash = get_hash_smaller(v);
                all_combs2[idx].dicehash_to_combs_sorted[idxhash] = combtarget;
                all_combs2[idx].combs_sorted_amount[combtarget]++;
            } while (std::next_permutation(v.begin(), v.end()));
        }
    }
    get_all_possible_combinations(number_dice_);
}

DiceIdx Simulator::get_random_fast()
{
    return diceRoller_.getDiceRollFast();
    //return helper->diceRoller_.getDiceRoll();
    //return (size_t)RandomGenerator::uniform_int_distribution(0, 5, gen_);
    //return distr_(gen_);
}

//simulator solver######################################################################################################################

std::vector<bool> Simulator::test_all_rolls(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, float & erg_odd)
{
    size_t fast_sims = 500000;
    size_t normal_sims = 750000;
    size_t large_sims = 1000000;
    size_t done_simuls = 0U;
    
    simcount_obj_ = fast_sims;
    std::vector<bool> bestbits = { false, false, false, false, false };
    size_t best_erg = 0;
    size_t best_erg_decimal = 0;
    size_t tmp_roll_at = roll_atemps;
    size_t tmp_rerolls = rerolls;
    bool dont_add_erg = false; // if false, we can reuse the results of the first phase
    //to get the best starting roll, we dont need more than 2 roll atemps and no rerolls
    if (roll_atemps >= 2)
    {
        tmp_roll_at = 2;
        tmp_rerolls = 0;
        if (tmp_roll_at != roll_atemps ||  tmp_rerolls != rerolls)
        {
            dont_add_erg = true;
        }
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    if (is_default_sim)
    {
        bestbits = { true, true, true, true, true };
    }
    else
    {
        if (roll_atemps >= 1)
        {
            // PHASE 1: test all possible re-throw combination and get best combination!
            done_simuls = fast_sims;
            size_t max = 1 << number_dice_;
            for (size_t i = 1; i < max; i++)
            {
                std::vector<bool> bits = Helpers::getBitArray(i, number_dice_);
                size_t erg = combo_sim_ultra_faster_reroll_DT(org_dice, bits, tmp_roll_at, tmp_rerolls, is_default_sim);
                for (auto b : bits)
                {
                    std::cout << b ? "1" : "0";
                }
                std::cout << " prob for " << i<< " is " << (100.0F*erg) / ((float)(fast_sims)) << "%" << std::endl;
                if (erg > best_erg)
                {
                    best_erg = erg;
                    bestbits = bits;
                    best_erg_decimal = i;
                }
            }
        }
        else
        {
            //for single rerolls, best stratgegy is just to reroll non matching ones
            const auto& dt = all_ability_combinations_dt[get_hash(org_dice)];
            bestbits[0] = dt.rerollers[0];
            bestbits[1] = dt.rerollers[1];
            bestbits[2] = dt.rerollers[2];
            bestbits[3] = dt.rerollers[3];
            bestbits[4] = dt.rerollers[4];// TODO
        }
    }

    size_t second_sim = is_default_sim ? large_sims : normal_sims;
    simcount_obj_ = second_sim;
    if (dont_add_erg)
    {
        done_simuls = 0U;
        best_erg = 0U;
    }
    done_simuls += second_sim;
    // PHASE 2: get more simulations on best combination
    best_erg += combo_sim_ultra_faster_reroll_DT(org_dice, bestbits, roll_atemps, rerolls, is_default_sim);

    float best_erg_f = (100U * best_erg) / ((float)(done_simuls));

    erg_odd = best_erg_f;
    return bestbits;
}

std::vector<bool> Simulator::test_all_rolls_chase(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, float& erg_odd)
{
    size_t fast_sims = 100000;
    size_t normal_sims = 2000000;
    size_t large_sims = 5000000;
    size_t done_simuls = 0U;

    simcount_obj_ = fast_sims;
    std::vector<bool> bestbits = { false, false, false, false, false };
    size_t best_erg = 0;
    size_t best_erg_decimal = 0;
    size_t tmp_roll_at = roll_atemps;
    size_t tmp_rerolls = rerolls;
    //to get the best starting roll, we dont need more than 2 roll atemps and no rerolls
    if (roll_atemps >= 2)
    {
        tmp_roll_at = 2;
        tmp_rerolls = 0;
    }
    auto start_time = std::chrono::high_resolution_clock::now();
    if (is_default_sim)
    {
        bestbits = { true, true, true, true, true };
    }
    else
    {
        if (roll_atemps >= 1)
        {
            // PHASE 1: test all possible re-throw combination and get best combination!
            done_simuls = fast_sims;
            size_t max = 1 << number_dice_;
            for (size_t i = 1; i < max; i++)
            {
                std::vector<bool> bits = Helpers::getBitArray(i, number_dice_);
                size_t erg = combo_sim_ultra_faster_reroll_DT(org_dice, bits, tmp_roll_at, tmp_rerolls, is_default_sim);
                /*for (auto b : bits)
                {
                    std::cout << b ? "1" : "0";
                }
                std::cout << " prob for " << i<< " is " << erg << "%" << std::endl;*/
                if (erg > best_erg)
                {
                    best_erg = erg;
                    bestbits = bits;
                    best_erg_decimal = i;
                }
            }
        }
        else
        {
            //for single rerolls, best stratgegy is just to reroll non matching ones
            const auto& dt = all_ability_combinations_dt[get_hash(org_dice)];
            bestbits[0] = dt.rerollers[0];
            bestbits[1] = dt.rerollers[1];
            bestbits[2] = dt.rerollers[2];
            bestbits[3] = dt.rerollers[3];
            bestbits[4] = dt.rerollers[4];
        }
    }

    size_t second_sim = is_default_sim ? large_sims : normal_sims;
    simcount_obj_ = second_sim;
    done_simuls = second_sim;
    // PHASE 2: get more simulations on best combination
    best_erg = combo_sim_ultra_faster_reroll_DT_chase(org_dice, bestbits, roll_atemps, rerolls, is_default_sim);

    for (auto& cd : chase_data_)
    {
        cd.erg = (100U * cd.amount) / ((float)(done_simuls));
    }

    float best_erg_f = (100U * best_erg) / ((float)(done_simuls));

    erg_odd = best_erg_f;

    return bestbits;
}

//ODDS SOLVER##############################################################################

size_t Simulator::get_amount_of_throw(ORPX& ORPX, size_t throw_idx)
{
    size_t number_bits = counted_bits_reroll[ORPX.last_move];
    const auto& db = (*ORPX.all_combs)[number_bits - 1];
    return  db.combs_sorted_amount[throw_idx];
}

float Simulator::solve_throw(size_t lvl, size_t& best_idx)
{
    //TODO  std::vector<size_t>&
    ORPX& child1 = strat_ORPXs_[lvl];
    //std::vector<size_t> moves = child1.get_rerolls();
    std::vector<DiceHash> moves = child1.get_rerolls(dont_reroll_, allways_reroll_, ability_target_, solver_anatomy_);
    float max_erg = -1.0F;
    size_t best_mov = 0;
    for (size_t mov : moves)
    {
        size_t success = 0;
        size_t rolls = 0;
        float child_ergs = 0.0F;

        child1.last_move = mov;
        if (child1.roll_atempts > 0)
        {
            child1.last_reroll_move = mov;
        }

        size_t roled_dice = child1.get_number_avail_dice_trows();
        float total = (float)child1.get_total_of_throws();
        for (size_t dt_move = 0; dt_move < roled_dice; dt_move++)
        {
            //size_t amount = child1.get_amount_of_throw(dt_move);
            size_t amount = get_amount_of_throw(child1, dt_move);
            rolls += amount;
            ORPX& child2 = throw_ORPXs_[lvl];
            //copy stuff
            child2.dice[0] = child1.dice[0];
            child2.dice[1] = child1.dice[1];
            child2.dice[2] = child1.dice[2];
            child2.dice[3] = child1.dice[3];
            child2.dice[4] = child1.dice[4];
            child2.last_move = child1.last_move;
            child2.rerolls = child1.rerolls;
            child2.roll_atempts = child1.roll_atempts;
            child2.win = false;

            child2.do_roll(dt_move);
            if (child2.is_finished())
            {
                if (child2.win)
                {
                    success += amount;
                }
            }
            else
            {
                ORPX& child3 = strat_ORPXs_[lvl + 1];
                child3.dice[0] = child2.dice[0];
                child3.dice[1] = child2.dice[1];
                child3.dice[2] = child2.dice[2];
                child3.dice[3] = child2.dice[3];
                child3.dice[4] = child2.dice[4];
                child3.last_move = child2.last_move;
                child3.rerolls = child2.rerolls;
                child3.roll_atempts = child2.roll_atempts;
                const float erg_t = (solve_throw(lvl + 1, best_idx));
                child_ergs += erg_t * (amount / total);
                /*if (lvl == 0 && mov == 31)
                {
                    std::cout << child2.dice[0] << child2.dice[1]<<child2.dice[2] << child2.dice[3] << child2.dice[4]<<" " << erg_t << std::endl;
                }*/
            }
        }

        float erg = success / ((float)rolls);
        float erg_ges = erg + child_ergs;

        if (max_erg < erg_ges)
        {
            max_erg = erg_ges;
            best_mov = mov;
        }
    }
    best_idx = best_mov;
    return max_erg;
}

// TODO DELETE and use solve_throw_reroller instead! 
float Simulator::solve_default_throw(size_t lvl, size_t& best_idx)
{
    //TODO  std::vector<size_t>&
    ORPX& child1 = strat_ORPXs_[lvl];
    std::vector<size_t> moves = { (1 << number_dice_) - 1 };
    float max_erg = -1.0F;
    size_t best_mov = 0;
    for (size_t mov : moves)
    {
        size_t success = 0;
        size_t rolls = 0;
        float child_ergs = 0.0F;

        child1.last_move = mov;
        if (child1.roll_atempts > 0)
        {
            child1.last_reroll_move = mov;
        }

        size_t roled_dice = child1.get_number_avail_dice_trows();
        float total = (float)child1.get_total_of_throws();
        for (size_t dt_move = 0; dt_move < roled_dice; dt_move++)
        {
            //size_t amount = child1.get_amount_of_throw(dt_move);
            size_t amount = get_amount_of_throw(child1, dt_move);
            rolls += amount;
            ORPX& child2 = throw_ORPXs_[lvl];
            //copy stuff
            child2.dice[0] = child1.dice[0];
            child2.dice[1] = child1.dice[1];
            child2.dice[2] = child1.dice[2];
            child2.dice[3] = child1.dice[3];
            child2.dice[4] = child1.dice[4];
            child2.last_move = child1.last_move;
            child2.rerolls = child1.rerolls;
            child2.roll_atempts = child1.roll_atempts;
            child2.win = false;

            child2.do_roll(dt_move);
            if (child2.is_finished())
            {
                if (child2.win)
                {
                    success += amount;
                }
            }
            else
            {
                ORPX& child3 = strat_ORPXs_[lvl + 1];
                child3.dice[0] = child2.dice[0];
                child3.dice[1] = child2.dice[1];
                child3.dice[2] = child2.dice[2];
                child3.dice[3] = child2.dice[3];
                child3.dice[4] = child2.dice[4];
                child3.last_move = child2.last_move;
                child3.rerolls = child2.rerolls;
                child3.roll_atempts = child2.roll_atempts;
                const float erg_t = (solve_throw(lvl + 1, best_idx));
                child_ergs += erg_t * (amount / total);
                /*if (lvl == 0 && mov == 31)
                {
                    std::cout << child2.dice[0] << child2.dice[1]<<child2.dice[2] << child2.dice[3] << child2.dice[4]<<" " << erg_t << std::endl;
                }*/
            }
        }

        float erg = success / ((float)rolls);
        float erg_ges = erg + child_ergs;

        if (max_erg < erg_ges)
        {
            max_erg = erg_ges;
            best_mov = mov;
        }
    }
    best_idx = best_mov;
    return max_erg;
}

float Simulator::solve_throw_reroller(size_t lvl, size_t& best_idx, bool is_default_sim)
{
    //TODO  std::vector<size_t>&
    ORPX& child1 = strat_ORPXs_[lvl];
    std::vector<DiceHash> moves = { (1 << number_dice_) - 1 };
    if (!is_default_sim)
    {
        moves = child1.get_rerolls(dont_reroll_, allways_reroll_, ability_target_, solver_anatomy_);
    }
    float max_erg = -1.0F;
    size_t best_mov = 0;
    for (size_t mov : moves)
    {
        size_t success = 0;
        size_t rolls = 0;
        float child_ergs = 0.0F;

        child1.last_move = mov;
        if (child1.roll_atempts > 0)
        {
            child1.last_reroll_move = mov;
        }

        size_t roled_dice = child1.get_number_avail_dice_trows();
        float total = (float)child1.get_total_of_throws();
        for (size_t dt_move = 0; dt_move < roled_dice; dt_move++)
        {
            //size_t amount = child1.get_amount_of_throw(dt_move);
            size_t amount = get_amount_of_throw(child1, dt_move);
            rolls += amount;
            ORPX& child2 = throw_ORPXs_[lvl];
            //copy stuff
            child2.dice[0] = child1.dice[0];
            child2.dice[1] = child1.dice[1];
            child2.dice[2] = child1.dice[2];
            child2.dice[3] = child1.dice[3];
            child2.dice[4] = child1.dice[4];
            child2.last_move = child1.last_move;
            child2.rerolls = child1.rerolls;
            child2.roll_atempts = child1.roll_atempts;
            child2.win = false;

            child2.do_roll(dt_move);
            if (child2.is_finished())
            {
                if (child2.win)
                {
                    success += amount;
                }
                if (lvl == 0)
                {
                    DiceHash current_hash = get_hash(child2.dice);
                    DiceHash targetidx = all_combs[number_dice_ - 1].dicehash_to_combs_sorted[current_hash];
                    const auto& reros = possible_list_with_cheat_dt[targetidx].rerollers;
                    temp_rerolls[targetidx].dice[0] = child2.dice[0];
                    temp_rerolls[targetidx].dice[1] = child2.dice[1];
                    temp_rerolls[targetidx].dice[2] = child2.dice[2];
                    temp_rerolls[targetidx].dice[3] = child2.dice[3];
                    temp_rerolls[targetidx].dice[4] = child2.dice[4];
                    temp_rerolls[targetidx].rerollers[0] = reros[0];
                    temp_rerolls[targetidx].rerollers[1] = reros[1];
                    temp_rerolls[targetidx].rerollers[2] = reros[2];
                    temp_rerolls[targetidx].rerollers[3] = reros[3];
                    temp_rerolls[targetidx].rerollers[4] = reros[4];
                }
            }
            else
            {
                ORPX& child3 = strat_ORPXs_[lvl + 1];
                child3.dice[0] = child2.dice[0];
                child3.dice[1] = child2.dice[1];
                child3.dice[2] = child2.dice[2];
                child3.dice[3] = child2.dice[3];
                child3.dice[4] = child2.dice[4];
                child3.last_move = child2.last_move;
                child3.rerolls = child2.rerolls;
                child3.roll_atempts = child2.roll_atempts;
                const float erg_t = (solve_throw(lvl + 1, best_idx));
                child_ergs += erg_t * (amount / total);

                if (lvl == 0)
                {
                    DiceHash current_hash = get_hash(child2.dice);
                    DiceHash targetidx = all_combs[number_dice_ - 1].dicehash_to_combs_sorted[current_hash];
                    const auto& reros = allowed_rerolls_vecs[best_idx];
                    temp_rerolls[targetidx].dice[0] = child2.dice[0];
                    temp_rerolls[targetidx].dice[1] = child2.dice[1];
                    temp_rerolls[targetidx].dice[2] = child2.dice[2];
                    temp_rerolls[targetidx].dice[3] = child2.dice[3];
                    temp_rerolls[targetidx].dice[4] = child2.dice[4];
                    temp_rerolls[targetidx].rerollers[0] = reros[0];
                    temp_rerolls[targetidx].rerollers[1] = reros[1];
                    temp_rerolls[targetidx].rerollers[2] = reros[2];
                    temp_rerolls[targetidx].rerollers[3] = reros[3];
                    temp_rerolls[targetidx].rerollers[4] = reros[4];
                }
            }
        }

        float erg = success / ((float)rolls);
        float erg_ges = erg + child_ergs;

        if (max_erg < erg_ges)
        {
            max_erg = erg_ges;
            best_mov = mov;
            if (lvl == 0)
            {
                best_rerolls = temp_rerolls;
            }
        }
    }
    best_idx = best_mov;
    return max_erg;
}


std::vector<bool> Simulator::test_solver(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, std::string dice_anatomy, std::string target, const std::vector<Card>& cards, bool default_odd, bool get_rerolls, float& erg_odd)
{
    if (!default_odd && all_ability_combinations_dt[get_hash(org_dice)].success)
    {
        erg_odd = 100.0F;
        std::vector<bool> bits = Helpers::getBitArray(0, number_dice_);
        return bits;
    }

    int tip_it_lvl = 0;
    for (const auto& c : cards)
    {
        if (c.function_to_call == 3)
        {
            tip_it_lvl = 1;
        }
        if (c.function_to_call == 4)
        {
            tip_it_lvl = 2;
        }
    }
    std::vector<DiceIdx> mydiceanatomy = Helpers::transformDiceAnatomy(dice_anatomy);
    std::vector<DiceIdx> target_ability = Helpers::transformAbility(target);

    bool use_dice_anatomy = true;// = tip_it_lvl == 0; // DONT USE if TIPIT and Straights!
    if (tip_it_lvl != 0 || target_ability[0] >= 7 || get_rerolls)
    {
        use_dice_anatomy = false;
    }
    std::vector<DiceIdx> invered_anatomie = { 0,1,2,3,4,5 };
    ORPX ORPX;
    ORPX.dice = org_dice;
    ORPX.roll_atempts = roll_atemps;
    ORPX.rerolls = rerolls;
    ORPX.all_ability_combinations_dt = &(this->all_ability_combinations_dt);
    ORPX.all_combs = &(this->all_combs);
    ORPX.allowed_rerolls_idx = &(this->allowed_rerolls_idx);
    ORPX.allowed_single_rerolls_idx = &(this->allowed_single_rerolls_idx);
    ORPX.allowed_rerolls_vecs = &(this->allowed_rerolls_vecs);
    ORPX.inverted_dice_anatomy = &invered_anatomie;
    ORPX.counted_bits = &counted_bits_reroll;

    ability_target_.clear();
    solver_anatomy_.clear();
    for (const auto& v : target_ability)
    {
        ability_target_.push_back((size_t)v);
    }
    for (const auto& v : mydiceanatomy)
    {
        solver_anatomy_.push_back((size_t)v);
    }

    //collect die, that you should not reroll (die with probability 1/6)
    dont_reroll_.clear();
    if (target_ability[0] < 7)
    {
        std::vector<size_t> probs = { 0,0,0,0,0,0 };
        for (size_t i = 0; i < 6; i++)
        {
            probs[mydiceanatomy[i]]++;
        }

        size_t minprob = 10;
        for (DiceIdx i = 0; i < 6; i++)
        {
            if (target_ability[i] > 0)
            {
                size_t prob = probs[i];
                if (minprob > prob)
                {
                    minprob = prob;
                }
            }
        }
        if (minprob == 1)
        {
            for (DiceIdx i = 0; i < 6U; i++)
            {
                size_t symbol = mydiceanatomy[i];
                if (target_ability[symbol] > 0 && probs[symbol] == minprob)
                {
                    dont_reroll_.push_back(i);
                }
            }
        }
    }

    //collect die, that you should allways reroll:
    allways_reroll_.clear();
    if (target_ability[0] < 7U)
    {
        for (DiceIdx i = 0; i < 6U; i++)
        {
            bool needed = false;
            size_t symbol = mydiceanatomy[i];
            if (target_ability[symbol] > 0)
            {
                needed = true;
            }
            if (tip_it_lvl >= 1)
            {
                if (i > 0)
                {
                    symbol = mydiceanatomy[i - 1];
                    if (target_ability[symbol] > 0)
                    {
                        needed = true;
                    }
                }
                if (i < 5)
                {
                    symbol = mydiceanatomy[i + 1];
                    if (target_ability[symbol] > 0)
                    {
                        needed = true;
                    }
                }

            }
            if (tip_it_lvl >= 2)
            {
                if (i > 1)
                {
                    symbol = mydiceanatomy[i - 2];
                    if (target_ability[symbol] > 0)
                    {
                        needed = true;
                    }
                }
                if (i < 4)
                {
                    symbol = mydiceanatomy[i + 2];
                    if (target_ability[symbol] > 0)
                    {
                        needed = true;
                    }
                }

            }
            if (needed == false)
            {
                allways_reroll_.push_back(i);
            }

        }
    }
    //allways_reroll_.clear();

    if (use_dice_anatomy)
    {
        generate_data_for_MCTS(mydiceanatomy);
        invered_anatomie = { 0,0,0,0,0,0 };
        for (DiceIdx i = 0; i < 6U; i++)
        {
            invered_anatomie[mydiceanatomy[i]] = i;
        }

        for (size_t i = 0; i < org_dice.size(); i++)
        {
            ORPX.dice[i] = (size_t)mydiceanatomy[org_dice[i]];
        }
        ORPX.all_combs = &(this->all_combs2);
        ORPX.inverted_dice_anatomy = &invered_anatomie;//maps a A to a 0, a B to a 4 and a C to a 5! (+1 for real dice values ;) )
    }
    const auto allowed1 = ORPX.get_rerolls();
    const auto allowed2 = ORPX.get_rerolls(dont_reroll_, allways_reroll_, ability_target_, solver_anatomy_);

    strat_ORPXs_.clear();
    throw_ORPXs_.clear();
    for (size_t i = 0; i < ORPX.roll_atempts + ORPX.rerolls + 1; i++) // +1 just to be sure :D
    {
        strat_ORPXs_.push_back(ORPX);
        throw_ORPXs_.push_back(ORPX);
    }

    //solve throw:
    size_t best_move = 0;
    float best_prob = 0.0F;

    if (get_rerolls)
    {
        best_rerolls = possible_list_with_cheat_dt;
        temp_rerolls = possible_list_with_cheat_dt;
        best_prob = solve_throw_reroller(0, best_move, default_odd);
    }
    else
    {
        if (default_odd)
        {
            best_prob = solve_default_throw(0, best_move);
        }
        else
        {
            best_prob = solve_throw(0, best_move);
        }
    }
    std::vector<bool> bits = Helpers::getBitArray(best_move, number_dice_);
    erg_odd = best_prob * 100.0F;
    return bits;
}

// calculators
bool  Simulator::use_solver(const std::string& diceanatomy, const std::string& rollTarget, size_t roll_atemps, size_t rerolls, const std::vector<Card>& cards, int cp, size_t anz_cards)
{
    //never use solver on tipit
    bool has_tipit = false;
    if (anz_cards >= 1)
    {
        for (const auto& c : cards)
        {
            if (c.card_id == 2 && c.cp_cost <= cp) //tipit
            {
                has_tipit = true;
            }
        }
    }
    bool isStraight = false;
    if (rollTarget == "SMALL" || rollTarget == "BIG")
    {
        isStraight = true;
    }
    
    size_t number_symbols = 0;
    std::vector<char> used_chars{};
    for (char const& c : diceanatomy)
    {
        bool found = false;
        for (const auto& ch : used_chars)
        {
            if (ch == c)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            number_symbols++;
            used_chars.push_back(c);
        }
    }
    size_t a = roll_atemps;
    size_t r = rerolls;
    // till: 2 roll_atemps 1 rerolls / 1 atemps, 6 rerolls/ 0 atemps, 10 rerolls  AND (tipit OR straight) -> calc odds
    // no straight / tipit: Xa, Yr = X atemps, Y rerolls
    // 4 symbols: 3a, 2r / 2a, 6r/ 1a, 10/ 0a, 15r
    // 3 symbols: 5a, 0r/ 4a, 4r/ 3a, 8r/ 2a, 12r/1a, 17r/ 0a, 23r
    if (isStraight || has_tipit)
    {
        if (a == 2 && r <= 1)
        {
            return true;
        }
        if (a == 1 && r <= 6)
        {
            return true;
        }
        if (a == 0 && r <= 10)
        {
            return true;
        }
    }
    else
    {
        if (number_symbols >= 4)
        {
            if (a == 3 && r <= 2)
            {
                return true;
            }
            if (a == 2 && r <= 6)
            {
                return true;
            }
            if (a == 1 && r <= 10)
            {
                return true;
            }
            if (a == 0 && r <= 15)
            {
                return true;
            }
        }
        if (number_symbols <= 3)
        {
            if (a == 5 && r <= 0)
            {
                return true;
            }
            if (a == 4 && r <= 4)
            {
                return true;
            }
            if (a == 3 && r <= 8)
            {
                return true;
            }
            if (a == 2 && r <= 12)
            {
                return true;
            }
            if (a == 1 && r <= 17)
            {
                return true;
            }
            if (a == 0 && r <= 23)
            {
                return true;
            }
        }
    }

    return false;
}

void Simulator::test_random_gens()
{
    /*size_t anz = 1000000000;
    float anzf = (float)anz;
    auto start_time = std::chrono::high_resolution_clock::now();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;
    std::vector<size_t> buckets = { 0,0,0,0,0,0 };
    //test
    start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < anz; i++)
    {
        size_t n1 = (size_t)RandomGenerator::uniform_int_distribution(0, 5, gen_);
        buckets[n1]++;
    }
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "default random " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;
    std::cout << "distribution " << buckets[0]<< " " << buckets[1] << " " << buckets[2] << " " << buckets[3] << " " << buckets[4] << " " << buckets[5] << std::endl;

    buckets = { 0,0,0,0,0,0 };
    diceRoller_.calcss = 0;
    start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < anz; i++)
    {
        size_t n1 = diceRoller_.getDiceRoll();
        buckets[n1]++;
    }
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "pcg32_fast " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds " << diceRoller_.calcss << std::endl;
    std::cout << "distribution " << buckets[0] << " " << buckets[1] << " " << buckets[2] << " " << buckets[3] << " " << buckets[4] << " " << buckets[5] << std::endl;

    buckets = { 0,0,0,0,0,0 };
    diceRoller_.calcss = 0;
    start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < anz; i++)
    {
        size_t n1 = diceRoller_.getDiceRoll1();
        buckets[n1]++;
    }
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "rand4-10bit " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds "<< diceRoller_.calcss << std::endl;
    std::cout << "distribution " << buckets[0] << " " << buckets[1] << " " << buckets[2] << " " << buckets[3] << " " << buckets[4] << " " << buckets[5] << std::endl;

    buckets = { 0,0,0,0,0,0 };
    diceRoller_.calcss = 0;
    start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < anz; i++)
    {
        unsigned char n1 = diceRoller_.getDiceRoll2();
        buckets[n1]++;
    }
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "3bit " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds " << diceRoller_.calcss << std::endl;
    std::cout << "distribution " << buckets[0] << " " << buckets[1] << " " << buckets[2] << " " << buckets[3] << " " << buckets[4] << " " << buckets[5] << std::endl;


    buckets = { 0,0,0,0,0,0 };
    diceRoller_.calcss = 0;
    start_time = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < anz; i++)
    {
        unsigned char n1 = diceRoller_.getDiceRoll3();
        buckets[n1]++;
    }
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "1 byte " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds " << diceRoller_.calcss << std::endl;
    std::cout << "distribution " << buckets[0] << " " << buckets[1] << " " << buckets[2] << " " << buckets[3] << " " << buckets[4] << " " << buckets[5] << std::endl;
    std::cout << std::endl;
    std::cout << std::endl;*/

    
}

void  Simulator::test_odds_calc()
{
    //test_random_gens();
    std::vector<std::string> abilities = {}; 
    std::string diceanatomy = "";
    Helpers::getHeroData("Barbarian", diceanatomy, abilities);
    std::vector<DiceIdx> org_dice = { 0,0,0,0,0 };
    size_t roll_atemps = 3;
    size_t rerolls = 0; 
    bool is_default_sim = true; 
    const CardData cardData{};

    auto odds = oddsCalculator(abilities, diceanatomy, org_dice, roll_atemps, rerolls, is_default_sim, cardData);
    for (const auto odd : odds)
    {
        std::cout << odd.ability << " " << odd.odd << " " << std::endl;
    }
}

void  Simulator::test_odds_calc_chase()
{
    //test_random_gens();
    std::string chased = "AAA";
    std::vector<std::string> abilities = {};
    std::string diceanatomy = "";
    Helpers::getHeroData("Barbarian", diceanatomy, abilities);
    std::vector<DiceIdx> org_dice = { 0,0,0,0,0 };
    size_t roll_atemps = 3;
    size_t rerolls = 0;
    bool is_default_sim = true;
    CardData cardData{};
    //cardData.lvlsixit = 1;

    auto odds = oddsCalculatorChase(chased, abilities, diceanatomy, org_dice, roll_atemps, rerolls, is_default_sim, cardData);
    for (const auto odd : odds)
    {
        std::cout << odd.ability << " " << odd.odd << " " << std::endl;
    }
}

std::vector<OddsResult> Simulator::get_probability(std::string hero_name, const std::vector<std::string>& abilities, std::string diceanatomy, bool default_sim, bool chase, std::string chased_ability, std::vector<DiceIdx> org_dice, const CardData& cardData, size_t roll_atemps, size_t rerolls)
{
    if (hero_name == "" && abilities.empty())
    {
        return {};
    }
    if (hero_name == "" && diceanatomy == "")
    {
        return {};
    }

    std::vector<std::string> abilities_tmp{};
    std::string diceanatomy_tmp = "";
    if (hero_name != "")
    {
        Helpers::getHeroData(hero_name, diceanatomy_tmp, abilities_tmp);
    }
    else
    {
        diceanatomy_tmp = diceanatomy;
    }
    if (!abilities.empty())
    {
        abilities_tmp = abilities;
    }
    if (diceanatomy != "")
    {
        diceanatomy_tmp = diceanatomy;
    }
    if (diceanatomy_tmp == "" || abilities_tmp.empty())
    {
        std::cout << "no data found" << std::endl;
        return{};
    }

    std::vector<OddsResult> resultData{};

    if (chase && chased_ability != "")
    {
        resultData = oddsCalculatorChase(chased_ability, abilities_tmp, diceanatomy_tmp, org_dice, roll_atemps, rerolls, default_sim, cardData);
    }
    else
    {
        resultData = oddsCalculator(abilities_tmp, diceanatomy_tmp, org_dice, roll_atemps, rerolls, default_sim, cardData);
    }

    return resultData;
}

std::vector<OddsResult> Simulator::oddsCalculator(std::vector<std::string> abilities, std::string diceanatomy, const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, const CardData cardData)
{
    std::vector<OddsResult> results{};
    bool aggressive = false;
    chase_data_.clear();
    int cp = cardData.cp;
    size_t use_max_cards = cardData.use_max_cards;
    std::vector<Card> cards = Helpers::getCards(cardData.lvlsixit, cardData.lvlsamesis, cardData.lvltip_it, cardData.lvlwild, cardData.lvltwiceWild, cardData.lvlslightlyWild, cardData.numberProbabilityManipulation, cardData.hasCheer);
    
    auto start_time = std::chrono::high_resolution_clock::now();

    solver_anatomy_ = Helpers::transformDiceAnatomy(diceanatomy);
    for (const auto& ability : abilities)
    {
        ability_target_ = Helpers::transformAbility(ability);

        bool readed = false;
        
        readed = read_ability(ability, diceanatomy, cards, cp, use_max_cards);

        if (!readed)
        {
            std::vector<DiceIdx> mydiceanatomy = solver_anatomy_;
            std::vector<DiceIdx> target_ability = ability_target_;
            std::cout << "Ability " << ability << " not found in DB, please generate its lookup-data, then calculations will speed up alot " << std::endl;
            get_all_positive_combs(mydiceanatomy, target_ability, cards, cp, use_max_cards);
        }

        float erg = 0.0f;
        std::vector<bool> best_rerolls{};

        /*bool doUseSolver = use_solver(diceanatomy, ability, roll_atemps, rerolls, cards, cp, use_max_cards);
        if (doUseSolver)
        {
            best_rerolls = test_solver(org_dice, roll_atemps, rerolls, diceanatomy, ability, cards, is_default_sim, false, erg);
        }
        else
        {
            best_rerolls = test_all_rolls(org_dice, roll_atemps, rerolls, is_default_sim, erg);
        }*/

        best_rerolls = createMarkovChainSolverEigenRow(org_dice, is_default_sim, roll_atemps, rerolls, diceanatomy, ability, cards, erg);

        OddsResult odds{};
        odds.ability = ability;
        odds.rerolls = best_rerolls;
        odds.odd = erg;
        results.emplace_back(odds);
    }
    helper.closeDB();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;
    std::cout << "calculation done in " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;
    return results;
}

std::vector<OddsResult> Simulator::oddsCalculatorChase(std::string ability, std::vector<std::string> abilities, std::string diceanatomy, const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls, bool is_default_sim, const CardData & cardData)
{
    chase_data_.clear();
    float erg = 0.0F;
    std::vector<bool> best_rerolls{true, true, true, true, true};
    if (number_dice_ == 4)
    {
        best_rerolls = {true, true, true, true};
    }
    std::vector<OddsResult> results{};
    bool aggressive = false;
    int cp = cardData.cp;
    size_t use_max_cards = cardData.use_max_cards;
    std::vector<Card> cards = Helpers::getCards(cardData.lvlsixit, cardData.lvlsamesis, cardData.lvltip_it, cardData.lvlwild, cardData.lvltwiceWild, cardData.lvlslightlyWild, cardData.numberProbabilityManipulation, cardData.hasCheer);

    auto start_time = std::chrono::high_resolution_clock::now();

    solver_anatomy_ = Helpers::transformDiceAnatomy(diceanatomy);
    std::vector<std::string> abilities_temp = abilities;

    //make sure chased ability is the last in abilities:
    for (size_t i = 0; i < abilities_temp.size(); i++)
    {
        const auto& abil = abilities_temp[i];
        if (abil == ability)
        {
            abilities_temp.erase(abilities_temp.begin() + i);
            break;
        }
    }
    abilities_temp.push_back(ability);
    

    //get data for all abilities
    for (const auto& abil : abilities_temp)
    {
        ability_target_ = Helpers::transformAbility(abil);

        bool readed = false;

        readed = read_ability(abil, diceanatomy, cards, cp, use_max_cards);

        if (!readed)
        {
            std::vector<DiceIdx> mydiceanatomy = solver_anatomy_;
            std::vector<DiceIdx> target_ability = ability_target_;
            std::cout << "Ability " << abil << " not found in DB, please generate its lookup-data, then calculations will speed up alot " << std::endl;
            get_all_positive_combs(mydiceanatomy, target_ability, cards, cp, use_max_cards);
        }
        ChaseData cd{};
        cd.ability = abil;
        cd.amount = 0U;
        cd.possible_list_with_cheat_dt = this->possible_list_with_cheat_dt;
        cd.all_ability_combinations_dt = this->all_ability_combinations_dt;
        cd.all_ability_combinations_no_cards_dt = this->all_ability_combinations_no_cards_dt;
        cd.erg = 0.0F;
        chase_data_.emplace_back(cd);
    };
    helper.closeDB();
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;
    std::cout << "reading done in " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;
    
    /*start_time = std::chrono::high_resolution_clock::now();
    
    best_rerolls = test_all_rolls_chase(org_dice, roll_atemps, rerolls, is_default_sim, erg);

    for (const auto& cd : chase_data_)
    {
        OddsResult odds{};
        odds.ability = cd.ability;
        odds.rerolls = best_rerolls;
        if (cd.ability == ability)
        {
            odds.odd = erg;
        }
        else
        {
            odds.odd = cd.erg;
        }
        results.emplace_back(odds);
    }

    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "simulation calculation done in " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;*/

    start_time = std::chrono::high_resolution_clock::now();
    best_rerolls = createMarkovChainSolverEigenRow(org_dice, is_default_sim, roll_atemps, rerolls, diceanatomy, ability, cards, erg);
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "markov chain calculation done in " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;

    for (const auto& cd : chase_data_)
    {
        //std::cout << "chain " << cd.ability << " " << cd.erg << std::endl;
        OddsResult odds{};
        odds.ability = cd.ability;
        odds.rerolls = best_rerolls;
        odds.odd = (int)(cd.erg * 100.0F) / 100.0F;
        results.emplace_back(odds);
    }
    chase_data_.clear();
    return results;
}

// stuff for Markov chain calculations ###############################################################################################################

//deprecated:
std::vector<std::vector<float>> ijkalgorithm(const std::vector<std::vector<float>>& A, const std::vector<std::vector<float>>& B) 
{
    size_t n = A.size();

    // initialise C with 0s
    std::vector<float> tmp(n, 0.0F);
    std::vector< std::vector<float> > C(n, tmp);

    for (size_t i = 0; i < n; i++) 
    {
        for (size_t k = 0; k < n; k++) 
        {
            if (A[i][k] == 0.0F)
            {
                continue;
            }
            for (size_t j = 0; j < n; j++)
            {
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
    return C;
}

std::vector<std::vector<float>> binary_exponentation(const std::vector<std::vector<float>>& A, unsigned e)
{
    // Ab hier: e != 0, d, h. irgendein Bit in e ist 1.
    unsigned h = ~(~0u >> 1); // h = binär 100...0
    while (!(e & h))
    {
        h >>= 1;
    }
    // h maskiert nun das erste gesetzte Bit in e. (#)
    std::vector<std::vector<float>> R = A;

    // solange weitere Bits zu prüfen sind (das erste wurde durch r = b bereits abgearbeitet),
    while (h >>= 1)
    {
        R = ijkalgorithm(R,R); // quadrieren
        if (e & h)
        {
            // falls Bit gesetzt, multiplizieren
            R = ijkalgorithm(R, A);
        }
    }
    // h == 0, d. h. alle Bits geprüft.
    return R;
}

Eigen::MatrixXf binary_exponentation(const Eigen::MatrixXf & A, unsigned e)
{
    // Ab hier: e != 0, d, h. irgendein Bit in e ist 1.
    unsigned h = ~(~0u >> 1); // h = binär 100...0
    while (!(e & h))
    {
        h >>= 1;
    }
    // h maskiert nun das erste gesetzte Bit in e. (#)
    Eigen::MatrixXf R = A;

    // solange weitere Bits zu prüfen sind (das erste wurde durch r = b bereits abgearbeitet),
    while (h >>= 1)
    {
        R = R*R; // quadrieren
        if (e & h)
        {
            // falls Bit gesetzt, multiplizieren
            R = R*A;
        }
    }
    // h == 0, d. h. alle Bits geprüft.
    return R;
}

std::vector<size_t> sort_indexes(const std::vector<DiceThrow> & values)
{
    std::vector<size_t> idx(values.size());
    std::vector<size_t> numbers(values.size());
    for ( size_t i = 0; i< values.size(); i++)
    {
        const auto& blubb = values[i];
        numbers[i] = 0;
        for (const auto& bo : blubb.rerollers)
        {
            if (bo)
            {
                numbers[i]++;
            }
        }
    }
    std::iota(idx.begin(), idx.end(),0);
    std::stable_sort(idx.begin(), idx.end(), [&numbers](size_t i1, size_t i2) {return numbers[i1] > numbers[i2];});
    return idx;
}

std::vector<std::vector<float>> Simulator::createMarkovAttemptSub(const std::vector<DiceIdx>& org_dice, bool default_sim, const std::vector<bool>& org_rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();
    if (default_sim)
    {
        n++;
    }

    // initialise C with 0s
    std::vector<float> tmp(n, 0.0F);
    std::vector< std::vector<float> > markovMatrix(n, tmp);

    size_t target_state = n - 1;
    if (!default_sim)
    {
        target_state = full_hash_to_sorted_hash[get_hash(org_dice)];
    }

    for (const auto& state : possible_list)
    {
        size_t matrix_idx_i = full_hash_to_sorted_hash[state.hash];
        if (state.success)
        {
            markovMatrix[matrix_idx_i][matrix_idx_i] = 1.0F;
            continue;
        }
        size_t anz_rerolls = 0;
        DiceHash hash_keepers = 0U;
        DiceHash current_factor = 1U;
        std::vector<bool> rerollers = org_rerolls;
        if (matrix_idx_i != target_state)
        {
            rerollers.resize(5);
            for (size_t i = 0; i < 5; i++)
            {
                rerollers[i] = state.rerollers[i];
            }
        }
        for (size_t i = 0; i < number_dice_; i++)
        {
            const auto& b = rerollers[i];
            if (b)
            {
                anz_rerolls++;
            }
            else
            {
                hash_keepers += state.dice[i] * current_factor;
                current_factor *= 6U;
            }
        }

        float prob = 1.0F / std::pow(6.0F, anz_rerolls);

        //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
        const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

        DiceHash current_factor_copy = current_factor;
        /*for (const auto& sub : all_sub_combs)
        {
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob;
        }*/
        for (size_t i = 0; i < all_sub_combs.size(); i++)
        {
            const auto& sub = all_sub_combs[i];
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob * amount_sub_combs[i];
        }
    }
    //reroll all state (last state):
    if (default_sim)
    {
        size_t matrix_idx_i = 0;
        size_t anz_rerolls = number_dice_;
        DiceHash current_factor = 1U;
        float prob = 1.0F / std::pow(6.0F, anz_rerolls);
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        for (const auto& sub : all_sub_combs)
        {
            current_factor = 1.0F;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob;
        }
    }
    return markovMatrix;
}

std::vector<std::vector<float>> Simulator::createMarkovAttempt(bool default_sim, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();
    if (default_sim)
    {
        n++;
    }

    // initialise C with 0s
    std::vector<float> tmp(n, 0.0F);
    std::vector< std::vector<float> > markovMatrix(n, tmp);

    for (const auto& state : possible_list)
    {
        size_t matrix_idx_i = full_hash_to_sorted_hash[state.hash];
        if (state.success)
        {
            markovMatrix[matrix_idx_i][matrix_idx_i] = 1.0F;
            continue;
        }
        size_t anz_rerolls = 0;
        DiceHash hash_keepers = 0U;
        DiceHash current_factor = 1U;
        for (size_t i = 0; i < number_dice_; i++)
        {
            const auto& b = state.rerollers[i];
            if (b)
            {
                anz_rerolls++;
            }
            else
            {
                hash_keepers += state.dice[i] * current_factor;
                current_factor *= 6U;
            }
        }

        float prob = 1.0F / std::pow(6.0F, anz_rerolls);

        //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
        const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

        DiceHash current_factor_copy = current_factor;
        /*for (const auto& sub : all_sub_combs)
        {
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob;
        }*/
        for (size_t i = 0; i < all_sub_combs.size(); i++)
        {
            const auto& sub = all_sub_combs[i];
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob * amount_sub_combs[i];
        }
    }
    //reroll all state (last state):
    if (default_sim)
    {
        size_t matrix_idx_i = 0;
        size_t anz_rerolls = number_dice_;
        DiceHash current_factor = 1U;
        float prob = 1.0F / std::pow(6.0F, anz_rerolls);
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        for (const auto& sub : all_sub_combs)
        {
            current_factor = 1U;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob;
        }
    }
    return markovMatrix;
}

std::vector<std::vector<float>> Simulator::createMarkovReroll(bool default_sim, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();
    if (default_sim)
    {
        n++;
    }

    // initialise C with 0s
    std::vector<float> tmp(n, 0.0F);
    std::vector< std::vector<float> > markovMatrix(n, tmp);

    for (const auto& state : possible_list)
    {
        size_t matrix_idx_i = full_hash_to_sorted_hash[state.hash];
        if (state.success)
        {
            markovMatrix[matrix_idx_i][matrix_idx_i] = 1.0F;
            continue;
        }
        size_t anz_rerolls = 0;
        DiceHash hash_keepers = 0U;
        DiceHash current_factor = 1U;
        bool found = false;
        for (size_t i = 0; i < number_dice_; i++)
        {
            const auto& b = state.rerollers[i];
            if (b && !found)
            {
                anz_rerolls++;
                found = true;
            }
            else
            {
                hash_keepers += state.dice[i] * current_factor;
                current_factor *= 6U;
            }
        }

        float prob = 1.0F / std::pow(6.0F, anz_rerolls);
        //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
        const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

        DiceHash current_factor_copy = current_factor;
        /*for (const auto& sub : all_sub_combs)
        {
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob;
        }*/
        for (size_t i = 0; i < all_sub_combs.size(); i++)
        {
            const auto& sub = all_sub_combs[i];
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob * amount_sub_combs[i];
        }
    }
    //reroll all state (last state):
    /*if (default_sim)
    {
        size_t matrix_idx_i = 0;
        size_t anz_rerolls = number_dice_;
        DiceHash current_factor = 1.0F;
        float prob = 1.0F / std::pow(6.0F, anz_rerolls);
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        for (const auto& sub : all_sub_combs)
        {
            current_factor = 1.0F;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6.0F;
            }
            DiceHash final_hash = hash_rerollers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix[matrix_idx_i][matrix_idx_j] += prob;
        }
    }*/
    return markovMatrix;
}

float Simulator::createMarkovChain(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls)
{
    //https://stackoverflow.com/questions/11110604/why-is-boosts-matrix-multiplication-slower-than-mine
    //https://math.stackexchange.com/questions/4032313/probability-of-yahtzee-straight-with-strict-re-roll-rules


    size_t n = possible_list_with_cheat_dt.size();
    if (default_sim)
    {
        n++;
    }
    std::vector<float> tmp(n, 0.0F);
    std::vector< std::vector<float> > markovMatrix(n, tmp);
    std::vector<std::vector<float>> rerollMatrix(n, tmp);
    //create identity
    for (size_t i = 0; i < n; i++)
    {
        markovMatrix[i][i] = 1.0F;
        rerollMatrix[i][i] = 1.0F;
    }

    const auto& full_hash_to_sorted_hash1 = this->all_combs[this->all_combs.size() - 1].dicehash_to_combs_sorted;
    const auto sorted_indexes = sort_indexes(possible_list_with_cheat_dt);
    std::vector<DiceHash> full_hash_to_sorted_hash(full_hash_to_sorted_hash1.size());
    for (size_t i = 0; i < full_hash_to_sorted_hash1.size(); i++)
    {
        DiceHash searched = full_hash_to_sorted_hash1[i];
        for (size_t j = 0; j < sorted_indexes.size(); j++)
        {
            if (searched == sorted_indexes[j])
            {
                full_hash_to_sorted_hash[i] = j + ((default_sim) ? 1 : 0);
                break;
            }
        }
    }

    if (roll_atempts >= 1)
    {
        markovMatrix = createMarkovAttempt(default_sim, full_hash_to_sorted_hash, possible_list_with_cheat_dt);
    }
    if (rerolls >= 1)
    {
        rerollMatrix = createMarkovReroll(default_sim, full_hash_to_sorted_hash, possible_list_with_cheat_dt);
    }
    // calculate Matrix ^ roll_atemps
    if (roll_atempts >= 2)
    {
        //copy matrix
        std::vector< std::vector<float> > markovMatrixCopy{};
        markovMatrixCopy = markovMatrix;
        for (size_t i = 1; i < roll_atempts; i++)
        {
            //TODO CLEVERER
            markovMatrix = ijkalgorithm(markovMatrix, markovMatrixCopy);
        }
    }
    // calculate Matrix ^ rerolls
    if (rerolls >= 2)
    {
        //copy matrix
        std::vector< std::vector<float> > markovMatrixCopy{};
        markovMatrixCopy = rerollMatrix;
        for (size_t i = 1; i < rerolls; i++)
        {
            //TODO CLEVERER
            rerollMatrix = ijkalgorithm(rerollMatrix, markovMatrixCopy);
        }
    }

    if (rerolls >= 1)
    {
        markovMatrix = ijkalgorithm(markovMatrix, rerollMatrix);
    }
    
    
    size_t matrix_idx_i = 0ULL; 
    if (default_sim)
    {
        matrix_idx_i = 0;
    }
    else
    {
        matrix_idx_i = full_hash_to_sorted_hash[get_hash(org_dice)];
    }
    const auto& start_row = markovMatrix[matrix_idx_i];
    float summ = 0.0F;
    for (const auto& state : possible_list_with_cheat_dt)
    {
        if (state.success)
        {
            size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
            summ += start_row[matrix_idx_j];
        }
    }

    return summ*100.0F;
}

std::vector<bool> Simulator::createMarkovChainSolver(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card> & cards, float & erg_odd)
{
    //https://stackoverflow.com/questions/11110604/why-is-boosts-matrix-multiplication-slower-than-mine
    //https://math.stackexchange.com/questions/4032313/probability-of-yahtzee-straight-with-strict-re-roll-rules


    size_t n = possible_list_with_cheat_dt.size();
    std::vector<float> tmp(n, 0.0F);
    std::vector<std::vector<float>> markovMatrix(n, tmp);
    std::vector<std::vector<float>> markovLastMatrix(n, tmp);
    std::vector<std::vector<float>> rerollMatrix(n, tmp);
    //create identity
    for (size_t i = 0; i < n; i++)
    {
        markovLastMatrix[i][i] = 1.0F;
        markovMatrix[i][i] = 1.0F;
        rerollMatrix[i][i] = 1.0F;
    }


    // get optimized rerollers:
    float temp_erg = 0.0F;
    size_t temp_rerollers = std::min(roll_atempts, 2ULL);
    if (rerolls >= 2)
    {
        temp_rerollers = 2;
    }
    std::vector<bool> best_rerollers = test_solver(org_dice, temp_rerollers, 0, anatomy, target, cards, default_sim, true, temp_erg);
    if (default_sim)
    {
        best_rerollers = { true, true, true, true, true };
        if (number_dice_ == 4)
        {
            best_rerollers = { true, true, true, true };
        }

    }
    else
    {
        if (temp_erg == 100.0F)
        {
            erg_odd = 100.0F;

            for (auto& cd : chase_data_)
            {
                if (cd.ability == target)
                {
                    cd.erg = 100.0F;
                    break;
                }
            }

            return best_rerollers;
        }
    }
    /*for (size_t i = 0; i < best_rerolls.size(); i++)
    {
        std::string s1 = best_rerolls[i].get_rerollers_string();
        std::string s2 = possible_list_with_cheat_dt[i].get_rerollers_string();
        if (s1 != s2)
        {
            std::cout << "best " << s1 << std::endl;
            std::cout << "naive " << s2 << std::endl;
        }
    }*/

    //Sort all dice-throws by number of dice to reroll descending. -> resulting matrix will be upper block-diagonal -> 
    // matrix ^ n will also be block-diagonal -> can save multiplications
    const auto& full_hash_to_sorted_hash1 = this->all_combs[this->all_combs.size() - 1].dicehash_to_combs_sorted;
    const auto sorted_indexes = sort_indexes(best_rerolls);
    std::vector<size_t> inverted_indexes(sorted_indexes.size(), 0 );

    for (size_t i = 0; i < sorted_indexes.size(); i++)
    {
        for (size_t j = 0; j < sorted_indexes.size(); j++)
        {
            if (i == sorted_indexes[j])
            {
                inverted_indexes[i] = j;
                break;
            }
        }
    }

    std::vector<DiceHash> full_hash_to_sorted_hash(full_hash_to_sorted_hash1.size());
    for (size_t i = 0; i < full_hash_to_sorted_hash1.size(); i++)
    {
        DiceHash searched = full_hash_to_sorted_hash1[i];
        full_hash_to_sorted_hash[i] = inverted_indexes[searched];
    }

    if (roll_atempts >= 1)
    {
        //first matrix: do best reroll on current dice, with optimized rerolls
        markovMatrix = createMarkovAttemptSub(org_dice, false, best_rerollers, full_hash_to_sorted_hash, best_rerolls);
    }
    if (roll_atempts >= 2)
    {
        //strategy for others: rolls with best reroll-strategy
        markovLastMatrix = createMarkovAttempt(false, full_hash_to_sorted_hash, best_rerolls);
    }
    if (rerolls >= 1)
    {
        rerollMatrix = createMarkovReroll(false, full_hash_to_sorted_hash, possible_list_with_cheat_dt);
    }

    // final matrix = firstMatrix * markovMatrix ^ (roll_atemps-1) * lastMatrix * reroll matrixes ^ rerolls
    //first matrix is not identity, if roll_atempts >= 1, lastMatrix is not identity if roll_attempts >= 2 and the middle matrix is not identiy, if roll attemps >= 3

    // calculate Middle-Matrix ^ roll_atemps-1
    if (roll_atempts >= 3)
    {
        markovLastMatrix = binary_exponentation(markovLastMatrix, roll_atempts - 1);
    }
    //calculate main-rerolll matrix:
    if (roll_atempts >= 2)
    {
        markovMatrix = ijkalgorithm(markovMatrix, markovLastMatrix);
    }
    // calculate Matrix ^ rerolls
    if (rerolls >= 2)
    {
        rerollMatrix = binary_exponentation(rerollMatrix, rerolls);
    }
    //multiply reroll matrix to final matrix
    if (rerolls >= 1)
    {
        markovMatrix = ijkalgorithm(markovMatrix, rerollMatrix);
    }


    size_t matrix_idx_i = 0ULL;
    if (default_sim)
    {
        matrix_idx_i = 0ULL;
    }
    else
    {
        matrix_idx_i = full_hash_to_sorted_hash[get_hash(org_dice)];
    }
    const auto& start_row = markovMatrix[matrix_idx_i];
    

    if (!chase_data_.empty())
    {
        for (auto& cd : chase_data_)
        {
            float summ = 0.0F;
            for (const auto& state : cd.possible_list_with_cheat_dt)
            {
                if (state.success)
                {
                    size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
                    summ += start_row[matrix_idx_j];
                }
            }
            cd.erg = summ * 100.0F;
        }
        erg_odd = 0.0F;
    }
    else
    {
        float summ = 0.0F;
        for (const auto& state : possible_list_with_cheat_dt)
        {
            if (state.success)
            {
                size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
                summ += start_row[matrix_idx_j];
            }
        }
        erg_odd = summ * 100.0F;
    }

    return best_rerollers;
}

//recommended markov chain stuff with eigen:

void Simulator::createMarkovAttemptSubEigen(Eigen::MatrixXf & markovMatrix, const std::vector<DiceIdx>& org_dice, const std::vector<bool>& org_rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();

    // initialise C with 0s
    markovMatrix.setZero();

    size_t target_state = full_hash_to_sorted_hash[get_hash(org_dice)];

    for (const auto& state : possible_list)
    {
        size_t matrix_idx_i = full_hash_to_sorted_hash[state.hash];
        if (state.success)
        {
            markovMatrix(matrix_idx_i,matrix_idx_i) = 1.0F;
            continue;
        }
        size_t anz_rerolls = 0;
        DiceHash hash_keepers = 0U;
        DiceHash current_factor = 1U;
        std::vector<bool> rerollers = org_rerolls;
        if (matrix_idx_i != target_state)
        {
            rerollers.resize(5);
            for (size_t i = 0; i < 5; i++)
            {
                rerollers[i] = state.rerollers[i];
            }
        }
        for (size_t i = 0; i < number_dice_; i++)
        {
            const auto& b = rerollers[i];
            if (b)
            {
                anz_rerolls++;
            }
            else
            {
                hash_keepers += state.dice[i] * current_factor;
                current_factor *= 6U;
            }
        }
        if (anz_rerolls == 0)
        {
            markovMatrix(matrix_idx_i, matrix_idx_i) = 1.0F;
            continue;
        }
        float prob = 1.0F / std::pow(6.0F, anz_rerolls);

        //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
        const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

        DiceHash current_factor_copy = current_factor;
        for (size_t i = 0; i < all_sub_combs.size(); i++)
        {
            const auto& sub = all_sub_combs[i];
            current_factor = current_factor_copy;
            DiceHash hash_rerollers = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * current_factor;
                current_factor *= 6U;
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix(matrix_idx_i,matrix_idx_j) += prob * amount_sub_combs[i];
        }
    }
    return;
}

void Simulator::createMarkovAttemptSubEigenRow(Eigen::MatrixXf& markovMatrix, const std::vector<DiceIdx>& org_dice, const std::vector<bool>& org_rerolls, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();

    // initialise C with 0s
    markovMatrix.setZero();

    size_t target_state = full_hash_to_sorted_hash[get_hash(org_dice)];

    std::vector<size_t> reroller_factors{ 1,1,1,1,1,1 };
    std::vector<size_t> org_factors{ 1296,216,36,6,1 };
    size_t matrix_idx_i = 0;
    auto state = possible_list[0];
    //search correct state/dice-throw
    // (we have to search, because we reordered them)
    for (const auto& s : possible_list)
    {
        size_t state_idx = full_hash_to_sorted_hash[s.hash];
        if (state_idx == target_state)
        {
            state = s;
        }
    }
    size_t anz_rerolls = 0;
    DiceHash hash_keepers = 0U;
    DiceHash current_factor = 1U;
    std::vector<bool> rerollers = org_rerolls;
    for (size_t i = 0; i < number_dice_; i++)
    {
        const auto& b = rerollers[i];
        if (b)
        {
            anz_rerolls++;
        }
        else
        {
            hash_keepers += state.dice[i] * current_factor;
            current_factor *= 6U;
        }
    }
    if (anz_rerolls == 0)
    {
        markovMatrix(matrix_idx_i, target_state) = 1.0F;
        return;
    }
    float prob = 1.0F / std::pow(6.0F, anz_rerolls);

    //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
    const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
    const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

    DiceHash current_factor_copy = current_factor;
    for (size_t i = 0; i < all_sub_combs.size(); i++)
    {
        const auto& sub = all_sub_combs[i];
        current_factor = current_factor_copy;
        DiceHash hash_rerollers = 0U;
        for (const auto& die : sub)
        {
            hash_rerollers += die * current_factor;
            current_factor *= 6U;
        }
        DiceHash final_hash = hash_rerollers + hash_keepers;
        size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
        markovMatrix(matrix_idx_i, matrix_idx_j) += prob * amount_sub_combs[i];
    }

    return;
}

void Simulator::createMarkovAttemptEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();

    // initialise C with 0s
    markovMatrix.setZero();

    std::vector<size_t> reroller_factors{ 1,1,1,1,1,1 };
    std::vector<size_t> org_factors{ 1296,216,36,6,1 };

    for (const auto& state : possible_list)
    {
        size_t matrix_idx_i = full_hash_to_sorted_hash[state.hash];
        if (state.success)
        {
            markovMatrix(matrix_idx_i,matrix_idx_i) = 1.0F;
            continue;
        }
        size_t anz_rerolls = 0;
        DiceHash hash_keepers = 0U;
        for (size_t i = 0; i < number_dice_; i++)
        {
            const auto& b = state.rerollers[i];
            if (b)
            {
                reroller_factors[anz_rerolls] = org_factors[i];
                anz_rerolls++;
            }
            else
            {
                hash_keepers += state.dice[i] * org_factors[i];
            }
        }
        if (anz_rerolls == 0)
        {
            markovMatrix(matrix_idx_i, matrix_idx_i) = 1.0F;
            continue;
        }
        float prob = 1.0F / std::pow(6.0F, anz_rerolls);

        //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
        const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

        for (size_t i = 0; i < all_sub_combs.size(); i++)
        {
            const auto& sub = all_sub_combs[i];
            DiceHash hash_rerollers = 0U;
            size_t die_idx = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * reroller_factors[die_idx++];
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix(matrix_idx_i,matrix_idx_j) += prob * amount_sub_combs[i];
        }
    }
    return;
}

void Simulator::createMarkovRerollEigen(Eigen::MatrixXf& markovMatrix, const std::vector<DiceHash>& full_hash_to_sorted_hash, const std::vector<DiceThrow>& possible_list)
{
    size_t n = possible_list.size();

    // initialise C with 0s
    markovMatrix.setZero();
    std::vector<size_t> reroller_factors{ 1,1,1,1,1,1 };
    std::vector<size_t> org_factors{ 1296,216,36,6,1 };
    for (const auto& state : possible_list)
    {
        size_t matrix_idx_i = full_hash_to_sorted_hash[state.hash];
        if (state.success)
        {
            markovMatrix(matrix_idx_i,matrix_idx_i) = 1.0F;
            continue;
        }
        size_t anz_rerolls = 0;
        DiceHash hash_keepers = 0U;
        bool found = false;
        for (size_t i = 0; i < number_dice_; i++)
        {
            const auto& b = state.rerollers[i];
            if (b && !found)
            {
                reroller_factors[anz_rerolls] = org_factors[i];
                anz_rerolls++;
                found = true;
            }
            else
            {
                hash_keepers += state.dice[i] * org_factors[i];
            }
        }
        if (anz_rerolls == 0)
        {
            markovMatrix(matrix_idx_i, matrix_idx_i) = 1.0F;
            continue;
        }
        float prob = 1.0F / std::pow(6.0F, anz_rerolls);
        //const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs;
        const auto& all_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted;
        const auto& amount_sub_combs = this->all_combs[anz_rerolls - 1].combs_sorted_amount;

        for (size_t i = 0; i < all_sub_combs.size(); i++)
        {
            const auto& sub = all_sub_combs[i];
            DiceHash hash_rerollers = 0U;
            size_t die_idx = 0U;
            for (const auto& die : sub)
            {
                hash_rerollers += die * reroller_factors[die_idx++];
            }
            DiceHash final_hash = hash_rerollers + hash_keepers;
            size_t matrix_idx_j = full_hash_to_sorted_hash[final_hash];
            markovMatrix(matrix_idx_i,matrix_idx_j) += prob * amount_sub_combs[i];
        }
    }
    return;
}

std::vector<bool> Simulator::createMarkovChainSolverEigen(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card>& cards, float& erg_odd)
{
    //https://stackoverflow.com/questions/11110604/why-is-boosts-matrix-multiplication-slower-than-mine
    //https://math.stackexchange.com/questions/4032313/probability-of-yahtzee-straight-with-strict-re-roll-rules


    size_t n = possible_list_with_cheat_dt.size();
    Eigen::MatrixXf eigen_LastMatrix{n, n};
    Eigen::MatrixXf eigen_markovMatrix{n,n};
    Eigen::MatrixXf eigen_rerollMatrix{n,n};
    eigen_LastMatrix.setIdentity();
    eigen_markovMatrix.setIdentity();
    eigen_rerollMatrix.setIdentity();

    // get optimized rerollers:
    float temp_erg = 0.0F;
    size_t temp_rerollers = std::min(roll_atempts, 2ULL);
    if (rerolls >= 2)
    {
        temp_rerollers = 2;
    }
    std::vector<bool> best_rerollers = test_solver(org_dice, temp_rerollers, 0, anatomy, target, cards, default_sim, true, temp_erg);
    if (default_sim)
    {
        best_rerollers = { true, true, true, true, true };
        if (number_dice_ == 4)
        {
            best_rerollers = { true, true, true, true };
        }

    }
    else
    {
        if (temp_erg == 100.0F)
        {
            erg_odd = 100.0F;

            for (auto& cd : chase_data_)
            {
                if (cd.ability == target)
                {
                    cd.erg = 100.0F;
                    break;
                }
            }

            return best_rerollers;
        }
    }

    //Sort all dice-throws by number of dice to reroll descending. -> resulting matrix will be upper block-diagonal -> 
    // matrix ^ n will also be block-diagonal -> can save multiplications
    const auto& full_hash_to_sorted_hash1 = this->all_combs[this->all_combs.size() - 1].dicehash_to_combs_sorted;
    const auto sorted_indexes = sort_indexes(best_rerolls);
    std::vector<size_t> inverted_indexes(sorted_indexes.size(), 0);

    for (size_t i = 0; i < sorted_indexes.size(); i++)
    {
        for (size_t j = 0; j < sorted_indexes.size(); j++)
        {
            if (i == sorted_indexes[j])
            {
                inverted_indexes[i] = j;
                break;
            }
        }
    }

    std::vector<DiceHash> full_hash_to_sorted_hash(full_hash_to_sorted_hash1.size());
    for (size_t i = 0; i < full_hash_to_sorted_hash1.size(); i++)
    {
        DiceHash searched = full_hash_to_sorted_hash1[i];
        full_hash_to_sorted_hash[i] = inverted_indexes[searched];
    }

    if (roll_atempts >= 1)
    {
        //first matrix: do best reroll on current dice, with optimized rerolls
        createMarkovAttemptSubEigen(eigen_markovMatrix, org_dice, best_rerollers, full_hash_to_sorted_hash, best_rerolls);
    }
    if (roll_atempts >= 2)
    {
        //strategy for others: rolls with best reroll-strategy
         createMarkovAttemptEigen(eigen_LastMatrix, full_hash_to_sorted_hash, best_rerolls);
    }
    if (rerolls >= 1)
    {
        createMarkovRerollEigen(eigen_rerollMatrix, full_hash_to_sorted_hash, possible_list_with_cheat_dt);
    }

    // final matrix = firstMatrix * markovMatrix ^ (roll_atemps-1) * lastMatrix * reroll matrixes ^ rerolls
    //first matrix is not identity, if roll_atempts >= 1, lastMatrix is not identity if roll_attempts >= 2 and the middle matrix is not identiy, if roll attemps >= 3

    // calculate Middle-Matrix ^ roll_atemps-1
    if (roll_atempts >= 3)
    {
        eigen_LastMatrix = binary_exponentation(eigen_LastMatrix, roll_atempts - 1);
    }
    //calculate main-rerolll matrix:
    if (roll_atempts >= 2)
    {
        eigen_markovMatrix = eigen_markovMatrix * eigen_LastMatrix;
    }
    // calculate Matrix ^ rerolls
    if (rerolls >= 2)
    {
        eigen_rerollMatrix = binary_exponentation(eigen_rerollMatrix, rerolls);
    }
    //multiply reroll matrix to final matrix
    if (rerolls >= 1)
    {
        eigen_markovMatrix = eigen_markovMatrix * eigen_rerollMatrix;
    }


    size_t matrix_idx_i = 0ULL;
    if (default_sim)
    {
        matrix_idx_i = 0ULL;
    }
    else
    {
        matrix_idx_i = full_hash_to_sorted_hash[get_hash(org_dice)];
    }
    const auto& start_row = eigen_markovMatrix.row(matrix_idx_i);


    if (!chase_data_.empty())
    {
        for (auto& cd : chase_data_)
        {
            float summ = 0.0F;
            for (const auto& state : cd.possible_list_with_cheat_dt)
            {
                if (state.success)
                {
                    size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
                    summ += start_row(matrix_idx_j);
                }
            }
            cd.erg = summ * 100.0F;
        }
        erg_odd = 0.0F;
    }
    else
    {
        float summ = 0.0F;
        for (const auto& state : possible_list_with_cheat_dt)
        {
            if (state.success)
            {
                size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
                summ += start_row(matrix_idx_j);
            }
        }
        erg_odd = summ * 100.0F;
    }

    return best_rerollers;
}

std::vector<bool> Simulator::createMarkovChainSolverEigenRow(const std::vector<DiceIdx>& org_dice, bool default_sim, size_t roll_atempts, size_t rerolls, const std::string& anatomy, const std::string& target, const std::vector<Card>& cards, float& erg_odd)
{
    //https://stackoverflow.com/questions/11110604/why-is-boosts-matrix-multiplication-slower-than-mine
    //https://math.stackexchange.com/questions/4032313/probability-of-yahtzee-straight-with-strict-re-roll-rules


    size_t n = possible_list_with_cheat_dt.size();
    Eigen::MatrixXf eigen_LastMatrix{ n, n };
    Eigen::MatrixXf eigen_markovMatrix{ 1,n };
    Eigen::MatrixXf eigen_rerollMatrix{ n,n };
    eigen_LastMatrix.setIdentity();
    eigen_markovMatrix.setZero();//set org-dice-idx to 1.0 later
    eigen_rerollMatrix.setIdentity();

    // get optimized rerollers:
    float temp_erg = 0.0F;
    size_t temp_rerollers = roll_atempts;
    size_t temp_rollers = 0;
    if (roll_atempts >= 2)
    {
        temp_rerollers = 2;
    }
    if (temp_rerollers < 2)
    {
        temp_rollers = rerolls;
        if (temp_rollers > 5)
        {
            temp_rollers = 5;
        }
    }
    std::vector<bool> best_rerollers = test_solver(org_dice, temp_rerollers, 0, anatomy, target, cards, default_sim, true, temp_erg);
    if (default_sim)
    {
        best_rerollers = { true, true, true, true, true };
        if (number_dice_ == 4)
        {
            best_rerollers = { true, true, true, true };
        }

    }
    else
    {
        if (temp_erg == 100.0F)
        {
            erg_odd = 100.0F;

            for (auto& cd : chase_data_)
            {
                if (cd.ability == target)
                {
                    cd.erg = 100.0F;
                    break;
                }
            }

            return best_rerollers;
        }
    }

    //Sort all dice-throws by number of dice to reroll descending. -> resulting matrix will be upper block-diagonal -> 
    // matrix ^ n will also be block-diagonal -> can save multiplications
    const auto& full_hash_to_sorted_hash1 = this->all_combs[this->all_combs.size() - 1].dicehash_to_combs_sorted;
    const auto sorted_indexes = sort_indexes(best_rerolls);
    std::vector<size_t> inverted_indexes(sorted_indexes.size(), 0);

    for (size_t i = 0; i < sorted_indexes.size(); i++)
    {
        for (size_t j = 0; j < sorted_indexes.size(); j++)
        {
            if (i == sorted_indexes[j])
            {
                inverted_indexes[i] = j;
                break;
            }
        }
    }

    std::vector<DiceHash> full_hash_to_sorted_hash(full_hash_to_sorted_hash1.size());
    for (size_t i = 0; i < full_hash_to_sorted_hash1.size(); i++)
    {
        DiceHash searched = full_hash_to_sorted_hash1[i];
        full_hash_to_sorted_hash[i] = inverted_indexes[searched];
    }

    size_t matrix_idx_i = full_hash_to_sorted_hash[get_hash(org_dice)];

    if (roll_atempts >= 1)
    {
        //first matrix: do best reroll on current dice, with optimized rerolls
        createMarkovAttemptSubEigenRow(eigen_markovMatrix, org_dice, best_rerollers, full_hash_to_sorted_hash, best_rerolls);
    }
    else
    {
        eigen_markovMatrix(0, matrix_idx_i) = 1.0F;
    }
    if (roll_atempts >= 2)
    {
        //strategy for others: rolls with best reroll-strategy
        createMarkovAttemptEigen(eigen_LastMatrix, full_hash_to_sorted_hash, best_rerolls);
    }
    if (rerolls >= 1)
    {
        createMarkovRerollEigen(eigen_rerollMatrix, full_hash_to_sorted_hash, possible_list_with_cheat_dt);
    }

    //final matrix = firstMatrix * markovMatrix ^ (roll_atemps-1) * lastMatrix * reroll matrixes ^ rerolls
    //first matrix is not identity, if roll_atempts >= 1, lastMatrix is not identity if roll_attempts >= 2 and the middle matrix is not identiy, if roll attemps >= 3

    for (size_t i = 1; i < roll_atempts; i++)
    {
        eigen_markovMatrix = eigen_markovMatrix * eigen_LastMatrix;
    }
    // calculate Matrix ^ rerolls
    for (size_t i = 0; i < rerolls; i++)
    {
        eigen_markovMatrix = eigen_markovMatrix * eigen_rerollMatrix;
    }
    if (!chase_data_.empty())
    {
        for (auto& cd : chase_data_)
        {
            float summ = 0.0F;
            for (const auto& state : cd.possible_list_with_cheat_dt)
            {
                if (state.success)
                {
                    size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
                    summ += eigen_markovMatrix(matrix_idx_j);
                }
            }
            cd.erg = summ * 100.0F;
        }
        erg_odd = 0.0F;
    }
    else
    {
        float summ = 0.0F;
        for (const auto& state : possible_list_with_cheat_dt)
        {
            if (state.success)
            {
                size_t matrix_idx_j = full_hash_to_sorted_hash[state.hash];
                summ += eigen_markovMatrix(matrix_idx_j);
            }
        }
        erg_odd = summ * 100.0F;
    }

    return best_rerollers;
}

void Simulator::combo_test()
{
    //test_random_gens();
    bool test_read_and_calc = false;
    std::vector<DiceIdx> org_dice{ 0,1,3,3,3 };
    //org_dice = { 0,1,2,4,4 };
    std::string ability = "BIG";
    std::string diceanatomy = "AAABBC";
    size_t roll_atemps = 0;
    size_t rerolls = 3;
    bool is_default_sim = false;

    

    std::string ability_name = ability;
    simcount_obj_ = 10000;
    
    bool aggressive = false;
    bool last_die_is_scarlett = false;

    //SMALL; 0 2; 2 3; 3 2; 5 3; 1 3
    int cp = 10;
    size_t use_max_cards = 0;

    size_t sixit = 0;
    size_t samesis = 0;
    size_t tip_it = 0;
    size_t wild = 0;
    size_t twiceWild = 0;
    size_t slightlyWild = 0;
    size_t probManipulation = 2;
    size_t cheer = 0;

    if (!is_default_sim)
    {
        ability_name += " starting with " + std::to_string(org_dice[0] + 1) + std::to_string(org_dice[1] + 1) + std::to_string(org_dice[2] + 1) + std::to_string(org_dice[3] + 1) + std::to_string(org_dice[4] + 1);
    }
    else
    {
        ability_name += " default prob ";
    }
    if (sixit>=1 || samesis >= 1 || tip_it >= 1 || wild >= 1 || twiceWild >= 1)
    {
        ability_name += " with ";
    }
    if (sixit >= 1)
    {
        ability_name += "Sixit and ";
    }
    if (samesis >= 1)
    {
        ability_name += "Samesis and ";
    }
    if (tip_it >= 1)
    {
        ability_name += "TipIt and ";
    }
    if (wild >= 1)
    {
        ability_name += "wild and ";
    }
    if (twiceWild >= 1)
    {
        ability_name += "TwiceAsWild and ";
    }
    if (slightlyWild >= 1)
    {
        ability_name += "SlightlyWild and ";
    }
    if (sixit + samesis + tip_it + wild + twiceWild + slightlyWild >= 1)
    {
        ability_name += std::to_string(cp) + "CP";
    }
    
    std::vector<Card> cards = Helpers::getCards(sixit, samesis, tip_it,  wild,  twiceWild,  slightlyWild, probManipulation, cheer); // TODO LVL OF CARDS
    // prepare + READ DATA #####################################################################################################################    
    
    solver_anatomy_ = Helpers::transformDiceAnatomy(diceanatomy);
    ability_target_ = Helpers::transformAbility(ability);
    
    bool readed = false;
    auto start_time = std::chrono::high_resolution_clock::now();
    readed = read_ability(ability, diceanatomy, cards, cp, use_max_cards);
    auto end_time = std::chrono::high_resolution_clock::now();
    auto time = end_time - start_time;
    std::cout << "reading took " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;

    if (test_read_and_calc)
    {
        readed = false;
    }
    if (!readed)
    {
        std::vector<DiceIdx> mydiceanatomy = Helpers::transformDiceAnatomy(diceanatomy);
        std::vector<DiceIdx> target_ability = Helpers::transformAbility(ability);
        std::cout << "Ability " << ability << " not found in DB, please generate its lookup-data, then calculations will speed up alot " << std::endl;
        start_time = std::chrono::high_resolution_clock::now();
        get_all_positive_combs(mydiceanatomy, target_ability, cards, cp, use_max_cards);
        end_time = std::chrono::high_resolution_clock::now();
        time = end_time - start_time;
        std::cout << "preparation took " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds" << std::endl;
    }
    if (test_read_and_calc)
    {
        test_loaded_and_gen();
    }
    helper.closeDB();
    // DO CALCS######################################################################################################################
    float ergo = 0.0F;
    std::vector<bool> best_reroll{false, false, false, false, false};

    start_time = std::chrono::high_resolution_clock::now();
    //ergo = createMarkovChain(org_dice, is_default_sim, roll_atemps, rerolls);
    best_reroll = createMarkovChainSolver(org_dice, is_default_sim, roll_atemps, rerolls, diceanatomy, ability, cards, ergo);
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "markov took " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds for " << ability_name << " result " << ergo << std::endl << std::endl;

    start_time = std::chrono::high_resolution_clock::now();
    //ergo = createMarkovChain(org_dice, is_default_sim, roll_atemps, rerolls);
    best_reroll = createMarkovChainSolverEigenRow(org_dice, is_default_sim, roll_atemps, rerolls, diceanatomy, ability, cards, ergo);
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "markov Eigen took " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds for " << ability_name << " result " << ergo << std::endl << std::endl;
    std::cout << "best reroll " << best_reroll[0] << best_reroll[1] << best_reroll[2] << best_reroll[3] << best_reroll[4] << std::endl << std::endl;

    start_time = std::chrono::high_resolution_clock::now();
    ergo = createMarkovChain(org_dice, is_default_sim, roll_atemps, rerolls);
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "markov default took " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds for " << ability_name << " result " << ergo << std::endl << std::endl;

    start_time = std::chrono::high_resolution_clock::now();
    best_reroll=test_solver(org_dice, roll_atemps, rerolls, diceanatomy, ability, cards, is_default_sim, false, ergo);
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "best reroll " << best_reroll[0] << best_reroll[1] << best_reroll[2] << best_reroll[3] << best_reroll[4] << std::endl << std::endl;
    std::cout << "solver took " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds for " << ability_name << " result "<< ergo << std::endl << std::endl;
    
    //float erg = combo_sim_ultra_faster(org_dice, last_die_is_scarlett);
    //std::cout << "prob for " << ability_name << " is " << erg << "%" << std::endl;
    start_time = std::chrono::high_resolution_clock::now();
    best_reroll=test_all_rolls(org_dice, roll_atemps, rerolls, is_default_sim, ergo);
    end_time = std::chrono::high_resolution_clock::now();
    time = end_time - start_time;
    std::cout << "best reroll " << best_reroll[0] << best_reroll[1] << best_reroll[2] << best_reroll[3] << best_reroll[4] << std::endl << std::endl;
    std::cout << "simulator " << (time / std::chrono::milliseconds(1)) / 1000.0 << " seconds for " << ability_name << " result " << ergo << std::endl << std::endl;
    
}


// Approach: Simulation
// default sim
bool Simulator::find_keepers_comb_ultra_fast(std::vector<DiceIdx>&dice, std::vector<bool>&rerollers, bool randomize, bool just_one_reroll, const std::vector<DiceThrow>& all_combis)
{
    if (randomize)
    {
        for (size_t i = 0; i < rerollers.size(); i++)
        {
            if (rerollers[i])
            {
                //dice[i] = get_random_die();
                dice[i] = get_random_fast();
                if (just_one_reroll)
                {
                    break;
                }
            }
        }
    }
    const auto& dt = all_combis[get_hash(dice)];
    if (dt.success)
    {
        return true;
    }
    rerollers[0] = dt.rerollers[0];
    rerollers[1] = dt.rerollers[1];
    rerollers[2] = dt.rerollers[2];
    rerollers[3] = dt.rerollers[3];
    rerollers[4] = dt.rerollers[4];
    return false;
}

// fastest version
const DiceThrow* Simulator::find_keepers_comb_ultra_faster21(const DiceThrow* rerollers)
{
    //done loop unrolling for performance
    //reroll dice
    //v[4] + v[3] * 6 + v[2] * 36 + v[1] * 216 + v[0] * 1296;
    size_t hash = (rerollers->rerollers[0] ? get_random_fast() : rerollers->dice[0]) * 1296ULL;
    hash += (rerollers->rerollers[1] ? get_random_fast() : rerollers->dice[1]) * 216ULL;
    hash += (rerollers->rerollers[2] ? get_random_fast() : rerollers->dice[2]) * 36ULL;
    hash += (rerollers->rerollers[3] ? get_random_fast() : rerollers->dice[3]) * 6ULL;
    hash += (rerollers->rerollers[4] ? get_random_fast() : rerollers->dice[4]);
    return &(all_ability_combinations_dt[hash]);
}

const DiceThrow* Simulator::find_keepers_comb_ultra_faster_one_reroll21(const DiceThrow* rerollers)
{
    //reroll dice
    size_t hash = 0U;
    bool reroll = true;
    static std::vector<size_t> factor{ 1296ULL,216ULL,36ULL,6ULL,1ULL };
    for (size_t i = 0; i < number_dice_; i++)
    {
        if (reroll && rerollers->rerollers[i])
        {
            reroll = false;
            hash += get_random_fast() * factor[i];
        }
        else
        {
            hash += rerollers->dice[i] * factor[i];
        }
    }
    return &(all_ability_combinations_dt[hash]);
}

bool Simulator::go_for_comb_ultra_faster2(size_t atmp, size_t reroll, const DiceThrow* rerollers)
{
    for (size_t i = 0; i < atmp; i++)
    {
        rerollers = find_keepers_comb_ultra_faster21(rerollers);
        if (rerollers->success)
        {
            return true;
        }
    }
    for (size_t i = 0; i < reroll; i++)
    {
        rerollers = find_keepers_comb_ultra_faster_one_reroll21(rerollers);
        if (rerollers->success)
        {
            return true;
        }
    }
    return false;
}

size_t Simulator::combo_sim_ultra_faster_reroll_DT(const std::vector<DiceIdx>& org_dice, std::vector<bool>& org_rerollers, size_t roll_atemps, size_t rerolls, bool is_default_sim)
{
    size_t simcount = simcount_obj_;
    std::vector<DiceIdx> dice = org_dice;
    int time = 0;
    DiceThrow dt_reroll{};
    std::vector<bool> rerollers = { false, false, false, false, false };

    if (!is_default_sim && find_keepers_comb_ultra_fast(dice, rerollers, false, false, all_ability_combinations_dt))
    {
        return simcount;
    }
    //todo real copy, needed loop onrolling only in for loop
    dt_reroll.rerollers[0] = org_rerollers[0];
    dt_reroll.rerollers[1] = org_rerollers[1];
    dt_reroll.rerollers[2] = org_rerollers[2];
    dt_reroll.rerollers[3] = org_rerollers[3];
    dt_reroll.rerollers[4] = org_rerollers[4];

    dt_reroll.dice[0] = org_dice[0];
    dt_reroll.dice[1] = org_dice[1];
    dt_reroll.dice[2] = org_dice[2];
    dt_reroll.dice[3] = org_dice[3];
    dt_reroll.dice[4] = org_dice[4];
        
    size_t count = 0;
    bool result = false;
    for (size_t i = 0; i < simcount; i++)
    {
        if (go_for_comb_ultra_faster2(roll_atemps, rerolls, &dt_reroll))
        {
            count++;
        }
    }
    return count;
}

//chase
const DiceThrow* Simulator::go_for_comb_ultra_faster2_chase(size_t atmp, size_t reroll, const DiceThrow* rerollers)
{
    for (size_t i = 0; i < atmp; i++)
    {
        rerollers = find_keepers_comb_ultra_faster21(rerollers);
        if (rerollers->success)
        {
            return rerollers;
        }
    }
    for (size_t i = 0; i < reroll; i++)
    {
        rerollers = find_keepers_comb_ultra_faster_one_reroll21(rerollers);
        if (rerollers->success)
        {
            return rerollers;
        }
    }
    return rerollers;
}

size_t Simulator::combo_sim_ultra_faster_reroll_DT_chase(const std::vector<DiceIdx>& org_dice, std::vector<bool>& org_rerollers, size_t roll_atemps, size_t rerolls, bool is_default_sim)
{
    size_t simcount = simcount_obj_;
    std::vector<DiceIdx> dice = org_dice;
    int time = 0;
    DiceThrow dt_reroll{};
    std::vector<bool> rerollers = { false, false, false, false, false };

    if (!is_default_sim && find_keepers_comb_ultra_fast(dice, rerollers, false, false, all_ability_combinations_dt))
    {
        return simcount;
    }
    //todo real copy, needed loop onrolling only in for loop
    dt_reroll.rerollers[0] = org_rerollers[0];
    dt_reroll.rerollers[1] = org_rerollers[1];
    dt_reroll.rerollers[2] = org_rerollers[2];
    dt_reroll.rerollers[3] = org_rerollers[3];
    dt_reroll.rerollers[4] = org_rerollers[4];

    dt_reroll.dice[0] = org_dice[0];
    dt_reroll.dice[1] = org_dice[1];
    dt_reroll.dice[2] = org_dice[2];
    dt_reroll.dice[3] = org_dice[3];
    dt_reroll.dice[4] = org_dice[4];

    const DiceThrow* resultDt= &dt_reroll;
    bool result = false;
    size_t counter = 0U;
    for (size_t i = 0; i < simcount; i++)
    {

        resultDt = go_for_comb_ultra_faster2_chase(roll_atemps, rerolls, &dt_reroll);
        size_t hash = resultDt->hash;
        if (resultDt->success)
        {
            counter++;
        }
        for (auto& cd : chase_data_)
        {
            if (cd.all_ability_combinations_dt[hash].success)
            {
                cd.amount++;
            }
        }
    }
    return counter;
}


//LOADING AND READING DATABASE######################################################################################################################################################

void Simulator::precalc_ability(std::string ability, std::string diceanatomy)
{
    std::vector<DiceIdx> mydiceanatomy = Helpers::transformDiceAnatomy(diceanatomy);
    std::vector<DiceIdx> target_ability = Helpers::transformAbility(ability);
    std::string ability_name = "";
    if (ability == "SMALL" || ability == "BIG")
    {
        ability_name = ability;
    }
    else
    {
        ability_name = ability + "-" + diceanatomy;
    }
    precalc_ability(ability_name, target_ability, mydiceanatomy, false);
    //precalc_ability(ability_name, target_ability, mydiceanatomy, true);//DTA
    return;
}

void Simulator::precalc_ability(std::string ability_name, const std::vector<DiceIdx>& target_ability, std::vector<DiceIdx>& mydiceanatomy, bool isDTA)
{
    static const size_t cp_anz = 20;
    static const size_t cards_anz = 11;
    generator_anatomy = mydiceanatomy;
    generator_target = target_ability;
    //calculate every card-combination
    std::vector<DiceIdx> stack;
    size_t sampleCount = 8U;
    std::vector<DiceIdx> options = { 0, 1, 2, 3, 4 };// maximum levels , 0 = no card
    if (!isDTA)
    {
        options = { 0, 1, 2};// maximum levels , 0 = no card
    }
    std::vector<std::vector<DiceIdx>> erg{};
    std::vector<std::vector<DiceIdx>> erg2{};
    Helpers::getCombinations(options, sampleCount, stack, erg);
    erg.erase(erg.begin());
    float max_anz = (isDTA ? 1129342.0F : 10366.0F)/2.0F;
    int done_counter_anz = 0;
    // TODO CREATE FOLDER
    //filename = "./precalcs/" + ability_name + ".txt";
    std::string filename = "./precalcs/" + ability_name + ".txt";
    if (isDTA)
    {
        filename = "./precalcsDTA/" + ability_name + ".txt";
    }
#ifdef __linux__
    filename = "/content/drive/MyDrive/DTSimulatorTest/precalcs/" + ability_name + ".txt";
#endif
    size_t start_number_lines = 0;
    if (Helpers::file_exists(filename))
    {
        start_number_lines = Helpers::get_number_lines(filename);
        if (start_number_lines >= max_anz)
        {
            return;
        }
    }

    std::ofstream myfile;
    myfile.open(filename, std::ios::out | std::ios::app);
    if (myfile.fail())
    {
        std::cout << " cant open file, make sure folder \"precalcs\" exists" << std::endl;
        return;
    }

    //create database for speedup
    std::vector<std::vector<std::vector<DiceThrow>>> save_db{};

    
    for (size_t i = 0; i < cp_anz; i++)
    {
        std::vector<std::vector<DiceThrow>> s1{};
        for (size_t j = 0; j < cards_anz; j++)
        {
            std::vector<DiceThrow> s2;
            s2.resize(possible_combs_blow_up_size_);
            s1.push_back(s2);
        }
        save_db.push_back(s1);
    }

    auto start_time = std::chrono::high_resolution_clock::now();
    auto print_time = ((start_time - start_time) / std::chrono::milliseconds(1))/1000.0;

    for (const std::vector<DiceIdx>& combi : erg)
    {
        
        bool swild = combi[0] >= 1;
        bool samesis = combi[1] >= 1;
        bool tip_it = combi[2] >= 1;
        bool wild = combi[3] >= 1;
        bool twiceWild = combi[4] >= 1;
        bool sixit = combi[5] >= 1;

        if (!isDTA && combi[0] >= 1)
        {
            continue;
        }
        if (!isDTA && (combi[1] >= 2 || combi[2] >= 2 || combi[3] >= 2 || combi[4] >= 2 || combi[6] >= 2 || combi[7] >= 2))
        {
            continue;
        }
        if (combi[0] > 3 || combi[0] == 2) //slightly wild 1 and 2 dont differ (only you can use it on teammates)
        {
            continue;
        }
        if (combi[1] > 3) //samesis
        {
            continue;
        }
        if (combi[2] > 3) //tip it
        {
            continue;
        }
        if (combi[3] > 3) //wild
        {
            continue;
        }
        if (combi[4] > 4) //twice as wild has 4 different stages
        {
            continue;
        }
        if (combi[5] > 2) //probbiltiy manipulation 0,1,2
        {
            continue;
        }
        if (combi[6] > 1) //cheer 0,1
        {
            continue;
        }
        if (combi[7] > 2) //sixit
        {
            continue;
        }
        
        std::vector<Card> cards = Helpers::getCards(combi[7], combi[1],  combi[2], combi[3],  combi[4],  combi[0], combi[5], combi[6]);
        
        size_t maxcp = 0;
        for (const auto& c : cards)
        {
            maxcp += c.cp_cost;
        }
        size_t max_cards = cards.size();

        //the very first run is with 0 cp -> thats the default run with no cards
        size_t tempcp = done_counter_anz == 0 ? 0 : std::min((size_t)1, maxcp);
        //clear saved list, it has to be calculated new

        if (max_cards >= 20 || maxcp >= 20)
        {
            std::cout << " anzcards or cp TO BIG " << max_cards << " " << maxcp << std::endl;
            return;
        }

        for (size_t i = 0; i < cp_anz; i++)
        {
            for (size_t j = 0; j < cards_anz; j++)
            {
                save_db[i][j].clear();
            }
        }
        for (size_t cp = 0; cp <= maxcp; cp++)
        {
            for (size_t anzcards = 0; anzcards <= max_cards; anzcards++)
            {   
                if (start_number_lines <= done_counter_anz)
                {
                    std::stringstream ss;
                    ss << Helpers::get_cards_string(cards) << " " << cp << " " << anzcards;
                    possible_list_with_cheat_dt_save_last2 = nullptr;//.clear();
                    possible_list_with_cheat_dt_save_last = nullptr;
                    if (cp > 0)
                    {
                        possible_list_with_cheat_dt_save_last2 = &(save_db[cp-1][anzcards]);
                    }
                    if (anzcards > 0)
                    {
                        possible_list_with_cheat_dt_save_last = &(save_db[cp][anzcards-1]);
                    }
                    get_all_positive_combs_storage_lower_bound(mydiceanatomy, target_ability, cards, cp, anzcards);
                    save_db[cp][anzcards] = possible_list_with_cheat_dt;
                    
                    auto end_time = std::chrono::high_resolution_clock::now();
                    auto time = end_time - start_time;
                    auto tmili = (time / std::chrono::milliseconds(1)) / 1000.0;
                    if (tmili > print_time)
                    {
                        print_time = tmili + 5;
                        float done = (done_counter_anz - start_number_lines) / (max_anz);
                        double timefor100 = tmili / done ;
                        double proz_done = done_counter_anz / (max_anz);
                        double time_remain = (timefor100 - (proz_done * timefor100)) / (60.0);
                        std::cout << ability_name << " " << ss.str() << " preparation took " << tmili << " seconds, " << int(10000.0 * proz_done) / 100.0 <<"%" << std::endl;
                        std::cout << ability_name << " " << ss.str() << " remaining time: " << time_remain << " min, " << time_remain/60.0 << " h " << std::endl;
                    }
                    size_t counter = 0;
                    for (const auto& data : possible_list_with_cheat_dt)
                    {
                        if (data.success)
                        {
                            counter++;
                        }
                    }
                    ss << std::endl;// << counter << ";";
                    for (const auto& data : possible_list_with_cheat_dt)
                    {
                        ss << data.get_save_string();
                    }
                    myfile << ss.str()<< std::endl;
                }
                done_counter_anz++;
            }
        }
        if (start_number_lines <= done_counter_anz)
        {
            myfile.close();
            myfile.open(filename, std::ios::out | std::ios::app);
            if (myfile.fail())
            {
                for (size_t i = 0; i < cp_anz; i++)
                {
                    for (size_t j = 0; j < cards_anz; j++)
                    {
                        save_db[i][j].clear();
                    }
                }
                std::cout << " cant open file" << std::endl;
                return;
            }
        }
    }
    for (size_t i = 0; i < cp_anz; i++)
    {
        for (size_t j = 0; j < cards_anz; j++)
        {
            save_db[i][j].clear();
        }
    }
    myfile.close();
}

bool Simulator::read_ability(std::string ability_name, std::string diceanatomy, const std::vector<Card>& cards, size_t cp, size_t numbercards)
{
    std::string table_name = ability_name + "_" + diceanatomy;
    if (ability_name == "SMALL" || ability_name == "BIG")
    {
        table_name = ability_name;
    }
    std::vector<Card> cardscopy = cards;
    size_t maxcp = 0;
    size_t max_cards = cards.size();
    for (const auto& c : cards)
    {
        maxcp += c.cp_cost;
    }
    size_t m_cp = std::min(cp, maxcp);
    size_t m_cards = std::min(numbercards, max_cards);
    if (m_cards == 0)
    {
        //we didnt save 0,0,0,0,0, 
        // we calculated 1,0,0,0,0 0 1 = sixit active, but zero cp, and 1 card
        cardscopy.clear();
        cardscopy.push_back(Helpers::generateCard("sixit", 1));
        m_cp = 0;
        m_cards = 1;
    }
    std::stringstream ss;
    ss << Helpers::get_cards_string(cardscopy) << " " << m_cp << " " << m_cards;

    std::string searched_line = ss.str();
    bool isDTA = false;
    for (const auto& c : cardscopy)
    {
        if (c.lvl >= 2 || c.card_id >= 5)
        {
            isDTA = true;
            break;
        }
    }
    //std::cout << table_name << " " << searched_line << std::endl;
    std::string sqlite_data = helper.sqlite_get_data(table_name, searched_line, isDTA);
    if (sqlite_data == "")
    {
        return false;
    }
    std::vector<std::string> splitter = Helpers::my_string_split(sqlite_data, "x", "y");

    for (size_t i = 0; i < splitter.size(); i++)
    {
        possible_combs_[i].load_from_string(splitter[i]);
    }
    //data is stored in possible_combs_ save it to possible_list_with_cheat_dt!
    possible_list_with_cheat_dt = possible_combs_;
    possible_list_with_cheat_dt_succ_only.clear();
    bool straight = ability_name == "BIG" || ability_name == "SMALL";
    bool is_big = ability_name == "BIG";
    //create list of dt that are only successfull with cards

    bool sixit = false;
    size_t sixit_cp = 1;
    for (size_t i = 0; i < cards.size(); i++)
    {
        const auto& c = cards[i];
        if (c.card_id == 0)
        {
            sixit = true;
            sixit_cp = c.cp_cost;
        }
    }

    for (size_t i = 0; i < possible_combs_.size(); i++)
    {
        const DiceThrow& dt = possible_combs_[i];
        if (!dt.success)
        {
            continue;
        }
        if (dt.manipula[0].card != 0)
        {
            //manipulated are allways added
            possible_list_with_cheat_dt_succ_only.push_back(dt);
            continue;
        }
        bool added = false;
        if (straight && !is_big)
        {
            if (!is_big_straight(dt))
            {
                possible_list_with_cheat_dt_succ_only.push_back(dt);
                added = true;
            }
        }
        else
        {
            possible_list_with_cheat_dt_succ_only.push_back(dt);
            added = true;
        }
        if (added && sixit && cp >= sixit_cp && numbercards >= 1 && possible_list_with_cheat_dt_succ_only.back().dice[number_dice_ - 1] == 5)
        {
            possible_list_with_cheat_dt_succ_only.back().rerollers[number_dice_ - 1] = true;
        }
    }
   
    blowUpData();
    loaded_all_ability_combinations_dt = all_ability_combinations_dt; // ONLY FOR TESTING

    return true;
}

void Simulator::test_loaded_and_gen()
{
    size_t counter = 0;
    for (size_t i = 0; i < all_ability_combinations_dt.size(); i++)
    {
        const DiceThrow& dt1 = all_ability_combinations_dt[i];
        const DiceThrow& dt2 = loaded_all_ability_combinations_dt[i];
        if (!dt1.isEqual3(dt2))
        {
            counter++;
        }
    }
    std::cout << "errors: " << counter << std::endl;
}

// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################
// CODE FOR PRE-CALCULATING DATA #############################################################################################################################################

bool Simulator::hit_target_ability(const std::vector<DiceIdx>& target, DiceThrow& dice, const std::vector<DiceIdx>& mydiceanatomy)
{
    std::vector<DiceIdx> symbols = target;
    for (size_t i = 0; i < number_dice_; i++)
    {
        size_t symidx = mydiceanatomy[dice.dice[i]];
        if (symbols[symidx] > 0)
        {
            symbols[symidx]--;
            dice.rerollers[i] = false;
        }
        else
        {
            dice.rerollers[i] = true;
        }
        
    }
    for (size_t i = 0; i < symbols.size(); i++)
    {
        if (symbols[i] > 0)
        {
            return false;
        }
    }
    return true;
}

void Simulator::get_samessis(const DiceThrow& dice, std::vector<DiceThrow>& samesis, size_t save_idx)
{
    size_t count = 0;
    for (size_t i = 0; i < number_dice_; i++)
    {
        for (size_t j = 0; j < number_dice_; j++)
        {
            if (i != j)
            {
                samesis[count] = dice;
                samesis[count].success = true;
                if (samesis[count].dice[i] == dice.dice[j])
                {
                    samesis[count].success = false;
                }
                samesis[count].manipula[save_idx].card = 1;
                samesis[count].manipula[save_idx].dice1val = samesis[count].dice[i];
                samesis[count].manipula[save_idx].dice1change = dice.dice[j];
                samesis[count].dice[i] = dice.dice[j];
                //samesis[count].rerollers[i] = false; //value of dice i doesnt matter
                if (samesis[count].reroll_manipulation[i] == 0)
                {
                    samesis[count].reroll_manipulation[i] = 1;
                }
                samesis[count].rerollers[j] = false;
                if (samesis[count].reroll_manipulation[j] <= 0)
                {
                    //we are not allowed to change this, if it was untuched or tiped before
                    //it is allowed to be rerolled, if it was sixed / wilded before
                    samesis[count].reroll_manipulation[j] = -2;
                }
                //samesis[count].order_matters[i] = true;
                //samesis[count].prev_manipulation[i] = 1;
                
                count++;
            }
        }
    }
}

void Simulator::get_samessis2(const DiceThrow& dice, std::vector<DiceThrow>& samesis, size_t save_idx)
{
    size_t count = 0;
    for (const auto & helper : samesis2_helper)
    {
        //copy dice i to j and k, 
        size_t i = helper[0];
        size_t j = helper[1];
        size_t k = helper[2];

        samesis[count] = dice;
        samesis[count].success = true;

        if (dice.dice[i] == dice.dice[j] || dice.dice[i] == dice.dice[k])
        {
            samesis[count].success = false;
        }
        samesis[count].manipula[save_idx].card = 1;
        samesis[count].manipula[save_idx].dice1val = samesis[count].dice[j];
        samesis[count].manipula[save_idx].dice1change = dice.dice[i];
        if (j != k)
        {
            samesis[count].manipula[save_idx].dice2val = samesis[count].dice[k];
            samesis[count].manipula[save_idx].dice2change = dice.dice[i];
        }
        samesis[count].dice[j] = dice.dice[i];
        samesis[count].dice[k] = dice.dice[i];
        //samesis[count].order_matters[j] = true;
        //samesis[count].prev_manipulation[j] = 1;
        //samesis[count].order_matters[j] = true;
        //samesis[count].prev_manipulation[j] = 1;
        //samesis[count].rerollers[i] = false; //value of dice i doesnt matter
        if (samesis[count].reroll_manipulation[j] == 0)
        {
            samesis[count].reroll_manipulation[j] = 1;
        }
        if (samesis[count].reroll_manipulation[k] == 0)
        {
            samesis[count].reroll_manipulation[k] = 1;
        }
        samesis[count].rerollers[i] = false;
        if (samesis[count].reroll_manipulation[i] <= 0)
        {
            //we are not allowed to change this, if it was untuched or tiped before
            //it is allowed to be rerolled, if it was sixed / wilded before
            samesis[count].reroll_manipulation[i] = -2;
        }

        count++;
    }
}

void Simulator::get_probabilitymanipulations(const DiceThrow& dice, std::vector<DiceThrow>& tips, size_t save_idx)
{
    for (size_t i = 0; i < tipit_anz_; i++)
    {
        size_t idx = i / 2;
        tips[i] = dice;
        int manipulator = 1;
        if (i % 2 == 1)
        {
            manipulator = -1;
        }
        if (dice.dice[idx] == 0  || dice.dice[idx] == 2 || dice.dice[idx] == 4 || dice.dice[idx] == 5 && manipulator == 1)
        {
            tips[i].success = false;
            continue;
        }

        tips[i].success = true;
        tips[i].manipula[save_idx].card = 2;
        tips[i].manipula[save_idx].dice1val = dice.dice[idx];

        tips[i].manipula[save_idx].dice1change = dice.dice[idx] + manipulator;


        tips[i].rerollers[idx] = false;
        tips[i].dice[idx] = dice.dice[idx] + manipulator;
        if (tips[i].reroll_manipulation[idx] == 0)
        {
            tips[i].reroll_manipulation[idx] = -1;
        }
    }
}

void Simulator::get_tipits(const DiceThrow& dice, std::vector<DiceThrow>& tips, size_t save_idx)
{
    for (size_t i = 0; i < tipit_anz_; i++)
    {
        size_t idx = i / 2;
        tips[i] = dice;
        int manipulator = 1;
        if (i % 2 == 1)
        {
            manipulator = -1;
        }
        if (dice.dice[idx] == 0 && manipulator == -1 || dice.dice[idx] == 5 && manipulator == 1)
        {
            tips[i].success = false;
            continue;
        }

        tips[i].success = true;
        tips[i].manipula[save_idx].card = 2;
        tips[i].manipula[save_idx].dice1val = dice.dice[idx];
        
        tips[i].manipula[save_idx].dice1change = dice.dice[idx] + manipulator;

        
        tips[i].rerollers[idx] = false;
        tips[i].dice[idx] = dice.dice[idx] + manipulator;
        if (tips[i].reroll_manipulation[idx] == 0)
        {
            tips[i].reroll_manipulation[idx] = -1;
        }
    }
}

void Simulator::get_tipits2(const DiceThrow& dice, std::vector<DiceThrow>& tips, size_t save_idx)
{
    for (size_t i = 0; i < tipit2_anz_; i++)
    {
        size_t idx = i / 4;
        tips[i] = dice;
        int manipulator = 2;
        if (i % 4 == 1)
        {
            manipulator = 1;
        }
        if (i % 4 == 2)
        {
            manipulator = -1;
        }
        if (i % 4 == 3)
        {
            manipulator = -2;
        }
        if (dice.dice[idx] == 0 && manipulator <= -1 || dice.dice[idx] == 5 && manipulator >= 1 || dice.dice[idx] == 1 && manipulator == -2 || dice.dice[idx] == 4 && manipulator == 2)
        {
            tips[i].success = false;
            continue;
        }

        tips[i].success = true;
        tips[i].manipula[save_idx].card = 2;
        tips[i].manipula[save_idx].dice1val = dice.dice[idx];

        tips[i].manipula[save_idx].dice1change = dice.dice[idx] + manipulator;


        tips[i].rerollers[idx] = false;
        tips[i].dice[idx] = dice.dice[idx] + manipulator;
        if (tips[i].reroll_manipulation[idx] == 0)
        {
            tips[i].reroll_manipulation[idx] = -1;
        }
        //tips[i].order_matters[idx] = true;
        //tips[i].prev_manipulation[idx] = 2;
    }
}

void Simulator::get_sixits(const DiceThrow& dice, std::vector<DiceThrow>& sixeds, size_t save_idx)
{

    for (size_t i = 0; i < sixit_anz_; i++)
    {
        sixeds[i] = dice;
        sixeds[i].success = true;
        if (sixeds[i].dice[i] == 5)
        {
            sixeds[i].success = false;
        }
        sixeds[i].manipula[save_idx].card = 3;
        sixeds[i].manipula[save_idx].dice1val = sixeds[i].dice[i];
        sixeds[i].dice[i] = 5;
        //mark, that the die can be rerolled if not needed for a samesis before
        if (sixeds[i].reroll_manipulation[i] >= -1)
        {
            sixeds[i].reroll_manipulation[i] = 1;
        }
        //sixeds[i].order_matters[i] = false;
        //sixeds[i].prev_manipulation[i] = 3;
    }
    
}

void Simulator::get_wilds(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx)
{
    size_t idx = 0U;
    DiceIdx number = 0U;
    for (size_t i = 0U; i < wilds_anz_; i++)
    {
        wilds[i] = dice;
        wilds[i].success = wilds[i].dice[idx] != number;
        wilds[i].manipula[save_idx].card = 4;
        wilds[i].manipula[save_idx].dice1val = wilds[i].dice[idx];
        wilds[i].manipula[save_idx].dice1change = number;
        wilds[i].dice[idx] = number;
        //wilds[i].order_matters[idx] = false;
        //wilds[i].prev_manipulation[idx] = 4;
        //mark, that the die can be rerolled if not needed for a samesis before
        if (wilds[i].reroll_manipulation[idx] >= -1)
        {
            wilds[i].reroll_manipulation[idx] = 1;
        }
        number++;
        if (number > 5U)//because of 6 sided die
        {
            number = 0U;
            idx++;
        }
    }
}

void Simulator::get_wilds2(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx)
{
    size_t sizer = wild2_helper.size();

    for (size_t i = 0; i < sizer; i++)
    {
        const auto& helper = wild2_helper[i];
        DiceIdx idx1 = helper[0];
        DiceIdx idx2 = helper[2];
        DiceIdx num1 = helper[1];
        DiceIdx num2 = helper[3];
        wilds[i] = dice;
        wilds[i].success = true;
        wilds[i].manipula[save_idx].card = 5;
        wilds[i].manipula[save_idx].dice1val = wilds[i].dice[idx1];
        wilds[i].manipula[save_idx].dice1change = num1;
        if (idx1 != idx2)
        {
            wilds[i].manipula[save_idx].dice2val = wilds[i].dice[idx2];
            wilds[i].manipula[save_idx].dice2change = num2;
        }
        wilds[i].success = wilds[i].dice[idx1] != num1 && wilds[i].dice[idx2] != num2;
        wilds[i].dice[idx1] = num1;
        wilds[i].dice[idx2] = num2;
        //mark, that the die can be rerolled if not needed for a samesis before
        if (wilds[i].reroll_manipulation[idx1] >= -1)
        {
            wilds[i].reroll_manipulation[idx1] = 1;
        }
        if (wilds[i].reroll_manipulation[idx2] >= -1)
        {
            wilds[i].reroll_manipulation[idx2] = 1;
        }
    }

}

void Simulator::get_slightlywilds(const DiceThrow& dice, std::vector<DiceThrow>& wilds, size_t save_idx)
{
    size_t idx = 0;
    int number = 0;
    for (size_t i = 0; i < slightlywild_anz_; i++)
    {
        wilds[i] = dice;
        wilds[i].success = wilds[i].dice[idx] != number;
        wilds[i].manipula[save_idx].card = 6U;
        wilds[i].manipula[save_idx].dice1val = wilds[i].dice[idx];
        wilds[i].manipula[save_idx].dice1change = number;
        wilds[i].dice[idx] = number;
        //wilds[i].order_matters[idx] = false;
        //wilds[i].prev_manipulation[idx] = 6;
        //mark, that the die can be rerolled if not needed for a samesis before
        if (wilds[i].reroll_manipulation[idx] >= -1)
        {
            wilds[i].reroll_manipulation[idx] = 1;
        }
        number++;
        if (number > 4) // because we can only manipulate each die from 1-5 ( = 0-4)
        {
            number = 0;
            idx++;
        }
    }
}

int Simulator::global_check_unsorted(const DiceThrow& dice)
{
    DiceHash number = get_hash(dice);
    if (possible_combs_blow_up_[number].success)
    {
        return (int)number;
    }

    return -1;
}

int Simulator::test_all_combis(const DiceThrow& dice, size_t cp, size_t useMaxCards, size_t combo_store_idx)
{
    //TODO possible speedup: WITHOUT tipit: samesis is allways played last:
    // you have A,B  samesis and wild: if you play samesis first, then wild, you can get AA, BB, WA , WB = same as only wild / samesis one die
    // if you play always samesis only at last you can get: AA, BB, AW, WB, WW
    size_t current_combo_store_idx = combo_store_idx;
    if (current_combo_store_idx >= combo_store_.size())
    {
        std::cout << "storeage to small" << std::endl;
        return -1;
    }
    
    size_t number_tokens = 0;
    for (const auto& c : card_store_[current_combo_store_idx])
    {
        number_tokens += c.card_id >= 7 ? 1 : 0;
    }
    if ((useMaxCards == 0 && number_tokens == 0))
    {
        return -1;
    }
    std::vector<DiceThrow>& wilds = combo_store_[current_combo_store_idx];
    std::vector<Card>& cards = card_store_[current_combo_store_idx];
    std::vector<Card>& cards2 = card_store_[current_combo_store_idx+1];
    size_t anz = 30;
    current_combo_store_idx++;
    
    for (size_t i = 0; i < cards.size(); i++)
    {
        const auto& card = cards[i];
        if (card.can_use && cp >= card.cp_cost)
        {
            //copy cards to next lvl
            cards2 = cards;
            size_t copycp = cp;
            cards2[i].can_use = false;
            copycp -= card.cp_cost;
            if (card.function_to_call == 0)
            {
                anz = sixit_anz_;
                get_sixits(dice, wilds, current_combo_store_idx-1);
            }
            if (card.function_to_call == 1)
            {
                anz = samesis_anz_;
                get_samessis(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 2)
            {
                anz = samesis2_anz_;
                get_samessis2(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 3)
            {
                anz = tipit_anz_;// anz = 8;
                get_tipits(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 4)
            {
                anz = tipit2_anz_;
                get_tipits2(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 5)
            {
                anz = wilds_anz_;              
                get_wilds(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 6)
            {
                anz = twiceaswild_anz_;
                get_wilds2(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 7)
            {
                anz = slightlywild_anz_;
                get_slightlywilds(dice, wilds, current_combo_store_idx - 1);
            }
            if (card.function_to_call == 8)
            {
                anz = tipit_anz_;
                get_probabilitymanipulations(dice, wilds, current_combo_store_idx - 1);
            }
            // mark unseccessfull
            for (size_t i = 0; i < anz; i++)
            {
                if (!wilds[i].success)
                {
                    continue;
                }
                wilds[i].hash = possible_combs_blow_up_to_sorted_idx[get_hash(wilds[i])];
            }
            for (size_t i = 0; i < anz; i++)
            {
                if (!wilds[i].success)
                {
                    continue;
                }
                size_t a = wilds[i].hash;
                for (size_t j = i + 1; j < anz; j++)
                {
                    if (!wilds[j].success)
                    {
                        continue;
                    }
                    if (a == wilds[j].hash)
                    {
                        wilds[j].success = false;
                    }

                }
            }
            // test if dicethrows are successfull
            for (size_t j = 0; j< anz; j++)
            {
                const auto& d1 = wilds[j];
                
                if (d1.success)
                {
                    int val = global_check_unsorted(d1);
                    if (val >= 0)
                    {
                        target_reroll_dice = possible_combs_blow_up_[val];
                        target_reroller_start = d1;
                        return val;
                    }
                }
            }
            bool can_play_cards = false;
            for (const auto& card : cards2)
            {
                if (card.can_use && card.cp_cost <= cp)
                {
                    can_play_cards = true;
                    break;
                }
            }
            if (can_play_cards)
            {
                
                for (size_t j = 0; j < anz; j++)
                {
                    const auto& d1 = wilds[j];
                    if (d1.success)
                    {
                        if (card.card_id < 6)
                        {
                            if (this->generator_target[0] < 7)
                            {
                                if (!fast_test(this->generator_target, d1, this->generator_anatomy, cards2, copycp, useMaxCards - 1))
                                {
                                    continue;
                                }
                            }
                            int val = test_all_combis(d1, copycp, useMaxCards - 1, current_combo_store_idx);
                            if (val >= 0)
                            {
                                return val;
                            }
                        }
                        else
                        {
                            // TOKEN (cardid 7 and 8)
                            if (this->generator_target[0] < 7)
                            {
                                if (!fast_test(this->generator_target, d1, this->generator_anatomy, cards2, copycp, useMaxCards))
                                {
                                    continue;
                                }
                            }
                            int val = test_all_combis(d1, copycp, useMaxCards, current_combo_store_idx);
                            if (val >= 0)
                            {
                                return val;
                            }
                        }
                    }
                }
                int val2 = test_all_combis(dice, cp, useMaxCards, current_combo_store_idx);
                if (val2 >= 0)
                {
                    return val2;
                }
            }
            
            if ((card.function_to_call >= 1 && card.function_to_call<= 4) || card.function_to_call == 8)
            {
                // tipit and samesis depens on play-order
                continue;
            }
            break;
        }
    }
    return -1;
}

void Simulator::update_rerollers(DiceThrow& dt)
{
    //search objective with most matching numbers
    int max_matches = -1;
    size_t best_idx = 0;
    for (size_t i = 0; i < possible_list_with_cheat_dt_succ_only.size(); i++)
    {
        const auto& o = possible_list_with_cheat_dt_succ_only[i];
        int matches = 0;
        size_t idxd = 0;
        size_t idxo = 0;

        //merge sort compare:
        while (idxd < number_dice_ && idxo < number_dice_)
        {
            if (dt.dice[idxd] == o.dice[idxo])
            {
                idxd++;
                idxo++;
                matches+=10;
            }
            else
            {
                if (dt.dice[idxd] < o.dice[idxo])
                {
                    idxd++;
                }
                else
                {
                    idxo++;
                }
            }

        }
        for (size_t j = 0; j < number_dice_; j++)
        {
            if (o.rerollers[j])
            {
                matches +=1;
            }
        }
        if (max_matches < matches)
        {
            max_matches = matches;
            best_idx = i;
        }

    }
    //get rerollers
    auto best_dt = possible_list_with_cheat_dt_succ_only[best_idx];
    for (size_t i = 0; i < number_dice_; i++)
    {
        dt.rerollers[i] = true;
    }
    size_t idxd = 0;
    size_t idxo = 0;
    //merge sort compare:
    while (idxd < number_dice_ && idxo < number_dice_)
    {
        if (dt.dice[idxd] == best_dt.dice[idxo])
        {
            if (best_dt.rerollers[idxo] == false)
            {
                dt.rerollers[idxd] = false;
            }
            idxd++;
            idxo++;
        }
        else
        {
            if (dt.dice[idxd] < best_dt.dice[idxo])
            {
                idxd++;
            }
            else
            {
                idxo++;
            }
        }

    }
}

bool Simulator::is_straight(DiceThrow& dt, bool is_big)
{
    size_t hits[6]{ 0 };
    for (size_t i = 0; i < number_dice_; i++)
    {
        dt.rerollers[i] = false;
        if (hits[dt.dice[i]]==0)
        {
            hits[dt.dice[i]] = 1;
        }
        else
        {
            dt.rerollers[i] = true;
        }
    }
    //big ones
    size_t b1 = hits[0] + hits[1] + hits[2] + hits[3] + hits[4];
    if (b1 == 5)
    {
        return true;
    }
    size_t b2 = b1 + hits[5] - hits[0];
    if (b2==5)
    {
        return true;
    }
    if (is_big)
    {
        if (b1 >= b2)
        {
            for (size_t i = 0; i < number_dice_; i++)
            {
                if (dt.dice[i] == 5)
                {
                    dt.rerollers[i] = true;
                }
            }
        }
        else
        {
            for (size_t i = 0; i < number_dice_; i++)
            {
                if (dt.dice[i] == 0)
                {
                    dt.rerollers[i] = true;
                }
            }
        }
        return false;
    }
    //small ones
    if (hits[0] && hits[1] && hits[2] && hits[3])
    {
        for (size_t i = 0; i < number_dice_; i++)
        {
            if (dt.dice[i] == 5)
            {
                dt.rerollers[i] = true;
            }
            if (dt.dice[i] == 4)
            {
                dt.rerollers[i] = true;
            }
        }
        return true;
    }
    if (hits[1] && hits[2] && hits[3] && hits[4])
    {
        for (size_t i = 0; i < number_dice_; i++)
        {
            if (dt.dice[i] == 5)
            {
                dt.rerollers[i] = true;
            }
            if (dt.dice[i] == 0)
            {
                dt.rerollers[i] = true;
            }
        }
        return true;
    }
    if (hits[2] && hits[3] && hits[4] && hits[5])
    {
        for (size_t i = 0; i < number_dice_; i++)
        {
            if (dt.dice[i] == 1)
            {
                dt.rerollers[i] = true;
            }
            if (dt.dice[i] == 0)
            {
                dt.rerollers[i] = true;
            }
        }
        return true;
    }

    return false;
}

bool  Simulator::is_big_straight(const DiceThrow& dt)
{
    bool hits[6]{ false };
    for (size_t i = 0; i < number_dice_; i++)
    {
        if (hits[dt.dice[i]] == false)
        {
            hits[dt.dice[i]] = true;
        }
    }
    //big ones
    if (hits[0] && hits[1] && hits[2] && hits[3] && hits[4])
    {
        return true;
    }
    if (hits[1] && hits[2] && hits[3] && hits[4] && hits[5])
    {
        return true;
    }
    return false;
}

bool Simulator::fast_test(const std::vector<DiceIdx>& target, const DiceThrow& dice, const std::vector<DiceIdx>& mydiceanatomy, const std::vector<Card>& cards, size_t cp, size_t use_max_cards)
{
    std::vector<DiceIdx> symbols = target;
    size_t target_count = 0;
    for (size_t i = 0; i < target.size(); i++)
    {
        target_count += target[i];
    }
    if (target_count > number_dice_)
    {
        return false;// we have more symbols to roll then die available
    }
    for (size_t i = 0; i < number_dice_; i++)
    {
        size_t symidx = mydiceanatomy[dice.dice[i]];
        if (symbols[symidx] > 0)
        {
            symbols[symidx]--;
        }
    }
    size_t missing_count = 0;
    for (size_t i = 0; i < symbols.size(); i++)
    {
        missing_count += symbols[i];
    }
    if (missing_count == 0)
    {
        return true;
    }
    size_t number_cards_available = 0;
    size_t number_tokens = 0;
    for (const auto& c : cards)
    {
        number_cards_available += c.can_use ? 1 : 0;
        number_tokens += c.card_id >= 7 ? 1:0;
    }
    if (use_max_cards == 0 && number_tokens == 0 || cards.empty())
    {
        return false;
    }

    std::vector<Card> cardCPorder = cards;
    std::vector<Card> cardManiporder = cards;
    std::sort(cardCPorder.begin(), cardCPorder.end(), [](Card a, Card b) {
        if (a.cp_cost == b.cp_cost)
        {
            return a.dice_manipulateable > b.dice_manipulateable;
        }
        return a.cp_cost < b.cp_cost;
        });
    std::sort(cardManiporder.begin(), cardManiporder.end(), [](Card a, Card b) {
        if (a.dice_manipulateable == b.dice_manipulateable)
        {
            return a.cp_cost < b.cp_cost;
           
        }
        return a.dice_manipulateable > b.dice_manipulateable;
        });

    size_t max_manipulatable_cp = 0;
    size_t max_manipulatable_cards = 0;
    size_t dice_0CP = 0;
    //first test: TAW
    size_t tempcp = cp;
    size_t tempanz = use_max_cards;
    for (const auto& card : cardCPorder)
    {
        if (!card.can_use)
        {
            continue;
        }
        if (tempcp >= card.cp_cost && tempanz > 0)
        {
            tempcp -= card.cp_cost;
            max_manipulatable_cp += card.dice_manipulateable;
            tempanz--;
        }
    }

    tempcp = cp;
    tempanz = use_max_cards;
    for (const auto& card : cardManiporder)
    {
        if (!card.can_use)
        {
            continue;
        }
        if (tempcp >= card.cp_cost && tempanz > 0)
        {
            tempcp -= card.cp_cost;
            max_manipulatable_cards += card.dice_manipulateable;
            tempanz--;
        }
    }
    if (max_manipulatable_cp >= missing_count || max_manipulatable_cards >= missing_count)
    {
        return true;
    }
    return false;
}

void Simulator::get_all_positive_combs(const std::vector<DiceIdx>& mydiceanatomy, const std::vector<DiceIdx> & target, const std::vector<Card>& cards, size_t cp, size_t use_max_cards)
{
    generator_anatomy = mydiceanatomy;
    generator_target = target;
    bool print = false;
    get_all_possible_combinations(number_dice_);
    possible_list_with_cheat_dt.clear();

    size_t combo_store_size = cards.size()+2;

    while (combo_store_size > combo_store_.size())
    {
        std::vector<DiceThrow> a1;
        a1.resize(max_combo_store_size);
        combo_store_.push_back(a1);
    }

    bool tip_it = false;
    bool sixit = false;
    size_t sixit_cp = 1;
    size_t sixit_idx = 0;
    for (size_t i = 0; i < cards.size(); i++)
    {
        const auto& c = cards[i];
        if (c.card_id == 0)
        {
            sixit = true;
            sixit_idx = i;
            sixit_cp = c.cp_cost;
        }
        if (c.card_id == 2)
        {
            tip_it = true;
        }
    }

    int pos_count = 0;
    std::vector<DiceThrow> combs;
    
    bool need_sixit = false;
    //perform sixit test, only if a 6 or a 5 with tip it! is needed
    if (sixit && target[mydiceanatomy[5]] > 0)
    {
        need_sixit = true;
    }
    if (tip_it && sixit)
    {
        if (target[mydiceanatomy[4]] > 0)
        {
            need_sixit = true;
        }
    }
    size_t count_no_cheat = 0;
    bool straight = false;
    bool is_big = false;
    if (target[0] >= 7)
    {
        straight = true;
        need_sixit = sixit;
        //Small or big straight!
        is_big = target[0] == 8;
        for (size_t j = 0; j < possible_combs_.size(); j++)
        {
            DiceThrow& dt = possible_combs_[j];
            dt.success = false;
            if (is_straight(dt, is_big))
            {
                dt.success = true;
                count_no_cheat++;
            }
        }
    }
    else
    {
        //check all legal combos without helper-cards
        
        for (size_t j = 0; j < possible_combs_.size(); j++)
        {
            DiceThrow& dt = possible_combs_[j];
            dt.success = false;
            if (hit_target_ability(target, dt, mydiceanatomy))
            {
                dt.success = true;
                count_no_cheat++;
            }
        }
    }
    if (print)
    {
        std::cout << "hits no cards: " << count_no_cheat << "/" << possible_combs_.size() << std::endl;
    }
    //blow up list for cardhelpers to speed next step up:
    //generate all combinations (so we dont have to sort dice-throws)
    possible_combs_blow_up_.clear();
    possible_combs_blow_up_to_sorted_idx.clear();
    possible_combs_blow_up_.resize(possible_combs_blow_up_size_);
    possible_combs_blow_up_to_sorted_idx.resize(possible_combs_blow_up_size_);
    for (size_t i = 0; i < possible_combs_.size(); i++)
    {
        const DiceThrow& dt = possible_combs_[i];

        std::vector<DiceThrowDice> v = dt.getvec(number_dice_);
        DiceHash org_idx = get_hash(v);
        do
        {
            size_t idx = get_hash(v);
            possible_combs_blow_up_[idx] = dt;
            for (size_t i = 0; i < number_dice_; i++)
            {
                possible_combs_blow_up_[idx].rerollers[i] = v[i].reroll;
                possible_combs_blow_up_[idx].dice[i] = v[i].die;
                possible_combs_blow_up_to_sorted_idx[idx] = org_idx;

            }
        } while (std::next_permutation(v.begin(), v.end(),
            [](const auto& lhs, const auto& rhs)
            { return lhs.die < rhs.die; }));
    }
    
    //create list for all successfully combinations (with card-helpers)
    possible_list_with_cheat_dt = possible_combs_;
    possible_list_with_cheat_dt_succ_only.clear();
    int positive_count = 0;

    for (size_t i = 0; i < possible_list_with_cheat_dt.size(); i++)
    {
        if (print)
        {
            std::cout << i << "\r";
        }
        const DiceThrow& dt = possible_list_with_cheat_dt[i];
        if (!dt.success)
        {
            DiceThrow dtc = dt;
            for (size_t ii = 0; ii < number_dice_; ii++)
            {
                dtc.reroll_manipulation[ii] = 0;
            }
            card_store_[0] = cards;
            if (cards.size() > 0 && !need_sixit && sixit)
            {
                card_store_[0][sixit_idx].can_use = false;
            }
            int val = test_all_combis(dtc, cp, use_max_cards, 0);
            if (val >= 0)
            {
                //dt.success = true;
                possible_list_with_cheat_dt[i].success = true;
                //get real rerollers!
                //sort rerollers
                for (size_t ii = 0; ii < number_dice_ - 1; ii++)
                {
                    if (dt.dice[ii] == dt.dice[ii + 1] && this->target_reroller_start.reroll_manipulation[ii] == 1 && this->target_reroller_start.reroll_manipulation[ii + 1] == 0)
                    {
                        std::swap(this->target_reroller_start.reroll_manipulation[ii], this->target_reroller_start.reroll_manipulation[ii+1]);
                        std::swap(this->target_reroll_dice.rerollers[ii], this->target_reroll_dice.rerollers[ii+1]); 
                    }
                }

                for (size_t ii = 0; ii < number_dice_;ii++)
                {
                    possible_list_with_cheat_dt[i].rerollers[ii] = this->target_reroll_dice.rerollers[ii];
                    if (target_reroller_start.reroll_manipulation[ii] >= 1)
                    {
                        possible_list_with_cheat_dt[i].rerollers[ii] = true;
                    }
                }
                possible_list_with_cheat_dt[i].manipula = this->target_reroller_start.manipula;
                possible_list_with_cheat_dt[i].reroll_manipulation = this->target_reroller_start.reroll_manipulation;
                possible_list_with_cheat_dt_succ_only.push_back(possible_list_with_cheat_dt[i]);
                positive_count++;
                
            }
            else
            {
                possible_list_with_cheat_dt[i].success = false;
            }
        }
        else
        {
            positive_count++;
            bool added = false;
            if (straight && !is_big)
            {
                if (!is_big_straight(dt))
                {
                    possible_list_with_cheat_dt_succ_only.push_back(dt);
                    added = true;
                }
            }
            else
            {
                possible_list_with_cheat_dt_succ_only.push_back(dt);
                added = true;
            }
            if (added && sixit && cp >= sixit_cp && use_max_cards >= 1&& possible_list_with_cheat_dt_succ_only.back().dice[number_dice_-1] == 5)
            {
                possible_list_with_cheat_dt_succ_only.back().rerollers[number_dice_-1] = true;
            }
        }
    }
    if (print)
    {
        std::cout << "hits cards: " << positive_count << "/" << possible_combs_.size() << std::endl;
        for (const auto& t : possible_list_with_cheat_dt)
        {
            if (t.success == false)
            {
                std::cout << t.get_string() << std::endl;
            }
        }
    }

    blowUpData();
}

void Simulator::blowUpData()
{
    //generate reroll list for unsuccesfull throws:
    for (size_t i = 0; i < possible_list_with_cheat_dt.size(); i++)
    {
        DiceThrow& dt = possible_list_with_cheat_dt[i];
        if (dt.success)
        {
            continue;
        }
        //std::cout << "no success " << dt.get_string() << std::endl;
        update_rerollers(dt);
    }


    //generate all combinations (so we dont have to sort dice-throws)
    if (all_ability_combinations_dt.size() != possible_combs_blow_up_size_)
    {
        all_ability_combinations_dt.clear();
        all_ability_combinations_dt.resize(possible_combs_blow_up_size_);//possible_combs_blow_up_size_
        for (auto& dt : all_ability_combinations_dt)
        {
            dt.hash = get_hash(dt.getvec());
        }
    }
    else
    {
        for (auto& blubb : all_ability_combinations_dt)
        {
            blubb.reset();
        }
    }

    for (size_t i = 0; i < possible_list_with_cheat_dt.size(); i++)
    {
        const DiceThrow& dt = possible_list_with_cheat_dt[i];

        std::vector<DiceThrowDice> v = dt.getvec(number_dice_);
        do
        {
            size_t idx = get_hash(v);
            all_ability_combinations_dt[idx] = dt;
            all_ability_combinations_dt[idx].hash = idx;
            for (size_t i = 0; i < number_dice_; i++)
            {
                all_ability_combinations_dt[idx].rerollers[i] = v[i].reroll;
                all_ability_combinations_dt[idx].dice[i] = v[i].die;
            }
        } while (std::next_permutation(v.begin(), v.end(),
            [](const auto& lhs, const auto& rhs)
            { return lhs.die < rhs.die; }));

    }

    //generate a list with no cards:
    if (all_ability_combinations_no_cards_dt.size() != possible_combs_blow_up_size_)
    {
        all_ability_combinations_no_cards_dt.clear();
        all_ability_combinations_no_cards_dt.resize(possible_combs_blow_up_size_);
    }
;
    for (size_t i = 0; i < all_ability_combinations_dt.size(); i++)
    {
        all_ability_combinations_no_cards_dt[i] = all_ability_combinations_dt[i];
        if (all_ability_combinations_no_cards_dt[i].success && all_ability_combinations_no_cards_dt[i].manipula[0].card != 0)
        {
            all_ability_combinations_no_cards_dt[i].success = false;
        }
    }

    for (auto& dt : possible_list_with_cheat_dt)
    {
        dt.hash = get_hash(dt.getvec());
    }
    /*for (auto& dt : all_ability_combinations_dt)
    {
        dt.hash = get_hash(dt.getvec());
    }
    for (auto& dt : all_ability_combinations_no_cards_dt)
    {
        dt.hash = get_hash(dt.getvec());
    }*/
}

void Simulator::get_all_positive_combs_storage_lower_bound(const std::vector<DiceIdx>& mydiceanatomy, const std::vector<DiceIdx>& target, const std::vector<Card>& cards, size_t cp, size_t use_max_cards)
{

    bool tip_it = false;
    bool sixit = false;
    size_t sixit_idx = 0;
    size_t sixit_cp = 1;
    for (size_t i = 0; i < cards.size(); i++)
    {
        const auto& c = cards[i];
        if (c.card_id == 0)
        {
            sixit = true;
            sixit_idx = i;
            sixit_cp = c.cp_cost;
        }
        if (c.card_id == 2)
        {
            tip_it = true;
        }
    }

    int pos_count = 0;
    std::vector<DiceThrow> combs;

    bool need_sixit = false;
    //perform sixit test, only if a 6 or a 5 with tip it! is needed
    if (sixit && target[mydiceanatomy[5]] > 0)
    {
        need_sixit = true;
    }
    if (tip_it && sixit)
    {
        if (target[mydiceanatomy[4]] > 0)
        {
            need_sixit = true;
        }
    }
    size_t count_no_cheat = 0;
    bool straight = false;
    bool is_big = false;

    if (possible_calced_ == false)
    {
        possible_calced_ = true;
        if (target[0] >= 7)
        {
            straight = true;
            need_sixit = sixit;
            //Small or big straight!
            is_big = target[0] == 8;
            for (size_t j = 0; j < possible_combs_.size(); j++)
            {
                DiceThrow& dt = possible_combs_[j];
                dt.success = false;
                if (is_straight(dt, is_big))
                {
                    dt.success = true;
                    count_no_cheat++;
                }
            }
        }
        else
        {
            //check all legal combos without helper-cards

            for (size_t j = 0; j < possible_combs_.size(); j++)
            {
                DiceThrow& dt = possible_combs_[j];
                dt.success = false;
                if (hit_target_ability(target, dt, mydiceanatomy))
                {
                    dt.success = true;
                    count_no_cheat++;
                }
            }
        }
        //std::cout << "hits no cards: " << count_no_cheat << "/" << possible_combs_.size() << std::endl;
        //blow up list for cardhelpers to speed next step up:
        //generate all combinations (so we dont have to sort dice-throws)
        possible_combs_blow_up_.clear();
        possible_combs_blow_up_to_sorted_idx.clear();
        possible_combs_blow_up_.resize(possible_combs_blow_up_size_);
        possible_combs_blow_up_to_sorted_idx.resize(possible_combs_blow_up_size_);
        for (size_t i = 0; i < possible_combs_.size(); i++)
        {
            const DiceThrow& dt = possible_combs_[i];

            std::vector<DiceThrowDice> v = dt.getvec();
            DiceHash org_idx = get_hash(v);
            do
            {
                size_t idx = get_hash(v);
                possible_combs_blow_up_[idx] = dt;
                for (size_t i = 0; i < number_dice_; i++)
                {
                    possible_combs_blow_up_[idx].rerollers[i] = v[i].reroll;
                    possible_combs_blow_up_[idx].dice[i] = v[i].die;
                    possible_combs_blow_up_to_sorted_idx[idx] = org_idx;

                }
            } while (std::next_permutation(v.begin(), v.end(),
                [](const auto& lhs, const auto& rhs)
                { return lhs.die < rhs.die; }));
        }
    }

    std::size_t skipps = 0;
    std::size_t skipps2 = 0;
    std::size_t skipps3 = 0;
    //create list for all successfully combinations (with card-helpers)
    if (possible_list_with_cheat_dt.empty())
    {
        possible_list_with_cheat_dt = possible_combs_;
    }
    int positive_count = 0;
    // we calculated positive stuff with every card and cp in possible_list_with_cheat_dt_save
    //current run have more restrictions -> only test the ones that where true in possible_list_with_cheat_dt_save if they are still true
    for (size_t i = 0; i < possible_combs_.size(); i++)
    {
        const DiceThrow& dt = possible_combs_[i];
        if (possible_list_with_cheat_dt_save_last != nullptr && !possible_list_with_cheat_dt_save_last->empty())
        {
            //last one is with more restriction-> is it possible, this run is also possible
            const DiceThrow& lastdt = (*possible_list_with_cheat_dt_save_last)[i];
            if (lastdt.success)
            {
                skipps2++;
                possible_list_with_cheat_dt[i] = lastdt;
                continue;
            }
        }
        if (possible_list_with_cheat_dt_save_last2 != nullptr && !possible_list_with_cheat_dt_save_last2->empty())
        {
            //last one is with more restriction-> is it possible, this run is also possible
            const DiceThrow& lastdt = (*possible_list_with_cheat_dt_save_last2)[i];
            if (lastdt.success)
            {
                skipps3++;
                possible_list_with_cheat_dt[i] = lastdt;
                continue;
            }
        }

        //fast test
        if (!straight && !dt.success && !fast_test(target, dt, mydiceanatomy, cards, cp, use_max_cards))
        {
            skipps++;
            possible_list_with_cheat_dt[i] = dt;
            continue;
        }
        if (!dt.success )
        {
            //throw was manipulated in complete list -> only test this one, if it is still successfull
            DiceThrow dtc = dt;
            for (size_t ii = 0; ii < number_dice_; ii++)
            {
                dtc.reroll_manipulation[ii] = 0;
            }

            card_store_[0] = cards;
            if (cards.size() > 0 && !need_sixit && sixit)
            {
                card_store_[0][sixit_idx].can_use = false;
            }
            int val = test_all_combis(dtc, cp, use_max_cards, 0);
            if (val >= 0)
            {
                //dt.success = true;
                possible_list_with_cheat_dt[i].success = true;
                //get real rerollers!
                //sort rerollers
                for (size_t ii = 0; ii < number_dice_-1; ii++)
                {
                    if (dt.dice[ii] == dt.dice[ii + 1] && this->target_reroller_start.reroll_manipulation[ii] == 1 && this->target_reroller_start.reroll_manipulation[ii + 1] == 0)
                    {
                        std::swap(this->target_reroller_start.reroll_manipulation[ii], this->target_reroller_start.reroll_manipulation[ii + 1]);
                        std::swap(this->target_reroll_dice.rerollers[ii], this->target_reroll_dice.rerollers[ii + 1]);
                    }
                }

                for (size_t ii = 0; ii < number_dice_; ii++)
                {
                    possible_list_with_cheat_dt[i].rerollers[ii] = this->target_reroll_dice.rerollers[ii];
                    if (target_reroller_start.reroll_manipulation[ii] >= 1)
                    {
                        possible_list_with_cheat_dt[i].rerollers[ii] = true;
                    }
                }
                possible_list_with_cheat_dt[i].manipula = this->target_reroller_start.manipula;
                possible_list_with_cheat_dt[i].reroll_manipulation = this->target_reroller_start.reroll_manipulation;
                positive_count++;

            }
            else
            {
                possible_list_with_cheat_dt[i].success = false;
            }
        }
        else
        {
            possible_list_with_cheat_dt[i] = dt;
        }
    }

#ifdef __linux__
    std::cout << "skipped with lower: " << skipps2 << " skipped lower 2: " << skipps3 << " skipps test " << skipps << std::endl;
#endif
    //std::cout << "skipped with lower: " << skipps2 << " skipped lower 2: " << skipps3<< " skipps test "<< skipps << std::endl;
    //std::cout << "hits cards: " << positive_count << "/" << possible_combs_.size() << std::endl;
    possible_list_with_cheat_dt_succ_only.clear();
    for (size_t i = 0; i < possible_list_with_cheat_dt.size(); i++)
    {
        const DiceThrow& dt = possible_list_with_cheat_dt[i];
        if (!dt.success)
        {
            continue;
        }
        if (dt.manipula[0].card != 0)
        {
            //manipulated are allways added
            possible_list_with_cheat_dt_succ_only.push_back(dt);
            continue;
        }
        bool added = false;
        if (straight && !is_big)
        {
            if (!is_big_straight(dt))
            {
                possible_list_with_cheat_dt_succ_only.push_back(dt);
                added = true;
            }
        }
        else
        {
            possible_list_with_cheat_dt_succ_only.push_back(dt);
            added = true;
        }
        if (added && sixit && cp >= sixit_cp && use_max_cards >= 1 && possible_list_with_cheat_dt_succ_only.back().dice[number_dice_ - 1] == 5)
        {
            possible_list_with_cheat_dt_succ_only.back().rerollers[number_dice_ - 1] = true;
        }
    }

    // generate reroll list for unsuccesfull throws :
    for (size_t i = 0; i < possible_list_with_cheat_dt.size(); i++)
    {
        DiceThrow& dt = possible_list_with_cheat_dt[i];
        if (dt.success)
        {
            continue;
        }
        //std::cout << "no success " << dt.get_string() << std::endl;
        update_rerollers(dt);
    }
    //possible_list_with_cheat_dt_save_last = possible_list_with_cheat_dt;
}

void Simulator::choose(size_t offset, size_t k, const std::vector<DiceIdx>& combogen, std::vector<DiceIdx>& combination, size_t& counter)
{
    if (k == 0)
    {
        DiceIdx cs = (DiceIdx)combination.size();
        for (DiceIdx i = 0; i < cs; i++)
        {
            possible_combs_[counter].dice[i] = combination[i]-i;
        }
        counter++;
        return;
    }
        
    for (size_t i = offset; i <= combogen.size() - k; ++i) 
    {
        combination.push_back(combogen[i]);
        choose(i + 1, k - 1, combogen, combination, counter);
        combination.pop_back();
    }
}

void Simulator::get_all_possible_combinations(size_t number_dice)
{
    DiceIdx number_dice_tmp = (DiceIdx)std::min(number_dice, (size_t)5);

    possible_combs_.clear();
    size_t vec_size = 252; // = (s+n-1)! / (s!* (n-1)!) , s = number of dice, N = sides of dice
    if (number_dice_tmp == 4)
    {
        vec_size = 126;
    }
    if (number_dice_tmp == 3)
    {
        vec_size = 56;
    }
    if (number_dice_tmp == 2)
    {
        vec_size = 21;
    }
    if (number_dice_tmp == 1)
    {
        vec_size = 6;
    }
    possible_combs_.resize(vec_size);
    DiceIdx kombis = 6 + number_dice_tmp - 1;
    std::vector<DiceIdx> combogen{};
   
    for (DiceIdx i = 0; i < kombis; ++i)
    { 
        combogen.push_back(i);
    }
    size_t counter = 0;
    std::vector<DiceIdx> combination;
    choose(0, number_dice, combogen, combination, counter);
    return;
}

void Simulator::get_all_possible_combinations(size_t number_dice, size_t number_sides)
{
    DiceIdx number_dice_tmp = (DiceIdx)std::min(number_dice, (size_t)5);
    DiceIdx number_sides_tmp = (DiceIdx)std::min(number_sides, (size_t)6);
    possible_combs_.clear();
    size_t vec_size = Helpers::factorial(number_dice_tmp + number_sides_tmp - 1) / (Helpers::factorial(number_dice_tmp) * Helpers::factorial(number_sides_tmp - 1)) ; // = (s+n-1)! / (s!* (n-1)!) , s = number of dice, N = sides of dice
    possible_combs_.resize(vec_size);
    DiceIdx kombis = number_sides_tmp + number_dice_tmp - 1;
    std::vector<DiceIdx> combogen{};
    for (DiceIdx i = 0; i < kombis; ++i)
    {
        combogen.push_back(i);
    }
    size_t counter = 0;
    std::vector<DiceIdx> combination;
    choose(0, number_dice_tmp, combogen, combination, counter);
    return;
}

/*std::vector<bool> Simulator::test_montecarlo(const std::vector<DiceIdx>& org_dice, size_t roll_atemps, size_t rerolls)
{
    bool use_dice_anatomy = true;
    ORPX ORPX;
    ORPX.dice = org_dice;
    ORPX.roll_atempts = roll_atemps;
    ORPX.rerolls = rerolls;
    ORPX.all_ability_combinations_dt = &this->all_ability_combinations_dt;
    ORPX.all_combs = &this->all_combs;
    ORPX.allowed_rerolls_idx = &this->allowed_rerolls_idx;
    ORPX.allowed_single_rerolls_idx = &this->allowed_single_rerolls_idx;
    ORPX.allowed_rerolls_vecs = &this->allowed_rerolls_vecs;
    std::vector<DiceIdx> invered_anatomie = { 0,1,2,3,4,5 };
    ORPX.inverted_dice_anatomy = &invered_anatomie;

    if (use_dice_anatomy)
    {
        std::vector<DiceIdx> dice_anatomy = { 0,0,0,1,1,2 };
        generate_data_for_MCTS(dice_anatomy);
        for (size_t i = 0; i < org_dice.size(); i++)
        {
            ORPX.dice[i] = dice_anatomy[org_dice[i]];
        }
        ORPX.all_combs = &this->all_combs2;
        invered_anatomie = { 0,3,5,0,0,0 };
        ORPX.inverted_dice_anatomy = &invered_anatomie;//maps a A to a 0, a B to a 4 and a C to a 5! (+1 for real dice values ;) )
    }

    MCTSDUCTSmallFast::ComputeOptions options{};
    options.max_time = -1.0F;
    options.max_time = 0.1F;
    options.max_iterations = 100000;
    //options.param = 20.0;
    options.seed = gen_();
    options.verbose = false;


    BattleResult br = compute_move(ORPX, options, ductNodes_);
    std::vector<bool> bits = Helpers::getBitArray(br.move, number_dice_);
    return bits;
}*/