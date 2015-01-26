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

#include <unity/scopes/qt/internal/QResultImpl.h>
#include <unity/scopes/qt/internal/QScopeVariant.h>

#include <unity/scopes/qt/QResult.h>
#include <unity/scopes/qt/QUtils.h>

#include <unity/scopes/Result.h>

using namespace unity::scopes;
using namespace unity::scopes::qt;
using namespace unity::scopes::qt::internal;

class APIResult : public Result
{
public:
    APIResult(APIResult const& other)
        : Result(other){};

    APIResult(APIResult&&) = default;

    APIResult& operator=(APIResult const& other)
    {
        if (this != &other)
        {
            *this = other;
        }
        return *this;
    }

    APIResult& operator=(APIResult&&) = default;

    APIResult(const QVariantMap& variant_map)
        : Result(qVariantMapToScopeVariantMap(variant_map)){};

    static VariantMap getValidEmptyMap()
    {
        VariantMap retMap;
        VariantMap emptyMap;
        VariantMap attrsMap;

        attrsMap["uri"] = Variant(std::string());
        retMap["internal"] = emptyMap;
        retMap["attrs"] = attrsMap;

        return retMap;
    }
};

QResultImpl::QResultImpl(unity::scopes::Result* api_result)
    : api_result_(api_result)
{
}

QResultImpl::QResultImpl(const QVariantMap& variant_map)
    : api_result_(new APIResult(variant_map))
{
}

QResultImpl::QResultImpl(internal::QResultImpl* impl)
    : api_result_(impl->api_result_.get())
{
}

QResultImpl::QResultImpl(QResultImpl const& other)
{
    other.sync_values();
    api_result_.reset(new Result(*other.api_result_.get()));
}

QResultImpl::QResultImpl(unity::scopes::Result const& other)
    : api_result_(new Result(other))
{
}

QResultImpl::~QResultImpl() = default;

void QResultImpl::store(QResult const& other, bool intercept_activation)
{
    api_result_->store(*other.p->api_result_, intercept_activation);
}

bool QResultImpl::has_stored_result() const
{
    return api_result_->has_stored_result();
}

QResult QResultImpl::retrieve() const
{
    return QResult(scopeVariantMapToQVariantMap(api_result_->retrieve().serialize()));
}

void QResultImpl::set_uri(QString const& uri)
{
    api_result_->set_uri(uri.toUtf8().data());
}

void QResultImpl::set_title(QString const& title)
{
    api_result_->set_title(title.toUtf8().data());
}

void QResultImpl::set_art(QString const& image)
{
    api_result_->set_art(image.toUtf8().data());
}

void QResultImpl::set_dnd_uri(QString const& dnd_uri)
{
    api_result_->set_dnd_uri(dnd_uri.toUtf8().data());
}

void QResultImpl::set_intercept_activation()
{
    api_result_->set_intercept_activation();
}

bool QResultImpl::direct_activation() const
{
    return api_result_->direct_activation();
}

ScopeProxy QResultImpl::target_scope_proxy() const
{
    return api_result_->target_scope_proxy();
}

QVariant& QResultImpl::operator[](QString const& key)
{
    sync_values();
    // look if the key already exists.
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_result_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    auto it = ret_variants_.find(key);

    unsync_variants_.push_back(it.value());
    return *it.value();
}

QVariant const& QResultImpl::operator[](QString const& key) const
{
    sync_values();
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_result_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_result_)[key.toUtf8().data()])));
    ret_variants_.insert(key, scopeVariant);

    return *ret_variants_.find(key).value();
}

QString QResultImpl::uri() const noexcept
{
    return QString::fromUtf8(api_result_->uri().c_str());
}

QString QResultImpl::title() const noexcept
{
    return QString::fromUtf8(api_result_->title().c_str());
}

QString QResultImpl::art() const noexcept
{
    return QString::fromUtf8(api_result_->art().c_str());
}

QString QResultImpl::dnd_uri() const noexcept
{
    return QString::fromUtf8(api_result_->dnd_uri().c_str());
}

bool QResultImpl::contains(QString const& key) const
{
    return api_result_->contains(key.toUtf8().data());
}

QVariant const& QResultImpl::value(QString const& key) const
{
    sync_values();
    if (ret_variants_.find(key) == ret_variants_.end())
    {
        std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_result_)[key.toUtf8().data()])));
        ret_variants_.insert(key, scopeVariant);
    }
    std::shared_ptr<QScopeVariant> scopeVariant(new QScopeVariant(&((*api_result_)[key.toUtf8().data()])));
    ret_variants_.insert(key, scopeVariant);

    return *ret_variants_.find(key).value();
}

QVariantMap QResultImpl::serialize() const
{
    sync_values();
    return scopeVariantMapToQVariantMap(api_result_->serialize());
}

void QResultImpl::sync_values() const
{
    QListIterator<std::shared_ptr<QScopeVariant>> it(unsync_variants_);
    while (it.hasNext())
    {
        it.next()->sync();
    }
    unsync_variants_.clear();
}
