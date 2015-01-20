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

#include <unity/scopes/qt/QResult.h>
#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/qt/internal/QResultImpl.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;

/// @cond
QResult::QResult()
    : p(new internal::QResultImpl(QVariantMap()))
{
}

QResult::QResult(internal::QResultImpl* impl)
    : p(impl)
{
}

QResult::QResult(const QVariantMap& variant_map)
    : p(new internal::QResultImpl(variant_map))
{
}

QResult::QResult(unity::scopes::Result const& result)
    : p(new internal::QResultImpl(result))
{
}

QResult::QResult(QResult const& other)
    : p(new internal::QResultImpl(*(other.p)))
{
}

QResult::~QResult()
{
}

QResult& QResult::operator=(QResult const& other)
{
    if (this != &other)
    {
        p.reset(new internal::QResultImpl(*(other.p)));
    }
    return *this;
}

QResult::QResult(QResult&&) = default;

QResult& QResult::operator=(QResult&&) = default;

void QResult::store(QResult const& other, bool intercept_activation)
{
    p->store(other, intercept_activation);
}

bool QResult::has_stored_result() const
{
    return p->has_stored_result();
}

QResult QResult::retrieve() const
{
    return p->retrieve();
}

void QResult::set_uri(QString const& uri)
{
    p->set_uri(uri);
}

void QResult::set_title(QString const& title)
{
    p->set_title(title);
}

void QResult::set_art(QString const& image)
{
    p->set_art(image);
}

void QResult::set_dnd_uri(QString const& dnd_uri)
{
    p->set_dnd_uri(dnd_uri);
}

void QResult::set_intercept_activation()
{
    p->set_intercept_activation();
}

bool QResult::direct_activation() const
{
    return p->direct_activation();
}

ScopeProxy QResult::target_scope_proxy() const
{
    return p->target_scope_proxy();
}

QScopeVariant& QResult::operator[](QString const& key)
{
    return (*p)[key];
}

QVariant const& QResult::operator[](QString const& key) const
{
    return p->value(key);
}

QString QResult::uri() const noexcept
{
    return p->uri();
}

QString QResult::title() const noexcept
{
    return p->title();
}

QString QResult::art() const noexcept
{
    return p->art();
}

QString QResult::dnd_uri() const noexcept
{
    return p->dnd_uri();
}

bool QResult::contains(QString const& key) const
{
    return p->contains(key);
}

QVariant const& QResult::value(QString const& key) const
{
    return p->value(key);
}

QVariantMap QResult::serialize() const
{
    return p->serialize();
}
/// @endcond
