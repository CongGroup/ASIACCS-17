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

#include "ore_blk.h"
#include "errors.h"
#include "flags.h"

#include <gmp.h>
#include <malloc.h>
#include <string.h>
#include <assert.h>


 // Helper macro for error handling
static int _error_flag;
#define ERR_CHECK(x) if((_error_flag = x) != ERROR_NONE) { return _error_flag; }

// The ceiling function
#define CEIL(x, y) (((x) + (y) - 1) / (y))

// The ORE encryption scheme is randomized, so the randomness used for
// encryption is derived from an internal PRG (implemented using AES in
// counter mode). This is for demo purposes only. For concrete applications,
// it may be preferable to use a different source for the encryption
// randomness. Note that this implementation is NOT thread-safe, and is
// intended primarily for demo purposes.
static bool _prg_initialized = false;
static uint64_t _counter = 0;
static ORE_AES_KEY _prg_key;

// The maximum supported block length in bite (chosen primarily for efficiency
// reasons).
static const int MAX_BLOCK_LEN = 16;
static const int MAX_CMP_LEN = 64;

/**
 * Checks if two ore_blk_params structs are equal
 *
 * @param params1 The first set of parameters
 * @param params2 The second set of parameters
 *
 * @return 1 if they are equal, 0 otherwise
 */
static bool _eq_ore_blk_params(ore_blk_params params1, ore_blk_params params2) {
	return (params1->initialized == params2->initialized) &&
		(params1->nbits == params2->nbits) &&
		(params1->block_len == params2->block_len);
}

/**
 * Checks if an ore_param struct is valid by ensuring that the
 * block length is non-zero and less than the maximum supported block
 * length (MAX_BLOCK_LEN).
 *
 * @param params The parameters to check
 *
 * @return true if the parameters are valid, false otherwise
 */
static bool _is_valid_params(ore_blk_params params) {
	if (!params->initialized) {
		return false;
	}
	else if (params->block_len == 0 || params->block_len > MAX_BLOCK_LEN ||
		params->cmp_len > MAX_CMP_LEN) {
		return false;
	}

	return true;
}

/**
 * Seeds the internal PRG (used to derive the encryption randomness). The PRG
 * uses a AES in counter mode. To seed the PRG, a fresh AES key is sampled and
 * the counter is initialized to 0.
 */
static void _seed_prg() {
	generate_aes_key(&_prg_key);
	_counter = 0;
	_prg_initialized = true;
}

/**
 * Gets the next block (16 byts) of output from the PRG. The next block is
 * computed by invoking AES on the current value of the counter. The value of
 * the counter is updated afterwards. This PRG implementation is NOT thread-
 * safe, so this library should not be used as is in a multi-threaded
 * execution environment.
 *
 * @param out  Buffer that will hold the next block of the PRG
 *
 * @return ERROR_NONE on success and a corresponding error code on failure
 *         (see errors.h for the full list of possible error codes)
 */
static int _next_prg_block(block* out) {
	return aes_eval(out, &_prg_key, MAKE_BLOCK(0, _counter++));
}

