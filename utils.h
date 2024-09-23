#pragma once

#include <sstream>
#include <sys/stat.h>
#include <vector>
#include <sys/types.h>

#ifdef _WIN32
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <windows.h>
#endif
#if defined(__linux__) || defined(__MINGW32__) || defined(__APPLE__)
#include <dirent.h>
#include <unistd.h>
#include <cstring>
#endif

namespace utils{
    /**
     * Check whether directory exists
     * @param path directory to be checked.
     * @return ture if directory exists, false otherwise.
     */
    static inline bool dirExists(const std::string& path){
        struct stat st{};
        int ret = stat(path.c_str(), &st);
        return ret == 0 && st.st_mode & S_IFDIR;
    }

    /**
     * list all filename in a directory
     * @param path directory path.
     * @param ret all files name in directory.
     * @return files number.
     */
#if defined(_WIN32) && !defined(__MINGW32__)
    static inline int scanDir(std::string path, std::vector<std::string> &ret){
        std::string extendPath;
        if(path[path.size() - 1] == '/'){
            extendPath = path + "*";
        }
        else{
            extendPath = path + "/*";
        }
        WIN32_FIND_DATAA fd;
        HANDLE h = FindFirstFileA(extendPath.c_str(), &fd);
        if(h == INVALID_HANDLE_VALUE){
            return 0;
        }
        while(true){
            std::string ss(fd.cFileName);
            if(ss[0] != '.'){
                ret.push_back(ss);
            }
            if(FindNextFile(h, &fd) ==false){
                break;
            }
        }
        FindClose(h);
        return ret.size();
    }
#endif
#if defined(__linux__) || defined(__MINGW32__) || defined(__APPLE__)
    static inline unsigned scanDir(const std::string& path, std::vector<std::string> &ret){
        DIR *dir;
        struct dirent *rent;
        dir = opendir(path.c_str());
        char s[100];
        while((rent = readdir(dir))){
            strcpy(s,rent->d_name);
            if (s[0] != '.'){
                ret.emplace_back(s);
            }
        }
        closedir(dir);
        return ret.size();
    }
#endif

    /**
     * Create directory
     * @param path directory to be created.
     * @return 0 if directory is created successfully, -1 otherwise.
     */
    static inline int _mkdir(const char *path){
#ifdef _WIN32
        return ::_mkdir(path);
#else
        return ::mkdir(path, 0775);
#endif
    }

    /**
     * Create directory recursively
     * @param path directory to be created.
     * @return 0 if directory is created successfully, -1 otherwise.
     */
    static inline int mkdir(const char *path){
        std::string currentPath;
        std::string dirName;
        std::stringstream ss(path);

        while (std::getline(ss, dirName, '/')){
            currentPath += dirName;
            if (!dirExists(currentPath) && _mkdir(currentPath.c_str()) != 0){
                return -1;
            }
            currentPath += "/";
        }
        return 0;
    }

    /**
     * Delete a empty directory
     * @param path directory to be deleted.
     * @return 0 if delete successfully, -1 otherwise.
     */
    static inline int rmdir(const char *path){
#ifdef _WIN32
        return ::_rmdir(path);
#else
        return ::rmdir(path);
#endif
    }

    /**
     * Delete a file
     * @param path file to be deleted.
     * @return 0 if delete successfully, -1 otherwise.
     */
    static inline int rmfile(const char *path){
#ifdef _WIN32
        return ::_unlink(path);
#else
        return ::unlink(path);
#endif
    }



}