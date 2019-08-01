#pragma once

#include <string>
#include <vector>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_tree.h>
#include "../types/Achievement.h"
#include "../types/Actions.h"
#include "../types/ProcessedRequest.h"

/**
 *  Encode a string into a YAJL handle and
 *  handle error and coercion to C types for a string
 */
void yajl_gen_string_wrap(yajl_gen handle, const char * a);

/**
 *  Encode an ack into the provided handle
 */
void encode_ack(yajl_gen handle);

/**
 *  Decode an ack. Return true if ack is present and good,
 *  false otherwise
 */
bool decode_ack(std::string response);

/**
 *  Encode a request
 */
void encode_request(yajl_gen handle, const char * request);

/**
 *  Decode a request
 */
ProcessedRequest decode_request(std::string request);

/**
 * Encode an achievement vector into a given YAJL handle
 * Encode an array at a time because decoding individual
 * achievements in YAJL would be messy
 */
void encode_achievements(yajl_gen handle, std::vector<Achievement_t> achievements);

/**
 * Decode the achievement vector from a json response
 */
std::vector<Achievement_t> decode_achievements(std::string response);

/**
 * Encode just achievement changes into a handle
 */
void encode_changes(yajl_gen handle, std::vector<AchievementChange_t> changes);

/**
 * Decode just achievement changes into a vector
 */
std::vector<AchievementChange_t> decode_changes(std::string request);
