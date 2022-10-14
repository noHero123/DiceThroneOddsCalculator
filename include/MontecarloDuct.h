#pragma once

#include "Types.h"
#include "ORP.h"
#include "RandomGenerator.h"
#include <chrono>
#include <unordered_map>
#include <math.h>
#include <iostream>
#include <string>
#include <sstream>

#include <algorithm>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

namespace MCTSDUCTSmallFast
{
	template <typename T>
	std::string to_string_with_precision(const T a_value, const int n = 6)
	{
		std::ostringstream out;
		out.precision(n);
		out << std::fixed << a_value;
		return out.str();
	}

	struct ComputeOptions
	{
		int number_of_threads;
		int max_iterations;
		double max_time;
		double param;
		bool verbose;
		size_t seed;

		ComputeOptions() :
			number_of_threads(1),
			max_iterations(-1), // default is no max iterations
			max_time(10.0), // default is 10 secs
			verbose(true),
			param(std::sqrt(2.0)),
			seed(0)
		{ }

		std::string get_name()
		{
			return "DUCT_" + std::to_string(max_iterations) + "_" + std::to_string(max_time) + "_sqrt(" + std::to_string(param * param) + ")";
		}

		std::string get_name_short()
		{
			return "DUCT_" + to_string_with_precision(max_time, 0) + "_sqrt(" + to_string_with_precision(param * param, 1) + ")";
		}
	};

	using std::cerr;
	using std::endl;
	using std::vector;
	using std::size_t;

	static void check(bool expr, const char* message);
	static void assertion_failed(const char* expr, const char* file, int line);

	//
	// This class is used to build the game tree. The root is created by the users and
	// the rest of the tree is created by add_node.
	//
	template<typename State>
	class Node
	{
	public:
		typedef typename State::Move Move;

		Node()
		{};

		Node(const State& state);

		void setNode(const State& state);

		~Node();


		//select child and add child if not present in childs
		template<typename RandomEngine>
		size_t select_child_DUCT(State& state, RandomEngine* engine, const double& c, Node<State>& newNode, size_t& newNodeID)
		{
			Move bestMove = { State::no_move};
			size_t bestMove_idx = State::no_move;
			if (!state.is_chance_node)
			{
				double bestValue = std::numeric_limits<double>::lowest();
				double parentLog = std::log(std::max(1.0, static_cast<double>(this->totalVisitCount)));
				size_t numChildren = moves.size();
				std::vector<size_t> bestmoveids;
				bestmoveids.reserve(numChildren);
				for (size_t i = 0; i < numChildren; i++)
				{
					double exploit = (this->visits[i] == 0) ? 1.0 : this->wins[i] / this->visits[i];
					double explore = c * std::sqrt(parentLog / std::max(1.0, static_cast<double>(this->visits[i])));

					double ucb1Value = exploit + explore;
					if (ucb1Value > bestValue)
					{
						bestValue = ucb1Value;
						bestmoveids.clear();
						bestmoveids.push_back(i);
						this->lastSelectedMovesPerPlayer = i;
					}
					else if (ucb1Value == bestValue)
					{
						bestmoveids.push_back(i);
					}
				}
				size_t best_idx = bestmoveids[0];
				if (bestmoveids.size() > 1)
				{
					size_t rand_idx = (size_t)RandomGenerator::uniform_int_distribution(0, bestmoveids.size() - 1, *engine);
					best_idx = bestmoveids[rand_idx];
				}
				bestMove = moves[best_idx];
				bestMove_idx = best_idx;
				this->lastSelectedMovesPerPlayer = best_idx;
			}
			else
			{
				size_t bestMoveIDX = state.get_random_move_idx(engine);
				bestMove = bestMoveIDX;
				bestMove_idx = bestMoveIDX;
				this->lastSelectedMovesPerPlayer = bestMoveIDX;
			}
			Move best_move = bestMove;

			//has to be done here
			state.do_move(best_move, engine);

			/*auto it = children.find(best_move);
			if (it != children.end())
			{
				//found
				return it->second;
			}*/
			auto best_idx = children[bestMove_idx];
			if (best_idx != State::no_move)
			{
				//found
				return best_idx;
			}

			// not found, create new one!
			size_t newID = newNodeID;
			//newNode = Node<State>(state, best_move, this->ownid, newNodeID);
			newNode.setNode(state, best_move, this->ownid, newNodeID);
			newNode.tree_lvl = this->tree_lvl + 1;
			children[bestMove_idx] = newNodeID;
			newNodeID++;
			return newID;
		}

