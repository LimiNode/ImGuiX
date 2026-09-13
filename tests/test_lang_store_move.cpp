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

    void write_text(const std::filesystem::path& path, const std::string& value) {
        std::ofstream output(path, std::ios::binary);
        if (!output) {
            throw std::runtime_error("Unable to create localization fixture");
        }
        output << value;
    }

    struct Fixture final {
        std::filesystem::path root =
            std::filesystem::temp_directory_path() / "imguix-lang-store-move";

        Fixture() {
            std::filesystem::remove_all(root);
            std::filesystem::create_directories(root / "en");
            std::filesystem::create_directories(root / "ru");
            write_text(root / "en" / "strings.json", R"({"Greeting":"Hello"})");
            write_text(root / "ru" / "strings.json", R"({"Greeting":"Privet"})");
        }

        ~Fixture() {
            std::error_code error;
            std::filesystem::remove_all(root, error);
        }
    };

}  // namespace

int main() {
    try {
        Fixture fixture;

        ImGuiX::I18N::LangStore source(fixture.root.u8string());
        source.set_language("ru");
        require(source.text("Greeting") == "Privet", "Source language must load.");
        require(std::string(source.label("Greeting")) == "Privet##Greeting",
                "Source label cache must load.");

        ImGuiX::I18N::LangStore moved(std::move(source));
        require(moved.language() == "ru", "Move construction must preserve language.");
        require(moved.text("Greeting") == "Privet",
                "Move construction must rebind the active language map.");
        require(std::string(moved.label("Greeting")) == "Privet##Greeting",
                "Move construction must preserve caches safely.");
        require(source.text("Greeting") == "##null",
                "Moved-from store must remain valid and empty.");

        ImGuiX::I18N::LangStore assigned(fixture.root.u8string());
        assigned = std::move(moved);
        require(assigned.language() == "ru", "Move assignment must preserve language.");
        require(assigned.text("Greeting") == "Privet",
                "Move assignment must rebind the active language map.");
        require(moved.text("Greeting") == "##null",
                "Move-assigned source must remain valid and empty.");
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "test_lang_store_move failed: " << error.what() << '\n';
        return 1;
    }
}
