#include "TranslationManager.hpp"

#include <BinaryData.h>

namespace magda::i18n {

TranslationManager& TranslationManager::getInstance() {
    static TranslationManager instance;
    return instance;
}

void TranslationManager::initialise(const std::string& localePreference) {
    localePreference_ = localePreference.empty() ? "system" : localePreference;
    resolvedLocale_ = resolveLocale(localePreference_);
    loadTranslationsFor(resolvedLocale_);
}

juce::String TranslationManager::translate(const juce::String& source) const {
    if (source.isEmpty())
        return source;

    auto it = translations_.find(source.toStdString());
    if (it == translations_.end())
        return source;

    return juce::String::fromUTF8(it->second.c_str());
}

juce::String TranslationManager::translateMenuLabel(const juce::String& source) const {
    auto tabIndex = source.indexOfChar('\t');
    if (tabIndex < 0)
        return translate(source);

    return translate(source.substring(0, tabIndex)) + source.substring(tabIndex);
}

std::string TranslationManager::getLocalePreference() const {
    return localePreference_;
}

std::string TranslationManager::getResolvedLocale() const {
    return resolvedLocale_;
}

std::string TranslationManager::resolveLocale(const std::string& localePreference) const {
    auto pref = juce::String(localePreference).trim();
    if (pref.isEmpty() || pref.equalsIgnoreCase("system")) {
        auto systemLanguage = juce::SystemStats::getUserLanguage().toLowerCase();
        if (systemLanguage.startsWith("zh"))
            return "zh-CN";
        return "en";
    }

    if (pref.equalsIgnoreCase("zh") || pref.equalsIgnoreCase("zh-cn") ||
        pref.equalsIgnoreCase("zh_cn")) {
        return "zh-CN";
    }

    return "en";
}

void TranslationManager::loadTranslationsFor(const std::string& resolvedLocale) {
    translations_.clear();

    if (resolvedLocale != "zh-CN")
        return;

    auto jsonText = juce::String::fromUTF8(BinaryData::lang_zh_CN_json,
                                           BinaryData::lang_zh_CN_jsonSize);
    juce::var parsed;
    auto result = juce::JSON::parse(jsonText, parsed);
    if (result.failed())
        return;

    auto* obj = parsed.getDynamicObject();
    if (obj == nullptr)
        return;

    for (const auto& property : obj->getProperties()) {
        translations_[property.name.toString().toStdString()] = property.value.toString().toStdString();
    }
}

}  // namespace magda::i18n
