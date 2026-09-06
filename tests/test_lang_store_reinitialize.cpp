#include <imguix/core/i18n.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

    void require(const bool condition, const char* message) {
        if (!condition) {
            throw std::runtime_error(message);
        }
    }

    void write_text(const std::filesystem::path& path, const std::string& content) {
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("Unable to create localization test file");
        }
        output << content;
    }

    struct TemporaryResources final {
        std::filesystem::path root =
            std::filesystem::temp_directory_path() / "imguix-lang-store-reinitialize";

        TemporaryResources() {
            std::filesystem::remove_all(root);
            std::filesystem::create_directories(root / "a" / "en");
            std::filesystem::create_directories(root / "a" / "ru");
            std::filesystem::create_directories(root / "b" / "en");
            std::filesystem::create_directories(root / "b" / "ru");
        }

        ~TemporaryResources() {
            std::error_code error;
            std::filesystem::remove_all(root, error);
        }
    };

}  // namespace

int main() {
    try {
        TemporaryResources resources;
        const auto base_a = resources.root / "a";
        const auto base_b = resources.root / "b";

        write_text(
            base_a / "en" / "strings.json",
            R"({"Greeting":"A-en","Help":"A help"})");
        write_text(
            base_a / "ru" / "strings.json",
            R"({"Greeting":"A-ru","Help":"A help ru"})");
        write_text(base_a / "ru" / "help.md", "A markdown");
        write_text(
            base_a / "plurals.json",
            R"({"ru":{"cardinal":[{"cat":"one","eq":2},{"cat":"other","true":true}]}})");

        write_text(
            base_b / "en" / "strings.json",
            R"({"Greeting":"B-en","Help":"B help"})");
        write_text(
            base_b / "ru" / "strings.json",
            R"({"Greeting":"B-ru","Help":"B help ru"})");
        write_text(base_b / "ru" / "help.md", "B markdown");
        write_text(
            base_b / "plurals.json",
            R"({"ru":{"cardinal":[{"cat":"one","eq":7},{"cat":"other","true":true}]}})");

        ImGuiX::I18N::LangStore store(base_a.u8string());
        store.set_language("ru");
        require(store.language() == "ru", "Language switch must select Russian.");
        require(store.text("Greeting") == "A-ru", "Initial language resources must load.");
        require(std::string(store.label("Greeting")) == "A-ru##Greeting",
                "Initial label cache must use the active language.");
        require(store.doc("help") == "A markdown", "Initial markdown must load.");
        require(store.plural_suffix(2) == "one", "Initial plural rules must load.");

        store.set_base_dir(base_b.u8string());
        require(store.language() == "ru", "Changing the base directory must preserve language.");
        require(store.text("Greeting") == "B-ru", "New language resources must be active.");
        require(std::string(store.label("Greeting")) == "B-ru##Greeting",
                "Label cache must be rebuilt after a base-directory change.");
        require(store.doc("help") == "B markdown",
                "Markdown cache must be rebuilt after a base-directory change.");
        require(store.plural_suffix(7) == "one",
                "Plural rules must be loaded from the new base directory.");

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "test_lang_store_reinitialize failed: " << error.what() << '\n';
        return 1;
    }
}
