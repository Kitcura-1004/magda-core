#pragma once

#include <juce_core/juce_core.h>

#include <map>
#include <string>

namespace magda::i18n {

class TranslationManager {
  public:
    static TranslationManager& getInstance();

    void initialise(const std::string& localePreference);
    juce::String translate(const juce::String& source) const;
    juce::String translateMenuLabel(const juce::String& source) const;

    std::string getLocalePreference() const;
    std::string getResolvedLocale() const;

  private:
    TranslationManager() = default;

    std::string resolveLocale(const std::string& localePreference) const;
    void loadTranslationsFor(const std::string& resolvedLocale);

    std::string localePreference_ = "system";
    std::string resolvedLocale_ = "en";
    std::map<std::string, std::string> translations_;
};

inline juce::String tr(const juce::String& source) {
    return TranslationManager::getInstance().translate(source);
}

inline juce::String tr(const char* source) {
    return TranslationManager::getInstance().translate(source);
}

inline juce::String trMenuLabel(const juce::String& source) {
    return TranslationManager::getInstance().translateMenuLabel(source);
}

}  // namespace magda::i18n
