/*
** Anitomy
** Copyright (C) 2014-2015, Eren Okka
** 
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
** 
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
** 
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <algorithm>
#include <set>

#include "keyword.h"
#include "string.h"
#include "tokenizer.h"

namespace anitomy {

Tokenizer::Tokenizer(const string_t& filename, Elements& elements,
                     const Options& options, token_container_t& tokens)
    : elements_(elements),
      filename_(filename),
      options_(options),
      tokens_(tokens) {
}

bool Tokenizer::Tokenize() {
  tokens_.reserve(32);  // Usually there are no more than 20 tokens

  TokenizeByBrackets();

  return !tokens_.empty();
}

////////////////////////////////////////////////////////////////////////////////

void Tokenizer::AddToken(TokenCategory category, bool enclosed,
                         const TokenRange& range) {
  tokens_.push_back(Token(category,
                          filename_.substr(range.offset, range.size),
                          enclosed));
}

  static /*const*/ std::vector<std::pair<char_t, char_t>> brackets;

  // This is basically std::find_first_of() customized to our needs
  string_t::const_iterator find_first_bracket(char_t &matching_bracket, string_t::const_iterator char_begin, const string_t::const_iterator char_end) {
    for (string_t::const_iterator it = char_begin; it != char_end; ++it) {
      for (std::vector<std::pair<char_t, char_t>>::const_iterator bracket_pair = brackets.begin(); bracket_pair != brackets.end(); ++bracket_pair) {
        if (*it == bracket_pair->first) {
          matching_bracket = bracket_pair->second;
          return it;
        }
      }
    }
    return char_end;
  };

void Tokenizer::TokenizeByBrackets() {
  {
      brackets.push_back(std::make_pair<char_t, char_t>(L'(', L')'));  // U+0028-U+0029 Parenthesis
      brackets.push_back(std::make_pair<char_t, char_t>(L'[', L']'));  // U+005B-U+005D Square bracket
      brackets.push_back(std::make_pair<char_t, char_t>(L'{', L'}'));  // U+007B-U+007D Curly bracket
      brackets.push_back(std::make_pair<char_t, char_t>(L'\u300C', L'\u300D'));  // Corner bracket
      brackets.push_back(std::make_pair<char_t, char_t>(L'\u300E', L'\u300F'));  // White corner bracket
      brackets.push_back(std::make_pair<char_t, char_t>(L'\u3010', L'\u3011'));  // Black lenticular bracket
      brackets.push_back(std::make_pair<char_t, char_t>(L'\uFF08', L'\uFF09'));  // Fullwidth parenthesis
  };

  bool is_bracket_open = false;
  char_t matching_bracket = L'\0';

  string_t::const_iterator char_begin = filename_.begin();
  const string_t::const_iterator char_end = filename_.end();

  string_t::const_iterator current_char = char_begin;

  while (current_char != char_end && char_begin != char_end) {
    if (!is_bracket_open) {
      current_char = find_first_bracket(matching_bracket, char_begin, char_end);
    } else {
      // Looking for the matching bracket allows us to better handle some rare
      // cases with nested brackets.
      current_char = std::find(char_begin, char_end, matching_bracket);
    }

    const TokenRange range(std::distance(filename_.begin(), char_begin),
                           std::distance(char_begin, current_char));

    if (range.size > 0)  // Found unknown token
      TokenizeByPreidentified(is_bracket_open, range);

    if (current_char != char_end) {  // Found bracket
      AddToken(kBracket, true, TokenRange(range.offset + range.size, 1));
      is_bracket_open = !is_bracket_open;
      char_begin = ++current_char;
    }
  }
}

void Tokenizer::TokenizeByPreidentified(bool enclosed, const TokenRange& range) {
  std::vector<TokenRange> preidentified_tokens;
  keyword_manager.Peek(filename_, range, elements_, preidentified_tokens);

  size_t offset = range.offset;
  TokenRange subrange(range.offset, 0);

  while (offset < range.offset + range.size) {
    for (std::vector<TokenRange>::const_iterator preidentified_token = preidentified_tokens.begin(); preidentified_token != preidentified_tokens.end(); ++preidentified_token) {
      if (offset == preidentified_token->offset) {
        if (subrange.size > 0)
          TokenizeByDelimiters(enclosed, subrange);
        AddToken(kIdentifier, enclosed, *preidentified_token);
        subrange.offset = preidentified_token->offset + preidentified_token->size;
        offset = subrange.offset - 1;  // It's going to be incremented below
        break;
      }
    }
    subrange.size = ++offset - subrange.offset;
  }

  // Either there was no preidentified token range, or we're now about to
  // process the tail of our current range.
  if (subrange.size > 0)
    TokenizeByDelimiters(enclosed, subrange);
}

