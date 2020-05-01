// ************************************************************************** //
//
//  Model-view-view-model framework for large GUI applications
//
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @authors   see AUTHORS
//
// ************************************************************************** //

#ifndef MVVM_SIGNALS_ITEMLISTENERBASE_H
#define MVVM_SIGNALS_ITEMLISTENERBASE_H

#include <mvvm/core/export.h>
#include <mvvm/signals/callback_types.h>

namespace ModelView
{

class SessionItem;

//! Provides sets of methods to subscribe to various signals generated by SessionItem.
//! Used to implement user actions on item change.

//! Automatically tracks the time of life of SessionItem. Unsubscribes from the item on
//! own destruction. Can be switched from tracking one item to another of the same type.

class CORE_EXPORT ItemListenerBase
{
public:
    explicit ItemListenerBase(SessionItem* item = nullptr);
    virtual ~ItemListenerBase();

    void setItem(SessionItem* item);

    void setOnItemDestroy(Callbacks::item_t f);
    void setOnDataChange(Callbacks::item_int_t f);
    void setOnPropertyChange(Callbacks::item_str_t f);
    void setOnChildPropertyChange(Callbacks::item_str_t f);
    void setOnItemInserted(Callbacks::item_tagrow_t f);
    void setOnItemRemoved(Callbacks::item_tagrow_t f);
    void setOnAboutToRemoveItem(Callbacks::item_tagrow_t f);

protected:
    virtual void subscribe() {} //! For necessary manipulations on new item.
    virtual void unsubscribe() {} //! For necessary manipulations on unsubscription.
    SessionItem* item() const;

private:
    void unsubscribe_from_current();
    SessionItem* m_item{nullptr};
};

} // namespace ModelView

#endif // MVVM_SIGNALS_ITEMLISTENERBASE_H