int init_ore_blk_params(ore_blk_params params, uint32_t nbits, uint32_t block_len, uint32_t cmp_len) {
	params->initialized = true;
	params->nbits = nbits;
	params->block_len = block_len;
	params->cmp_len = cmp_len;

	if (!_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	return ERROR_NONE;
}

int ore_blk_setup(ore_blk_secret_key sk, ore_blk_params params) {
	if (!_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	ERR_CHECK(generate_aes_key(&sk->prf_key));
	ERR_CHECK(generate_aes_key(&sk->prp_key));

	memcpy(sk->params, params, sizeof(ore_blk_params));

	sk->initialized = true;

	return ERROR_NONE;
}

int ore_blk_cleanup(ore_blk_secret_key sk) {
	memset(sk, 0, sizeof(ore_blk_secret_key));

	return ERROR_NONE;
}


#ifdef USE_AES_RO
/**
 * Evaluates a keyed hash function on a particular value (used to construct
 * the right ciphertexts). This function uses AES to instantiate the random
 * oracle: H(k, x) = AES(x, k). This is sound, for instance, if we model
 * AES as an ideal cipher.
 *
 * @param out  An output buffer to store the output of the hash function
 *             (assumed to be of the correct size)
 * @param outlen  the length of output buffer, the max size if 64 bit
 * @param key  The key to the keyed hash function
 * @param val  The value to evaluate the hash function on
 *
 * @return ERROR_NONE on success and a corresponding error code on failure
 *         (see errors.h for the full list of possible error codes)
 */
static inline int _eval_keyed_hash_aes_ro(uint64_t* out, uint32_t outlen, const block key, const block val) {

	ORE_AES_KEY aes_key;
	setup_aes_key(&aes_key, (byte*)&val, sizeof(block));


	block output;
	ERR_CHECK(aes_eval(&output, &aes_key, key));

	uint64_t outmask = ((outlen == 64) ? 0xffffffffffffffff : (((uint64_t)1 << outlen) - 1));


	*out = (*(uint64_t*)&output) & outmask;

	return ERROR_NONE;
}

/**
 * Evaluates a keyed hash function on a single value using multiple keys
 * (used to construct the right ciphertexts). This function uses AES to
 * instantiate the particular random oracle: H(k, x) = lsb(AES(x, k)). Batch
 * evaluation of AES can be pipelined (assuming support for the AES-NI
 * instruction set), becuase the same value is used (x is reused across all
 * of the invocations).
 *
 * @param out      An output buffer to store the vector of outputs of the
 *                 hash function (assumed to be of the correct size)
 * @param outlen  the length of output buffer, the max size if 64 bit
 * @param nblocks  The number of hash function evaluations
 * @param keys     The vector of keys (of length nblocks) used to apply the
 *                 hash function
 * @param val      The value to evaluate the hash functions on
 *
 * @return ERROR_NONE on success and a corresponding error code on failure
 *         (see errors.h for the full list of possible error codes)
 */
static inline int _eval_keyed_hash_batch_aes_ro(uint64_t* out, uint32_t outlen, uint32_t nblocks,
	const block* keys, const block val) {

	ORE_AES_KEY aes_key;
	setup_aes_key(&aes_key, (byte*)&val, sizeof(block));


	block* outputs = malloc(nblocks * sizeof(block));
	ERR_CHECK(aes_eval_blocks(outputs, nblocks, &aes_key, keys));

	uint64_t outmask = ((outlen == 64) ? 0xffffffffffffffff : (((uint64_t)1 << outlen) - 1));


	for (int i = 0; i < nblocks; i++) {
		out[i] = (*(uint64_t*)&outputs[i]) & outmask;
	}

	free(outputs);

	return ERROR_NONE;
}
#else
/**
 * Evaluates a keyed hash function on a particular value (used to construct
 * the right ciphertexts). This function uses SHA-256 to instantiate the
 * random oracle: H(k, x) = lsb(SHA-256(k || x)).
 *
 * @param out  An output buffer to store the output of the hash functiond
 *             (assumed to be of the correct size)
 * @param outlen  the length of output buffer, the max size if 64 bit
 * @param key  The key to the keyed hash function
 * @param val  The value to evaluate the hash function on
 *
 * @return ERROR_NONE on success and a corresponding error code on failure
 *         (see errors.h for the full list of possible error codes)
 */
static inline int _eval_keyed_hash_sha256(uint64_t* out, uint32_t outlen, const block key, const block val) {

	static byte inputbuf[AES_OUTPUT_BYTES + sizeof(block)];

	memcpy(inputbuf, &key, sizeof(block));
	memcpy(inputbuf + sizeof(block), &val, sizeof(block));


	byte dst[SHA256_OUTPUT_BYTES];
	ERR_CHECK(sha_256(dst, sizeof(dst), inputbuf, sizeof(inputbuf)));

	uint64_t outmask = ((outlen == 64) ? 0xffffffffffffffff : ((uint64_t)1 << outlen - 1));


	*out = *((uint64_t*)&dst) & outmask;

	return ERROR_NONE;
}
#endif

/**
 * Evaluates a keyed hash function on a particular value (used to construct
 * the right ciphertexts). The precise details are described in Section 3.1 of
 * the paper (https://eprint.iacr.org/2016/612.pdf). In the security analysis,
 * the hash function is modeled as a random oracle. We give two instantiations
 * based on different choices of the random oracle. The first is based on AES
 * (provably secure if we model AES as an ideal cipher) and the second is
 * based on the more traditional SHA-256. The choice of hash function can be
 * controlled by setting/unsetting the USE_AES_RO flag in flags.h.
 *
 * @param out  An output buffer to store the output of the hash function
 *             (assumed to be of the correct size)
 * @param outlen  the length of output buffer, the max size if 64 bit
 * @param key  The key to the keyed hash function
 * @param val  The value to evaluate the hash function on
 *
 * @return ERROR_NONE on success and a corresponding error code on failure
 *         (see errors.h for the full list of possible error codes)
 */

static inline int _eval_keyed_hash(uint64_t* out, uint32_t outlen, const block key, const block val) {
#ifdef USE_AES_RO
	return _eval_keyed_hash_aes_ro(out, outlen, key, val);
#else
	return _eval_keyed_hash_sha256(out, outlen, key, val);
#endif
}

/**
 * Evaluates a keyed hash function using multiple keys on the same block.
 * Using the AES-based random oracle instantiation together with AES-NI,
 * the batch version is faster (by pipelining the evaluations of the AES
 * round functions). With SHA-256, we just invoke the keyed hash separately
 * using each of the keys.
 *
 * @param out      An output buffer to store the vector of outputs of the hash
 *                 function (assumed to be of the correct size)
 * @param outlen  the length of output buffer, the max size if 64 bit
 * @param nblocks  The number of hash function evaluations
 * @param keys     The vector of keys (of length nblocks) used to apply the hash function
 * @param val      The value to evaluate the hash functions on
 *
 * @return ERROR_NONE on success and a corresponding error code on failure
 *         (see errors.h for the full list of possible error codes)
 */

static inline int _eval_keyed_hash_batch(uint64_t* out, uint32_t outlen, uint32_t nblocks,
	const block* keys, const block val) {
#ifdef USE_AES_RO
	return _eval_keyed_hash_batch_aes_ro(out, outlen, nblocks, keys, val);
#else
	for (int i = 0; i < nblocks; i++) {
		ERR_CHECK(_eval_keyed_hash_sha256(&out[i], outlen, keys[i], val));
	}
	return ERROR_NONE;
#endif
}

/**
  * Encrypts a single block of the ORE ciphertext using the small-domain ORE.
  * This algorithm is described in Section 3 of the paper
  * (https://eprint.iacr.org/2016/612.pdf). The output ciphertext consists of
  * a left ciphertext and a right ciphertext with the property that the right
  * ciphertext provides semantic security.
  *
  * @param comp_left   A buffer to hold the left ciphertext component
  * @param comp_right  A buffer to hold the right ciphertext component
  * @param sk          The secret key for the ORE scheme
  * @param nonce       The nonce used for encryption (should be unique for
  *                    each ciphertext)
  * @param block_ind   The index of the current block (used to construct a PRF
  *                    on variable-length inputs)
  * @param prefix      The prefix of the current block (used for key
  *                    derivation for encrypting the current block)
  * @param val         The value of the block to be encrypted (at the current
  *                    index)
  * @param cname  The input Column name
  * @param ccum   The input row No.
  *
  * @return ERROR_NONE on success and a corresponding error code on failure
  *         (see errors.h for the full list of possible error codes)
  */

static int _ore_blk_encrypt_block(byte* comp_left, byte* comp_right, ore_blk_secret_key sk,
	block nonce, uint64_t block_ind, uint64_t prefix, uint64_t val,
	char* cname, uint32_t rnum) {

	uint32_t block_len = sk->params->block_len;
	uint32_t cmp_len = CEIL(sk->params->cmp_len, 8);
	uint32_t nslots = 1 << block_len;


	block prp_key_buf;
	ERR_CHECK(aes_eval(&prp_key_buf, &sk->prp_key, MAKE_BLOCK(block_ind, prefix)));


	ORE_AES_KEY prp_key;
	ERR_CHECK(setup_aes_key(&prp_key, (byte*)&prp_key_buf, sizeof(block)));

	// construct left ciphertext (PRP evaluation on the value)
	uint64_t pix = 0;
	ERR_CHECK(prp_eval((byte*)&pix, &prp_key, (byte*)&val, block_len));


	block key;

	uint64_t prefix_shifted = prefix << block_len;

	ERR_CHECK(aes_eval(&key, &sk->prf_key, MAKE_BLOCK(block_ind, prefix_shifted | pix)));

	memcpy(comp_left, &key, sizeof(block));
	memcpy(comp_left + sizeof(block), &pix, CEIL(block_len, 8));

	// construct right ciphertext (encryption of comparison vector under keys
	// derived from PRF)
	block* inputs = malloc(sizeof(block) * nslots);
	block* keys = malloc(sizeof(block) * nslots);

	for (int i = 0; i < nslots; i++) {
		inputs[i] = MAKE_BLOCK(block_ind, prefix_shifted | i);
	}

	ERR_CHECK(aes_eval_blocks(keys, nslots, &sk->prf_key, inputs));


	uint64_t* pi_inv = malloc(sizeof(uint64_t) * nslots);
	ERR_CHECK(prp_inv_eval_all(pi_inv, &prp_key, block_len));

	uint64_t* r = malloc(sizeof(uint64_t) * nslots);
	ERR_CHECK(_eval_keyed_hash_batch(r, sk->params->cmp_len, nslots, keys, nonce));

	byte hash_name[SHA256_OUTPUT_BYTES];
	ERR_CHECK(sha_256(hash_name, SHA256_OUTPUT_BYTES, cname, strlen(cname)));

	ORE_AES_KEY cmp_key_name;
	ORE_AES_KEY cmp_key_row;
	block block_rnum = MAKE_BLOCK(0, rnum);
	assert(AES_KEY_BYTES < SHA256_OUTPUT_BYTES);
	ERR_CHECK(setup_aes_key(&cmp_key_name, (byte*)hash_name, AES_KEY_BYTES));
	ERR_CHECK(setup_aes_key(&cmp_key_row, (byte*)&block_rnum, sizeof(block)));

	block* inputs_cmp = malloc(sizeof(block) * nslots);

	for (int i = 0; i < nslots; i++) {
		uint8_t v = (pi_inv[i] == val) ? ORE_EQUAL : ((pi_inv[i] < val) ? ORE_SMALL : ORE_LARGE);

		inputs_cmp[i] = MAKE_BLOCK(*(uint64_t*)hash_name, i << 1 | v);
		block fcmp;
		aes_eval(&fcmp, &cmp_key_name, inputs_cmp[i]);
		aes_eval(inputs_cmp + i, &cmp_key_row, fcmp);
		uint64_t g_mask = ((sk->params->cmp_len == 64) ? 0xffffffffffffffff : (((uint64_t)1 << sk->params->cmp_len) - 1));
		uint64_t g_num = *(uint64_t*)(inputs_cmp + i);

		r[i] ^= g_num&g_mask;

		memcpy(comp_right + cmp_len*i, &r[i], sizeof(r[i]));
	}

	free(inputs);
	free(keys);
	free(pi_inv);
	free(r);
	free(inputs_cmp);

	return ERROR_NONE;
}

int ore_blk_encrypt_ui(ore_blk_ciphertext ctxt, ore_blk_secret_key sk, uint64_t msg,
	char* cname, uint32_t rnum) {
	if (!sk->initialized) {
		return ERROR_SK_NOT_INITIALIZED;
	}

	if (!ctxt->initialized) {
		return ERROR_CTXT_NOT_INITIALIZED;
	}

	if (!_eq_ore_blk_params(ctxt->params, sk->params)) {
		return ERROR_PARAMS_MISMATCH;
	}

	if (!_is_valid_params(ctxt->params)) {
		return ERROR_PARAMS_INVALID;
	}

	if (!_prg_initialized) {
		_seed_prg();
	}

	uint32_t nbits = ctxt->params->nbits;
	uint32_t block_len = ctxt->params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);
	uint32_t cmp_len = CEIL(sk->params->cmp_len, 8);

	uint32_t block_mask = (1 << block_len) - 1;
	block_mask <<= (block_len * (nblocks - 1));

	// choose nonce
	block nonce;
	ERR_CHECK(_next_prg_block(&nonce));
	memcpy(ctxt->comp_right, &nonce, sizeof(block));

	// set up left and right pointers for each block
	byte* comp_left = ctxt->comp_left;
	byte* comp_right = ctxt->comp_right + sizeof(block);

	uint32_t len_left_block = AES_BLOCK_LEN + CEIL(nbits, 8);
	uint32_t len_right_block = cmp_len*nslots;

	// process each block
	uint64_t prefix = 0;
	for (int i = 0; i < nblocks; i++) {
		uint32_t cur_block = msg & block_mask;
		cur_block >>= block_len * (nblocks - i - 1);

		block_mask >>= block_len;

		ERR_CHECK(_ore_blk_encrypt_block(comp_left, comp_right, sk, nonce, i, prefix, cur_block, cname, rnum));

		// update prefix
		prefix <<= block_len;
		prefix |= cur_block;

		// update block pointers
		comp_left += len_left_block;
		comp_right += len_right_block;
	}

	return ERROR_NONE;
}

int ore_blk_query(byte* dst, uint32_t dstlen, ore_blk_secret_key sk, uint8_t aim, uint64_t msg, char* cname)
{
	if (!sk->initialized) {
		return ERROR_SK_NOT_INITIALIZED;
	}

	uint32_t nbits = sk->params->nbits;
	uint32_t block_len = sk->params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);

	uint32_t block_mask = (1 << block_len) - 1;
	block_mask <<= (block_len * (nblocks - 1));

	block* comp = (block*)dst;
	uint64_t prefix = 0;
	for (int i = 0; i < nblocks; i++) {
		uint32_t cur_block = msg & block_mask;
		cur_block >>= block_len * (nblocks - i - 1);

		block_mask >>= block_len;

		block prp_key_buf;
		ERR_CHECK(aes_eval(&prp_key_buf, &sk->prp_key, MAKE_BLOCK(i, prefix)));

		ORE_AES_KEY prp_key;
		ERR_CHECK(setup_aes_key(&prp_key, (byte*)&prp_key_buf, sizeof(block)));

		uint64_t pix = 0;
		ERR_CHECK(prp_eval((byte*)&pix, &prp_key, (byte*)&cur_block, block_len));

		byte hash_name[SHA256_OUTPUT_BYTES];
		ERR_CHECK(sha_256(hash_name, SHA256_OUTPUT_BYTES, cname, strlen(cname)));

		ORE_AES_KEY cmp_key_name;
		ERR_CHECK(setup_aes_key(&cmp_key_name, (byte*)hash_name, AES_KEY_BYTES));

		block fcmp = MAKE_BLOCK(*(uint64_t*)hash_name, pix << 1 | aim);
		aes_eval(comp + i, &cmp_key_name, fcmp);


		prefix <<= block_len;
		prefix |= cur_block;
	}
	return ERROR_NONE;
}

