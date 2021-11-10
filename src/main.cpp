#include <filesystem>
#include <fstream>
#include <iostream>
#include <mutex>
#include <unordered_map>

#include <inotify-cpp/FileSystemAdapter.h>
#include <inotify-cpp/NotifierBuilder.h>

using namespace inotify;

int main(int argc, char** argv) {
  if (argc <= 1) {
    std::cout << "Usage: " << argv[0] << " <directory>" << std::endl;
    exit(0);
  }
  std::unordered_map<std::string, size_t> files;
  std::unordered_map<std::string, std::string> buffers;
  std::mutex mut;

  // Read all files in the directory and record all the file sizes
  using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
  for (const auto& entry : recursive_directory_iterator(argv[1])) {
    if (!entry.is_regular_file()) continue;
    files[entry.path().string()] = entry.file_size();
  }

  std::cout << "Initialized " << files.size() << " files." << std::endl;

  inotifypp::filesystem::path path(argv[1]);

  auto handleNotification = [&](Notification notification) {
    auto full_file_path = notification.path.string();
    auto filename = notification.path.filename().string();

    if (notification.event == inotify::Event::modify || notification.event == inotify::Event::create) {
      size_t current_size = std::filesystem::file_size(notification.path);

      // Lookup last known filesize for file (otherwise assume zero)
      std::unique_lock lock(mut);
      if (files.find(full_file_path) == files.end()) {
        files[full_file_path] = 0;
      }

      // Seek to the last known position
      std::ifstream fd(full_file_path);
      if (current_size > files[full_file_path]) {
        fd.seekg(files[full_file_path]);
      }

      // Assume the file was truncated if the current file size is less than previously known (reset to beginning)
      if (current_size < files[full_file_path]) {
        files[full_file_path] = 0;
        fd.seekg(0);
        std::cout << filename << ": *** truncated ***" << std::endl;
      }

      // Keep reading as long as we're still behind the current files last position
      while (true) {
        size_t read = current_size - files[full_file_path];
        if (read <= 0) {
          break;
        }
        std::vector<char> buffer;
        buffer.resize(read);
        size_t r = fd.readsome(buffer.data(), read);

        // Add read chunk of data to the buffer (note that we might read blocks of multiple- and/or partial lines)
        buffers[full_file_path].insert(buffers[full_file_path].end(), buffer.begin(), buffer.end());

        // Advance our pointer to what we've read just know
        files[full_file_path] += r;

        // Keep printing lines as long as we have lines in the buffer.
        while (true) {
          auto pos = buffers[full_file_path].find("\n");
          if (pos == std::string::npos) {
            break;
          }
          // Print prefixed with the basename of the file
          std::cout << filename << ": " << buffers[full_file_path].substr(0, pos) << std::endl;
          buffers[full_file_path] = buffers[full_file_path].substr(pos + 1);
        }
      }
    } else if (notification.event == inotify::Event::remove) {
      files.erase(full_file_path);
      buffers.erase(full_file_path);
    } else {
      std::cout << "Event " << notification.event << " on " << notification.path << " at "
                << notification.time.time_since_epoch().count() << " was triggered." << std::endl;
    }
  };

  auto handleUnexpectedNotification = [](Notification notification) {};
  auto events = {Event::create, Event::modify, Event::remove, Event::move};
  auto notifier = BuildNotifier()
                      .watchPathRecursively(path)
                      .onEvents(events, handleNotification)
                      .onUnexpectedEvent(handleUnexpectedNotification);

  std::cout << "Listening with inotify.. Press Control+C to stop the process." << std::endl << "---" << std::endl;

  notifier.run();

  return 0;
}
