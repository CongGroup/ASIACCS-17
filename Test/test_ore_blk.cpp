/**
 * Copyright (c) 2016, David J. Wu, Kevin Lewi
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "../fastore/aes.h"
#include "../fastore/crypto.h"
#include "../fastore/ore_blk.h"
#include "errors.h"
#include "../fastore/OREHelper.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include <string.h>
#include <stdint.h>

using namespace std;

static int _error;
#define ERR_CHECK(x) if((_error = x) != ERROR_NONE) { return _error; }

// The ceiling function
#define CEIL(x, y) (((x) + (y) - 1) / (y))

static int N_TRIALS = 25000;


static void print128_num(__m128i var)
{
	int64_t *v64val = (int64_t*)&var;
	printf("%.16llx %.16llx\n", v64val[1], v64val[0]);
}

static int print_aes_key(AES_KEY key)
{
	int i, j, r = key.rounds;
	printf("round is %d\n", r);
	for (i = 0; i < r; ++i)
	{
		printf("round[%d] is :", i);
		print128_num(key.rd_key[i]);
		printf("\n");
	}
}

static int print_byte(byte* b, uint16_t n)
{
	for (int i = 0; i < n; ++i)
		printf("%.2X ", *(b + i));

	printf("\n");
}

static int bit_compare(const byte* byte1, const byte* byte2, uint16_t n)
{
	for (int i = 0; i < n; ++i)
	{
		if (*(byte1++) != *(byte2++))
			return *(byte1 - 1) > *(byte2 - 1) ? ORE_LARGE : ORE_SMALL;
	}
	return ORE_EQUAL;
}

uint64_t n1;
uint64_t n2;
uint8_t cmp_aim;

/**
 * Generates two random 32-bit integers and encrypts them (with an 8-bit block size).
 *
 * The encrypted integers are chosen randomly.
 *
 * @return 0 on success, -1 on failure, and an error if it occurred during the
 * encryption or comparison phase
 */
static int check_ore_blk() {

	N_TRIALS = 1;

	int nbits = 32;
	int block_len = 8;
	int cmp_len = 64;

	n1 = rand() % (((uint64_t)1) << nbits);
	n2 = rand() % (((uint64_t)1) << nbits);

	n1 = 0x7AA3E587;
	n2 = 0x68225FD2;

	print_byte((byte*)&n1, 4);
	print_byte((byte*)&n2, 4);

	ore_blk_params params;
	ERR_CHECK(init_ore_blk_params(params, nbits, block_len, cmp_len));

	ore_blk_secret_key sk;
	ERR_CHECK(ore_blk_setup(sk, params));

	ore_blk_ciphertext ctxt1;
	ERR_CHECK(init_ore_blk_ciphertext(ctxt1, params));

	ore_blk_ciphertext ctxt2;
	ERR_CHECK(init_ore_blk_ciphertext(ctxt2, params));

	printf("before enc 1\n");
	ERR_CHECK(ore_blk_encrypt_ui(ctxt1, sk, n1, "hello", 1));
	printf("before enc 2\n");
	ERR_CHECK(ore_blk_encrypt_ui(ctxt2, sk, n2, "hello", 1));

	int ret = 0;
	int res = 0;

	uint32_t nblocks = CEIL(nbits, block_len);

	uint32_t len_dst = sizeof(block) * nblocks;
	block* cmp_query = malloc(len_dst);
	uint8_t cmp_res = n1 == n2 ? ORE_EQUAL : (n1 > n2 ? ORE_LARGE : ORE_SMALL);
	uint8_t cmp_aim = rand() % 2;
	cmp_aim = cmp_aim == 0 ? ORE_SMALL : ORE_LARGE;
	cmp_aim = cmp_res;


	ore_blk_query((byte*)cmp_query, len_dst, sk, cmp_aim, n1, "hello");

	if (true)
	{
		int64_t *v64val = (int64_t*)cmp_query;
		printf("the query is : %.16llx %.16llx\n", v64val[1], v64val[0]);
	}

	printf("before compare\n");

	ERR_CHECK(ore_blk_compare(&res, cmp_query, ctxt1, ctxt2));

	printf("complete\n");


	if (cmp_aim == cmp_res)
	{
		if (res < 0) ret = -1;
		else printf("PASS in %d\n", res);
	}
	else
	{
		if (res > 0) ret = -1;
		else printf("PASS in %d\n", res);
	}

	ERR_CHECK(clear_ore_blk_ciphertext(ctxt1));
	ERR_CHECK(clear_ore_blk_ciphertext(ctxt2));

	return ret;
}

static int check_ore(int nb, int bl, int cl)
{
	int nbits = nb;
	int block_len = bl;
	int cmp_len = cl;

	n1 = rand() % (((uint64_t)1) << nbits);
	n2 = rand() % (((uint64_t)1) << nbits);

	ore_params params;
	ERR_CHECK(init_ore_params(params, nbits, block_len, cmp_len));

	ore_key sk;
	ERR_CHECK(ore_key_setup("Hello World!", sk, params));

	ore_query ctxt1;
	ERR_CHECK(ore_query_init(ctxt1, params));

	ore_index ctxt2;
	ERR_CHECK(ore_index_init(ctxt2, params));

	uint8_t cmp_res = n1 == n2 ? ORE_EQUAL : (n1 > n2 ? ORE_LARGE : ORE_SMALL);
	cmp_aim = rand() % 2;
	cmp_aim = cmp_aim == 0 ? ORE_SMALL : ORE_LARGE;

	char* cname = "age";
	//byte hash_name[SHA256_OUTPUT_BYTES];
	//ERR_CHECK(sha_256(hash_name, SHA256_OUTPUT_BYTES, cname, strlen(cname)));

	uint8_t rownum = 3;
	block block_rnum = MAKE_BLOCK(rownum, 0);

	ERR_CHECK(ore_query_setup(ctxt1, sk, n1, cname, cmp_aim));
	ERR_CHECK(ore_index_setup(ctxt2, sk, n2, cname, &block_rnum));

	int ret = 0;
	int res = -1;

	ERR_CHECK(ore_compare(&res, ctxt1, ctxt2, params, &block_rnum));

	if (cmp_aim == cmp_res)
	{
		if (res < 0) ret = -1;
	}
	else
	{
		if (res > 0) ret = -1;
	}

	ERR_CHECK(ore_query_cleanup(ctxt1, params));
	ERR_CHECK(ore_index_cleanup(ctxt2, params));

	return ret;
}

