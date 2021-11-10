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
  const auto dirname_length = std::string(argv[1]).size() + 1;

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
    auto filename = notification.path.string();

    if (notification.event == inotify::Event::modify || notification.event == inotify::Event::create) {
      size_t current_size = std::filesystem::file_size(notification.path);

      // Lookup last known filesize for file (otherwise assume zero)
      std::unique_lock lock(mut);
      if (files.find(filename) == files.end()) {
        files[filename] = 0;
      }

      // Seek to the last known position
      std::ifstream fd(filename);
      if (current_size > files[filename]) {
        fd.seekg(files[filename]);
      }

      // Assume the file was truncated if the current file size is less than previously known (reset to beginning)
      if (current_size < files[filename]) {
        files[filename] = 0;
        fd.seekg(0);
        std::cout << filename.substr(dirname_length) << ": *** truncated ***" << std::endl;
      }

      // Keep reading as long as we're still behind the current files last position
      while (true) {
        size_t read = current_size - files[filename];
        if (read <= 0) {
          break;
        }
        std::vector<char> buffer;
        buffer.resize(read);
        size_t r = fd.readsome(buffer.data(), read);

        // Add read chunk of data to the buffer (note that we might read blocks of multiple- and/or partial lines)
        buffers[filename].insert(buffers[filename].end(), buffer.begin(), buffer.end());

        // Advance our pointer to what we've read just know
        files[filename] += r;

        // Keep printing lines as long as we have lines in the buffer.
        while (true) {
          auto pos = buffers[filename].find("\n");
          if (pos == std::string::npos) {
            break;
          }
          // Print prefixed with the basename of the file
          std::cout << filename.substr(dirname_length) << ": " << buffers[filename].substr(0, pos) << std::endl;
          buffers[filename] = buffers[filename].substr(pos + 1);
        }
      }
    } else if (notification.event == inotify::Event::remove) {
      files.erase(filename);
      buffers.erase(filename);
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
