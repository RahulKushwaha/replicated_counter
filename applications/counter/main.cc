//
// Created by Rahul  Kushwaha on 3/30/23.
//

#include<fstream>
#include<sstream>

#include "glog/logging.h"
#include "google/protobuf/text_format.h"
#include "log/server/proto/ServerConfig.pb.h"
#include "CounterAppServer.h"

using namespace rk::projects::server;

auto main(int argc, char *argv[]) -> int {
  if (argc < 2) {
    LOG(ERROR) << "config file not provided";
    return 1;
  }

  std::string fileName{argv[1]};
  std::ifstream file(fileName);

  if (!file) {
    LOG(INFO) << "file could not be opened";
    return 1;
  }

  std::ostringstream ss;
  ss << file.rdbuf();
  std::string fileContents = ss.str();

  ServerConfig srv1Config{};
  auto result =
      google::protobuf::TextFormat::ParseFromString(fileContents, &srv1Config);

  if (!result) {
    LOG(INFO) << "failed to parse config. quitting";
    return 1;
  }

  LOG(INFO) << "Server Config: " << srv1Config.DebugString();

  rk::projects::counter_app::CounterAppServer appServer{srv1Config};

  appServer.start().semi().get();

  std::this_thread::sleep_for(std::chrono::nanoseconds{
      std::numeric_limits<std::int64_t>::max()});

  return 0;
}