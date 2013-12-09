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
#include <scopes/Annotation.h>
#include <scopes/CategoryRenderer.h>
#include <scopes/internal/CategoryRegistry.h>
#include <scopes/internal/AnnotationImpl.h>
#include <unity/UnityExceptions.h>

using namespace unity::api::scopes;
using namespace unity::api::scopes::internal;

TEST(Annotation, hyperlink)
{
    {
        Query query("scope-A", "foo", "dep1");

        Annotation annotation(AnnotationType::Hyperlink);
        annotation.add_hyperlink("Link1", query);

        EXPECT_EQ(1, annotation.num_of_hyperlinks());
        auto link = annotation.hyperlink(0);
        EXPECT_EQ("Link1", link->label());
        EXPECT_EQ(query, link->query());
    }
}

// emblem hyperlink is a regular hyperlink + an icon
TEST(Annotation, emblemHyperlink)
{
    {
        Query query("scope-A", "foo", "dep1");

        Annotation annotation(AnnotationType::Hyperlink);
        annotation.set_icon("icon");
        annotation.add_hyperlink("Link1", query);

        EXPECT_EQ("icon", annotation.icon());
        EXPECT_EQ(1, annotation.num_of_hyperlinks());
        auto link = annotation.hyperlink(0);
        EXPECT_EQ("Link1", link->label());
        EXPECT_EQ(query, link->query());
    }
}

TEST(Annotation, hyperlink_exceptions)
{
    {
        Query query("scope-A", "foo", "dep1");

        Annotation annotation(AnnotationType::Hyperlink);
        annotation.add_hyperlink("Link1", query);
        EXPECT_THROW(annotation.add_hyperlink("Link2", query), unity::InvalidArgumentException); // only one hyperlink allowed
        EXPECT_EQ(1, annotation.num_of_hyperlinks());
        EXPECT_NO_THROW(annotation.set_label("label"));
        // no design case for hyperlink with a label (makes sense for a group only), but we shouldn't throw
        EXPECT_NO_THROW(annotation.label());
    }
}

TEST(Annotation, groupedHyperlink)
{
    {
        Query query1("scope-A", "foo", "dep1");
        Query query2("scope-B", "foo", "dep1");

        Annotation annotation(AnnotationType::GroupedHyperlink);
        annotation.set_label("Group");
        annotation.add_hyperlink("Link1", query1);
        annotation.add_hyperlink("Link2", query2);

        EXPECT_EQ("Group", annotation.label());
        EXPECT_EQ(2, annotation.num_of_hyperlinks());
        auto link1 = annotation.hyperlink(0);
        auto link2 = annotation.hyperlink(1);
        EXPECT_EQ("Link1", link1->label());
        EXPECT_EQ(query1, link1->query());
        EXPECT_EQ("Link2", link2->label());
        EXPECT_EQ(query2, link2->query());
    }
}

TEST(Annotation, groupedHyperlink_exceptions)
{
    {
        Query query1("scope-A", "foo", "dep1");
        Query query2("scope-B", "foo", "dep1");

        Annotation annotation(AnnotationType::GroupedHyperlink);
        annotation.set_label("Group");
        annotation.add_hyperlink("Link1", query1);
        annotation.add_hyperlink("Link2", query2);
        // no design case for hyperlink group with an icon, but we shouldn't throw
        EXPECT_NO_THROW(annotation.set_icon("icon"));
        EXPECT_NO_THROW(annotation.icon());
    }
}

TEST(Annotation, card)
{
    {
        Query query("scope-A", "foo", "dep1");

        Annotation annotation(AnnotationType::Card);
        annotation.set_icon("icon");
        annotation.add_hyperlink("Link1", query);

        EXPECT_EQ("icon", annotation.icon());
        EXPECT_EQ(1, annotation.num_of_hyperlinks());
        auto link = annotation.hyperlink(0);
        EXPECT_EQ("Link1", link->label());
        EXPECT_EQ(query, link->query());
    }
}