int ore_blk_compare(int* result_p, block* cmp_query, ore_blk_ciphertext ctxt1, ore_blk_ciphertext ctxt2) {
	if (!ctxt1->initialized || !ctxt2->initialized) {
		return ERROR_CTXT_NOT_INITIALIZED;
	}

	if (!_eq_ore_blk_params(ctxt1->params, ctxt2->params)) {
		return ERROR_PARAMS_MISMATCH;
	}

	if (!_is_valid_params(ctxt1->params)) {
		return ERROR_PARAMS_INVALID;
	}

	uint32_t nbits = ctxt1->params->nbits;
	uint32_t block_len = ctxt1->params->block_len;
	uint32_t cmp_len = CEIL(ctxt1->params->cmp_len, 8);
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);

	block nonce = *(block*)ctxt2->comp_right;

	uint32_t offset_left = 0;
	uint32_t offset_right = sizeof(block);

	uint32_t len_left_block = AES_BLOCK_LEN + CEIL(nbits, 8);
	uint32_t len_right_block = nslots * cmp_len;

	ORE_AES_KEY cmp_key_row;
	block row_num = MAKE_BLOCK(0, 1);
	ERR_CHECK(setup_aes_key(&cmp_key_row, (byte*)&row_num, sizeof(block)));

	// compare each block
	bool is_equal = true;
	for (int i = 0; i < nblocks; i++) {


		uint64_t index = 0;
		memcpy(&index, ctxt1->comp_left + offset_left + AES_KEY_BYTES, CEIL(block_len, 8));

		block key_block;
		memcpy(&key_block, ctxt1->comp_left + offset_left, sizeof(block));

		uint64_t r;
		ERR_CHECK(_eval_keyed_hash(&r, ctxt1->params->cmp_len, key_block, nonce));

		block gres;
		aes_eval(&gres, &cmp_key_row, *(cmp_query + i));
		uint64_t g_mask = ((ctxt1->params->cmp_len == 64) ? 0xffffffffffffffff : (((uint64_t)1 << ctxt1->params->cmp_len) - 1));
		uint64_t g_num = *(uint64_t*)(ctxt2->comp_right + offset_right + index*cmp_len);
		uint64_t gcmp = *(uint64_t*)(&gres) &g_mask;

		r ^= g_num & g_mask;


		if (gcmp == r) {
			*result_p = i + 1;
			return ERROR_NONE;
		}

		offset_left += len_left_block;
		offset_right += len_right_block;
	}

	*result_p = 0;
	return ERROR_NONE;
}