		void update(double result, size_t& max_win, size_t& max_visits)
		{
			/*if (isNewNode == false)
			{
				//leave node is NOT updated, its stats are saved in its parent
				if (State::no_move != lastSelectedMovesPerPlayer)
				{
					visits[lastSelectedMovesPerPlayer] += 1;
					wins[lastSelectedMovesPerPlayer] += result;
					totalVisitCount += 1;
				}

			}
			//next time we will search this node, it will be no leave
			isNewNode = false;*/

			//new update
			if (this->tree_lvl % 2 == 1)
			{
				//lvl 1,3,5,7
				//decision node
				if (isNewNode == true)
				{
					isNewNode = false;
					max_visits = 1;
					max_win = result;
					return;
				}
				real_visits[lastSelectedMovesPerPlayer]++;
				visits[lastSelectedMovesPerPlayer] = max_visits;
				wins[lastSelectedMovesPerPlayer] = max_win;

				max_win = 0;
				max_visits = 0;
				
				for (size_t i = 0; i < visits.size(); i++)
				{
					max_win += wins[i];
					max_visits += visits[i];
				}
				totalVisitCount++;

			}
			else
			{
				//lvl 0,2,4,6
				//random node will also be last nodes in tree
				//return maximum of childs
				if (this->is_leafnode)
				{
					own_visists++;
					own_wins += result;
					max_visits = own_visists;
					max_win = own_wins;
					isNewNode = false;
					return;
				}
				// it is not a leaf node, but first visit
				if (isNewNode == true)
				{
					max_visits = 1;
					max_win = result;
					isNewNode = false;
					return;
				}
				else
				{
					visits[lastSelectedMovesPerPlayer] = max_visits;
					wins[lastSelectedMovesPerPlayer] = max_win;
					real_visits[lastSelectedMovesPerPlayer]++;
					max_visits = 0;
					max_win = 0;
					float erg = 0.0;
					for (size_t i = 0; i < visits.size(); i++)
					{
						if (visits[i] > 0)
						{
							float tmp = wins[i] / (float)visits[i];
							if (tmp > erg)
							{
								max_win = wins[i];
								max_visits = visits[i];
								erg = tmp;
							}
							else
							{
								if (tmp == erg && max_visits < visits[i])
								{
									max_win = wins[i];
									max_visits = visits[i];
									erg = tmp;
								}
							}
						}
					}
					totalVisitCount++;
				}
				isNewNode = false;
			}
		}

		std::string to_string() const
		{
			std::stringstream sout;
			sout << "[" << " ";
			/*for (const auto& m : move)
			{
				sout << "M:" << m << " ";
			}*/

			//sout << "W/V: " << wins << "/" << visits << " "
			//	<< "U: " << moves.size() << "]\n";
			return sout.str();
		}

		std::string tree_to_string(int max_depth = 1000000, int indent = 0) const
		{
			if (indent >= max_depth) {
				return "";
			}

			std::string s = indent_string(indent) + to_string();
			/*for (auto child : children) {
				s += child.second->tree_to_string(max_depth, indent + 1);
			}*/
			return s;
		}

		Move move;
		size_t parent = 0;
		size_t ownid = 0;
		std::vector<double> wins;
		std::vector<size_t> visits;
		std::vector<size_t> real_visits;
		size_t totalVisitCount = 0;
		bool isNewNode = true;
		std::vector<DiceHash> moves;
		std::vector<size_t> children;
		//std::unordered_map<Move, size_t> children;
		//phmap::flat_hash_map<Move, size_t> children;
		size_t lastSelectedMovesPerPlayer = 0;
		size_t leafnode_counter = 0;
		bool is_win = false;
		size_t tree_lvl = 0;
		//
		bool is_leafnode = false;
		size_t own_visists = 0;
		size_t own_wins = 0;

