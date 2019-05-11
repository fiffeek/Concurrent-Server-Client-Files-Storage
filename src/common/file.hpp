#ifndef SIK_ZAD2_FILE_HPP
#define SIK_ZAD2_FILE_HPP

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <string>
#include <vector>
#include "tcp_socket.hpp"
#include "type.hpp"

namespace sik::common {
    namespace {
        namespace fs = boost::filesystem;
    }

    class file {
    public:
        file(const fs::path &path)
                : file_path(path) {}

        void sendto(sik::common::tcp_socket& sock) {
            if (!fs::is_regular_file(file_path))
                throw std::runtime_error("Specified file is not a regular file");

            size_t bytes_sent = 0;
            size_t file_size = fs::file_size(file_path);
            open_file(OPEN_OPT);

            while (bytes_sent < file_size) {
                size_t left = file_size - bytes_sent;
                size_t this_chunks_size = (left < CHUNKS_SIZE) ? left : CHUNKS_SIZE;

                if (fread((void *) buff, 1, this_chunks_size, file_ptr) != this_chunks_size)
                    throw std::runtime_error("Could not read from file");

                if (!sock.write(buff, sock.get_sock(), this_chunks_size))
                    throw std::runtime_error("Could not write to the socket");

                bytes_sent += this_chunks_size;
            }
        }

        void createfrom(sik::common::tcp_socket& sock, const std::string& file_name) {
            fs::path single_path{file_path};
            single_path.append(file_name);

            open_file(CREATE_OPT, single_path);

            for (;;) {
                ssize_t rcv_bytes = 0;
                if ((rcv_bytes = sock.read(buff, sock.get_sock(), CHUNKS_SIZE)) < 0)
                    throw std::runtime_error("Could not read");

                if (rcv_bytes == sik::common::OK)
                    break;

                if (fwrite(buff, 1, rcv_bytes, file_ptr) != (size_t) rcv_bytes)
                    throw std::runtime_error("Could not write to the file");
            }
        }

        ~file() {
            fclose(file_ptr);
        };

    private:
        void open_file(const char* opts) {
           open_file(opts, file_path);
        }

        void open_file(const char* opts, const fs::path& path) {
            std::cout << file_path.string() << std::endl;
            file_ptr = fopen(path.c_str(), opts);

            if (file_ptr == nullptr)
                throw std::runtime_error("Could not open the file");
        }

        fs::path file_path;
        sik::common::byte buff[CHUNKS_SIZE];
        FILE* file_ptr;
    };
}

#endif //SIK_ZAD2_FILE_HPP