/**
 * Computes the length of a left ciphertext.
 *
 * @param params The parameters for the ORE scheme
 *
 * @return the length of a left ciphertext for the specific choice of
 *         parameters
 */
static inline int _ore_blk_ciphertext_len_left(ore_blk_params params) {
	uint32_t nblocks = CEIL(params->nbits, params->block_len);

	return (AES_BLOCK_LEN + CEIL(params->nbits, 8)) * nblocks;
}

/**
 * Computes the length of a right ciphertext.
 *
 * @param params The parameters for the ORE scheme
 *
 * @return the length of a right ciphertext for the specific choice of
 *         parameters
 */
static inline int _ore_blk_ciphertext_len_right(ore_blk_params params) {
	uint32_t block_len = params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(params->nbits, block_len);
	uint32_t cmp_len = CEIL(params->cmp_len, 8);

	return AES_BLOCK_LEN + cmp_len* nslots * nblocks;
}

int init_ore_blk_ciphertext(ore_blk_ciphertext ctxt, ore_blk_params params) {
	if (!_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	if (ctxt == NULL || params == NULL) {
		return ERROR_NULL_POINTER;
	}

	ctxt->comp_left = malloc(_ore_blk_ciphertext_len_left(params));
	if (ctxt->comp_left == NULL) {
		return ERROR_MEMORY_ALLOCATION;
	}

	ctxt->comp_right = malloc(_ore_blk_ciphertext_len_right(params));
	if (ctxt->comp_right == NULL) {
		return ERROR_MEMORY_ALLOCATION;
	}

	memcpy(ctxt->params, params, sizeof(ore_blk_params));

	ctxt->initialized = true;

	return ERROR_NONE;
}

int clear_ore_blk_ciphertext(ore_blk_ciphertext ctxt) {
	if (ctxt == NULL) {
		return ERROR_NONE;
	}

	if (!_is_valid_params(ctxt->params)) {
		return ERROR_PARAMS_INVALID;
	}

	memset(ctxt->comp_left, 0, _ore_blk_ciphertext_len_left(ctxt->params));
	free(ctxt->comp_left);

	memset(ctxt->comp_right, 0, _ore_blk_ciphertext_len_right(ctxt->params));
	free(ctxt->comp_right);

	memset(ctxt, 0, sizeof(ore_blk_ciphertext));

	return ERROR_NONE;
}

int ore_blk_ciphertext_size(ore_blk_params params) {
	return _ore_blk_ciphertext_len_left(params) + _ore_blk_ciphertext_len_right(params);
}


static bool ore_is_valid_params(ore_params params)
{
	if (!params->initialized) {
		return false;
	}
	else if (params->block_len == 0 || params->block_len > MAX_BLOCK_LEN ||
		params->cmp_len > MAX_CMP_LEN) {
		return false;
	}

	return true;
}

int init_ore_params(ore_params params, uint32_t nbits, uint32_t block_len, uint32_t cmp_len)
{
	params->initialized = true;
	params->nbits = nbits;
	params->block_len = block_len;
	params->cmp_len = cmp_len;

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	return ERROR_NONE;
}

int ore_key_setup(char* input, ore_key sk, ore_params params)
{
	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}
	block left;
	block right;
	block middle;

	memset(sk->org_key, 0, sizeof(sk->org_key));
	strcpy(sk->org_key, input);

	byte* res256 = malloc(SHA256_OUTPUT_BYTES);
	sha_256(res256, SHA256_OUTPUT_BYTES, (byte*)sk->org_key, sizeof(sk->org_key));

	memcpy(&left, res256, AES_BLOCK_LEN);
	memcpy(&right, res256 + AES_BLOCK_LEN, AES_BLOCK_LEN);
	memcpy(&right, res256 + AES_BLOCK_LEN / 2, AES_BLOCK_LEN);

	ERR_CHECK(setup_aes_key(&sk->prf_key, (byte*)&left, AES_BLOCK_LEN));
	ERR_CHECK(setup_aes_key(&sk->prp_key, (byte*)&right, AES_BLOCK_LEN));
	ERR_CHECK(setup_aes_key(&sk->prg_key, (byte*)&middle, AES_BLOCK_LEN));

	sk->prg_counter = 0;

	memcpy(sk->params, params, sizeof(ore_params));

	sk->initialized = true;

	return ERROR_NONE;
}