void Tokenizer::TokenizeByDelimiters(bool enclosed, const TokenRange& range) {
  const string_t delimiters = GetDelimiters(range);

  if (delimiters.empty()) {
    AddToken(kUnknown, enclosed, range);
    return;
  }

  string_t::const_iterator char_begin = filename_.begin() + range.offset;
  const string_t::const_iterator char_end = char_begin + range.size;
  string_t::const_iterator current_char = char_begin;

  while (current_char != char_end) {
    current_char = std::find_first_of(current_char, char_end,
                                      delimiters.begin(), delimiters.end());

    const TokenRange subrange(std::distance(filename_.begin(), char_begin),
                              std::distance(char_begin, current_char));

    if (subrange.size > 0)  // Found unknown token
      AddToken(kUnknown, enclosed, subrange);

    if (current_char != char_end) {  // Found delimiter
      AddToken(kDelimiter, enclosed,
               TokenRange(subrange.offset + subrange.size, 1));
      char_begin = ++current_char;
    }
  }

  ValidateDelimiterTokens();
}

////////////////////////////////////////////////////////////////////////////////

bool is_delimiter(const char_t c, const Tokenizer *tokenizer) {
	if (!IsAlphanumericChar(c))
		if (tokenizer->options_.allowed_delimiters.find(c) != string_t::npos)
			if (tokenizer->delimiters.find(c) == string_t::npos)
				return true;
	return false;
};

string_t Tokenizer::GetDelimiters(const TokenRange& range) const {
  delimiters.clear();

  std::remove_copy_if(filename_.begin() + range.offset,
               filename_.begin() + range.offset + range.size,
               std::back_inserter(delimiters), std::not1(std::bind2nd(std::ptr_fun(is_delimiter), this)));

  return delimiters;
}

bool is_delimiter_token(token_container_t& tokens_, token_iterator_t it) {
	return it != tokens_.end() && it->category == kDelimiter;
};
bool is_unknown_token(token_container_t& tokens_, token_iterator_t it) {
	return it != tokens_.end() && it->category == kUnknown;
};
bool is_single_character_token(token_container_t& tokens_, token_iterator_t it) {
	return is_unknown_token(tokens_, it) && it->content.size() == 1;
};
void append_token_to(token_iterator_t token,
						  token_iterator_t append_to) {
							  append_to->content.append(token->content);
							  token->category = kInvalid;
};
bool is_invalid_token(const Token& token) {
	return token.category == kInvalid;
}

void Tokenizer::ValidateDelimiterTokens() {
  for (token_iterator_t token = tokens_.begin(); token != tokens_.end(); ++token) {
    if (token->category != kDelimiter)
      continue;
    char_t delimiter = token->content.at(0);
    token_iterator_t prev_token = FindPreviousToken(tokens_, token, kFlagValid);
    token_iterator_t next_token = FindNextToken(tokens_, token, kFlagValid);

    // Check for single-character tokens to prevent splitting group names,
    // keywords, episode number, etc.
    if (delimiter != L' ' && delimiter != L'_') {
      if (is_single_character_token(tokens_, prev_token)) {
        append_token_to(token, prev_token);
        while (is_unknown_token(tokens_, next_token)) {
          append_token_to(next_token, prev_token);
          next_token = FindNextToken(tokens_, next_token, kFlagValid);
          if (is_delimiter_token(tokens_, next_token) &&
              next_token->content.at(0) == delimiter) {
            append_token_to(next_token, prev_token);
            next_token = FindNextToken(tokens_, next_token, kFlagValid);
          }
        }
        continue;
      }
      if (is_single_character_token(tokens_, next_token)) {
        append_token_to(token, prev_token);
        append_token_to(next_token, prev_token);
        continue;
      }
    }

    // Check for adjacent delimiters
    if (is_unknown_token(tokens_, prev_token) && is_delimiter_token(tokens_, next_token)) {
      char_t next_delimiter = next_token->content.at(0);
      if (delimiter != next_delimiter && delimiter != ',') {
        if (next_delimiter == ' ' || next_delimiter == '_') {
          append_token_to(token, prev_token);
        }
      }
    }
  }

  token_iterator_t remove_if_invalid = std::remove_if(tokens_.begin(), tokens_.end(), is_invalid_token);
  tokens_.erase(remove_if_invalid, tokens_.end());
}

}  // namespace anitomy