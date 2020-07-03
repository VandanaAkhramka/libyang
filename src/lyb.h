/**
 * @file lyb.h
 * @author Michal Vasko <mvasko@cesnet.cz>
 * @brief Header for LYB format printer & parser
 *
 * Copyright (c) 2020 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#ifndef LY_LYB_H_
#define LY_LYB_H_

#include <stddef.h>
#include <stdint.h>

#include "set.h"
#include "tree.h"

struct hash_table;
struct ly_ctx;
struct lyd_node;
struct lysc_node;

/**
 * @brief Internal structure for LYB parser/printer.
 */
struct lyd_lyb_ctx {
    struct lyd_lyb_subtree {
        size_t written;
        size_t position;
        uint8_t inner_chunks;
    } *subtrees;
    LY_ARRAY_COUNT_TYPE subtree_size;

    size_t byte_count;  /**< printed/parsed bytes */
    const struct ly_ctx *ctx;
    union {
        struct {
            int parse_options;
            int validate_options;
        };
        int print_options;
    };

    /* LYB parser only */
    const char *data;
    int int_opts;
    const struct lys_module **models;
    struct ly_set unres_node_type;
    struct ly_set unres_meta_type;
    struct ly_set when_check;
    struct lyd_node *op_ntf;

    /* LYB printer only */
    struct lyd_lyb_sib_ht {
        struct lysc_node *first_sibling;
        struct hash_table *ht;
    } *sib_hts;
};

/**
 * LYB format
 *
 * Unlike XML or JSON, it is binary format so most data are represented in similar way but in binary.
 * Some notable differences:
 *
 * - schema nodes are identified based on their hash instead of their string name. In case of collisions
 * an array of hashes is created with each next hash one bit shorter until a unique sequence of all these
 * hashes is found and then all of them are stored.
 *
 * - tree structure is represented as individual strictly bounded subtrees. Each subtree begins
 * with its metadata, which consist of 1) the whole subtree length in bytes and 2) number
 * of included metadata chunks of nested subtrees.
 *
 * - since length of a subtree is not known before it is printed, holes are first written and
 * after the subtree is printed, they are filled with actual valid metadata. As a consequence,
 * LYB data cannot be directly printed into streams!
 *
 * - data are preceded with information about all the used modules. It is needed because of
 * possible augments and deviations which must be known beforehand, otherwise schema hashes
 * could be matched to the wrong nodes.
 */

/* just a shortcut */
#define LYB_LAST_SUBTREE(lybctx) lybctx->subtrees[LY_ARRAY_COUNT(lybctx->subtrees) - 1]

/* struct lyd_lyb_subtree allocation step */
#define LYB_SUBTREE_STEP 4

/* current LYB format version */
#define LYB_VERSION_NUM 0x10

/* LYB format version mask of the header byte */
#define LYB_VERSION_MASK 0x10

/**
 * LYB schema hash constants
 *
 * Hash is divided to collision ID and hash itself.
 *
 * First bits are collision ID until 1 is found. The rest is truncated 32b hash.
 * 1xxx xxxx - collision ID 0 (no collisions)
 * 01xx xxxx - collision ID 1 (collision ID 0 hash collided)
 * 001x xxxx - collision ID 2 ...
 */

/* Number of bits the whole hash will take (including hash collision ID) */
#define LYB_HASH_BITS 8

/* Masking 32b hash (collision ID 0) */
#define LYB_HASH_MASK 0x7f

/* Type for storing the whole hash (used only internally, publicly defined directly) */
#define LYB_HASH uint8_t

/* Need to move this first >> collision number (from 0) to get collision ID hash part */
#define LYB_HASH_COLLISION_ID 0x80

/* How many bytes are reserved for one data chunk SIZE (8B is maximum) */
#define LYB_SIZE_BYTES 1

/* Maximum size that will be written into LYB_SIZE_BYTES (must be large enough) */
#define LYB_SIZE_MAX UINT8_MAX

/* How many bytes are reserved for one data chunk inner chunk count */
#define LYB_INCHUNK_BYTES 1

/* Maximum size that will be written into LYB_INCHUNK_BYTES (must be large enough) */
#define LYB_INCHUNK_MAX UINT8_MAX

/* Just a helper macro */
#define LYB_META_BYTES (LYB_INCHUNK_BYTES + LYB_SIZE_BYTES)

/* Type large enough for all meta data */
#define LYB_META uint16_t

#endif /* LY_LYB_H_ */