static int check_ui_num(int nb, int bl, int cl)
{
	int nbits = nb;
	int block_len = bl;
	int cmp_len = cl;

	uint64_t query_num = 0x7AA3E587;
	uint64_t index_num1 = 0x68225FD2;
	uint64_t index_num2 = 0x1234567890abcde;
	uint64_t index_num3 = 0x1244567890abcde;

	byte* query_ore;
	uint32_t len_query_ore;

	byte* index_ore1;
	uint32_t len_index_ore1;

	byte* index_ore2;
	uint32_t len_index_ore2;
	byte* index_ore3;
	uint32_t len_index_ore3;

	ore_params params;
	ERR_CHECK(init_ore_params(params, nbits, block_len, cmp_len));

	ore_key sk;
	ERR_CHECK(ore_key_setup("Hello World!", sk, params));

	char* cname = "age";

	uint8_t rownum = 5;

	//uint8_t cmp_res = ORE_EQUAL;
	uint8_t cmp_res = ORE_LARGE;
	//uint8_t cmp_res = ORE_SMALL;


	ore_query_ui(&query_ore, &len_query_ore, query_num, sk, cname, cmp_res);
	ore_index_ui(&index_ore1, &len_index_ore1, index_num1, sk, cname, rownum);
	ore_index_ui(&index_ore2, &len_index_ore2, index_num2, sk, cname, rownum);
	ore_index_ui(&index_ore3, &len_index_ore3, index_num3, sk, cname, rownum);


	int res1, res2, res3;
	ore_compare_ui(&res1, query_ore, len_query_ore, index_ore1, len_index_ore1, params, rownum);
	ore_compare_ui(&res2, query_ore, len_query_ore, index_ore2, len_index_ore2, params, rownum);
	ore_compare_ui(&res3, query_ore, len_query_ore, index_ore3, len_index_ore3, params, rownum);

	printf("each enc part is %d bits and divide to %d bits\n", nbits, block_len);
	printf("each compare result need %d bits to storage\n", cmp_len);
	printf("the client produce %d bytes data for query each part\n", ore_query_left_len(params));
	printf("the server storage %d bytes data for index each part\n", ore_index_right_len(params));
	printf("the aim query is to find %s\n", cmp_res == ORE_EQUAL ? "equal" : cmp_res == ORE_LARGE ? "large" : "small");
	printf("\n");
	printf("query  in num is :%.16llX\n", query_num);
	printf("index1 in num is :%.16llX\tres is %d\n", index_num1, res1);
	printf("index2 in num is :%.16llX\tres is %d\n", index_num2, res2);
	printf("index3 in num is :%.16llX\tres is %d\n", index_num3, res3);
	printf("\n");
	printf("query  in byte is :"); print_byte((byte*)&query_num, sizeof(query_num));
	printf("index1 in byte is :"); print_byte((byte*)&index_num1, sizeof(index_num1));
	printf("index2 in byte is :"); print_byte((byte*)&index_num2, sizeof(index_num2));
	printf("index3 in byte is :"); print_byte((byte*)&index_num3, sizeof(index_num3));

}


int main(int argc, char** argv) {


	OREHelper oreHelper;

	oreHelper.Init("123", DEF_BLOCK_SIZE_INBIT);

	char szLeft[DEF_ORELEFT_MAXSIZE];
	char szRight[DEF_ORERIGHT_MAXSIZE];

	oreHelper.CreateLeft(szLeft, sizeof(szLeft), 1, "XXX", "XX", 10);
	oreHelper.CreateRight(szRight, sizeof(szRight), "XXX", "XX", 0, 20);
	int iRes = oreHelper.CompareORE(szLeft, sizeof(szLeft), szRight, sizeof(szRight), 0);


	cout << "The compare 10 > 20, the result is" << iRes << endl;


	cout << "End of the first experiment." << endl;

	// srand((unsigned) time(NULL));
	srand(0x123456);
	//printf("running with seed: %lu\n", seed);
	printf("Testing ORE... \n");
	fflush(stdout);

	int nb = 32;
	int bl = 8;
	int cl = 64;

	if (argc == 4)
	{
		sscanf(argv[1], "%u", &nb);
		sscanf(argv[2], "%u", &bl);
		sscanf(argv[3], "%u", &cl);
	}
	else
	{
		int nb = 32;
		int bl = 8;
		int cl = 64;

	}


	check_ui_num(nb, bl, cl);

	for (int i = 0; i < N_TRIALS; i++) {
		if (check_ore(nb, bl, cl) != ERROR_NONE) {
			printf("FAIL IN:%d, ERROR:%d.\n", i, _error);
			printf("n1 is:%.16llX  n2 is:%.16llx  aim is:%d\n", n1, n2, cmp_aim);
			return -1;
		}
	}

	printf("PASS %d Times Random Num Test.\n", N_TRIALS);
	return 0;
}
