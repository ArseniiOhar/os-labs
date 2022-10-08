#pragma once
#include <filesystem>
class FileExplorer
{
public:
	std::filesystem::path path = std::filesystem::current_path();
	void goTo(std::filesystem::path path);
};

