// Copyright 2017 bwze, crankyoldgit
// Midea

#include "ir_Midea.h"
#include <algorithm>
#ifndef ARDUINO
#include <string>
#endif
#include "IRrecv.h"
#include "IRsend.h"
#include "IRutils.h"

// Midea A/C added by (send) bwze/crankyoldgit & (decode) crankyoldgit
//
// Equipment it seems compatible with:
//  * Pioneer System Model RYBO12GMFILCAD (12K BTU)
//  * Pioneer System Model RUBO18GMFILCAD (18K BTU)
//  * <Add models (A/C & remotes) you've gotten it working with here>

// Ref:
//   https://docs.google.com/spreadsheets/d/1TZh4jWrx4h9zzpYUI9aYXMl1fYOiqu-xVuOOMqagxrs/edit?usp=sharing

// Constants
const uint16_t kMideaTick = 80;
const uint16_t kMideaBitMarkTicks = 7;
const uint16_t kMideaBitMark = kMideaBitMarkTicks * kMideaTick;
const uint16_t kMideaOneSpaceTicks = 21;
const uint16_t kMideaOneSpace = kMideaOneSpaceTicks * kMideaTick;
const uint16_t kMideaZeroSpaceTicks = 7;
const uint16_t kMideaZeroSpace = kMideaZeroSpaceTicks * kMideaTick;
const uint16_t kMideaHdrMarkTicks = 56;
const uint16_t kMideaHdrMark = kMideaHdrMarkTicks * kMideaTick;
const uint16_t kMideaHdrSpaceTicks = 56;
const uint16_t kMideaHdrSpace = kMideaHdrSpaceTicks * kMideaTick;
const uint16_t kMideaMinGapTicks =
    kMideaHdrMarkTicks + kMideaZeroSpaceTicks + kMideaBitMarkTicks;
const uint16_t kMideaMinGap = kMideaMinGapTicks * kMideaTick;
const uint8_t kMideaTolerance = 30;  // Percent

#if SEND_MIDEA
// Send a Midea message
//
// Args:
//   data:   Contents of the message to be sent.
//   nbits:  Nr. of bits of data to be sent. Typically kMideaBits.
//   repeat: Nr. of additional times the message is to be sent.
//
// Status: Alpha / Needs testing against a real device.
//
void IRsend::sendMidea(uint64_t data, uint16_t nbits, uint16_t repeat) {
  if (nbits % 8 != 0) return;  // nbits is required to be a multiple of 8.

  // Set IR carrier frequency
  enableIROut(38);

  for (uint16_t r = 0; r <= repeat; r++) {
    // The protcol sends the message, then follows up with an entirely
    // inverted payload.
    for (size_t inner_loop = 0; inner_loop < 2; inner_loop++) {
      // Header
      mark(kMideaHdrMark);
      space(kMideaHdrSpace);
      // Data
      //   Break data into byte segments, starting at the Most Significant
      //   Byte. Each byte then being sent normal, then followed inverted.
      for (uint16_t i = 8; i <= nbits; i += 8) {
        // Grab a bytes worth of data.
        uint8_t segment = (data >> (nbits - i)) & 0xFF;
        sendData(kMideaBitMark, kMideaOneSpace, kMideaBitMark, kMideaZeroSpace,
                 segment, 8, true);
      }
      // Footer
      mark(kMideaBitMark);
      space(kMideaMinGap);  // Pause before repeating

      // Invert the data for the 2nd phase of the message.
      // As we get called twice in the inner loop, we will always revert
      // to the original 'data' state.
      data = ~data;
    }
  }
}
#endif

// Code to emulate Midea A/C IR remote control unit.
// Warning: Consider this very alpha code.

// Initialise the object.
IRMideaAC::IRMideaAC(const uint16_t pin) : _irsend(pin) { this->stateReset(); }

// Reset the state of the remote to a known good state/sequence.
void IRMideaAC::stateReset(void) {
  // Power On, Mode Auto, Fan Auto, Temp = 25C/77F
  remote_state = 0xA1826FFFFF62;
}

// Configure the pin for output.
void IRMideaAC::begin(void) { _irsend.begin(); }