int ore_key_cleanup(ore_key sk)
{
	memset(sk, 0, sizeof(ore_key));

	return ERROR_NONE;
}

static int next_prg_block(block* out, ore_key sk) {
	return aes_eval(out, &sk->prg_key, MAKE_BLOCK(0, sk->prg_counter++));
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

static int ore_setup_helper(ore_index ctxt, ore_query cque, ore_key sk, uint64_t msg, char* cname, uint32_t rnum, uint8_t aim)
{
	bool isquery = ctxt == 0 ? true : false;

	if (!sk->initialized)
		return ERROR_SK_NOT_INITIALIZED;

	if (isquery)
	{
		if (cque == NULL)
			return ERROR_NULL_POINTER;

		if (!cque->initialized)
			return ERROR_CTXT_NOT_INITIALIZED;
	}
	else
	{
		if (ctxt == NULL)
			return ERROR_NULL_POINTER;

		if (!ctxt->initialized)
			return ERROR_CTXT_NOT_INITIALIZED;
	}

	if (!ore_is_valid_params(sk->params))
		return ERROR_PARAMS_INVALID;

	ore_params params;
	memcpy(params, sk->params, sizeof(ore_params));

	uint32_t nbits = params->nbits;
	uint32_t block_len = params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);
	uint32_t cmp_len = CEIL(params->cmp_len, 8);

	byte* comp_left = 0;
	byte* comp_right = 0;

	if (isquery)
	{
		comp_left = cque->comp_left;
	}
	else
	{
		comp_right = ctxt->comp_right + AES_BLOCK_LEN;
	}

	uint32_t len_left_block = AES_BLOCK_LEN * 2 + CEIL(nbits, 8);
	uint32_t len_right_block = cmp_len*nslots;

	uint64_t block_mask = (1 << block_len) - 1;
	block_mask <<= (block_len * (nblocks - 1));

	block nonce;
	if (!isquery)
	{
		ERR_CHECK(_next_prg_block(&nonce));
		memcpy(ctxt->comp_right, &nonce, AES_BLOCK_LEN);
	}

	byte hash_name[SHA256_OUTPUT_BYTES];
	ERR_CHECK(sha_256(hash_name, SHA256_OUTPUT_BYTES, cname, strlen(cname)));

	block block_rnum = MAKE_BLOCK(rnum, 0);

	uint64_t prefix = 0;
	for (int i = 0; i < nblocks; i++) {
		uint64_t cur_block = msg & block_mask;
		cur_block >>= block_len * (nblocks - i - 1);

		block_mask >>= block_len;

		block prp_key_buf;
		ERR_CHECK(aes_eval(&prp_key_buf, &sk->prp_key, MAKE_BLOCK(i, prefix)));
		ORE_AES_KEY prp_key;
		ERR_CHECK(setup_aes_key(&prp_key, (byte*)&prp_key_buf, sizeof(block)));

		if (isquery)
		{
			uint64_t pix = 0;
			ERR_CHECK(prp_eval((byte*)&pix, &prp_key, (byte*)&cur_block, block_len));
			block key;
			uint64_t prefix_shifted = prefix << block_len;
			ERR_CHECK(aes_eval(&key, &sk->prf_key, MAKE_BLOCK(i, prefix_shifted | pix)));

			ORE_AES_KEY cmp_key_name;
			ERR_CHECK(setup_aes_key(&cmp_key_name, (byte*)hash_name, AES_KEY_BYTES));

			block cmp_buf = MAKE_BLOCK(*(uint64_t*)hash_name, pix << (8 * sizeof(aim)) | aim);
			block fcmp;
			ERR_CHECK(aes_eval(&fcmp, &cmp_key_name, cmp_buf));

			memcpy(comp_left, &key, AES_BLOCK_LEN);
			memcpy(comp_left + sizeof(block), &fcmp, AES_BLOCK_LEN);
			memcpy(comp_left + sizeof(block) * 2, &pix, CEIL(block_len, 8));

		}
		else
		{
			//Build CipherText
			block* inputs = malloc(sizeof(block) * nslots);
			block* keys = malloc(sizeof(block) * nslots);

			uint64_t prefix_shifted = prefix << block_len;
			//uint64_t prefix_shifted = prefix << block_len;
			for (int j = 0; j < nslots; j++)
				inputs[j] = MAKE_BLOCK(i, prefix_shifted | j);

			ERR_CHECK(aes_eval_blocks(keys, nslots, &sk->prf_key, inputs));

			uint64_t* pi_inv = malloc(sizeof(uint64_t) * nslots);
			ERR_CHECK(prp_inv_eval_all(pi_inv, &prp_key, block_len));

			uint64_t* r = malloc(sizeof(uint64_t) * nslots);
			ERR_CHECK(_eval_keyed_hash_batch(r, sk->params->cmp_len, nslots, keys, nonce));

			ORE_AES_KEY cmp_key_name;
			ORE_AES_KEY cmp_key_row;
			ERR_CHECK(setup_aes_key(&cmp_key_name, (byte*)hash_name, AES_KEY_BYTES));
			ERR_CHECK(setup_aes_key(&cmp_key_row, (byte*)&block_rnum, AES_KEY_BYTES));

			block* inputs_cmp = malloc(AES_KEY_BYTES * nslots);

			uint64_t g_mask = ((params->cmp_len == 64) ? 0xffffffffffffffff : (((uint64_t)1 << params->cmp_len) - 1));

			for (int j = 0; j < nslots; j++)
			{
				uint8_t v = (pi_inv[j] == cur_block) ? ORE_EQUAL : ((pi_inv[j] < cur_block) ? ORE_SMALL : ORE_LARGE);
				//uint8_t v = bit_compare(&pi_inv, &cur_block, CEIL(block_len,8));

				if (v != ORE_EQUAL)
				{
					inputs_cmp[j] = MAKE_BLOCK(*(uint64_t*)hash_name, j << (8 * sizeof(aim)) | v);
					block fcmp;
					ERR_CHECK(aes_eval(&fcmp, &cmp_key_name, inputs_cmp[j]));
					ERR_CHECK(aes_eval(inputs_cmp + j, &cmp_key_row, fcmp));
					r[j] ^= (*(uint64_t*)(inputs_cmp + j)) & g_mask;
				}
				memcpy(comp_right + cmp_len*j, &r[j], cmp_len);
			}

			free(inputs);
			free(keys);
			free(pi_inv);
			free(r);
			free(inputs_cmp);
		}

		// update prefix
		prefix <<= block_len;
		prefix |= cur_block;

		comp_left += len_left_block;
		comp_right += len_right_block;
	}

	return ERROR_NONE;
}

