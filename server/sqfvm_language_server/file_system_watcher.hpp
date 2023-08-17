//
// Created by marco.silipo on 14.08.2023.
//

#ifndef SQFVM_LANGUAGE_SERVER_FILE_SYSTEM_WATCHER_HPP
#define SQFVM_LANGUAGE_SERVER_FILE_SYSTEM_WATCHER_HPP

#include <Poco/DirectoryWatcher.h>
#include <unordered_map>
#include <filesystem>
#include <functional>
#include <mutex>

namespace sqfvm::language_server {
    class file_system_watcher {
        std::unordered_map<std::filesystem::path, std::shared_ptr<Poco::DirectoryWatcher>> m_watchers;
        std::mutex m_watchers_mutex;
        std::function<void(const std::filesystem::path &, bool is_directory)> m_callback_add;
        std::function<void(const std::filesystem::path &, bool is_directory)> m_callback_remove;
        std::function<void(const std::filesystem::path &, bool is_directory)> m_callback_modify;

        void watch_directory(const std::filesystem::path &path);

        void unwatch_directory(const std::filesystem::path &path);

    public:
        void watch(const std::filesystem::path &path);

        bool unwatch(const std::filesystem::path &path);

        void callback_add(std::function<void(const std::filesystem::path &, bool is_directory)> callback);

        void callback_remove(std::function<void(const std::filesystem::path &, bool is_directory)> callback);

        void callback_modify(std::function<void(const std::filesystem::path &, bool is_directory)> callback);

        void handle_item_added(const Poco::DirectoryWatcher::DirectoryEvent &);

        void handle_item_removed(const Poco::DirectoryWatcher::DirectoryEvent &);

        void handle_item_modified(const Poco::DirectoryWatcher::DirectoryEvent &);
    };
}

#endif //SQFVM_LANGUAGE_SERVER_FILE_SYSTEM_WATCHER_HPP
