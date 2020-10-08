// ************************************************************************** //
//
//  Model-view-view-model framework for large GUI applications
//
//! @license   GNU General Public License v3 or higher (see COPYING)
//! @authors   see AUTHORS
//
// ************************************************************************** //

#include <QJsonArray>
#include <QJsonObject>
#include <mvvm/model/sessionitem.h>
#include <mvvm/model/sessionitemcontainer.h>
#include <mvvm/model/sessionitemtags.h>
#include <mvvm/model/tagrow.h>
#include <mvvm/serialization/compatibilityutils.h>
#include <mvvm/serialization/jsonitem_types.h>
#include <mvvm/serialization/jsonitemcontainerconverter.h>
#include <mvvm/serialization/jsonitemformatassistant.h>
#include <mvvm/serialization/jsontaginfoconverter.h>

using namespace ModelView;

struct JsonItemContainerConverter::JsonItemContainerConverterImpl {
    std::unique_ptr<JsonTagInfoConverterInterface> m_taginfo_converter;
    ConverterCallbacks m_converter_callbacks;

    JsonItemContainerConverterImpl(ConverterCallbacks callbacks = {})
        : m_converter_callbacks(std::move(callbacks))
    {
        m_taginfo_converter = std::make_unique<JsonTagInfoConverter>();
    }

    QJsonObject create_json(const SessionItem& item)
    {
        return m_converter_callbacks.m_create_json ? m_converter_callbacks.m_create_json(item)
                                                   : QJsonObject();
    }

    std::unique_ptr<SessionItem> create_item(const QJsonObject& json)
    {
        return m_converter_callbacks.m_create_item ? m_converter_callbacks.m_create_item(json)
                                                   : std::unique_ptr<SessionItem>();
    }

    void update_item(const QJsonObject& json, SessionItem* item)
    {
        if (m_converter_callbacks.m_update_item)
            m_converter_callbacks.m_update_item(json, item);
    }

    void process_single_property_tag(const QJsonObject& json, SessionItemContainer& container)
    {
        for (const auto obj : json[JsonItemFormatAssistant::itemsKey].toArray())
            update_item(obj.toObject(), container.itemAt(0));
    }

    void process_universal_property_tag(const QJsonObject& json, SessionItemContainer& container)
    {
        for (const auto obj : json[JsonItemFormatAssistant::itemsKey].toArray()) {
            if (auto item = create_item(obj.toObject()); item)
                container.insertItem(item.release(), container.itemCount());
        }
    }

    //! Populates container with content reconstructed from JSON object. Container must be empty.

    void populate_container(const QJsonObject& json, SessionItemContainer& container)
    {
        if (!container.empty())
            throw std::runtime_error(
                "Error in JsonItemContainerConverter: container is not empty.");

        TagInfo tagInfo =
            m_taginfo_converter->from_json(json[JsonItemFormatAssistant::tagInfoKey].toObject());

        if (tagInfo.name() != container.tagInfo().name())
            throw std::runtime_error("Error in JsonItemContainerConverter: attempt to update "
                                     "container from JSON representing another container.");

        for (const auto obj : json[JsonItemFormatAssistant::itemsKey].toArray()) {
            if (auto item = create_item(obj.toObject()); item)
                container.insertItem(item.release(), container.itemCount());
        }
    }
};

JsonItemContainerConverter::JsonItemContainerConverter(ConverterCallbacks callbacks)
    : p_impl(std::make_unique<JsonItemContainerConverterImpl>(std::move(callbacks)))
{
}

JsonItemContainerConverter::~JsonItemContainerConverter() = default;

QJsonObject JsonItemContainerConverter::to_json(const SessionItemContainer& container)
{
    QJsonObject result;
    result[JsonItemFormatAssistant::tagInfoKey] =
        p_impl->m_taginfo_converter->to_json(container.tagInfo());

    QJsonArray itemArray;
    for (auto item : container)
        itemArray.append(p_impl->create_json(*item));
    result[JsonItemFormatAssistant::itemsKey] = itemArray;

    return result;
}

// FIXME restore functionality

// void JsonItemContainerConverter::from_json(const QJsonObject& json, SessionItemContainer&
// container)
//{
//    static JsonItemFormatAssistant assistant;

//    if (!assistant.isSessionItemContainer(json))
//        throw std::runtime_error("Error in JsonItemContainerConverter: given JSON can't represent
//        "
//                                 "SessionItemContainer.");

//    TagInfo tagInfo = p_impl->m_taginfo_converter->from_json(
//        json[JsonItemFormatAssistant::tagInfoKey].toObject());

//    if (Compatibility::IsCompatibleSinglePropertyTag(container, tagInfo))
//        p_impl->process_single_property_tag(json, container);

//    else if (Compatibility::IsCompatibleUniversalTag(container, tagInfo))
//        p_impl->process_universal_property_tag(json, container);

//    else
//        throw std::runtime_error("Error in JsonItemContainerConverter: can't convert json");
//}

//! Reconstructs SessionItemContainer from the content of JSON object. Can work in two modes:
//! + If SessionItemContainer is empty, the content will be reconstructed from JSON
//! + If SessionItemContainer contains some items already, they will be populated from JSON.
//! Second mode is used when loading project from disk to allow back compatibility.

void JsonItemContainerConverter::from_json(const QJsonObject& json, SessionItemContainer& container)
{
    static JsonItemFormatAssistant assistant;

    if (!assistant.isSessionItemContainer(json))
        throw std::runtime_error("Error in JsonItemContainerConverter: given JSON can't represent "
                                 "SessionItemContainer.");

    if (!container.empty())
        throw std::runtime_error(
            "Error in JsonItemContainerConverter: intended for empty container.");

    p_impl->populate_container(json, container);
}