int ore_index_setup(ore_index ctxt, ore_key sk, uint64_t msg, char* cname, uint32_t rnum)
{
	return ore_setup_helper(ctxt, 0, sk, msg, cname, rnum, 0);
}

int ore_query_setup(ore_query ctxt, ore_key sk, uint64_t msg, char* cname, uint8_t aim)
{
	return ore_setup_helper(0, ctxt, sk, msg, cname, 0, aim);
}

int ore_query_left_len(ore_params params)
{
	uint32_t nblocks = CEIL(params->nbits, params->block_len);
	return (AES_BLOCK_LEN + AES_BLOCK_LEN + CEIL(params->nbits, 8)) * nblocks;
}

int ore_index_right_len(ore_params params)
{
	uint32_t block_len = params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(params->nbits, block_len);
	uint32_t cmp_len = CEIL(params->cmp_len, 8);
	return AES_BLOCK_LEN + cmp_len* nslots * nblocks;
}

int ore_query_init(ore_query ctxt, ore_params params)
{
	if (ctxt == NULL || params == NULL) {
		return ERROR_NULL_POINTER;
	}

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	ctxt->comp_left = malloc(ore_query_left_len(params));
	if (ctxt->comp_left == NULL) {
		return ERROR_MEMORY_ALLOCATION;
	}

	ctxt->initialized = true;

	return ERROR_NONE;
}

