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

#ifndef ANITOMY_OPTIONS_H
#define ANITOMY_OPTIONS_H

#include <vector>

#include "string.h"

namespace anitomy {

struct Options {
  string_t allowed_delimiters;
  std::vector<string_t> ignored_strings;

  bool parse_episode_number;
  bool parse_episode_title;
  bool parse_file_extension;
  bool parse_release_group;

  Options()
  {
  allowed_delimiters = L" _.&+,|";

  parse_episode_number = true;
  parse_episode_title = true;
  parse_file_extension = true;
  parse_release_group = true;
  }
};

}  // namespace anitomy

#endif  // ANITOMY_OPTIONS_H