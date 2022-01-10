#include "Arsenal21.h"

std::string readFileToString(const char* file) {
	std::ifstream fileh;
	fileh.open(file, std::ios::in);
	if (fileh.rdstate() != std::ios_base::goodbit) {
		fprintf(stderr, "File read failed ");
	}
	std::string r((std::istreambuf_iterator<char>(fileh)), (std::istreambuf_iterator<char>()));
	fileh.close();
	return r;
}

size_t getFileSize(const char* file) {
	try { return std::filesystem::file_size(std::filesystem::path(file)); }
	catch (std::filesystem::filesystem_error e) {
		try { return std::filesystem::file_size(std::filesystem::current_path().append(file)); }
		catch (std::filesystem::filesystem_error e) { return 0; }
	}
}
uint32_t randnum() {
	std::random_device random_device;
	std::mt19937 random_engine(random_device());
	std::uniform_int_distribution<uint32_t> dist(1, 4294967295);
	return dist(random_engine);
}
void timestamp() {
	auto hr = std::chrono::high_resolution_clock::now();
	auto sc = std::chrono::system_clock::now().time_since_epoch();
	auto ms = std::chrono::duration_cast<std::chrono::microseconds>(sc);
	auto sec = std::chrono::duration_cast<std::chrono::seconds>(sc);
	std::cout << sec.count() << '.' << ms.count() - (sec.count() * 1000000) << std::endl;
}