int ore_query_cleanup(ore_query ctxt, ore_params params)
{
	if (ctxt == NULL)
	{
		return ERROR_NONE;
	}
	if (!ore_is_valid_params(params))
	{
		return ERROR_PARAMS_INVALID;
	}
	memset(ctxt->comp_left, 0, ore_query_left_len(params));
	free(ctxt->comp_left);
	//memset(ctxt->comp_query, 0, ore_query_query_len(params));
	//free(ctxt->comp_query);
	memset(ctxt, 0, sizeof(ore_query));
	return ERROR_NONE;
}

int ore_index_init(ore_index ctxt, ore_params params)
{
	if (ctxt == NULL || params == NULL) {
		return ERROR_NULL_POINTER;
	}

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	uint32_t block_len = params->block_len;
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(params->nbits, block_len);
	uint32_t cmp_len = CEIL(params->cmp_len, 8);
	uint32_t right_len = ore_index_right_len(params);

	ctxt->comp_right = malloc(right_len);
	if (ctxt->comp_right == NULL) {
		return ERROR_MEMORY_ALLOCATION;
	}

	ctxt->initialized = true;

	return ERROR_NONE;
}

int ore_index_cleanup(ore_index ctxt, ore_params params)
{
	if (ctxt == NULL)
	{
		return ERROR_NONE;
	}
	if (!ore_is_valid_params(params))
	{
		return ERROR_PARAMS_INVALID;
	}
	uint32_t right_len = ore_index_right_len(params);
	memset(ctxt->comp_right, 0, right_len);
	free(ctxt->comp_right);

	memset(ctxt, 0, sizeof(ore_index));
	return ERROR_NONE;
}

