//---------------------------------------------------------
// Copyright (C) D4Games Inc. All rights reserved.

// ********************************************************
// File name : json-log.cpp
// Created   : 2017/09/27 by SungHoon Ko
// ********************************************************
//
// Title : Json to CSV Converter in Windows command mode.
//

#include "stdafx.h"
#include <boost/filesystem.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>
#include "json.hpp"

namespace fs = boost::filesystem;
using std::string;
using json = nlohmann::json;

string current_path() {
	return fs::current_path().generic_string();
}

string get_path(string pathName) {
	pathName.insert(pathName.begin(), '/');
	fs::path current_depth = fs::current_path().append(pathName);
	fs::path parent_depth = fs::current_path().parent_path().append(pathName);

	if (fs::exists(current_depth)) {
		return current_depth.generic_string();
	}
	else if (fs::exists(parent_depth)) {
		return parent_depth.generic_string();
	}

	return{};
}

int main(int argc, char *argv[]) {	
	if (argc < 2) {
		std::cout << "D4Games Json to CSV Converter Ver.0.6.171013" << std::endl;
		std::cout << "Usage : <Folder Name> \n" << "ex) json-log SampleFolder" << std::endl;
		return 0;
	}

	std::string pathName(argv[1]);

	auto argPath = get_path(pathName);
	if (argPath.empty()) {
		std::cout << "Invalid path name - " << pathName << std::endl;
		std::cout << "Current path - " << current_path() << std::endl;
		return 0;
	}

	fs::path p(argPath);
	auto dirEntryList = [=] { return boost::make_iterator_range(fs::directory_iterator(p), {}); };
	auto unaryFunc = [](fs::directory_entry const& e) { return e.path().string(); };
	std::vector<std::string> fileList;
	
	std::cout << "D4Games Json to CSV Converter Ver.0.6.171013" << std::endl;
	std::cout << "D4Games Log Converter Start (Json to CSV)" << std::endl;
	std::cout << "Please wait for Check to " << p << " folder contents..." << std::endl;
	boost::transform(dirEntryList(), std::back_inserter(fileList), unaryFunc);
	std::cout << "Folder : " << p << "\t" << "Total Files : " << fileList.size() << std::endl << std::flush;	

	if (fileList.empty()) {
		std::cout << p << " is empty." << std::endl;
		return 0;
	}

	std::this_thread::sleep_for(std::chrono::seconds(1));
	
	std::vector<std::string> csv;
	std::stringstream row;
	bool header = true;

	long maxCount = fileList.size();
	long curCount = 0;
	long mod = maxCount / 100;
	mod = mod > 0 ? mod : 1;

	try {
		csv.reserve(240);
		std::cout << "\nMemory allocated : " << csv.capacity() << '\n' << std::endl;
	}
	catch (const std::bad_alloc& e) {
		std::cout << "Error : Memory allocate fail. - " << e.what() << std::endl;
		return 0;
	}	


	auto csvFileName = pathName + "-log.csv";
	csvFileName.insert(0, "./");
	std::ofstream saveFile(csvFileName, std::ofstream::out | std::ofstream::trunc);

	for (const auto& fileName : fileList) {
		if (curCount % mod == 0) {
			std::cout << "In progress -> " << curCount << " / " << maxCount << std::endl;
		}
		++curCount;

		std::ifstream ifStream;
		std::stringstream strStream;
		ifStream.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			ifStream.open(fileName);
			strStream.str("");
			strStream << ifStream.rdbuf();
			ifStream.close();
		}
		catch (std::ifstream::failure e) {
			std::cout << "Error : File Stream Fail - " << e.what() << std::endl;
			break;
		}
		
		json log;
		try {
			log << strStream;
		}
		catch (std::exception& e) {
			std::cout << "message: " << e.what() << std::endl;
			continue;
		}

		if (log.empty()) continue;

		for (auto& item : json::iterator_wrapper(log)) {
			if (header)
				row << item.key();
			else
				row << item.value();
			
			try {
				csv.emplace_back(row.str());
				csv.emplace_back(std::string("\t"));
			}
			catch (const std::bad_alloc& e) {
				std::cout << "Error : Memory allocate fail. - " << e.what() << std::endl;
				return 0;
			}
			row.str("");
		}

		std::string& endToken = (*(csv.rbegin()));
		if (endToken.compare("\t") == 0)
			endToken.assign("\n");

		if (header)
			header = false;

		std::copy(csv.begin(), csv.end(), std::ostream_iterator<std::string>(saveFile));
		csv.clear();
	}
	
	std::cout << "Complete save file - " << current_path() << "/" << pathName + "-log.csv" << std::endl;

	return 0;
}