	private:
		Node(const State& state, const Move& move_, size_t parent_, size_t ownID_) :
			move(move_),
			parent(parent_),
			ownid(ownID_),
			totalVisitCount(0),
			isNewNode{ true }
		{
			moves.clear();
			moves = state.get_movesPlayer();
			if (state.is_chance_node)
			{
				wins = std::vector<double>(moves[0], 0.0);
				visits = std::vector<size_t>(moves[0], 0);
				real_visits = std::vector<size_t>(moves[0], 0);
				children = std::vector<size_t>(moves[0], State::no_move);
			}
			else
			{
				wins = std::vector<double>(moves.size(), 0.0);
				visits = std::vector<size_t>(moves.size(), 0);
				real_visits = std::vector<size_t>(moves.size(), 0);
				children = std::vector<size_t>(moves.size(), State::no_move);
			}
			
			lastSelectedMovesPerPlayer = State::no_move;
		}

		void setNode(const State& state, const Move& move_, size_t parent_, size_t ownID_)
		{
			move = move_;
			parent = parent_;
			ownid = ownID_;
			totalVisitCount = 0;
			isNewNode = true;
			children.clear();

			moves.clear();
			moves = state.get_moves();

			if (state.is_chance_node)
			{
				wins = std::vector<double>(moves[0], 0.0);
				visits = std::vector<size_t>(moves[0], 0);
				real_visits = std::vector<size_t>(moves[0], 0);
				children = std::vector<size_t>(moves[0], State::no_move);
			}
			else
			{
				wins = std::vector<double>(moves.size(), 0.0);
				visits = std::vector<size_t>(moves.size(), 0);
				real_visits = std::vector<size_t>(moves.size(), 0);
				children = std::vector<size_t>(moves.size(), State::no_move);
			}

			lastSelectedMovesPerPlayer = State::no_move;
		}

		std::string indent_string(int indent) const;

		//Node(const Node&);
		//Node& operator = (const Node&);
	};

	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////


	template<typename State>
	void Node<State>::setNode(const State& state)
	{
		move = State::no_move;
		parent = 42000000;
		ownid = 0;
		totalVisitCount = 0;
		isNewNode = false;
		children.clear();

		moves = state.get_moves();

		if (state.is_chance_node)
		{
			wins = std::vector<double>(moves[0], 0.0);
			visits = std::vector<size_t>(moves[0], 0);
			real_visits = std::vector<size_t>(moves[0], 0);
			children = std::vector<size_t>(moves[0], State::no_move);
		}
		else
		{
			wins = std::vector<double>(moves.size(), 0.0);
			visits = std::vector<size_t>(moves.size(), 0);
			real_visits = std::vector<size_t>(moves.size(), 0);
			children = std::vector<size_t>(moves.size(), State::no_move);
		}
		
		lastSelectedMovesPerPlayer = State::no_move;
	}

	template<typename State>
	Node<State>::~Node()
	{
	}

	template<typename State>
	std::string Node<State>::indent_string(int indent) const
	{
		std::string s = "";
		for (int i = 1; i <= indent; ++i) {
			s += "| ";
		}
		return s;
	}

	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////

	template<typename State>
	void print_state(const Node<State>& node)
	{
		std::string tabs = "";
		for (size_t i = 0; i < node.tree_lvl; i++)
		{
			tabs += "\t";
		}
		std::cout<< tabs << "move " << node.move << " parent " << node.parent << " tree lvl " << node.tree_lvl << std::endl;
		for (size_t i = 0; i < node.visits.size(); i++)
		{
			std::cout<< tabs << "win " << node.wins[i] << " vis " << node.visits[i] << std::endl;
		}
	}

