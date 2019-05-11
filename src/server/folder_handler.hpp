#ifndef SIK_ZAD2_FOLDER_HANDLER_HPP
#define SIK_ZAD2_FOLDER_HANDLER_HPP

#include <string>
#include <vector>
#include <dirent.h>
#include <stdexcept>
#include <sys/stat.h>
#include <unordered_map>
#include <unordered_set>
#include <iostream>
#include "../common/const.h"
#include <boost/filesystem.hpp>

namespace sik::server {
    namespace {
        namespace fs = boost::filesystem;
    }

    class folder {
    private:
        void index_file(const fs::path& single_file) {
            if (fs::is_regular_file(single_file)) {
                std::string filename = single_file.filename().string();
                uint64_t single_file_size = fs::file_size(single_file);

                files.insert(filename);
                file_size[filename] = single_file_size;
                folder_size += single_file_size;
            }
        }

    public:
        folder(const std::string &folderName, uint64_t max_space)
        : folder_name(folderName), folder_size(0), max_space(max_space) {}

        void index_files() {
            files.clear();
            file_size.clear();
            folder_size = 0;

            const fs::path directory{folder_name};

            if (fs::exists(directory) && fs::is_directory(directory)) {
                for (const fs::directory_entry& dir : fs::directory_iterator{directory})
                    index_file(dir.path());

                if (max_space < folder_size)
                    throw std::runtime_error("Folder size exceeds the maximal server size");
            } else {
                throw std::runtime_error("Cannot open the given directory.");
            }
        }

        std::vector<std::string> filter_and_get_files(const std::string& filter) {
            std::vector<std::string> aux;

            std::for_each(
                    files.begin(),
                    files.end(),
                    [&] (const std::string& file) {
                      if (file.find(filter) != std::string::npos) {
                          aux.push_back(file);
                      }
                    }
                    );

            return aux;
        }

        uint64_t get_free_space() {
            return max_space - folder_size;
        }

        bool contains(const std::string& file) {
            return files.find(file) != files.end();
        }

        fs::path file_path(const std::string& file) {
            fs::path single_path{folder_name};
            return single_path.append(file);
        }

        void remove(const std::string& filename) {
            if (!contains(filename))
                throw std::runtime_error("File does not exist");

            fs::path single_path{folder_name};
            single_path.append(filename);

            if (::remove(single_path.string().c_str()) < 0)
                throw std::runtime_error("Could not remove the file");

            files.erase(filename);
            folder_size -= file_size[filename];
            file_size.erase(filename);
        }

        friend std::ostream& operator<<(std::ostream& os, folder& fldr);

    private:
        std::string folder_name;
        std::unordered_set<std::string> files;
        std::unordered_map<std::string, uint64_t> file_size;
        uint64_t folder_size;
        uint64_t max_space;
    };

    std::ostream& operator<<(std::ostream& os, folder& fldr) {
        os << "--- Single Folder --- \n";
        os << "Folder name: " << fldr.folder_name << "\n" <<
           "Folder size: " << fldr.folder_size << "\n" <<
           "Files: \n";

        for (const auto& file: fldr.files) {
            os << file << " " << fldr.file_size[file] << "\n";
        }

        os << "--- End of Folder --- \n";
        return os;
    }
}

#endif //SIK_ZAD2_FOLDER_HANDLER_HPP