TEST(Annotation, card_exceptions)
{
    {
        Query query("scope-A", "foo", "dep1");

        Annotation annotation(AnnotationType::Card);
        annotation.set_icon("icon");
        annotation.add_hyperlink("Link1", query);

        // label currently makes sense for group only, but we shouldn't throw
        EXPECT_NO_THROW(annotation.set_label("Label"));
        EXPECT_THROW(annotation.add_hyperlink("Link2", query), unity::InvalidArgumentException); // only one hyperlink allowed
        EXPECT_EQ(1, annotation.num_of_hyperlinks());
    }
}

TEST(Annotation, serialize)
{
    {
        Query query("scope-A", "foo", "dep1");
        Annotation annotation(AnnotationType::Hyperlink);
        annotation.add_hyperlink("Link1", query);

        auto vm = annotation.serialize();
        EXPECT_EQ("hyperlink", vm["type"].get_string());
        EXPECT_TRUE(vm.find("label") == vm.end());
        EXPECT_TRUE(vm.find("icon") == vm.end());
        auto links = vm["hyperlinks"].get_array();
        EXPECT_EQ(1, links.size());
        auto linkvm = links[0].get_dict();
        EXPECT_EQ("Link1", linkvm["label"].get_string());
        Query qout(linkvm["query"].get_dict());
        EXPECT_EQ("scope-A", qout.scope_name());
        EXPECT_EQ("foo", qout.query_string());
        EXPECT_EQ("dep1", qout.department_id());
    }
}

TEST(Annotation, deserialize)
{
    CategoryRegistry reg;
    CategoryRenderer rdr;
    auto cat = reg.register_category("1", "title", "icon", rdr);
    Query query("scope-A", "foo", "dep1");
    {
        Annotation annotation(AnnotationType::GroupedHyperlink);
        annotation.set_label("Foo");
        annotation.add_hyperlink("Link1", query);
        auto var = annotation.serialize();
        AnnotationImpl impl(reg, var);
    }
    {
        Annotation annotation(AnnotationType::Hyperlink);
        annotation.set_icon("Icon");
        annotation.add_hyperlink("Link1", query);
        auto var = annotation.serialize();
        AnnotationImpl impl(reg, var);
    }
    {
        Annotation annotation(AnnotationType::Card);
        annotation.set_icon("Icon");
        annotation.set_category(cat);
        annotation.add_hyperlink("Link1", query);
        auto var = annotation.serialize();
        AnnotationImpl impl(reg, var);
    }
}

TEST(Annotation, deserialize_exceptions)
{
    {
        CategoryRegistry reg;
        CategoryRenderer rdr;
        Query query("scope-A", "foo", "dep1");
        auto cat = reg.register_category("1", "title", "icon", rdr);
        {
            VariantMap var;
            try
            {
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "";
            try
            {
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "hyperlink";
            try
            {
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "groupedhyperlink";
            try
            {
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {
            VariantMap var;
            var["type"] = "card";
            try
            {
                AnnotationImpl impl(reg, var);
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
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {   // deserialize with unknown category
            Annotation annotation(AnnotationType::Card);
            annotation.set_icon("Icon");
            annotation.set_category(cat);
            annotation.add_hyperlink("Link1", query);
            auto var = annotation.serialize();
            var["cat_id"] = "2";
            try
            {
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
        {   // deserialize with empty hyperlinks array
            Annotation annotation(AnnotationType::Hyperlink);
            annotation.add_hyperlink("Link1", query);
            auto var = annotation.serialize();
            var["hyperlinks"] = VariantArray();
            try
            {
                AnnotationImpl impl(reg, var);
                FAIL();
            }
            catch (unity::InvalidArgumentException const& e) {}
        }
    }
}

TEST(Annotation, copy)
{
    {
        Query query("scope-A", "foo", "dep1");
        Annotation annotation(AnnotationType::GroupedHyperlink);
        annotation.set_label("Group");
        annotation.add_hyperlink("Link1", query);
        Annotation copy(annotation);

        EXPECT_EQ("Group", copy.label());
        EXPECT_EQ(AnnotationType::GroupedHyperlink, copy.annotation_type());
        annotation.add_hyperlink("Link2", query);
        EXPECT_EQ(1, copy.num_of_hyperlinks());
    }
}