int ore_compare(int* result_p, ore_query ctxt1, ore_index ctxt2, ore_params params, uint32_t rnum)
{
	if (!ctxt1->initialized || !ctxt2->initialized) {
		return ERROR_CTXT_NOT_INITIALIZED;
	}

	if (!ore_is_valid_params(params)) {
		return ERROR_PARAMS_INVALID;
	}

	uint32_t nbits = params->nbits;
	uint32_t block_len = params->block_len;
	uint32_t cmp_len = CEIL(params->cmp_len, 8);
	uint32_t nslots = 1 << block_len;
	uint32_t nblocks = CEIL(nbits, block_len);

	block nonce = *(block*)ctxt2->comp_right;

	uint32_t offset_left = 0;
	uint32_t offset_right = sizeof(block);
	//uint32_t offset_query = 0;

	uint32_t len_left_block = AES_BLOCK_LEN * 2 + CEIL(nbits, 8);
	uint32_t len_right_block = nslots * cmp_len;
	//uint32_t len_query_block = AES_BLOCK_LEN;

	block block_rnum = MAKE_BLOCK(rnum, 0);
	ORE_AES_KEY cmp_key_row;
	ERR_CHECK(setup_aes_key(&cmp_key_row, (byte*)&block_rnum, sizeof(block)));

	for (int i = 0; i < nblocks; i++)
	{
		uint64_t index = 0;
		memcpy(&index, ctxt1->comp_left + offset_left + AES_KEY_BYTES * 2, CEIL(block_len, 8));

		block fcmp;
		memcpy(&fcmp, ctxt1->comp_left + offset_left + AES_KEY_BYTES, AES_KEY_BYTES);

		block key_block;
		memcpy(&key_block, ctxt1->comp_left + offset_left, AES_KEY_BYTES);

		uint64_t r;
		ERR_CHECK(_eval_keyed_hash(&r, params->cmp_len, key_block, nonce));

		uint64_t g_mask = ((params->cmp_len == 64) ? 0xffffffffffffffff : (((uint64_t)1 << params->cmp_len) - 1));

		uint64_t right = 0;
		memcpy(&right, ctxt2->comp_right + offset_right + index*cmp_len, cmp_len);
		r ^= right & g_mask;

		if (r != 0)
		{
			block gcmp;
			aes_eval(&gcmp, &cmp_key_row, fcmp);
			uint64_t g = (*(uint64_t*)(&gcmp)) & g_mask;
			if (g == r)
			{
				*result_p = 1;
				return ERROR_NONE;
			}
			else
			{
				*result_p = -1;
				return ERROR_NONE;
			}
		}
		offset_left += len_left_block;
		offset_right += len_right_block;
	}
	*result_p = 0;
	return ERROR_NONE;
}

int ore_query_ui(byte** dst, uint32_t* dstlen, uint64_t src, ore_key sk, char* cname, uint8_t aim)
{
	ore_params params;
	memcpy(params, sk->params, sizeof(ore_params));

	ore_query query_tmp;
	ERR_CHECK(ore_query_init(query_tmp, params));
	ERR_CHECK(ore_query_setup(query_tmp, sk, src, cname, aim));

	*dstlen = ore_query_left_len(params);
	*dst = query_tmp->comp_left;

	return ERROR_NONE;
}

int ore_index_ui(byte** dst, uint32_t* dstlen, uint64_t src, ore_key sk, char* cname, uint32_t rnum)
{
	ore_params params;
	memcpy(params, sk->params, sizeof(ore_params));

	ore_index index_tmp;
	ERR_CHECK(ore_index_init(index_tmp, params));
	ERR_CHECK(ore_index_setup(index_tmp, sk, src, cname, rnum));

	*dstlen = ore_index_right_len(params);
	*dst = index_tmp->comp_right;

	return ERROR_NONE;
}

int ore_compare_ui(int* result_p, byte* query, uint32_t querylen, byte* index, uint32_t indexlen,
	ore_params params, uint32_t rnum)
{
	uint32_t nbits = params->nbits;
	uint32_t block_len = params->block_len;
	uint32_t nblocks = CEIL(nbits, block_len);
	uint32_t len_query_block = ore_query_left_len(params);
	uint32_t times_query_block = querylen / len_query_block;
	uint32_t len_index_block = ore_index_right_len(params);
	uint32_t times_index_block = indexlen / len_index_block;

	uint32_t times = times_query_block > times_index_block ? times_index_block : times_query_block;

	ore_query query_tmp;
	ore_index index_tmp;
	query_tmp->comp_left = query;
	query_tmp->initialized = true;
	index_tmp->comp_right = index;
	index_tmp->initialized = true;

	int res = 0;
	for (int i = 0; i < times; i++)
	{
		ERR_CHECK(ore_compare(&res, query_tmp, index_tmp, params, rnum));

		if (res != 0)
		{
			if (res > 0)
			{
				//*result_p = nblocks*i + res;
				*result_p = 1;
				return ERROR_NONE;
			}
			else
			{
				//*result_p = -(nblocks*i) + res;
				*result_p = -1;
				return ERROR_NONE;
			}
		}
		query_tmp->comp_left += len_query_block;
		index_tmp->comp_right += len_index_block;
	}

	*result_p = 0;
	return ERROR_NONE;
}
