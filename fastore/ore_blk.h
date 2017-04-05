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

 /**
  * This is an implementation of the order-revealing encryption scheme from the
  * paper "Order-Revealing Encryption: New Constructions, Applications, and
  * Lower Bounds" (http://eprint.iacr.org/2016/612.pdf) published in CCS, 2016.
  * For options on instantiating the random oracle, refer to the "USE_AES_RO"
  * flag in flags.h. This implementation currently only supports encrypting
  * 64-bit values. We may extend the functionality in a future implementation
  * to support encrypting values from larger plaintext spaces.
  *
  * In this version of the code, we only encode the less-than-or-equal-to
  * relation in the right ciphertexts (this means that for using a block size
  * of n bits, the length of each right ciphertext block is exactly 2^n bits).
  * However, this means that the comparison algorithm requires both the left
  * and the right ciphertext components. It is easy to modify the comparison to
  * only require the left ciphertext of one and the right ciphertext of the
  * other by encoding the "less-than/equal/greater-than" relation in the right
  * ciphertexts. This increases the ciphertext size by a factor of 1.6-2x
  * (depending on the implementation). See Remark 3.1 in the paper for a more
  * complete discussion.
  */

#ifndef __ORE_BLK_H__
#define __ORE_BLK_H__


#include <stdbool.h>
#include "crypto.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ORE_SMALL (1)
#define ORE_EQUAL (2)
#define ORE_LARGE (3)

	// the public parameters for the encryption scheme, used to compare ciphertexts
	typedef struct {
		bool initialized;    // whether or not these parameters have been initialized
		uint32_t nbits;      // the number of bits in the plaintext elements
		uint32_t block_len;  // the number of bits in each block of the plaintext
		uint32_t cmp_len;    // the number of bits in compare result
	} ore_blk_params[1];

	// the secret key for the encryption scheme
	typedef struct {
		bool initialized;         // whether or not the secret key has been initalized
		ORE_AES_KEY         prf_key;  // key for the PRF (for deriving the keys from each prefix)
		ORE_AES_KEY         prp_key;  // key for the PRP (for permuting the slots within a block)
		ore_blk_params  params;
	} ore_blk_secret_key[1];

	// the ciphertexts of the encryption scheme
	typedef struct {
		bool initialized;            // whether or not the ciphertext has been initialized
		byte*           comp_left;   // the left ciphertext
		byte*           comp_right;  // the right ciphertext
		ore_blk_params  params;
	} ore_blk_ciphertext[1];

	/**
	 * Initializes an ore_blk_params type by setting its parameters, the number of bits
	 * and block length.
	 *
	 * @param params     The params to initialize
	 * @param nbits      The number of bits of an input to the encryption scheme
	 * @param block_len  The length (in bits) of each block in the plaintext space
	 * @param cmp_len    The compare result length we used in ore_right.
	 *
	 * @return ERROR_NONE on success, ERROR_PARAMS_INVALID if the parameter settings
	 *         are invalid.
	 */
	int init_ore_blk_params(ore_blk_params params, uint32_t nbits, uint32_t block_len, uint32_t cmp_len);

	/**
	 * Initializes the secret key of the ORE scheme (with respect to a specific set of
	 * public parameters).
	 *
	 * The public parameters passed to this function should have been previous
	 * initialized (via a call to init_ore_blk_params).
	 *
	 * @param sk     The secret key to initialize
	 * @param params The public parameters for the ORE scheme (should be initialized)
	 *
	 * @return ERROR_NONE on success, and the corresponding error code on failure
	 *         (see errors.h for the full list of possible error codes)
	 */
	int ore_blk_setup(ore_blk_secret_key sk, ore_blk_params params);

	/**
	 * Cleans up any memory used by the secret key.
	 *
	 * @param sk The secret key to clean up
	 *
	 * @return ERROR_NONE on success
	 */
	int ore_blk_cleanup(ore_blk_secret_key sk);

	/**
	 * Encrypts a message (up to 64 bits) using the ORE scheme.
	 *
	 * Both the secret key and the ciphertext must be initialized (by a call to
	 * ore_blk_setup and init_ore_blk_ciphertext) before calling this function.
	 *
	 * @param ctxt The ciphertext to store the encryption (which must have been
	 *             initialized)
	 * @param sk   The secret key (which must have been initialized)
	 * @param msg  The input (represented as an unsigned 64-bit integer)
	 * @param cname  The input Column name
	 * @param ccum   The input row No.
	 *
	 * @return ERROR_NONE on success, and a corresponding error code on failure
	 *         (see errors.h for the full list of possible error codes)
	 */
	int ore_blk_encrypt_ui(ore_blk_ciphertext ctxt, ore_blk_secret_key sk, uint64_t msg, char* cname, uint32_t rnum);

	int ore_blk_query(byte* dst, uint32_t dstlen, ore_blk_secret_key sk, uint8_t aim, uint64_t msg, char* cname);

	/**
	 * Performs the comparison of two ciphertexts to determine the ordering of their
	 * underlying plaintexts.
	 *
	 * The two ciphertexts must have been initialized (by a call to
	 * init_ore_blk_ciphertext) before calling this function.
	 *
	 * @param result_p A pointer containing the result of the comparison, which is 1
	 *                 if ctxt1 encrypts a plaintext greater than ctxt2, -1 if ctxt1
	 *                 encrypts a plaintext less than ctxt2, and 0 if they encrypt
	 *                 equal plaintexts.
	 * @param ctxt1    The first ciphertext
	 * @param ctxt2    The second ciphertext
	 *
	 * @return ERROR_NONE on success, and a corresponding error code on failure
	 *         (see errors.h for the full list of possible error codes)
	 */
	int ore_blk_compare(int* result_p, block* cmp_query, ore_blk_ciphertext ctxt1, ore_blk_ciphertext ctxt2);

	/**
	 * Initializes an ORE ciphertext with the parameters described by params.
	 *
	 * This function assumes that init_ore_blk_params has been called on the parameters.
	 *
	 * @param ctxt   The ciphertext to initialize
	 * @param params The parameters to initialize the ciphertext with
	 *
	 * @return ERROR_NONE on success and a corresponding error code on failure
	 *         (see errors.h for the full list of possible error codes)
	 */
	int init_ore_blk_ciphertext(ore_blk_ciphertext ctxt, ore_blk_params params);

	/**
	 * Frees up any memory associated with the targeted ciphertext.
	 *
	 * @param ctxt The ciphertext to clear
	 *
	 * @return ERROR_NONE on success and a corresponding error code on failure
	 *         (see errors.h for the full list of possible error codes)
	 */
	int clear_ore_blk_ciphertext(ore_blk_ciphertext ctxt);

	/**
	 * Computes the size of an ORE ciphertext.
	 *
	 * @param params The parameters for the ORE scheme
	 *
	 * @return the size of an ORE ciphertext
	 */
	int ore_blk_ciphertext_size(ore_blk_params params);


	typedef struct {
		bool initialized;    // whether or not these parameters have been initialized
		uint32_t nbits;      // the number of bits in the plaintext elements,max is 64
		uint32_t block_len;  // the number of bits in each block of the plaintext, max is 16
		uint32_t cmp_len;    // the number of bits in compare result,max is 64
	} ore_params[1];
	typedef struct {
		bool initialized;         // whether or not the secret key has been initalized
		char			org_key[128];  // the original key from user input
		ORE_AES_KEY         prf_key;  // key for the PRF (for deriving the keys from each prefix)
		ORE_AES_KEY         prp_key;  // key for the PRP (for permuting the slots within a block)
		ORE_AES_KEY         prg_key;  // key for the PRG (for permuting the slots within a block)
		uint64_t		prg_counter;// current prg msg
		ore_params  params;
	} ore_key[1];
	typedef struct {
		bool initialized;
		byte*           comp_left;   // the left ciphertext
	} ore_query[1];

	typedef struct {
		bool initialized;
		byte*           comp_right;  // the right ciphertext
	} ore_index[1];


   /**
	* Initialize the ore function parameters like init_ore_blk_params 
	*
	* @param params     The params to initialize
	* @param nbits      The number of bits of an input to the encryption scheme
	* @param block_len  The length (in bits) of each block in the plaintext space
	* @param cmp_len    The compare result length we used in ore_right.
	*
	* @return ERROR_NONE on success and a corresponding error code on failure
	*         (see errors.h for the full list of possible error codes)
	*/
	int init_ore_params(ore_params params, uint32_t nbits, uint32_t block_len, uint32_t cmp_len);

    /*	
	 * A group of function to control ore_key
	 */
	int ore_key_setup(char* input, ore_key sk, ore_params params);
	int ore_key_cleanup(ore_key sk);

	/*
	* A group of function to control ore_query
	*/
	int ore_query_left_len(ore_params params);
	int ore_query_init(ore_query ctxt, ore_params params);
	int ore_query_setup(ore_query ctxt, ore_key sk, uint64_t msg, char* cname, uint8_t aim);
	int ore_query_cleanup(ore_query ctxt, ore_params params);

	/*
	* A group of function to control ore_index
	*/
	int ore_index_right_len(ore_params params);
	int ore_index_init(ore_index ctxt, ore_params params);
	int ore_index_setup(ore_index ctxt, ore_key sk, uint64_t msg, char* cname, uint32_t rnum);
	int ore_index_cleanup(ore_index ctxt, ore_params params);

	/**
	* Performs the comparison of ore_query and ore_index to determine the ordering whether
	* correspond the query need.
	*
	* @param result_p A pointer containing the result of the comparison, which is 1
	*                 if compare success, -1 if compare failed, and 0 if they encrypt
	*                 equal plaintexts.
	* @param ctxt1    The compare query include the comparison.
	* @param ctxt2    The index to be compared.
	* @param params   The ore_parames will be used in this function.
	* @param rnum     The input row No.
	*
	* @return ERROR_NONE on success, and a corresponding error code on failure
	*         (see errors.h for the full list of possible error codes)
	*/
	int ore_compare(int* result_p, ore_query ctxt1, ore_index ctxt2, ore_params params, uint32_t rnum);

	
	int ore_query_ui(byte** dst, uint32_t* dstlen, uint64_t src, ore_key sk, char* cname, uint8_t aim);
	int ore_index_ui(byte** dst, uint32_t* dstlen, uint64_t src, ore_key sk, char* cname, uint32_t rnum);
	int ore_compare_ui(int* result_p, byte* query, uint32_t querylen, byte* index, uint32_t indexlen,ore_params params, uint32_t rnum);

#ifdef __cplusplus
}
#endif

#endif