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

#ifndef UNITY_INTERNAL_SEARCHMETADATAIMPL_H
#define UNITY_INTERNAL_SEARCHMETADATAIMPL_H

#include <unity/scopes/internal/QueryMetadataImpl.h>
#include <unity/scopes/SearchMetadata.h>

namespace unity
{

namespace scopes
{

namespace internal
{

class SearchMetadataImpl : public QueryMetadataImpl
{
public:
    SearchMetadataImpl(std::string const& locale, std::string const& form_factor);
    SearchMetadataImpl(int cardinality, std::string const& locale, std::string const& form_factor);
    SearchMetadataImpl(VariantMap const& var);
    ~SearchMetadataImpl() = default;

    void set_cardinality(int cardinality);
    int cardinality() const;

    bool contains_hint(std::string const& key) const;
    void set_hint(std::string const& key, Variant const& value);
    VariantMap hints() const;
    Variant& hint(std::string const& key);
    Variant const& hint(std::string const& key) const;

    static SearchMetadata create(VariantMap const& var);

protected:
    std::string metadata_type() const override;
    void serialize(VariantMap &var) const override;

    void check_cardinality(std::string const& func_name, int cardinality);

private:
    int cardinality_;
    VariantMap hints_;
};

} // namespace internal

} // namespace scopes

} // namespace unity

#endif
