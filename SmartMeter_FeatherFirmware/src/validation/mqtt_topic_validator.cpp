#include "mqtt_topic_validator.h"
#include "../helpers/string_helper.h"

namespace SmartMeter {

  bool MqttTopicValidator::is_valid(String value) {
    if (value == "") {
      this->validation_error("Value should not be empty.");
      return false;
    } else if (value[0] == '/') {
      this->validation_error("Value should not start with a leading slash '/'.");
      return false;
    } else if (value[value.length()-1] == '/') {
      this->validation_error("Value should not end with a trailing slash '/'.");
      return false;
    }

    value.toLowerCase();
    String validChars = "- /.@_";
    for (auto c : value) {
      if (!((c >= 'a' && c <= 'z') || (c >= '0' && c <= '9'))) {
        if (validChars.indexOf(c) < 0) {
          this->validation_error("Value should not contain special characters - only permitted: " + validChars + "");
          return false;
        }
      }
    }

    return true;
  }

};