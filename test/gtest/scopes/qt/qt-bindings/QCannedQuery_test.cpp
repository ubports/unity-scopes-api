/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Xavi Garcia <xavi.garcia.mena@canonical.com>
 */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#include <gtest/gtest.h>
#pragma GCC diagnostic pop

#include <unity/scopes/qt/QCannedQuery.h>
#include <unity/scopes/qt/internal/QCannedQueryImpl.h>
#include <unity/scopes/qt/internal/QUtils.h>

#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/OptionSelectorFilter.h>

using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

TEST(QCannedQuery, bindings)
{
    internal::QCannedQueryImpl* impl = new internal::QCannedQueryImpl("scopeA", "foo", "dep1");
    QCannedQuery query = internal::QCannedQueryImpl::create(impl);

    // get the internal api query
    unity::scopes::CannedQuery* api_query = impl->get_api_query();

    // start checking that the internal class has the right attributes
    EXPECT_EQ(api_query->scope_id(), "scopeA");
    EXPECT_EQ(api_query->query_string(), "foo");
    EXPECT_EQ(api_query->department_id(), "dep1");
    EXPECT_EQ(api_query->to_uri(), query.to_uri().toStdString());

    EXPECT_EQ("scopeA", query.scope_id());
    EXPECT_EQ("foo", query.query_string());
    EXPECT_EQ("dep1", query.department_id());

    unity::scopes::VariantMap api_map = api_query->serialize();
    QVariantMap qt_map = query.serialize();
    EXPECT_EQ(int(api_map.size()), qt_map.size());
    EXPECT_TRUE(qt_map.size() != 0);
    EXPECT_EQ(variantmap_to_qvariantmap(api_map), qt_map);

    unity::scopes::FilterState fstate;
    {
        auto filter = unity::scopes::OptionSelectorFilter::create("f1", "Choose an option", false);
        auto option1 = filter->add_option("o1", "Option 1");
        filter->add_option("o2", "Option 2");
        filter->update_state(fstate, option1, true);
    }
    query.set_filter_state(fstate);
    // filters is {"f1":["o1"]}
    EXPECT_EQ("scope://scopeA?q=foo&dep=dep1&filters=%7B%22f1%22%3A%5B%22o1%22%5D%7D%0A", query.to_uri());
    EXPECT_EQ("scope://scopeA?q=foo&dep=dep1&filters=%7B%22f1%22%3A%5B%22o1%22%5D%7D%0A", api_query->to_uri());
    auto fs = query.filter_state();
    EXPECT_TRUE(fs.has_filter("f1"));

    auto q = QCannedQuery::from_uri("scope://scopeX?q=bar&dep=dep2&filters=%7B%22f1%22%3A%5B%22o1%22%5D%7D%0A");
    EXPECT_EQ("scopeX", q.scope_id());
    EXPECT_EQ("bar", q.query_string());
    EXPECT_EQ("dep2", q.department_id());

    // build a query with the normal qt interface
    // and check that it has the same data
    QCannedQuery query2("scopeA", "foo", "dep1");
    EXPECT_EQ(variantmap_to_qvariantmap(api_map), query2.serialize());

    // change a value using the qt interface
    query.set_department_id("new_department");
    query.set_query_string("new_query");
    // check that the value is the same in the internal instance
    EXPECT_EQ(api_query->department_id(), "new_department");
    EXPECT_EQ(api_query->query_string(), "new_query");
    EXPECT_EQ(variantmap_to_qvariantmap(api_query->serialize()), query.serialize());
}

TEST(QCannedQuery, construct_assign)
{
    QCannedQuery q("scope_id");
    EXPECT_EQ("scope_id", q.scope_id());

    QCannedQuery q2(q);
    EXPECT_EQ("scope_id", q2.scope_id());

    QCannedQuery q3("blah");
    q3 = q2;
    EXPECT_EQ("scope_id", q3.scope_id());
}
