#include <vector>
// RapidJSON
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include <rapidjson/writer.h>
//modern json
#include <nlohmann/json.hpp>
using json = nlohmann::json;

// metrics
class NetworkMetrics
{
public:
  std::string src_addr;
  std::string sent_packets;
  std::string received_packets;
  std::string lost_packets;
  std::string packet_delivery_ratio;
  std::string packet_loss;
  std::string delay;
  std::string jitter;
  std::string throughput;
};

// drone
class DroneData
{
public:
  unsigned id;
  NetworkMetrics metrics;
  // location
};

//time series
class TimeSeries
{
public:
  unsigned second;
  std::vector<DroneData> metrics;
  // location
};