#if SEND_MIDEA
// Send the current desired state to the IR LED.
void IRMideaAC::send(const uint16_t repeat) {
  this->checksum();  // Ensure correct checksum before sending.
  _irsend.sendMidea(remote_state, kMideaBits, repeat);
}
#endif  // SEND_MIDEA

// Return a pointer to the internal state date of the remote.
uint64_t IRMideaAC::getRaw(void) {
  this->checksum();
  return remote_state & kMideaACStateMask;
}

// Override the internal state with the new state.
void IRMideaAC::setRaw(const uint64_t newState) {
  remote_state = newState & kMideaACStateMask;
}

// Set the requested power state of the A/C to off.
void IRMideaAC::on(void) { remote_state |= kMideaACPower; }

// Set the requested power state of the A/C to off.
void IRMideaAC::off(void) {
  remote_state &= (kMideaACStateMask ^ kMideaACPower);
}

// Set the requested power state of the A/C.
void IRMideaAC::setPower(const bool on) {
  if (on)
    this->on();
  else
    this->off();
}

// Return the requested power state of the A/C.
bool IRMideaAC::getPower(void) { return (remote_state & kMideaACPower); }

// Set the temperature.
// Args:
//   temp:       Temp. in degrees.
//   useCelsius: Degree type to use. Celsius (true) or Fahrenheit (false)
void IRMideaAC::setTemp(const uint8_t temp, const bool useCelsius) {
  uint8_t new_temp = temp;
  if (useCelsius) {
    new_temp = std::max(kMideaACMinTempC, new_temp);
    new_temp = std::min(kMideaACMaxTempC, new_temp);
    // Convert and add 0.5 for rounding.
    new_temp = celsiusToFahrenheit(new_temp) + 0.5;
  }
  new_temp = std::max(kMideaACMinTempF, new_temp);
  new_temp = std::min(kMideaACMaxTempF, new_temp);
  new_temp -= kMideaACMinTempF;
  remote_state &= kMideaACTempMask;
  remote_state |= ((uint64_t)new_temp << 24);
}

// Return the set temp.
// Args:
//   useCelsius: Flag indicating if the results are in Celsius or Fahrenheit.
// Returns:
//   A uint8_t containing the temperature.
uint8_t IRMideaAC::getTemp(const bool useCelsius) {
  uint8_t temp = ((remote_state >> 24) & 0x1F) + kMideaACMinTempF;
  if (useCelsius) {
    temp = fahrenheitToCelsius(temp);
  }
  return temp;
}

// Set the speed of the fan,
// 1-3 set the speed, 0 or anything else set it to auto.
void IRMideaAC::setFan(const uint8_t fan) {
  uint64_t new_fan;
  switch (fan) {
    case kMideaACFanLow:
    case kMideaACFanMed:
    case kMideaACFanHigh:
      new_fan = fan;
      break;
    default:
      new_fan = kMideaACFanAuto;
  }
  remote_state &= kMideaACFanMask;
  remote_state |= (new_fan << 35);
}

// Return the requested state of the unit's fan.
uint8_t IRMideaAC::getFan(void) { return (remote_state >> 35) & 0b111; }

// Get the requested climate operation mode of the a/c unit.
// Returns:
//   A uint8_t containing the A/C mode.
uint8_t IRMideaAC::getMode(void) { return ((remote_state >> 32) & 0b111); }

// Set the requested climate operation mode of the a/c unit.
void IRMideaAC::setMode(const uint8_t mode) {
  switch (mode) {
    case kMideaACAuto:
    case kMideaACCool:
    case kMideaACHeat:
    case kMideaACDry:
    case kMideaACFan:
      remote_state &= kMideaACModeMask;
      remote_state |= ((uint64_t)mode << 32);
      return;
    default:
      this->setMode(kMideaACAuto);
  }
}

// Set the Sleep state of the A/C.
void IRMideaAC::setSleep(const bool on) {
  if (on)
    remote_state |= kMideaACSleep;
  else
    remote_state &= (kMideaACStateMask ^ kMideaACSleep);
}

// Return the Sleep state of the A/C.
bool IRMideaAC::getSleep(void) { return (remote_state & kMideaACSleep); }

// Calculate the checksum for a given array.
// Args:
//   state:  The state to calculate the checksum over.
// Returns:
//   The 8 bit checksum value.
uint8_t IRMideaAC::calcChecksum(const uint64_t state) {
  uint8_t sum = 0;
  uint64_t temp_state = state;

  for (uint8_t i = 0; i < 5; i++) {
    temp_state >>= 8;
    sum += reverseBits((temp_state & 0xFF), 8);
  }
  sum = 256 - sum;
  return reverseBits(sum, 8);
}

