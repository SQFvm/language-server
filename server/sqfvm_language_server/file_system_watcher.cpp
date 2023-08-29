//
// Created by marco.silipo on 14.08.2023.
//

#include "file_system_watcher.hpp"
#include "Poco/Delegate.h"

#include <utility>
#include <cassert>

void sqfvm::language_server::file_system_watcher::watch_directory(
        const std::filesystem::path &path) {
    if (!std::filesystem::is_directory(path)) {
        throw std::runtime_error(path.string() + " is not a directory");
    }
    auto lock = std::lock_guard(m_watchers_mutex);
    if (m_watchers.find(path) != m_watchers.end()) {
        throw std::runtime_error("Already watching " + path.string());
    }
    auto watcher = std::make_shared<Poco::DirectoryWatcher>(
            path.string(),
            Poco::DirectoryWatcher::DW_FILTER_ENABLE_ALL,
            2048);
    watcher->itemAdded += Poco::delegate(
            this,
            &sqfvm::language_server::file_system_watcher::handle_item_added);
    watcher->itemRemoved += Poco::delegate(
            this,
            &sqfvm::language_server::file_system_watcher::handle_item_removed);
    watcher->itemModified += Poco::delegate(
            this,
            &sqfvm::language_server::file_system_watcher::handle_item_modified);
    m_watchers[path] = watcher;
}

void sqfvm::language_server::file_system_watcher::unwatch_directory(
        const std::filesystem::path &path) {
    auto lock = std::lock_guard(m_watchers_mutex);
    auto find_res = m_watchers.find(path);
    if (find_res == m_watchers.end()) {
        throw std::runtime_error("Not watching " + path.string());
    }
    auto watcher = find_res->second;
    watcher->itemAdded -= Poco::delegate(
            this,
            &sqfvm::language_server::file_system_watcher::handle_item_added);
    watcher->itemRemoved -= Poco::delegate(
            this,
            &sqfvm::language_server::file_system_watcher::handle_item_removed);
    watcher->itemModified -= Poco::delegate(
            this,
            &sqfvm::language_server::file_system_watcher::handle_item_modified);
    m_watchers.erase(find_res);
    assert(m_watchers.find(path) == m_watchers.end());
}

void sqfvm::language_server::file_system_watcher::callback_add(
        std::function<void(const std::filesystem::path &, bool is_directory)> callback) {
    m_callback_add = std::move(callback);
}

void sqfvm::language_server::file_system_watcher::callback_remove(
        std::function<void(const std::filesystem::path &, bool is_directory)> callback) {
    m_callback_remove = std::move(callback);
}

void sqfvm::language_server::file_system_watcher::callback_modify(
        std::function<void(const std::filesystem::path &, bool is_directory)> callback) {
    m_callback_modify = std::move(callback);
}

void sqfvm::language_server::file_system_watcher::handle_item_added(
        const Poco::DirectoryWatcher::DirectoryEvent &event) {
    std::filesystem::path path = event.item.path();

    auto is_directory = std::filesystem::is_directory(path);
    if (is_directory) {
        watch(path);
    }

    if (m_callback_add)
        m_callback_add(path, is_directory);
}

void sqfvm::language_server::file_system_watcher::handle_item_removed(
        const Poco::DirectoryWatcher::DirectoryEvent &event) {
    std::filesystem::path path = event.item.path();

    auto is_directory =  unwatch(path);

    if (m_callback_remove)
        m_callback_remove(path, is_directory);
}

void sqfvm::language_server::file_system_watcher::handle_item_modified(
        const Poco::DirectoryWatcher::DirectoryEvent &event) {
    std::filesystem::path path = event.item.path();
    if (m_callback_modify)
        m_callback_modify(path, is_directory(path));
}

void sqfvm::language_server::file_system_watcher::watch(
        const std::filesystem::path &path) {
    if (!std::filesystem::is_directory(path))
        return;
    watch_directory(path);
    for (auto &p: std::filesystem::recursive_directory_iterator(path)) {
        if (std::filesystem::is_directory(p))
            watch_directory(p);
    }
}

bool sqfvm::language_server::file_system_watcher::unwatch(
        const std::filesystem::path &path) {
    std::vector<std::filesystem::path> to_unwatch;
    {
        auto lock = std::lock_guard(m_watchers_mutex);
        for (auto &p: m_watchers) {
            if (p.first.string().starts_with(path.string()))
                to_unwatch.push_back(p.first);
        }
    }
    for (auto &p: to_unwatch)
        unwatch_directory(p);
    return !to_unwatch.empty();
}