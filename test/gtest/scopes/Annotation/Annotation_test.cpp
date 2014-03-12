/*
 * Copyright (C) 2013 Canonical Ltd
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

#include <gtest/gtest.h>
#include <unity/scopes/Annotation.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/internal/CannedQueryImpl.h>
#include <unity/scopes/internal/AnnotationImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::scopes;
using namespace unity::scopes::internal;

TEST(Annotation, link)
{
    {
        CannedQuery query("scope-A", "foo", "dep1");

        Annotation annotation(Annotation::Type::Link);
        annotation.add_link("Link1", query);

        EXPECT_EQ(1u, annotation.links().size());
        auto link = annotation.links().front();
        EXPECT_EQ("Link1", link->label());
        EXPECT_EQ(query.scope_id(), link->query().scope_id());
        EXPECT_EQ(query.department_id(), link->query().department_id());
        EXPECT_EQ(query.query_string(), link->query().query_string());
    }
}

// emblem link is a regular link + an icon
TEST(Annotation, emblemLink)
{
    {
        CannedQuery query("scope-A", "foo", "dep1");

        Annotation annotation(Annotation::Type::Link);
        annotation.set_icon("icon");
        annotation.add_link("Link1", query);

        EXPECT_EQ("icon", annotation.icon());
        EXPECT_EQ(1u, annotation.links().size());
        auto link = annotation.links().front();
        EXPECT_EQ("Link1", link->label());
        EXPECT_EQ(query.scope_id(), link->query().scope_id());
        EXPECT_EQ(query.department_id(), link->query().department_id());
        EXPECT_EQ(query.query_string(), link->query().query_string());
    }
}

TEST(Annotation, link_exceptions)
{
    {
        CannedQuery query("scope-A", "foo", "dep1");

        Annotation annotation(Annotation::Type::Link);
        annotation.add_link("Link1", query);
        EXPECT_THROW(annotation.add_link("Link2", query), unity::InvalidArgumentException); // only one link allowed
        EXPECT_EQ(1u, annotation.links().size());
        EXPECT_NO_THROW(annotation.set_label("label"));
        // no design case for link with a label (makes sense for a group only), but we shouldn't throw
        EXPECT_NO_THROW(annotation.label());
    }
}

TEST(Annotation, groupedLink)
{
    {
        CannedQuery query1("scope-A", "foo", "dep1");
        CannedQuery query2("scope-B", "foo", "dep1");

        Annotation annotation(Annotation::Type::GroupedLink);
        annotation.set_label("Group");
        annotation.add_link("Link1", query1);
        annotation.add_link("Link2", query2);

        EXPECT_EQ("Group", annotation.label());
        EXPECT_EQ(2u, annotation.links().size());
        auto link1 = annotation.links().front();
        auto link2 = annotation.links().back();
        EXPECT_EQ("Link1", link1->label());
        EXPECT_EQ(query1.scope_id(), link1->query().scope_id());
        EXPECT_EQ(query1.department_id(), link1->query().department_id());
        EXPECT_EQ(query1.query_string(), link1->query().query_string());
        EXPECT_EQ("Link2", link2->label());
        EXPECT_EQ(query2.scope_id(), link2->query().scope_id());
        EXPECT_EQ(query2.department_id(), link2->query().department_id());
        EXPECT_EQ(query2.query_string(), link2->query().query_string());
    }
}

TEST(Annotation, groupedLink_exceptions)
{
    {
        CannedQuery query1("scope-A", "foo", "dep1");
        CannedQuery query2("scope-B", "foo", "dep1");

        Annotation annotation(Annotation::Type::GroupedLink);
        annotation.set_label("Group");
        annotation.add_link("Link1", query1);
        annotation.add_link("Link2", query2);
        // no design case for link group with an icon, but we shouldn't throw
        EXPECT_NO_THROW(annotation.set_icon("icon"));
        EXPECT_NO_THROW(annotation.icon());
    }
}

TEST(Annotation, serialize)
{
    {
        CannedQuery query("scope-A", "foo", "dep1");
        Annotation annotation(Annotation::Type::Link);
        annotation.add_link("Link1", query);

        auto vm = annotation.serialize();
        EXPECT_EQ("link", vm["type"].get_string());
        EXPECT_TRUE(vm.find("label") == vm.end());
        EXPECT_TRUE(vm.find("icon") == vm.end());
        auto links = vm["links"].get_array();
        EXPECT_EQ(1u, links.size());
        auto linkvm = links[0].get_dict();
        EXPECT_EQ("Link1", linkvm["label"].get_string());
        CannedQuery qout = internal::CannedQueryImpl::create(linkvm["query"].get_dict());
        EXPECT_EQ("scope-A", qout.scope_id());
        EXPECT_EQ("foo", qout.query_string());
        EXPECT_EQ("dep1", qout.department_id());
    }
}

TEST(Annotation, deserialize)
{
    CannedQuery query("scope-A", "foo", "dep1");
    {
        Annotation annotation(Annotation::Type::GroupedLink);
        annotation.set_label("Foo");
        annotation.add_link("Link1", query);
        auto var = annotation.serialize();
        AnnotationImpl impl(var);
    }
    {
        Annotation annotation(Annotation::Type::Link);
        annotation.set_icon("Icon");
        annotation.add_link("Link1", query);
        auto var = annotation.serialize();
        AnnotationImpl impl(var);
    }
}

TEST(Annotation, deserialize_exceptions)
{
    {
        CannedQuery query("scope-A", "foo", "dep1");
        {
            VariantMap var;
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "";
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "link";
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "groupedlink";
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "card";
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "card";
            var["icon"] = "Icon";
            var["cat_id"] = "unknowncat";
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {   // deserialize with empty links array
            Annotation annotation(Annotation::Type::Link);
            annotation.add_link("Link1", query);
            auto var = annotation.serialize();
            var["links"] = VariantArray();
            try
            {
                AnnotationImpl impl(var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
    }
}

TEST(Annotation, copy)
{
    {
        CannedQuery query("scope-A", "foo", "dep1");
        Annotation annotation(Annotation::Type::GroupedLink);
        annotation.set_label("Group");
        annotation.add_link("Link1", query);
        Annotation copy(annotation);

        EXPECT_EQ("Group", copy.label());
        EXPECT_EQ(Annotation::Type::GroupedLink, copy.annotation_type());
        annotation.add_link("Link2", query);
        EXPECT_EQ(1u, copy.links().size());
    }
}
