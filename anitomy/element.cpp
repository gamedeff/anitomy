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
#include <functional>

#include "element.h"

namespace anitomy {

bool Elements::empty() const {
  return elements_.empty();
}

size_t Elements::size() const {
  return elements_.size();
}

////////////////////////////////////////////////////////////////////////////////

element_iterator_t Elements::begin() {
  return elements_.begin();
}

element_const_iterator_t Elements::begin() const {
  return elements_.begin();
}

element_const_iterator_t Elements::cbegin() const {
  return elements_.begin();
}

element_iterator_t Elements::end() {
  return elements_.end();
}

element_const_iterator_t Elements::end() const {
  return elements_.end();
}

element_const_iterator_t Elements::cend() const {
  return elements_.end();
}

////////////////////////////////////////////////////////////////////////////////

element_pair_t& Elements::at(size_t position) {
  return elements_.at(position);
}

const element_pair_t& Elements::at(size_t position) const {
  return elements_.at(position);
}

element_pair_t& Elements::operator[](size_t position) {
  return elements_[position];
}

const element_pair_t& Elements::operator[](size_t position) const {
  return elements_[position];
}

////////////////////////////////////////////////////////////////////////////////

const string_t empty_element = L"";

const string_t& Elements::get(ElementCategory category) {
  element_container_t::iterator element = find(category);

  if (element == elements_.end())
	  return empty_element;
    //element = elements_.insert(elements_.end(),
    //                           std::make_pair(category, string_t()));

  return element->second;
}

std::vector<string_t> Elements::get_all(ElementCategory category) const {
  std::vector<string_t> elements;

  for (element_container_t::const_iterator element = elements_.begin(); element != elements_.end(); ++element)
    if (element->first == category)
      elements.push_back(element->second);

  return elements;
}

////////////////////////////////////////////////////////////////////////////////

void Elements::clear() {
  elements_.clear();
}

void Elements::insert(ElementCategory category, const string_t& value) {
  if (!value.empty())
    elements_.push_back(std::make_pair(category, value));
}

bool is_category(const element_pair_t element, const ElementCategory category)
{
	return element.first == category;
}

void Elements::erase(ElementCategory category) {
  element_container_t::iterator iterator = std::remove_if(elements_.begin(), elements_.end(), std::bind2nd(std::ptr_fun(is_category), category));
  elements_.erase(iterator, elements_.end());
}

element_iterator_t Elements::erase(element_iterator_t iterator) {
  return elements_.erase(iterator);
}

////////////////////////////////////////////////////////////////////////////////

size_t Elements::count(ElementCategory category) const {
  return std::count_if(elements_.begin(), elements_.end(), std::bind2nd(std::ptr_fun(is_category), category));
}

bool Elements::empty(ElementCategory category) const {
  return find(category) == elements_.end();
}

element_iterator_t Elements::find(ElementCategory category) {
  return std::find_if(elements_.begin(), elements_.end(), std::bind2nd(std::ptr_fun(is_category), category));
}

element_const_iterator_t Elements::find(ElementCategory category) const {
  return std::find_if(elements_.begin(), elements_.end(), std::bind2nd(std::ptr_fun(is_category), category));
}

}  // namespace anitomy