	template<typename State>
	float get_prob(
		const std::vector<Node<State>>& nodes, const Node<State>& best_children)
	{
		//print_state(best_children);
		size_t e_0_leaf_wins = 0;
		float total = 0;
		float child_ergs = 0.0F;
		for (size_t val : best_children.visits)
		{
			total += (float)val;
		}
		for (size_t i = 0;i < best_children.children.size(); i++)
		{
			size_t cs_idx = best_children.children[i];
			if (cs_idx == State::no_move)
			{
				continue;
			}
			//random childs
			const auto& css = nodes[cs_idx];
			if (css.is_leafnode)
			{
				e_0_leaf_wins += css.own_wins;
				continue;
			}
			else
			{
				size_t max_targt = 0;
				float max_visi = 0;
				for (size_t j = 0; j < css.visits.size(); j++)
				{
					float temp = css.wins[j] / (float)css.visits[j];
					if (temp > max_visi)
					{
						max_targt = j;
						max_visi = temp;
					}
				}
				if (css.children[max_targt] != State::no_move)
				{
					const auto& best_children2 = nodes[css.children[max_targt]];
					//print_state(best_children2);
					float fac3 = (best_children.real_visits[i] / (float)best_children.totalVisitCount);
					float child_probs = get_prob(nodes, best_children2);
					child_ergs += child_probs * fac3;
				}
				
			}
			
		}

		//float e0_prob = e_0_leaf_wins / (total);
		float e0_prob = (best_children.totalVisitCount == 0) ? 0.0F : (e_0_leaf_wins / ((float)best_children.totalVisitCount));
		//std::cout << e0_prob << " " << child_ergs<<std::endl;
		float prob = e0_prob + child_ergs;
		return prob;
	}

	template<typename State>
	void printer(
		const std::vector<Node<State>>& nodes, size_t target)
	{
		if (target == 4)
		{
			std::cout << "test" << std::endl;
		}
		const auto& best_children = nodes[nodes[0].children[target]];
		size_t e_0_leaf_wins = 0;
		size_t e_0_leaf_visits = 0;
		size_t leaf_wins = 0;
		size_t leaf_visits = 0;
		for (size_t val : best_children.visits)
		{
			e_0_leaf_visits += val;
		}
		for (auto cs_idx : best_children.children)
		{
			//random childs
			const auto& css = nodes[cs_idx];
			if (css.is_leafnode)
			{
				e_0_leaf_wins += css.own_wins;
				continue;
			}
			size_t best_strategy = 10000000;
			double best_strategy_score{ 0.0F };
			for (size_t itr = 0; itr < css.visits.size(); itr++)
			{
				double v = css.visits[itr];
				double w = css.wins[itr];
				// Expected success rate assuming a uniform prior (Beta(1, 1)).
				// https://en.wikipedia.org/wiki/Beta_distribution
				double expected_success_rate = (w + 1) / (v + 2);
				if (v > 0 && expected_success_rate > best_strategy_score) {
					auto it = css.children[css.moves[itr]];
					if (it != State::no_move)
					{
						//found
						best_strategy = it;
						best_strategy_score = expected_success_rate;
					}

				}

			}
			if (best_strategy < 10000000)
			{
				const auto& best_strat_node = nodes[best_strategy];
				for (const auto& it : best_strat_node.children)
				{
					const auto& best_strat_child = nodes[it];
					leaf_wins += (best_strat_child.is_win) ? best_strat_child.leafnode_counter : 0;
					leaf_visits += best_strat_child.leafnode_counter;
				}

			}
		}



		float e0_prob = e_0_leaf_wins / ((float)e_0_leaf_visits);
		float e1_prob = leaf_wins / ((float)leaf_visits);
		float prob = e0_prob + (1.0F - e0_prob) * e1_prob;
		std::cout << "child " << target << " has prob " << prob << "%" << std::endl;
	}

