//
// Created by Rahul  Kushwaha on 3/26/23.
//
#include "server/proto/ServerConfig.pb.h"
#include "google/protobuf/text_format.h"
#include "glog/logging.h"
#include<fstream>
#include<sstream>

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

  return 0;
}
