#include <iostream>
#include <string>
#include <thread>
#include <fstream>
#include <filesystem>
#include <vector>
#include <mutex>
#include <regex>
#include "Windows.h"

// Copyright (c) 2021 https://penguin2233.gq, no warranty is provided

std::mutex coutMtx;
std::mutex queueMtx;
std::string searchTermInput;
std::vector<std::string> workQueue;
std::vector<std::string> searchResults;

bool searchF(std::string fileName, std::string searchTerm) {
	bool found = false;
	std::ifstream fileHandle(fileName);
	std::string searchBuffer;
	while (std::getline(fileHandle, searchBuffer)) {
		if (searchBuffer.find(searchTerm) != std::string::npos) {
			found = true;
			coutMtx.lock();
			std::cout << fileName << '\n';
			searchResults.push_back(fileName);
			coutMtx.unlock();
		}
	}
	return found;
}

void worker() {
	while (!workQueue.empty()) {
		std::string pathToFile;
		queueMtx.lock();
		pathToFile = workQueue.back();
		workQueue.pop_back();
		queueMtx.unlock();
		searchF(pathToFile, searchTermInput);
	}
}

int main() {
	std::cout << "cpp-mt-text-search, compiled at " << __DATE__ << ' ' << __TIME__ << ".\n\n";
	TCHAR currentWorkingDirectoryBuffer[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentWorkingDirectoryBuffer);
	std::wstring wStr = currentWorkingDirectoryBuffer;
	std::string searchPath = std::string(wStr.begin(), wStr.end());
	std::string pathInput;
	std::cout << "enter search directory path, leave empty for current working directory (" << searchPath << ").\n";
	std::getline(std::cin, pathInput);
	if (pathInput != "") {
		if (std::filesystem::exists(pathInput) == false) {
			std::cout << "path is not valid!\n";
			return 0;
		}
		else
			searchPath = pathInput;
	}
	std::cout << "enter search term\n";
	std::getline(std::cin, searchTermInput);
	if (searchTermInput == "") {
		std::cout << "nothing to search!\n";
		return 0;
	}
	std::vector<std::string> filesToSearch;
	std::cout << "begin search\n";
	for (const auto& entry : std::filesystem::directory_iterator(searchPath))
		filesToSearch.push_back(entry.path().string());
	workQueue = filesToSearch;
	std::cout << "begin threading\n";
	int numThreads = std::thread::hardware_concurrency() - 1; // = 2;
	std::vector<std::thread> threads;
	for (int i = 0; i < numThreads; i++)
	{
		std::cout << '\n';
		threads.push_back(std::thread(worker));
	}
	while (!workQueue.empty()) {
		Sleep(5000);
	}

	// goodbye
	for (int i = 0; i < numThreads; i++)
	{
		threads[i].join();
	}
	std::cout << "\ntype exit then press enter to exit, or type save then press enter to save search output to file \n";
	std::string postSearchDecide;
	std::cin >> postSearchDecide;
	if (postSearchDecide == "save") {
		std::ofstream resultsFileHandle("results.txt");
		for (int i = 0; i < searchResults.size(); i++)
		{
			resultsFileHandle << searchResults[i] << '\n';
		}
		resultsFileHandle.close();
	}
	return 0;
}