	template<typename State>
	Node<State> compute_tree(const State root_state,
		const ComputeOptions options,
		std::mt19937::result_type initial_seed,
		std::vector<Node<State>>& nodes)
	{
		typedef std::chrono::high_resolution_clock Time;
		std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
		std::chrono::steady_clock::time_point print_time = start_time;
		std::mt19937 random_engine(options.seed);

		nodes[0].setNode(root_state);
		//auto root = std::unique_ptr<Node<State>>(new Node<State>(root_state));
		State state;
		//statistics:
		size_t calls = 0;
		size_t calls2 = 0;
		size_t terminals = 0;
		size_t addChilds = 0;
		size_t ducts = 0;
		size_t domoves = 0;

		//memory management
		size_t nodeidx = 0;
		size_t nextfreenode = 1;

		double c = options.param;
		int iter = 1;
		for (iter = 1; iter <= options.max_iterations || options.max_iterations < 0; ++iter)
		{
			if (iter >= options.max_iterations - 100)
			{
				nodeidx = 0;
			}
			nodeidx = 0;
			//auto node = root.get();
			state = root_state;
			//std::vector<size_t> played_moves;
			// Select a path through the tree to a leaf node and expand!
			while (true)
			{
				if (!state.has_moves())
				{
					//terminal state
					//calls++;
					break;
				}

				nodeidx = nodes[nodeidx].select_child_DUCT(state, &random_engine, c, nodes[nextfreenode], nextfreenode);
				ducts++;
				domoves++;

				if (nodes[nodeidx].isNewNode)
				{
					// We've expanded a new node, time for playout!
					//calls++;
					break;
				}
			}

			// We now play randomly until the game ends.
			if (state.has_moves())
			{
				calls++;
			}
			else
			{
				terminals++;
			}
			if (nodes[nodeidx].is_leafnode == false && !state.has_moves())
			{
				nodes[nodeidx].is_leafnode = true;
				//nodes[nodeidx].is_win = (state.get_result() == 1.0) ? true : false;
			}
			while (state.has_moves())
			{
				state.do_random_move(&random_engine);
				calls2++;
			}

			// We have now reached a final state. Backpropagate the result
			// up the tree to the root node.
			double value = state.get_result();
			if (nodes[nodeidx].tree_lvl >= 4 && nodes[nodeidx].own_visists >= 10)
			{
				size_t wins = 0;
			}
			
			size_t wins = 0;
			size_t visi = 0;
			while (nodeidx != 42000000)
			{
				nodes[nodeidx].update(value, wins, visi);
				nodeidx = nodes[nodeidx].parent;
			}

			if (options.verbose || options.max_time > 0.0) {
				std::chrono::steady_clock::time_point time = std::chrono::steady_clock::now();
				typedef std::chrono::duration<double> fsec;
				fsec fs = time - print_time;
				fsec fs2 = time - start_time;

				if (options.verbose && (fs.count() >= 1.0 || iter == options.max_iterations)) {
					std::cerr << iter << " games played (" << double(iter) / fs2.count() << " / second). time elapsed " << fs2.count() << endl;
					print_time = time;
					std::cout << "calls " << calls << std::endl;
					std::cout << "calls2 " << calls2 << std::endl;
					std::cout << "childs " << addChilds << std::endl;
					std::cout << "ucts " << ducts << std::endl;
					std::cout << "moves " << domoves << std::endl;
					calls = 0;
					calls2 = 0;
					addChilds = 0;
					ducts = 0;
					domoves = 0;
				}

				if ((options.max_time > 0.0 && (fs2).count() >= options.max_time))
				{
					if (options.verbose)
						std::cerr << iter << " games played (" << double(iter) / fs2.count() << " / second). time elapsed " << fs2.count() << endl;
					break;
				}
			}
		}
		//std::cout << "calls " << calls << std::endl;
		//std::cout << "calls2 " << calls2 << std::endl;
		//std::cout << "terminal nodes " << terminals << std::endl;
		nodes[0].parent = iter;
		return nodes[0];
	}

