/*
* Brian R Taylor
* brian.taylor@bolderflight.com
* 
* Copyright (c) 2020 Bolder Flight Systems
*/

#include "ublox/ublox.h"
#include "Eigen/Core"
#include "Eigen/Dense"
#include "core/core.h"
#include "global_defs/global_defs.h"

namespace sensors {

Ublox::Ublox(HardwareSerial* bus) {
  bus_ = bus;
}
bool Ublox::Begin(uint32_t baud) {
  bus_->begin(baud);
  parser_pos_ = 0;
  ubx_nav_pvt_parsed_ = false;
  ubx_nav_hpposllh_parsed_ = false;
  use_high_precision_ = false;
  read_status_ = false;
  bool start = false;
  bus_->flush();
  elapsedMillis timer_ms = 0;
  while (timer_ms < TIMEOUT_MS_) {
    if (Parse()) {
      switch (msg_) {
        case UBX_NAV_PVT: {
          ubx_nav_pvt_parsed_ = true;
          break;
        }
        case UBX_NAV_HPPOSLLH: {
          use_high_precision_ = true;
          break;
        }
        case UBX_NAV_EOE: {
          if (!start) {
            start = true;
          } else {
            bool init_status = ubx_nav_pvt_parsed_;
            ubx_nav_pvt_parsed_ = false;
            ubx_nav_hpposllh_parsed_ = false;
            read_status_ = false;
            return init_status;
          }
        }
      }
    }
  }
  return false;
}
bool Ublox::Read() {
  bool status = false;
  while (bus_->available()) {
    if (Epoch()) {
      tow_ms_ = ubx_nav_pvt_.itow;
      year_ = ubx_nav_pvt_.year;
      month_ = ubx_nav_pvt_.month;
      day_  = ubx_nav_pvt_.day;
      hour_ = ubx_nav_pvt_.hour;
      min_  = ubx_nav_pvt_.min;
      sec_  = ubx_nav_pvt_.sec;
      nano_sec_ = ubx_nav_pvt_.nano;
      fix_  = static_cast<FixType>(ubx_nav_pvt_.fix);
      num_satellites_ = ubx_nav_pvt_.numsv;
      ned_velocity_mps_(0) = static_cast<float>(ubx_nav_pvt_.veln) / 1000.0f;
      ned_velocity_mps_(1) = static_cast<float>(ubx_nav_pvt_.vele) / 1000.0f;
      ned_velocity_mps_(2) = static_cast<float>(ubx_nav_pvt_.veld) / 1000.0f;
      time_accuracy_ns_ = ubx_nav_pvt_.tacc;
      velocity_accuracy_mps_ = static_cast<float>(ubx_nav_pvt_.sacc) / 1000.0f;
      bool valid_date = ubx_nav_pvt_.valid & 0x01;
      bool valid_time = ubx_nav_pvt_.valid & 0x02;
      bool fully_resolved = ubx_nav_pvt_.valid & 0x04;
      bool validity_confirmed = ubx_nav_pvt_.flags2 & 0x20;
      bool confirmed_date = ubx_nav_pvt_.flags2 & 0x40;
      bool confirmed_time = ubx_nav_pvt_.flags2 & 0x80;
      valid_time_and_date_ = valid_date && valid_time && fully_resolved && validity_confirmed && confirmed_date && confirmed_time;
      bool gnss_ok = ubx_nav_pvt_.flags & 0x01;
      bool invalid_llh = ubx_nav_pvt_.flags3 & 0x01;
      valid_gnss_fix_ = gnss_ok && !invalid_llh;
      if (use_high_precision_) {
        lla_rad_m_(0) = global::conversions::Deg_to_Rad((static_cast<double>(ubx_nav_hpposllh_.lat) + static_cast<double>(ubx_nav_hpposllh_.lathp) * 1e-2) * 1e-7);
        lla_rad_m_(1) = global::conversions::Deg_to_Rad((static_cast<double>(ubx_nav_hpposllh_.lon) + static_cast<double>(ubx_nav_hpposllh_.lonhp) * 1e-2) * 1e-7);
        lla_rad_m_(2) = (static_cast<double>(ubx_nav_hpposllh_.height) + static_cast<double>(ubx_nav_hpposllh_.heighthp) * 0.1f) * 0.001f;
        horizontal_accuracy_m_ = static_cast<float>(ubx_nav_hpposllh_.hacc) / 10000.0f;
        vertical_accuracy_m_ = static_cast<float>(ubx_nav_hpposllh_.vacc) / 10000.0f;
      } else {
        lla_rad_m_(0) = global::conversions::Deg_to_Rad(static_cast<double>(ubx_nav_pvt_.lat) * 1e-7);
        lla_rad_m_(1) = global::conversions::Deg_to_Rad(static_cast<double>(ubx_nav_pvt_.lon) * 1e-7);
        lla_rad_m_(2) = static_cast<double>(ubx_nav_pvt_.height) * 0.001f;
        horizontal_accuracy_m_ = static_cast<float>(ubx_nav_hpposllh_.hacc) / 1000.0f;
        vertical_accuracy_m_ = static_cast<float>(ubx_nav_hpposllh_.vacc) / 1000.0f;
      }
      status = true;
    }
  }
  return status;
}
uint32_t Ublox::tow_ms() {
  return tow_ms_;
}
uint16_t Ublox::year() {
  return year_;
}
uint8_t Ublox::month() {
  return month_;
}
uint8_t Ublox::day() {
  return day_;
}
uint8_t Ublox::hour() {
  return hour_;
}
uint8_t Ublox::min() {
  return min_;
}
uint8_t Ublox::sec() {
  return sec_;
}
int32_t Ublox::nano_sec() {
  return nano_sec_;
}
Ublox::FixType Ublox::fix() {
  return fix_;
}
uint8_t Ublox::num_satellites() {
  return num_satellites_;
}
Eigen::Vector3d Ublox::lla_rad_m() {
  return lla_rad_m_;
}
double Ublox::lat_rad() {
  return lla_rad_m_(0);
}
double Ublox::lon_rad() {
  return lla_rad_m_(1);
}
float Ublox::alt_m() {
  return static_cast<float>(lla_rad_m_(2));
}
Eigen::Vector3f Ublox::ned_velocity_mps() {
  return ned_velocity_mps_;
}
float Ublox::north_velocity_mps() {
  return ned_velocity_mps_(0);
}
float Ublox::east_velocity_mps() {
  return ned_velocity_mps_(1);
}
float Ublox::down_velocity_mps() {
  return ned_velocity_mps_(2);
}
uint32_t Ublox::time_accuracy_ns() {
  return time_accuracy_ns_;
}
float Ublox::horizontal_accuracy_m() {
  return horizontal_accuracy_m_;
}
float Ublox::vertical_accuracy_m() {
  return vertical_accuracy_m_;
}
float Ublox::velocity_accuracy_mps() {
  return velocity_accuracy_mps_;
}
bool Ublox::valid_time_and_date() {
  return valid_time_and_date_;
}
bool Ublox::valid_gnss_fix() {
  return valid_gnss_fix_;
}
bool Ublox::Epoch() {
  if (Parse()) {
    switch (msg_) {
      case UBX_NAV_PVT: {
        ubx_nav_pvt_parsed_ = true;
        break;
      }
      case UBX_NAV_HPPOSLLH: {
        ubx_nav_hpposllh_parsed_ = true;
        break;
      }
      case UBX_NAV_EOE: {
        read_status_ = false;
        if (use_high_precision_) {
          read_status_ = ubx_nav_pvt_parsed_ && ubx_nav_hpposllh_parsed_;
        } else {
          read_status_ = ubx_nav_pvt_parsed_;
        }
        ubx_nav_pvt_parsed_ = false;
        ubx_nav_hpposllh_parsed_ = false;
        return read_status_;
      }
    }
  }
  return false;
}
bool Ublox::Parse() {
  while (bus_->available()) {
    uint8_t byte_read = bus_->read();
    /* Identify the packet header */
    if (parser_pos_ < 2) {
      if (byte_read == UBX_HEADER_[parser_pos_]) {
        parser_pos_++;
      } else {
        parser_pos_ = 0;
      }
    /* Message class */
    } else if (parser_pos_ == 2) {
      if (byte_read == UBX_NAV_CLASS_) {
        rx_buffer_[parser_pos_ - sizeof(UBX_HEADER_)] = byte_read;
        parser_pos_++;
      } else {
        parser_pos_ = 0;
      }
    /* Message ID */
    } else if (parser_pos_ == 3) {
      if ((byte_read == UBX_NAV_PVT) || (byte_read == UBX_NAV_HPPOSLLH) || (byte_read == UBX_NAV_EOE))  {
        msg_ = static_cast<Msg>(byte_read);
        rx_buffer_[parser_pos_ - sizeof(UBX_HEADER_)] = byte_read;
        parser_pos_++;
      } else {
        parser_pos_ = 0;
      }
    /* Messgae length */
    } else if (parser_pos_ == 4) {
      msg_len_buffer_[0] = byte_read;
      rx_buffer_[parser_pos_ - sizeof(UBX_HEADER_)] = byte_read;
      parser_pos_++;
    /* Message length */
    } else if (parser_pos_ == 5) {
      msg_len_buffer_[1] = byte_read;
      msg_len_ = static_cast<uint16_t>(msg_len_buffer_[1]) << 8 | msg_len_buffer_[0];
      rx_buffer_[parser_pos_ - sizeof(UBX_HEADER_)] = byte_read;
      switch (msg_) {
        case UBX_NAV_PVT: {
          if (msg_len_ == UBX_PVT_LEN_) {
            parser_pos_++;
          } else {
            parser_pos_ = 0;
          }
          break;
        }
        case UBX_NAV_HPPOSLLH: {
          if (msg_len_ == UBX_HPPOSLLH_LEN_) {
            parser_pos_++;
          } else {
            parser_pos_ = 0;
          }
          break;
        }
        case UBX_NAV_EOE: {
          if (msg_len_ == UBX_EOE_LEN_) {
            parser_pos_++;
          } else {
            parser_pos_ = 0;
          }
          break;
        }
      }
    /* Message payload */
    } else if (parser_pos_ < (msg_len_ + UBX_HEADER_LEN_)) {
      rx_buffer_[parser_pos_ - sizeof(UBX_HEADER_)] = byte_read;
      parser_pos_++;
    /* Checksum */
    } else  if (parser_pos_ == (msg_len_ + UBX_HEADER_LEN_)) {
      checksum_buffer_[0] = byte_read;
      parser_pos_++;
    } else {
      checksum_buffer_[1] = byte_read;
      uint16_t received_checksum = static_cast<uint16_t>(checksum_buffer_[1]) << 8 | checksum_buffer_[0];
      uint16_t computed_checksum = Checksum(rx_buffer_, msg_len_ + UBX_HEADER_LEN_);
      if (computed_checksum == received_checksum) {
        switch (msg_) {
          case UBX_NAV_PVT: {
            memcpy(&ubx_nav_pvt_, rx_buffer_ + UBX_PAYLOAD_OFFSET_, UBX_PVT_LEN_);
            break;
          }
          case UBX_NAV_HPPOSLLH: {
            memcpy(&ubx_nav_hpposllh_, rx_buffer_ + UBX_PAYLOAD_OFFSET_, UBX_HPPOSLLH_LEN_);
            break;
          }
          case UBX_NAV_EOE: {
            memcpy(&ubx_nav_eoe_, rx_buffer_ + UBX_PAYLOAD_OFFSET_, UBX_EOE_LEN_);
            break;
          }
        }
        parser_pos_ = 0;
        return true;
      } else {
        parser_pos_ = 0;
        return false;
      }
    }
  }
  return false;
}
uint16_t Ublox::Checksum(uint8_t *data, uint16_t len) {
  if (!data) {
    return 0;
  }
  uint8_t checksum_buffer[2] = {0, 0};
	for (unsigned int i = 0; i < len; i++) {
		checksum_buffer[0] += data[i];
		checksum_buffer[1] += checksum_buffer[0];
	}
  return static_cast<uint16_t>(checksum_buffer_[1]) << 8 | checksum_buffer_[0];
}

}  // namespace sensors
