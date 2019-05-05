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

namespace sik::server {
    class folder {
    private:
        using c_dirent = struct dirent;
        using c_stat = struct stat;
        using c_directory = DIR;

        void index_file(const std::string& single_file, c_dirent *dir) {
            c_stat st = {0};

            if (lstat(single_file.c_str(), &st) == sik::common::C_ERR) {
                throw std::runtime_error("Cannot stat the file in the given directory");
            }

            if (S_ISREG(st.st_mode)) {
                std::string filename = dir->d_name;

                files.insert(filename);
                file_size[filename] = (uint64_t) st.st_size;
                folder_size += (uint64_t) st.st_size;
            }
        }

    public:
        folder(const std::string &folderName, uint64_t max_space)
        : folder_name(folderName), folder_size(0), max_space(max_space) {}

        void index_files() {
            to_free = nullptr;

            c_directory *directory = opendir(folder_name.c_str());
            c_dirent *dir;

            if (directory) {
                to_free = directory;

                while ((dir = readdir(directory)) != nullptr) {
                    index_file(folder_name + "/" + dir->d_name, dir);
                }

                if (closedir(directory) == sik::common::OK) to_free = nullptr;
                else throw std::runtime_error("Cannot close the opened directory");

                if (max_space < folder_size)
                    throw std::runtime_error("Folder size exceeds the maximal server size");
            } else {
                throw std::runtime_error("Cannot open the given directory.");
            }
        }

        ~folder() {
            if (to_free != nullptr) {
                closedir(to_free); // does not matter if it ends with success here
            }
        }

        friend std::ostream& operator<<(std::ostream& os, folder& fldr);

    private:
        DIR *to_free; //< helper, to close the directory after indexing throws
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
