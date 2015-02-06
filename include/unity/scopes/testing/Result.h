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
 * Authored by: Thomas Voß <thomas.voss@canonical.com>
 */

#pragma once

#include <unity/scopes/Result.h>

namespace unity
{

namespace scopes
{

namespace testing
{

/**
 \brief A simple class implementation of unity::scopes::Result that provides a default constructor.

 This class makes it possible to create dummy results with no attributes, for testing purposes.
*/
class Result : public unity::scopes::Result
{
public:
    /**
     \brief Default constructor, creates Result instance with no attributes.
     */
    Result();
};

} // namespace testing

} // namespace scopes

} // namespace unity
