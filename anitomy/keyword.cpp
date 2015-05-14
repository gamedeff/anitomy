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

#include "keyword.h"
#include "token.h"

namespace anitomy {

KeywordManager keyword_manager;

KeywordOptions::KeywordOptions(bool identifiable, bool searchable, bool valid)
    : identifiable(identifiable), searchable(searchable), valid(valid) {
}

Keyword::Keyword(ElementCategory category, const KeywordOptions& options)
    : category(category), options(options) {
}

////////////////////////////////////////////////////////////////////////////////

#define Add_ { const char_t* k[] = 
#define With(a, b) ; std::vector<string_t> v(k, k + _countof(k)); Add(a, b, v); }

KeywordManager::KeywordManager() {
  const KeywordOptions options_default;
  const KeywordOptions options_invalid(true, true, false);
  const KeywordOptions options_unidentifiable(false, true, true);
  const KeywordOptions options_unidentifiable_invalid(false, true, false);
  const KeywordOptions options_unidentifiable_unsearchable(false, false, true);

  Add_{
	  L"SAISON", L"SEASON"} With (kElementAnimeSeasonPrefix, options_unidentifiable);

  Add_{
      L"GEKIJOUBAN", L"MOVIE",
      L"OAD", L"OAV", L"ONA", L"OVA",
      L"SPECIAL", L"SPECIALS",
      L"TV"} With (kElementAnimeType, options_unidentifiable);
  Add_{
      L"SP"} With (kElementAnimeType, options_unidentifiable_unsearchable);  // e.g. "Yumeiro Patissiere SP Professional"
  Add_{
      L"ED", L"ENDING", L"NCED",
      L"NCOP", L"OP", L"OPENING",
      L"PREVIEW", L"PV"} With (kElementAnimeType, options_unidentifiable_invalid);

  Add_{
      // Audio channels
      L"2.0CH", L"2CH", L"5.1", L"5.1CH", L"DTS", L"DTS-ES", L"DTS5.1",
      L"TRUEHD5.1",
      // Audio codec
      L"AAC", L"AACX2", L"AACX3", L"AACX4", L"AC3", L"FLAC", L"FLACX2",
      L"FLACX3", L"FLACX4", L"LOSSLESS", L"MP3", L"OGG", L"VORBIS",
      // Audio language
      L"DUALAUDIO", L"DUAL AUDIO"} With (kElementAudioTerm, options_default);

  Add_{
      L"IPAD3", L"IPHONE5", L"IPOD", L"PS3", L"XBOX", L"XBOX360"} With (kElementDeviceCompatibility, options_default);
  Add_{
      L"ANDROID"} With (kElementDeviceCompatibility, options_unidentifiable);

  Add_{
      L"EP", L"EP.", L"EPS", L"EPS.", L"EPISODE", L"EPISODE.", L"EPISODES",
      L"VOL", L"VOL.", L"VOLUME",
      L"CAPITULO", L"EPISODIO", L"FOLGE"} With (kElementEpisodePrefix, options_default);
  Add_{
      L"E", L"\x7B2C"} With (kElementEpisodePrefix, options_invalid);  // single-letter episode keywords are not valid tokens

  Add_{
      L"3GP", L"AVI", L"DIVX", L"FLV", L"M2TS", L"MKV", L"MOV", L"MP4", L"MPG",
      L"OGM", L"RM", L"RMVB", L"WEBM", L"WMV"} With (kElementFileExtension, options_default);
  Add_{
      L"AAC", L"AIFF", L"FLAC", L"M4A", L"MP3", L"MKA", L"OGG", L"WAV", L"WMA",
      L"7Z", L"RAR", L"ZIP",
      L"ASS", L"SRT"} With (kElementFileExtension, options_invalid);

  Add_{
      L"ENG", L"ENGLISH", L"ESPANOL", L"JAP", L"PT-BR", L"SPANISH", L"VOSTFR"} With (kElementLanguage, options_default);
  Add_{
      L"ESP", L"ITA"} With (kElementLanguage, options_unidentifiable);  // e.g. "Tokyo ESP", "Bokura ga Ita"

  Add_{
      L"REMASTER", L"REMASTERED", L"UNCENSORED", L"UNCUT",
      L"TS", L"VFR", L"WIDESCREEN", L"WS"} With (kElementOther, options_default);

  Add_{
      L"THORA"} With (kElementReleaseGroup, options_default);

  Add_{
      L"BATCH", L"COMPLETE", L"PATCH", L"REMUX"} With (kElementReleaseInformation, options_default);
  Add_{
      L"END", L"FINAL"} With (kElementReleaseInformation, options_unidentifiable);  // e.g. "The End of Evangelion", "Final Approach"

  Add_{
      L"V0", L"V1", L"V2", L"V3", L"V4"} With (kElementReleaseVersion, options_default);

  Add_{
      L"BD", L"BDRIP", L"BLURAY", L"BLU-RAY",
      L"DVD", L"DVD5", L"DVD9", L"DVD-R2J", L"DVDRIP", L"DVD-RIP",
      L"R2DVD", L"R2J", L"R2JDVD", L"R2JDVDRIP",
      L"HDTV", L"HDTVRIP", L"TVRIP", L"TV-RIP",
      L"WEBCAST", L"WEBRIP"} With (kElementSource, options_default);

  Add_{
      L"ASS", L"BIG5", L"DUB", L"DUBBED", L"HARDSUB", L"RAW", L"SOFTSUB",
      L"SOFTSUBS", L"SUB", L"SUBBED", L"SUBTITLED"} With (kElementSubtitles, options_default);

  Add_{
      // Frame rate
      L"23.976FPS", L"24FPS", L"29.97FPS", L"30FPS", L"60FPS", L"120FPS",
      // Video codec
      L"8BIT", L"8-BIT", L"10BIT", L"10BITS", L"10-BIT", L"10-BITS", L"HI10P",
      L"H264", L"H.264", L"X264", L"X.264",
      L"AVC", L"DIVX", L"DIVX5", L"DIVX6", L"XVID",
      // Video format
      L"AVI", L"RMVB", L"WMV", L"WMV3", L"WMV9",
      // Video quality
      L"HQ", L"LQ",
      // Video resolution
      L"HD", L"SD"} With (kElementVideoTerm, options_default);
}

#undef With
#undef Add_

void KeywordManager::Add(ElementCategory category,
                         const KeywordOptions& options,
                         const std::vector<string_t>& keywords) {
  KeywordManager::keyword_container_t& keys = GetKeywordContainer(category);
  for (std::vector<string_t>::const_iterator keyword = keywords.begin(); keyword != keywords.end(); ++keyword) {
    if (keyword->empty())
      continue;
    if (keys.find(*keyword) != keys.end())
      continue;
    keys.insert(std::make_pair(*keyword, Keyword(category, options)));
  }
}

bool KeywordManager::Find(ElementCategory category, const string_t& str) const {
  const KeywordManager::keyword_container_t& keys = GetKeywordContainer(category);
  const KeywordManager::keyword_container_t::const_iterator it = keys.find(str);
  if (it != keys.end() && it->second.category == category)
    return true;

  return false;
}

bool KeywordManager::Find(const string_t& str, ElementCategory& category,
                          KeywordOptions& options) const {
  const KeywordManager::keyword_container_t& keys = GetKeywordContainer(category);
  const KeywordManager::keyword_container_t::const_iterator it = keys.find(str);
  if (it != keys.end()) {
    if (category == kElementUnknown) {
      category = it->second.category;
    } else if (it->second.category != category) {
      return false;
    }
    options = it->second.options;
    return true;
  }

  return false;
}

string_t KeywordManager::Normalize(const string_t& str) const {
  return StringToUpperCopy(str);
}

KeywordManager::keyword_container_t& KeywordManager::GetKeywordContainer(
    ElementCategory category) const {
  return category == kElementFileExtension ?
      const_cast<keyword_container_t&>(file_extensions_) :
      const_cast<keyword_container_t&>(keys_);
}

////////////////////////////////////////////////////////////////////////////////

void KeywordManager::Peek(const string_t& filename,
                          const TokenRange& range,
                          Elements& elements,
                          std::vector<TokenRange>& preidentified_tokens) const {
  typedef std::pair<ElementCategory, std::vector<string_t>> entry_t;
  static /*const*/ std::vector<entry_t> entries;
  { ElementCategory c = kElementAudioTerm;       const char_t* k[] = {L"Dual Audio"}; entry_t e; e.first = c; e.second.assign(k, k + _countof(k)); entries.push_back(e); }
  { ElementCategory c = kElementVideoTerm;       const char_t* k[] = {L"H264", L"H.264", L"h264", L"h.264"}; entry_t e; e.first = c; e.second.assign(k, k + _countof(k)); entries.push_back(e); }
  { ElementCategory c = kElementVideoResolution; const char_t* k[] = {L"480p", L"720p", L"1080p"}; entry_t e; e.first = c; e.second.assign(k, k + _countof(k)); entries.push_back(e); }
  { ElementCategory c = kElementSource;          const char_t* k[] = {L"Blu-Ray"}; entry_t e; e.first = c; e.second.assign(k, k + _countof(k)); entries.push_back(e); }

  string_t::const_iterator it_begin = filename.begin() + range.offset;
  string_t::const_iterator it_end = it_begin + range.size;

  for(std::vector<entry_t>::const_iterator entry = entries.begin(); entry != entries.end(); ++entry) {
    for(std::vector<string_t>::const_iterator keyword = entry->second.begin(); keyword != entry->second.end(); ++keyword) {
      string_t::const_iterator it = std::search(it_begin, it_end, keyword->begin(), keyword->end());
      if (it != it_end) {
        size_t offset = it - filename.begin();
        elements.insert(entry->first, *keyword);
        preidentified_tokens.push_back(TokenRange(offset, keyword->size()));
      }
    }
  }
}

}  // namespace anitomy