	template<typename State>
	BattleResult compute_move(const State root_state,
		const MCTSDUCTSmallFast::ComputeOptions options,
		std::vector<MCTSDUCTSmallFast::Node<ORP>>& nodes)
	{
		using namespace std;


		auto moves = root_state.get_moves();
		//attest(moves.size() > 0);
		if (moves.size() == 1) {
			BattleResult br2;
			br2.move = moves[0];
			return br2;
		}



		// Start all jobs to compute trees.
		vector<future<Node<State>>> root_futures;
		ComputeOptions job_options = options;
		job_options.verbose = false;
		for (int t = 0; t < options.number_of_threads; ++t)
		{
			auto func = [t, &root_state, &job_options, &nodes]() -> Node<State>
			{
				return compute_tree(root_state, job_options, 1012411 * t + 12515, nodes);
			};

			root_futures.push_back(std::async(std::launch::async, func));
		}

		// Collect the results.
		vector<Node<State>> roots;
		for (int t = 0; t < options.number_of_threads; ++t)
		{
			roots.push_back(std::move(root_futures[t].get()));
		}

		typename State::Move best_joint_move = typename State::Move();
		typename State::Move best_move0 = typename State::Move();
		typename State::Move best_move1 = typename State::Move();
		best_joint_move = 0;
		// Merge the children of all root nodes.
		BattleResult br;

		map<typename State::Move, size_t> visits;
		map<typename State::Move, double> wins;
		long long games_played = 0;
		for (int t = 0; t < options.number_of_threads; ++t) {
			auto root = roots[t];
			games_played += root.totalVisitCount;
			br.iterations = root.parent;// :)
			for (int i = 0; i < root.moves.size(); i++)
			{
				typename State::Move m = root.moves[i];
				double wis = root.wins[i];
				size_t vis = root.visits[i];
				visits[m] += vis;
				wins[m] += wis;
			}
		}

		// Find the node with the highest score.
		double best_score = -1;
		typename State::Move best_move = typename State::Move();
		for (auto itr : visits) {
			auto move = itr.first;
			double v = static_cast<double>(itr.second);
			double w = wins[move];
			// Expected success rate assuming a uniform prior (Beta(1, 1)).
			// https://en.wikipedia.org/wiki/Beta_distribution
			double expected_success_rate = (w + 1) / (v + 2);
			if (expected_success_rate > best_score) {
				best_move = move;
				best_score = expected_success_rate;
			}

		}

		if (options.verbose)
		{
			float tot_games_play = 0.0;
			for (size_t i = 0; i < wins.size(); i++)
			{
				tot_games_play += visits[i];
			}
			for (size_t i = 0; i < wins.size(); i++)
			{
				auto winss = wins[i]+1;
				auto viss = visits[i]+2;
				cerr << i << " " << 100.0 * visits[i] / tot_games_play << "% visits)"
					<< " (" << 100.0 * winss / viss << "% wins)" << endl;
			}
			auto best_wins = wins[best_move];
			auto best_visits = visits[best_move];
			cerr << "----" << endl;
			cerr << "Best: " << best_move
				<< " (" << 100.0 * best_visits / double(games_played) << "% visits)"
				<< " (" << 100.0 * best_wins / best_visits << "% wins)" << endl;
			//printer(nodes, 15);
			float ergo = get_prob(nodes, nodes[nodes[0].children[best_move-1]]);
			std::cout << "real prob for " << best_move << " is " << ergo << "%" << std::endl;
			/*for (size_t iii = 1; iii < 32; iii++)
			{
				printe(nodes, iii);
			}*/
		}
		float ergo = get_prob(nodes, nodes[nodes[0].children[best_move - 1]]);
		std::cout << "real prob for " << best_move << " is " << ergo*100.0F << "%" << std::endl;
		auto best_wins = wins[best_move];
		auto best_visits = visits[best_move];
		best_move0 = best_move;
		br.proz = 100.0 * best_wins / best_visits;


		best_joint_move = best_move;
		//root_state.printAttacks(best_joint_move);
		br.move = best_joint_move;
		return br;
	}

	/////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////


	static void check(bool expr, const char* message)
	{
		if (!expr) {
			throw std::invalid_argument(message);
		}
	}

	static void assertion_failed(const char* expr, const char* file_cstr, int line)
	{
		using namespace std;

		// Extract the file name only.
		string file(file_cstr);
		auto pos = file.find_last_of("/\\");
		if (pos == string::npos) {
			pos = 0;
		}
		file = file.substr(pos + 1);  // Returns empty string if pos + 1 == length.

		stringstream sout;
		sout << "Assertion failed: " << expr << " in " << file << ":" << line << ".";
		throw runtime_error(sout.str().c_str());
	}

}
