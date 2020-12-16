#include "datagram_publisher.h"

namespace SmartMeter {
  #define BASETOPIC "sensors/smartmeter/"     // TODO - Config option!

  #define Debug(...) if(this->debugSerial) this->debugSerial->print(__VA_ARGS__); 
  #define DebugLn(...) if(this->debugSerial) this->debugSerial->println(__VA_ARGS__); 

  DatagramPublisher::DatagramPublisher(const char * host, uint16_t port, HardwareSerial * debugSerial) {
    this->debugSerial = debugSerial;
    mqttClient.setServer(host, port);
    setup_callbacks();

    // Pass instance pointer as ID to timer which can only be used with function/static method callbacks
    // Bit of a hack
    reconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(RECONNECT_TIME_MS), pdFALSE,
      (void*)this, &DatagramPublisher::reconnect_timer_callback);
  }

  void DatagramPublisher::setup_callbacks(void) {

    // std::function<void(bool sessionPresent)> OnConnectUserCallback;
    AsyncMqttClientInternals::OnConnectUserCallback onConnected
      = std::bind(&DatagramPublisher::on_connected, this, std::placeholders::_1 );
    
    mqttClient.onConnect(onConnected);

    // typedef std::function<void(AsyncMqttClientDisconnectReason reason)> OnDisconnectUserCallback;
    AsyncMqttClientInternals::OnDisconnectUserCallback onDisconnected
      = std::bind(&DatagramPublisher::on_disconnected, this, std::placeholders::_1 );
    
    mqttClient.onDisconnect(onDisconnected);
  }

  void DatagramPublisher::connect(void) {
    _shouldBeConnected = true;
    DebugLn("DGP - Connecting to MQTT broker");
    mqttClient.connect();
  }

  void DatagramPublisher::disconnect(void) {
    DebugLn("DGP - Disconnecting fromt MQTT broker");
    stop_reconnect_timer();
    _shouldBeConnected = false;
    mqttClient.disconnect();
  }

  void DatagramPublisher::on_connected(bool sessionPresent) {
    stop_reconnect_timer();
    _connected = true;
    DebugLn("DGP - Connected to MQTT broker");
  }

  void DatagramPublisher::on_disconnected(AsyncMqttClientDisconnectReason reason) {
    _connected = false;
    DebugLn("DGP - Disconnected from MQTT broker");
    if (_shouldBeConnected) {
      start_reconnect_timer();
    }
  }

  void DatagramPublisher::reconnect_timer_callback(TimerHandle_t timer) {
    auto self = (DatagramPublisher*)pvTimerGetTimerID(timer);
    self->connect();
  }

  void DatagramPublisher::start_reconnect_timer(void) {
    xTimerStart(reconnectTimer, 0);
  }

  void DatagramPublisher::stop_reconnect_timer(void) {
    xTimerStop(reconnectTimer, 0);
  }

  void DatagramPublisher::publish(Datagram * datagram) {
    std::vector<String> keys = datagram->keys();
    for (String key : keys) {
      String topic = BASETOPIC + key;
      String data = String(datagram->get(key));
      mqttClient.publish(topic.c_str(), 1, true, data.c_str());
    }
  }

};