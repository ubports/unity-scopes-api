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
#pragma once

#include <unity/util/DefinesPtrs.h>

#include <QtCore/QVariantMap>

namespace unity
{

namespace scopes
{

namespace qt
{

class QPreviewQueryBaseAPI;
class QScopeVariant;

namespace internal
{
class QActionMetadataImpl;
class QPreviewQueryBaseImpl;
}

/**
\brief Metadata passed to scopes for preview and activation.
\see unity::scopes::qt::QScopeBase::preview, unity::scopes::ScopeBase::activate,
unity::scopes::ScopeBase::perform_action
*/
class QActionMetadata final
{
public:
    /// @cond
    UNITY_DEFINES_PTRS(QActionMetadata);

    ~QActionMetadata();
    /// @endcond

    /**
    \brief Create ActionMetadata with the given locale and form factor.
    \param locale locale string, eg. en_EN
    \param form_factor form factor name, e.g. phone, desktop, phone-version etc.
    */
    QActionMetadata(QString const& locale, QString const& form_factor);

    /**
     \brief Attach arbitrary data to this ActionMetadata.
     \param data The data value to attach.
     */
    void set_scope_data(QVariant const& data);

    /**
     \brief Get data attached to this ActionMetadata.
     \return The attached data, or QVariant::null.
     */
    QVariant scope_data() const;

    /**@name Copy and assignment
    Copy and assignment operators (move and non-move versions) have the usual value semantics.
    */
    //{@
    QActionMetadata(QActionMetadata const& other);
    QActionMetadata(QActionMetadata&&);

    QActionMetadata& operator=(QActionMetadata const& other);
    QActionMetadata& operator=(QActionMetadata&&);
    //@}

    /**
    \brief Sets a hint.

    \param key The name of the hint.
    \param value Hint value
    */
    void set_hint(QString const& key, QVariant const& value);

    /**
    \brief Get all hints.

    \return Hints dictionary.
    \throws unity::NotFoundException if no hints are available.
    */
    QVariantMap hints() const;

    /**
    \brief Check if this SearchMetadata has a hint.
    \param key The hint name.
    \return True if the hint is set.QScopeVariant
    */
    bool contains_hint(QString const& key) const;

    /**
    \brief Returns a reference to a hint.

    This method can be used to read or set hints. Setting a value of an existing hint overwrites
    its previous value.
    Referencing a non-existing hint automatically creates it with a default value of QVariant::Type::Null.
    \param key The name of the hint.
    \return A reference to the hint.
    */
    QVariant& operator[](QString const& key);

    /**
    \brief Returns a const reference to a hint.

    This method can be used for read-only access to hints.
    Referencing a non-existing hint throws unity::InvalidArgumentException.
    \param key The name of the hint.
    \return A const reference to the hint.
    \throws unity::NotFoundException if no hint with the given name exists.
    */
    QVariant const& operator[](QString const& key) const;

private:
    /// @cond
    QActionMetadata(internal::QActionMetadataImpl* impl);

    std::unique_ptr<internal::QActionMetadataImpl> p;
    friend class internal::QActionMetadataImpl;
    friend class internal::QPreviewQueryBaseImpl;
    friend class QPreviewQueryBaseAPI;
    /// @endcond
};

}  // namespace qt

}  // namespace scopes

}  // namespace unity
