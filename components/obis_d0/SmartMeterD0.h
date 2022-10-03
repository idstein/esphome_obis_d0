#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <regex>
#include <string>

#include "esphome/components/uart/uart.h"
#include "esphome/core/component.h"

namespace esphome
{
    namespace obis_d0
    {
        // input string formats for float/int sensors
        enum ValueFormat : uint8_t
        {
            ValueFormat_Float,
            ValueFormat_Hex,
        };

        class ISmartMeterD0Sensor
        {
        public:
            virtual ~ISmartMeterD0Sensor() = default;

            virtual void publish_val(const std::string& value) = 0;
            virtual void publish_invalid() = 0;
            virtual bool has_timed_out() = 0;
            virtual const std::string& get_obis_code() const = 0;
        };

        class SmartMeterD0SensorBase : public ISmartMeterD0Sensor
        {
        public:
            SmartMeterD0SensorBase(std::string obis_code, std::string value_regex, int timeout_ms) :
                obis_code_{std::move(obis_code)},
                value_regex_{value_regex},
                timeout_{static_cast<uint32_t>(timeout_ms)}
            {
            }

            const std::string& get_obis_code() const override { return obis_code_; }

        protected:
            std::string obis_code_;

            bool check_value(const std::string& value);

            void reset_timeout_counter();
            bool has_timed_out() override;

        private:
            std::regex value_regex_;
            uint32_t lastUpdate_{0}; // in milliseconds
            const uint32_t timeout_; // in milliseconds
        };

        class SmartMeterD0 : public Component,
                             public uart::UARTDevice
        {
        public:
            SmartMeterD0();

            void setup() override
            {
                // nothing to do here
            }

            void loop() override;

            void register_sensor(ISmartMeterD0Sensor* sensor);

        protected:
            void reset();

            void search_start_of_telegram();
            void search_end_of_record();

            void parse_record();
            void parse_identification();
            void parse_obis();

        private:
            std::array<uint8_t, 150> buffer_;
            uint16_t length_{0}; // used bytes in buffer_

            using SearchFct = void (SmartMeterD0::*)();
            SearchFct search_{nullptr};

            std::map<std::string, ISmartMeterD0Sensor*> sensors_;
        };

    } // namespace obis_d0
} // namespace esphome
