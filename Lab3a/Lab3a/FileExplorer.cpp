#include <filesystem>
class FileExplorer {
public:
	std::filesystem::path path = std::filesystem::current_path();
	void goTo(std::filesystem::path p) {
		if (p.string().compare(".")) {
			if (p.string() == "..") {
				this->path = this->path.parent_path();
			}
			else {
				this->path = p;
			}

		}

	}

};