// Verify the checksum is valid for a given state.
// Args:
//   state:  The state to verify the checksum of.
// Returns:
//   A boolean.
bool IRMideaAC::validChecksum(const uint64_t state) {
  return ((state & 0xFF) == calcChecksum(state));
}

// Calculate & set the checksum for the current internal state of the remote.
void IRMideaAC::checksum(void) {
  // Stored the checksum value in the last byte.
  remote_state &= kMideaACChecksumMask;
  remote_state |= calcChecksum(remote_state);
}


// Convert a standard A/C mode into its native mode.
uint8_t IRMideaAC::convertMode(const stdAc::opmode_t mode) {
  switch (mode) {
    case stdAc::opmode_t::kCool:
      return kMideaACCool;
    case stdAc::opmode_t::kHeat:
      return kMideaACHeat;
    case stdAc::opmode_t::kDry:
      return kMideaACDry;
    case stdAc::opmode_t::kFan:
      return kMideaACFan;
    default:
      return kMideaACAuto;
  }
}

// Convert a standard A/C Fan speed into its native fan speed.
uint8_t IRMideaAC::convertFan(const stdAc::fanspeed_t speed) {
  switch (speed) {
    case stdAc::fanspeed_t::kMin:
    case stdAc::fanspeed_t::kLow:
      return kMideaACFanLow;
    case stdAc::fanspeed_t::kMedium:
      return kMideaACFanMed;
    case stdAc::fanspeed_t::kHigh:
    case stdAc::fanspeed_t::kMax:
      return kMideaACFanHigh;
    default:
      return kMideaACFanAuto;
  }
}

// Convert a native mode to it's common equivalent.
stdAc::opmode_t IRMideaAC::toCommonMode(const uint8_t mode) {
  switch (mode) {
    case kMideaACCool: return stdAc::opmode_t::kCool;
    case kMideaACHeat: return stdAc::opmode_t::kHeat;
    case kMideaACDry: return stdAc::opmode_t::kDry;
    case kMideaACFan: return stdAc::opmode_t::kFan;
    default: return stdAc::opmode_t::kAuto;
  }
}

// Convert a native fan speed to it's common equivalent.
stdAc::fanspeed_t IRMideaAC::toCommonFanSpeed(const uint8_t speed) {
  switch (speed) {
    case kMideaACFanHigh: return stdAc::fanspeed_t::kMax;
    case kMideaACFanMed: return stdAc::fanspeed_t::kMedium;
    case kMideaACFanLow: return stdAc::fanspeed_t::kMin;
    default: return stdAc::fanspeed_t::kAuto;
  }
}

// Convert the A/C state to it's common equivalent.
stdAc::state_t IRMideaAC::toCommon(void) {
  stdAc::state_t result;
  result.protocol = decode_type_t::MIDEA;
  result.model = -1;  // No models used.
  result.power = this->getPower();
  result.mode = this->toCommonMode(this->getMode());
  result.celsius = true;
  result.degrees = this->getTemp(result.celsius);
  result.fanspeed = this->toCommonFanSpeed(this->getFan());
  result.sleep = this->getSleep() ? 0 : -1;
  // Not supported.
  result.swingv = stdAc::swingv_t::kOff;
  result.swingh = stdAc::swingh_t::kOff;
  result.quiet = false;
  result.turbo = false;
  result.clean = false;
  result.econo = false;
  result.filter = false;
  result.light = false;
  result.beep = false;
  result.clock = -1;
  return result;
}

