#include "repo.h"
#include "artifact.h"
#include "manifest.h"

#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Create new repo directory.
// Recursively iterate through entire project tree and copy contents to destination repo.
// Case to check if path is a file.
void createRepo(const std::string& root, const std::string& dst) {
	fs::path repo = dst;
	auto manifest_path = getManifestPath(repo);
	createManifest(manifest_path, root, dst);

	for (auto &content : fs::recursive_directory_iterator(root)) {
		auto repo_content = repo / fs::relative(content, root);

		fs::create_directories(repo_content);

		if (fs::is_regular_file(content.path())) {
			fs::path repo_content_file = repo_content / content.path().filename();
			fs::copy_file(content.path().string(), repo_content_file.string());
			
			std::fstream repo_content_file_contents(repo_content_file.string());

			auto id = getArtifactID(repo_content_file, repo_content_file_contents);

			repo_content_file_contents.close();

			fs::rename(repo_content_file.string(), id.string());

			writeToManifest(manifest_path, fs::relative(id, repo));
		}
	}
}

// src argument is manifest path
// dst is target repo path
void checkout(const std::string& src, const std::string& dst) {
	fs::path potential_manifest = src;
	fs::path target_repo = dst;
	
	if (fs::exists(potential_manifest)) {
		std::cout << "checkout from dst\n\n";
	}
}

// overloaded if user wants label as an argument
void checkout( const std::string& src, const std::string& dst, const std::string& label ) {
	fs::path potential_manifest = src;
	fs::path target_repo = dst;
	fs::path manifest_path = findManifestByLabel( src, label );
	fs::create_directory( dst );
	if( fs::is_empty( manifest_path ) ) {
		std::cerr << "Label does not exist!" << '\n';
	}
	else {
		// Copy everything from parent path of manifest
		fs::copy( manifest_path.parent_path(), target_repo, fs::copy_options::recursive );
		fs::path checkout_manifest = target_repo / manifest_path.filename();
		// Insert checkout commands to copied manifest
		std::ofstream temp( "temp.txt" );
		std::ifstream manifest( checkout_manifest );

		temp << "check-out Arguments: " << src << ' ' << dst << ' ' << label << '\n';
		temp << manifest.rdbuf();

		manifest.close();
		temp.close();

		fs::remove( checkout_manifest );
		fs::rename( "temp.txt", checkout_manifest);
	}
}