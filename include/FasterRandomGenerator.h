#pragma once
#include <stdint.h>
#include<random>
#include <iostream>
#include "Types.h"

class DiceRoller
{
public:

	DiceRoller() : DiceRoller(500000000ULL)
	{
	}

	DiceRoller(size_t amount_rands)
	{
		size_t number_randoms = amount_rands;
		//std::cout << number_randoms << std::endl;

		std::random_device rd;
		std::mt19937 gen(rd());
		uint64_t random32bit = gen();
		uint64_t random32bit2 = gen();
		//we dont want zeros
		while (random32bit == 0)
		{
			random32bit = gen();
		}
		while (random32bit2 == 0)
		{
			random32bit2 = gen();
		}

		s[0] = random32bit;
		s[1] = random32bit2;

		pcg32_fast_init(random32bit);

		buffer2_ = pcg32_fast();
		bp2_ = (char*)&buffer2_;
		bp2start_ = (char*)&buffer2_;
		bp2end_ = bp2start_ + sizeof buffer2_;

		state_ = random32bit;
		buffer_ = xorshift64star();
		bp_ = (char*)&buffer_;
		bpstart_ = (char*)&buffer_;
		bpend_ = bpstart_ + sizeof buffer_;

		max_numbers = number_randoms;
		generated_randoms.resize(max_numbers);

		for (size_t i = 0; i < max_numbers; i++)
		{
			generated_randoms[i] = getDiceRoll();
		}
		generate_pointer = &(generated_randoms[0]);
		generate_start_pointer = &(generated_randoms[0]);
		generate_end_pointer = &(generated_randoms[generated_randoms.size() - 1]);
	}



	DiceIdx getDiceRollFast() {
		if (generate_pointer == generate_end_pointer)
		{
			generate_pointer = generate_start_pointer;
		}
		return *generate_pointer++;
	}

	DiceIdx getDiceRoll(){
		uint64_t random32bit, multiresult;
		uint32_t leftover;
		uint32_t threshold;
		const uint32_t range = 6;
		random32bit = pcg32_fast();
		multiresult = random32bit * range;
		leftover = (uint32_t)multiresult;
		if (leftover < range) {
			threshold = (0U - range) % range;
			while (leftover < threshold) {
				random32bit = pcg32_fast();
				multiresult = random32bit * range;
				leftover = (uint32_t)multiresult;
				calcss++;
			}
		}
		return (DiceIdx)(multiresult >> 32); // [0, range)
	}

	DiceIdx getDiceRoll1() {
		uint64_t random32bit, multiresult;
		uint32_t leftover;
		uint32_t threshold;
		const uint32_t range = 6;
		const uint32_t bits = 32;
		random32bit = flip64_10();
		multiresult = random32bit * range;
		leftover = (uint32_t)multiresult;
		if (leftover < range) {
			threshold = (0U - range) % range;
			while (leftover < threshold) {
				random32bit = flip64_10();
				multiresult = random32bit * range;
				leftover = (uint32_t)multiresult;
				calcss++;
			}
		}
		return (DiceIdx)(multiresult >> bits); // [0, range)
	}

	DiceIdx getDiceRoll2() {
		DiceIdx val;
		do {
			val = flip32_3bit();
		} while (val > 5);
		return val;
	}

	DiceIdx getDiceRoll3() {
		DiceIdx val;
		do {
			val = flip64_8bit();
		} while (val > 5);
		return val;
	}


	uint64_t calcss = 0U;
	uint64_t flipps = 0U;
	uint64_t pcgs = 0U;

private:

	uint64_t xorshift64star() {
		uint64_t & x = state_;	/* state nicht mit 0 initialisieren */
		x ^= x >> 12; // a
		x ^= x << 25; // b
		x ^= x >> 27; // c
		return x * 0x2545F4914F6CDD1D;
	}

	uint64_t s[2]; // nicht komplett mit 0 initialisieren

	uint64_t xorshift128plus(void) {
		uint64_t x = s[0];
		uint64_t const y = s[1];
		s[0] = y;
		x ^= x << 23; // a
		s[1] = x ^ y ^ (x >> 17) ^ (y >> 26); // b, c
		return s[1] + y;
	}

	uint64_t mcg_state = 0xcafef00dd15ea5e5u;	// Must be odd
	uint64_t const multiplier = 6364136223846793005u;

	uint32_t pcg32_fast(void)
	{
		uint64_t x = mcg_state;
		unsigned count = (unsigned)(x >> 61);	// 61 = 64 - 3

		mcg_state = x * multiplier;
		x ^= x >> 22;
		return (uint32_t)(x >> (22 + count));	// 22 = 32 - 3 - 7
	}

	void pcg32_fast_init(uint64_t seed)
	{
		mcg_state = 2 * seed + 1;
		(void)pcg32_fast();
	}


	uint64_t flip_word = 0U;
	unsigned char flip_pos = 0U;
	unsigned char k = 64U;

	uint32_t flip64_10() {
		if (flip_pos == 0U) {
			flip_word = xorshift128plus();// xorshift128plus();
			flip_pos = k;
		}
		flip_pos -= 32U;
		return (flip_word >> flip_pos)& 4294967295;// 1023;
	}

	uint32_t flip64_8bit() {
		if (bp_ >= bpend_) {
			buffer_ = xorshift128plus();// xorshift128plus();
			bp_ = bpstart_;
		}
		return (*bp_++) & 7;// 1023;
	}
	
	uint32_t flip_word32 = 0U;
	unsigned char flip_pos32 = 0U;
	unsigned char k32 = 30U;

	uint32_t flip32_10() {
		if (flip_pos32 == 0U) {
			flip_word32 = pcg32_fast();
			flip_pos32 = k32;
		}
		flip_pos32 -= 16;
		return (flip_word32 >> flip_pos32)& 65535;// 1023;
	}

	unsigned char flip32_3bit() {
		if (flip_pos32 == 0U) {
			flip_word32 = pcg32_fast();
			flip_pos32 = 30U;
		}
		flip_pos32 -= 3U;
		return (flip_word32 >> flip_pos32) & 7;
	}

	unsigned char flip32_8bit() {
		if (bp2_ >= bp2end_) {
			buffer2_ = pcg32_fast();
			bp2_ = bp2start_;
		}
		return (*bp2_++) & 7;
	}



	uint64_t state_ = 123456789;
	char* bp_;
	char* bpstart_;
	char* bpend_;
	uint64_t buffer_{0};

	char* bp2_;
	char* bp2start_;
	char* bp2end_;
	uint32_t buffer2_{ 0 };

	std::vector<DiceIdx> generated_randoms{};
	DiceIdx* generate_pointer;
	DiceIdx* generate_start_pointer;
	DiceIdx* generate_end_pointer;
	size_t max_numbers{ 0U };

};