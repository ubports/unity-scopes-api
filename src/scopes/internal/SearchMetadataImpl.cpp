/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
*/

#include <unity/scopes/internal/SearchMetadataImpl.h>
#include <unity/scopes/internal/Utils.h>
#include <unity/scopes/ScopeExceptions.h>

#include <unity/UnityExceptions.h>

namespace unity
{

namespace scopes
{

namespace internal
{

std::mutex SearchMetadataImpl::mutex_;

SearchMetadataImpl::SearchMetadataImpl(std::string const& locale, std::string const& form_factor)
    : QueryMetadataImpl(locale, form_factor),
      cardinality_(0)
{
}

SearchMetadataImpl::SearchMetadataImpl(int cardinality, std::string const& locale, std::string const& form_factor)
    : QueryMetadataImpl(locale, form_factor),
      cardinality_(cardinality)
{
    check_cardinality("SearchMetadataImpl()", cardinality);
}

SearchMetadataImpl::SearchMetadataImpl(VariantMap const& var)
    : QueryMetadataImpl(var)
{
    auto it = find_or_throw("SearchMetadataImpl()", var, "cardinality");
    cardinality_ = it->second.get_int();
    check_cardinality("SearchMetadataImpl(VariantMap)", cardinality_);

    try
    {
        it = find_or_throw("SearchMetadataImpl()", var, "location");
        location_ = Location(it->second.get_dict());
    }
    catch (std::exception &e)
    {
    }

    // History of sender/receiver scope ID pairs this query has visited
    try
    {
        it = find_or_throw("SearchMetadataImpl()", var, "history");
        for (auto const& v : it->second.get_array())
        {
            auto sender = v.get_dict()["s"].get_string();
            auto receiver = v.get_dict()["r"].get_string();
            history_.push_back(make_pair(sender, receiver));
        }
    }
    catch (std::exception &e)
    {
    }
}

void SearchMetadataImpl::set_cardinality(int cardinality)
{
    check_cardinality("SearchMetadataImpl::set_cardinality()", cardinality);
    cardinality_ = cardinality;
}

int SearchMetadataImpl::cardinality() const
{
    return cardinality_;
}

void SearchMetadataImpl::set_location(Location const& location)
{
    location_ = location;
}

Location SearchMetadataImpl::location() const
{
    if (has_location())
    {
        return *location_;
    }
    throw NotFoundException("SearchMetadata::location()", "location");
}

bool SearchMetadataImpl::has_location() const
{
    return location_;
}

bool SearchMetadataImpl::add_to_history(SendRecvPair const& pair)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (std::find(history_.begin(), history_.end(), pair) != history_.end())
    {
        return false;
    }
    history_.push_back(pair);
    return true;
}

SearchMetadataImpl::History SearchMetadataImpl::history() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return history_;
}

void SearchMetadataImpl::set_history(History const& h)
{
    std::lock_guard<std::mutex> lock(mutex_);
    history_ = h;
}

std::string SearchMetadataImpl::metadata_type() const
{
    static const std::string t("search_metadata");
    return t;
}

void SearchMetadataImpl::serialize(VariantMap& var) const
{
    QueryMetadataImpl::serialize(var);
    var["cardinality"] = Variant(cardinality_);
    if (location_)
    {
        var["location"] = location_->serialize();
    }

    std::lock_guard<std::mutex> lock(mutex_);  // history_ is mutable

    VariantArray hist;
    for (auto const& pair : history_)
    {
        VariantMap vm;
        vm["s"] = pair.first;         // Sender scope ID
        vm["r"] = pair.first;         // Receiver scope ID
        hist.push_back(Variant(vm));
    }
    var["history"] = hist;
}

VariantMap SearchMetadataImpl::serialize() const
{
    VariantMap vm;
    serialize(vm);
    return vm;
}

void SearchMetadataImpl::check_cardinality(std::string const& func_name, int cardinality)
{
    if (cardinality < 0)
    {
        throw InvalidArgumentException(func_name + ": " + std::to_string(cardinality));
    }
}

SearchMetadata SearchMetadataImpl::create(VariantMap const& var)
{
    return SearchMetadata(new SearchMetadataImpl(var));
}

} // namespace internal

} // namespace scopes

} // namespace unity
