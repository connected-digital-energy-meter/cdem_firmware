/*
 * VIVES college Research
 *
 * Author Ronny Mees & Nico De Witte
 *
 * Class to decode the P1 telegram to data
 *
*/
#pragma once

#include "datagram.h"

namespace SmartMeter {

  class Decoder {

    private:
      enum class Datablock { FIRST, SECOND };

    public:
      // _buffer should contain null-terminated datagram
      static Datagram decode(char * buffer, size_t bufferlength);

    private:
      static String find_data_string(char * buffer, const char * key, bool hasUnit=true, unsigned int datablockNumber=1);
      
      // Actual parsing
      static bool parse_data_value(char * buffer, const char * key, double * value, bool hasUnit=true, unsigned int datablockNumber=1);
  };

};