// Convert the internal state into a human readable string.
String IRMideaAC::toString(void) {
  String result = "";
  result.reserve(70);  // Reserve some heap for the string to reduce fragging.
  result += F("Power: ");
  if (getPower())
    result += F("On");
  else
    result += F("Off");
  result += F(", Mode: ");
  result += uint64ToString(getMode());
  switch (getMode()) {
    case kMideaACAuto:
      result += F(" (AUTO)");
      break;
    case kMideaACCool:
      result += F(" (COOL)");
      break;
    case kMideaACHeat:
      result += F(" (HEAT)");
      break;
    case kMideaACDry:
      result += F(" (DRY)");
      break;
    case kMideaACFan:
      result += F(" (FAN)");
      break;
    default:
      result += F(" (UNKNOWN)");
  }
  result += F(", Temp: ");
  result += uint64ToString(getTemp(true));
  result += F("C/");
  result += uint64ToString(getTemp(false));
  result += F("F, Fan: ");
  result += uint64ToString(getFan());
  switch (getFan()) {
    case kMideaACFanAuto:
      result += F(" (AUTO)");
      break;
    case kMideaACFanLow:
      result += F(" (LOW)");
      break;
    case kMideaACFanMed:
      result += F(" (MED)");
      break;
    case kMideaACFanHigh:
      result += F(" (HI)");
      break;
  }
  result += F(", Sleep: ");
  if (getSleep())
    result += F("On");
  else
    result += F("Off");
  return result;
}

#if DECODE_MIDEA
// Decode the supplied Midea message.
//
// Args:
//   results: Ptr to the data to decode and where to store the decode result.
//   nbits:   The number of data bits to expect. Typically kMideaBits.
//   strict:  Flag indicating if we should perform strict matching.
// Returns:
//   boolean: True if it can decode it, false if it can't.
//
// Status: Alpha / Needs testing against a real device.
//
bool IRrecv::decodeMidea(decode_results *results, uint16_t nbits, bool strict) {
  if (nbits % 8 != 0)  // nbits has to be a multiple of nr. of bits in a byte.
    return false;

  uint8_t min_nr_of_messages = 1;
  if (strict) {
    if (nbits != kMideaBits) return false;  // Not strictly a MIDEA message.
    min_nr_of_messages = 2;
  }

  // The protocol sends the data normal + inverted, alternating on
  // each byte. Hence twice the number of expected data bits.
  if (results->rawlen <
      min_nr_of_messages * (2 * nbits + kHeader + kFooter) - 1)
    return false;  // Can't possibly be a valid MIDEA message.

  uint64_t data = 0;
  uint64_t inverted = 0;
  uint16_t offset = kStartOffset;

  if (nbits > sizeof(data) * 8)
    return false;  // We can't possibly capture a Midea packet that big.

  for (uint8_t i = 0; i < min_nr_of_messages; i++) {
    // Header
    if (!matchMark(results->rawbuf[offset], kMideaHdrMark)) return false;
    // Calculate how long the common tick time is based on the header mark.
    uint32_t m_tick = results->rawbuf[offset++] * kRawTick / kMideaHdrMarkTicks;
    if (!matchSpace(results->rawbuf[offset], kMideaHdrSpace)) return false;
    // Calculate how long the common tick time is based on the header space.
    uint32_t s_tick =
        results->rawbuf[offset++] * kRawTick / kMideaHdrSpaceTicks;

    // Data (Normal)
    match_result_t data_result = matchData(
        &(results->rawbuf[offset]), nbits, kMideaBitMarkTicks * m_tick,
        kMideaOneSpaceTicks * s_tick, kMideaBitMarkTicks * m_tick,
        kMideaZeroSpaceTicks * s_tick, kMideaTolerance);
    if (data_result.success == false) return false;
    offset += data_result.used;
    if (i % 2 == 0)
      data = data_result.data;
    else
      inverted = data_result.data;

    // Footer
    if (!matchMark(results->rawbuf[offset++], kMideaBitMarkTicks * m_tick,
                   kMideaTolerance))
      return false;
    if (offset < results->rawlen &&
        !matchAtLeast(results->rawbuf[offset++], kMideaMinGapTicks * s_tick,
                      kMideaTolerance))
      return false;
  }

  // Compliance
  if (strict) {
    // Protocol requires a second message with all the data bits inverted.
    // We should have checked we got a second message in the previous loop.
    // Just need to check it's value is an inverted copy of the first message.
    uint64_t mask = (1ULL << kMideaBits) - 1;
    if ((data & mask) != ((inverted ^ mask) & mask)) return false;
    if (!IRMideaAC::validChecksum(data)) return false;
  }

  // Success
  results->decode_type = MIDEA;
  results->bits = nbits;
  results->value = data;
  results->address = 0;
  results->command = 0;
  return true;
}
#endif  // DECODE_MIDEA
