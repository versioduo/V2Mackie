// © Kay Sievers <kay@versioduo.com>, 2020-2022
// SPDX-License-Identifier: Apache-2.0

#include "V2Mackie.h"

namespace Mackie {
// System Exclusive Messages.
namespace Message {
  // Header:  vendor prefix, device type, message type.
  namespace Header {
    enum {
      Vendor,
      Device = 3,
      Type,
      Message,
    };
  };

  // 3 bytes MIDI vendor ID.
  static constexpr uint8_t Vendor[3]{0x00, 0x00, 0x66};

  namespace Device {
    enum {
      Control   = 20,
      ControlXT = 21,
    };
  };

  namespace Type {
    enum {
      TranportButtonClick = 10,

      // 0..127 minutes
      BacklightTimout = 11,

      TouchlessFader = 12,

      // strip, 0..5
      TouchSensitivity = 14,

      TimeDisplay = 16,

      ModeDisplay = 17,

      // offset, characters
      Display = 18,

      Version      = 19,
      VersionReply = 20,

      // strip, mode
      MeterMode = 32,

      // 0 = horizontal / 1 = vertical
      MeterOrientation = 33,

      FaderHome = 97,
      LEDsOff   = 98,
      Reset     = 99
    };
  };

  // 1 byte index, up to 56 * 2 bytes text.
  namespace Display::Header {
    enum {
      Index,
      Text,
    };
  };
};

namespace Display {
  // 56 character, 2 row LCD display.
  // 8 channel strips * 7 characters == 56.
  namespace Strip {
    enum Note { NameValue = 52 };
  };

  // 10 digits, 3-2-2-3 grouping.
  namespace Time {
    enum CC {
      // 64-73, reverse order/right to left.
      Digit = 64
    };

    // Switch between Hours-Minutes-Seconds-Frames and Bars-Beats-SubDivision-Ticks mode.
    enum Note { SMPTEBeats = 53 };
  };

  // 2 digits.
  namespace Mode {
    enum CC {
      // 74-57, reverse order/right to left.
      Digit = 74
    };
  };
};

namespace ChannelStrip {
  // Push and rotary control.
  namespace VPot {
    enum Mode {
      Single,
      Boost,
      Bar,
      Spread,
    };

    enum CC {
      // Bit 0..5: steps
      // Bit 6:    0 == clockwise, 1 == counter clockwise
      Encoder = 16,

      // Bit 0..3: value
      // Bit 4..5: mode
      // Bit 6:    center dot
      LED = 48,
    };

    enum Note {
      Push       = 32,
      Track      = 40,
      Send       = 41,
      Pan        = 42,
      PlugIn     = 43,
      Equalizer  = 44,
      Instrument = 45,
    };
  };

  // Buttons and fader touch.
  namespace Button {
    enum Note {
      Arm    = 0,
      Solo   = 8,
      Mute   = 16,
      Select = 24,
    };
  };

  // The fader controls pitch bend channel 1-8.
  namespace Fader {
    enum Note {
      Touch = 104,
    };
  };

  // The meter is Channel Aftertouch 4 bit index + value 0..12 + overload flag.
};

// The main fader.
namespace Main {
  enum Note {
    // Value 0/127.
    Touch = 112
  };

  // The main fader controls pitch bend channel 9
};

namespace Bank {
  enum Note {
    // Move 8/16/32 channel strips. Up to three extension units, each adds 8
    // channel strips to a bank.
    Previous = 46,
    Next     = 47,

    // Move a single channel.
    PreviousChannel = 48,
    NextChannel     = 49,

    Flip = 50,
    Edit = 51,
  };
};

namespace Function {
  enum Note {
    F1  = 54,
    F2  = 55,
    F3  = 56,
    F4  = 57,
    F5  = 58,
    F6  = 59,
    F7  = 60,
    F8  = 61,
    F9  = 62,
    F10 = 63,
    F11 = 64,
    F12 = 65,
    F13 = 66,
    F14 = 67,
    F15 = 68,
    F16 = 69,
  };
};

namespace Modifier {
  enum Note {
    Shift   = 70,
    Option  = 71,
    Control = 72,
    Alt     = 73,
  };
};

namespace Automation {
  enum Note {
    On       = 74,
    Record   = 75,
    Snapshot = 77,
    Touch    = 78,
  };
};

namespace Utility {
  enum Note {
    Undo   = 76,
    Cancel = 80,
    Enter  = 81,
    Redo   = 79,
    Marker = 82,
    Mixer  = 83,
  };
};

namespace Marker {
  enum Note {
    PreviousFrame = 84,
    NextFrame     = 85,
    Loop          = 86,
    PointIn       = 87,
    PointOut      = 88,
    Home          = 89,
    End           = 90,
  };
};

namespace Transport {
  enum Note {
    Rewind  = 91,
    Forward = 92,
    Stop    = 93,
    Play    = 94,
    Record  = 95,
  };
};

namespace Navigation {
  enum CC {
    // Value CW=1/CCW=65.
    Jog = 60
  };

  enum Note {
    Up    = 96,
    Down  = 97,
    Left  = 98,
    Right = 99,
    Zoom  = 100,
    Scrub = 101,
  };
};

namespace UserSwitch {
  enum Note {
    S1 = 102,
    S2 = 103,
  };
};

namespace Protocol {
  enum Note {
    // TotalMix: Channel == 16, Velocity == 90, sent every ~800ms.
    Ping = 127
  };
};
};

V2MIDI::Packet *V2Mackie::setStripMeter(V2MIDI::Packet *packet, uint8_t strip, float fraction) {
  const uint8_t value = fraction * 12.f;
  return packet->setAftertouchChannel(0, strip << 4 | value);
}

V2MIDI::Packet *V2Mackie::setStripMeterOverload(V2MIDI::Packet *packet, uint8_t strip, bool overload) {
  return packet->setAftertouchChannel(0, strip << 4 | overload ? 14 : 15);
}

V2MIDI::Packet *V2Mackie::setStripFader(V2MIDI::Packet *packet, uint8_t strip, float fraction) {
  const int16_t range = (float)(8176 + 8192) * fraction;
  const int16_t value = range - 8192;
  return packet->setPitchBend(strip, value);
}

uint8_t V2Mackie::setStripText(uint8_t *buffer, uint8_t strip, uint8_t row, const char *text) {
  uint8_t len   = 0;
  buffer[len++] = 0xf0;
  memcpy(buffer + len, Mackie::Message::Vendor, sizeof(Mackie::Message::Vendor));
  len += sizeof(Mackie::Message::Vendor);

  buffer[len++] = Mackie::Message::Device::Control;
  buffer[len++] = Mackie::Message::Type::Display;
  buffer[len++] = (56 * row) + (7 * strip);

  uint8_t textlen = strlen(text);
  memcpy(buffer + len, text, textlen);
  len += textlen;
  memset(buffer + len, ' ', 7 - textlen);
  len += 7 - textlen;
  buffer[len++] = 0xf7;
  return len;
}

V2MIDI::Packet *V2Mackie::setStripIndex(V2MIDI::Packet *packet, uint8_t strip) {
  switch (packet->getType()) {
    case V2MIDI::Packet::Status::NoteOn:
      if (packet->getChannel() != 0)
        return NULL;

      switch (packet->getNote()) {
        case Mackie::ChannelStrip::VPot::Push... Mackie::ChannelStrip::VPot::Push + 7:
          return packet->setNote(0, Mackie::ChannelStrip::VPot::Push + strip, packet->getNoteVelocity());

        case Mackie::ChannelStrip::Button::Arm... Mackie::ChannelStrip::Button::Arm + 7:
          return packet->setNote(0, Mackie::ChannelStrip::Button::Arm + strip, packet->getNoteVelocity());

        case Mackie::ChannelStrip::Button::Solo... Mackie::ChannelStrip::Button::Solo + 7:
          return packet->setNote(0, Mackie::ChannelStrip::Button::Solo + strip, packet->getNoteVelocity());

        case Mackie::ChannelStrip::Button::Mute... Mackie::ChannelStrip::Button::Mute + 7:
          return packet->setNote(0, Mackie::ChannelStrip::Button::Mute + strip, packet->getNoteVelocity());

        case Mackie::ChannelStrip::Button::Select... Mackie::ChannelStrip::Button::Select + 7:
          return packet->setNote(0, Mackie::ChannelStrip::Button::Select + strip, packet->getNoteVelocity());

        case Mackie::ChannelStrip::Fader::Touch... Mackie::ChannelStrip::Fader::Touch + 7:
          return packet->setNote(0, Mackie::ChannelStrip::Fader::Touch + strip, packet->getNoteVelocity());
      }
      break;

    case V2MIDI::Packet::Status::NoteOff:
      if (packet->getChannel() != 0)
        return NULL;

      switch (packet->getNote()) {
        case Mackie::ChannelStrip::VPot::Push... Mackie::ChannelStrip::VPot::Push + 7:
          return packet->setNoteOff(0, Mackie::ChannelStrip::VPot::Push + strip);

        case Mackie::ChannelStrip::Button::Arm... Mackie::ChannelStrip::Button::Arm + 7:
          return packet->setNoteOff(0, Mackie::ChannelStrip::Button::Arm + strip);

        case Mackie::ChannelStrip::Button::Solo... Mackie::ChannelStrip::Button::Solo + 7:
          return packet->setNoteOff(0, Mackie::ChannelStrip::Button::Solo + strip);

        case Mackie::ChannelStrip::Button::Mute... Mackie::ChannelStrip::Button::Mute + 7:
          return packet->setNoteOff(0, Mackie::ChannelStrip::Button::Mute + strip);

        case Mackie::ChannelStrip::Button::Select... Mackie::ChannelStrip::Button::Select + 7:
          return packet->setNoteOff(0, Mackie::ChannelStrip::Button::Select + strip);

        case Mackie::ChannelStrip::Fader::Touch... Mackie::ChannelStrip::Fader::Touch + 7:
          return packet->setNoteOff(0, Mackie::ChannelStrip::Fader::Touch + strip);
      }
      break;

    case V2MIDI::Packet::Status::ControlChange:
      if (packet->getChannel() != 0)
        return NULL;

      switch (packet->getController()) {
        case Mackie::ChannelStrip::VPot::LED... Mackie::ChannelStrip::VPot::LED + 7:
          return packet->setControlChange(0, Mackie::ChannelStrip::VPot::LED + strip, packet->getControllerValue());
      }
      break;

    case V2MIDI::Packet::Status::AftertouchChannel: {
      if (packet->getChannel() != 0)
        return NULL;

      const uint8_t index = packet->getAftertouchChannel() >> 4;
      if (index > 7)
        return NULL;

      const uint8_t value = packet->getAftertouchChannel() & 0xf;
      return packet->setAftertouchChannel(0, strip << 4 | value);
    }

    case V2MIDI::Packet::Status::PitchBend:
      switch (packet->getChannel()) {
        case 0 ... 7:
          return packet->setPitchBend(strip, packet->getPitchBend());
      }
      break;
  }

  return NULL;
}

V2MIDI::Packet *V2Mackie::setFader(V2MIDI::Packet *packet, float fraction) {
  const int16_t range = (float)(8176 + 8192) * fraction;
  const int16_t value = range - 8192;
  return packet->setPitchBend(8, value);
}

V2MIDI::Packet *V2Mackie::setTouch(V2MIDI::Packet *packet, bool on) {
  return packet->setNote(0, Mackie::Main::Touch, on ? 127 : 0);
}

V2MIDI::Packet *V2Mackie::setStripVPotDisplay(V2MIDI::Packet *packet, uint8_t strip, uint8_t value) {
  return packet->setControlChange(0, Mackie::ChannelStrip::VPot::LED + strip, value);
}

V2MIDI::Packet *V2Mackie::setStripButton(V2MIDI::Packet *packet, uint8_t strip, StripButton button, bool on) {
  switch (button) {
    case StripButton::VPot:
      return packet->setNote(0, Mackie::ChannelStrip::VPot::Push + strip, on ? 127 : 0);

    case StripButton::Arm:
      return packet->setNote(0, Mackie::ChannelStrip::Button::Arm + strip, on ? 127 : 0);

    case StripButton::Mute:
      return packet->setNote(0, Mackie::ChannelStrip::Button::Mute + strip, on ? 127 : 0);

    case StripButton::Select:
      return packet->setNote(0, Mackie::ChannelStrip::Button::Select + strip, on ? 127 : 0);

    case StripButton::Solo:
      return packet->setNote(0, Mackie::ChannelStrip::Button::Solo + strip, on ? 127 : 0);

    case StripButton::Touch:
      return packet->setNote(0, Mackie::ChannelStrip::Fader::Touch + strip, on ? 127 : 0);

    default:
      return NULL;
  }
}

V2MIDI::Packet *V2Mackie::setTransportButton(V2MIDI::Packet *packet, TransportButton button, bool on) {
  switch (button) {
    case TransportButton::Rewind:
      return packet->setNote(0, Mackie::Transport::Rewind, on ? 127 : 0);

    case TransportButton::Forward:
      return packet->setNote(0, Mackie::Transport::Forward, on ? 127 : 0);

    case TransportButton::Stop:
      return packet->setNote(0, Mackie::Transport::Stop, on ? 127 : 0);

    case TransportButton::Play:
      return packet->setNote(0, Mackie::Transport::Play, on ? 127 : 0);

    case TransportButton::Record:
      return packet->setNote(0, Mackie::Transport::Record, on ? 127 : 0);

    default:
      return NULL;
  }
}

V2MIDI::Packet *V2Mackie::setBankButton(V2MIDI::Packet *packet, BankButton button, bool on) {
  switch (button) {
    case BankButton::Previous:
      return packet->setNote(0, Mackie::Bank::Previous, on ? 127 : 0);

    case BankButton::Next:
      return packet->setNote(0, Mackie::Bank::Next, on ? 127 : 0);

    case BankButton::PreviousChannel:
      return packet->setNote(0, Mackie::Bank::PreviousChannel, on ? 127 : 0);

    case BankButton::NextChannel:
      return packet->setNote(0, Mackie::Bank::NextChannel, on ? 127 : 0);

    case BankButton::Flip:
      return packet->setNote(0, Mackie::Bank::Flip, on ? 127 : 0);

    case BankButton::Edit:
      return packet->setNote(0, Mackie::Bank::Edit, on ? 127 : 0);

    default:
      return NULL;
  }
}

V2MIDI::Packet *V2Mackie::setModifierButton(V2MIDI::Packet *packet, ModifierButton button, bool on) {
  switch (button) {
    case ModifierButton::Shift:
      return packet->setNote(0, Mackie::Modifier::Shift, on ? 127 : 0);

    case ModifierButton::Option:
      return packet->setNote(0, Mackie::Modifier::Option, on ? 127 : 0);

    case ModifierButton::Control:
      return packet->setNote(0, Mackie::Modifier::Control, on ? 127 : 0);

    case ModifierButton::Alt:
      return packet->setNote(0, Mackie::Modifier::Alt, on ? 127 : 0);

    default:
      return NULL;
  }
}

V2MIDI::Packet *V2Mackie::setNavigationButton(V2MIDI::Packet *packet, NavigationButton button, bool on) {
  switch (button) {
    case NavigationButton::Up:
      return packet->setNote(0, Mackie::Navigation::Up, on ? 127 : 0);

    case NavigationButton::Down:
      return packet->setNote(0, Mackie::Navigation::Down, on ? 127 : 0);

    case NavigationButton::Left:
      return packet->setNote(0, Mackie::Navigation::Left, on ? 127 : 0);

    case NavigationButton::Right:
      return packet->setNote(0, Mackie::Navigation::Right, on ? 127 : 0);

    case NavigationButton::Zoom:
      return packet->setNote(0, Mackie::Navigation::Zoom, on ? 127 : 0);

    case NavigationButton::Scrub:
      return packet->setNote(0, Mackie::Navigation::Scrub, on ? 127 : 0);

    default:
      return NULL;
  }
}

V2MIDI::Packet *V2Mackie::setFunctionButton(V2MIDI::Packet *packet, uint8_t function, bool on) {
  return packet->setNote(0, Mackie::Function::F1 + function, on ? 127 : 0);
}

void V2Mackie::reset() {
  _active_usec = 0;
  _display     = {};
  memset(_display.strip, ' ', sizeof(_display.strip));
  memset(_strips, 0, sizeof(_strips));
  _main       = {};
  _bank       = {};
  _transport  = {};
  _navigation = {};
}

void V2Mackie::loop() {
  if (_active_usec > 0 && (unsigned long)(micros() - _active_usec) > 5000 * 1000) {
    _active_usec = 0;
    handleTimeout();
  }

  for (uint8_t i = 0; i < 8; i++) {
    if (_strips[i].meter.fraction <= 0.f)
      continue;

    if ((unsigned long)(micros() - _strips[i].meter.usec) < 1000 * 1000)
      continue;

    _strips[i].meter = {};
    handleStripMeter(i, 0, 0);
  }
}

void V2Mackie::getStripDisplay(uint8_t strip, uint8_t row, char text[8]) {
  memcpy(text, _display.strip + (56 * row) + (7 * strip), 7);
  text[7] = '\0';

  // Trim trailing spaces.
  for (uint8_t e = 7; e > 0 && text[e - 1] == ' '; e--)
    text[e - 1] = '\0';
}

static char get7Segment(uint8_t b) {
  // Remove dot.
  b &= 63;

  if (b < 32)
    return b + 64;

  return b;
}

static uint16_t getNumber(uint8_t *digits, uint8_t length) {
  uint16_t number = 0;
  uint8_t factor  = 1;
  for (uint8_t i = 0; i < length; i++) {
    uint8_t digit = 0;
    const char c  = get7Segment(digits[length - 1 - i]);
    if (c >= '0' && c <= '9')
      digit = c - '0';
    number += digit * factor;
    factor *= 10;
  }

  return number;
}

void V2Mackie::getTime(Time &time) {
  switch (_display.time.type) {
    case Time::Type::SMPTE:
      time.type          = Time::Type::SMPTE;
      time.smpte.hours   = getNumber(_display.time.digits, 3);
      time.smpte.minutes = getNumber(_display.time.digits + 3, 2);
      time.smpte.seconds = getNumber(_display.time.digits + 5, 2);
      time.smpte.frames  = getNumber(_display.time.digits + 7, 3);
      break;

    case Time::Type::Beats:
      time.type              = Time::Type::Beats;
      time.beats.bars        = getNumber(_display.time.digits, 3);
      time.beats.beats       = getNumber(_display.time.digits + 3, 2);
      time.beats.subdivision = getNumber(_display.time.digits + 5, 2);
      time.beats.ticks       = getNumber(_display.time.digits + 7, 3);
      break;
  }
}

void V2Mackie::dispatchNote(uint8_t channel, uint8_t note, uint8_t velocity) {
  switch (channel) {
    case 0: {
      switch (note) {
        case Mackie::ChannelStrip::VPot::Push... Mackie::ChannelStrip::VPot::Push + 7: {
          const uint8_t strip       = note - Mackie::ChannelStrip::VPot::Push;
          const bool on             = velocity == 127;
          _strips[strip].vpot.click = on;
          handleStripButton(strip, StripButton::VPot, on);
        } break;

        case Mackie::ChannelStrip::Button::Arm... Mackie::ChannelStrip::Button::Arm + 7: {
          const uint8_t strip       = note - Mackie::ChannelStrip::Button::Arm;
          const bool on             = velocity == 127;
          _strips[strip].button.arm = on;
          handleStripButton(strip, StripButton::Arm, on);
        } break;

        case Mackie::ChannelStrip::Button::Solo... Mackie::ChannelStrip::Button::Solo + 7: {
          const uint8_t strip        = note - Mackie::ChannelStrip::Button::Solo;
          const bool on              = velocity == 127;
          _strips[strip].button.solo = on;
          handleStripButton(strip, StripButton::Solo, on);
        } break;

        case Mackie::ChannelStrip::Button::Mute... Mackie::ChannelStrip::Button::Mute + 7: {
          const uint8_t strip        = note - Mackie::ChannelStrip::Button::Mute;
          const bool on              = velocity == 127;
          _strips[strip].button.mute = on;
          handleStripButton(strip, StripButton::Mute, on);
        } break;

        case Mackie::ChannelStrip::Button::Select... Mackie::ChannelStrip::Button::Select + 7: {
          const uint8_t strip          = note - Mackie::ChannelStrip::Button::Select;
          const bool on                = velocity == 127;
          _strips[strip].button.select = on;
          handleStripButton(strip, StripButton::Select, on);
        } break;

        case Mackie::ChannelStrip::Fader::Touch... Mackie::ChannelStrip::Fader::Touch + 7: {
          const uint8_t strip        = note - Mackie::ChannelStrip::Fader::Touch;
          const bool on              = velocity == 127;
          _strips[strip].fader.touch = on;
          handleStripButton(strip, StripButton::Touch, on);
        } break;

        case Mackie::Transport::Rewind: {
          const bool on     = velocity == 127;
          _transport.rewind = on;
          handleTransportButton(TransportButton::Rewind, on);
        } break;

        case Mackie::Transport::Forward: {
          const bool on      = velocity == 127;
          _transport.forward = on;
          handleTransportButton(TransportButton::Forward, on);
        } break;

        case Mackie::Transport::Stop: {
          const bool on   = velocity == 127;
          _transport.stop = on;
          handleTransportButton(TransportButton::Stop, on);
        } break;

        case Mackie::Transport::Play: {
          const bool on   = velocity == 127;
          _transport.play = on;
          handleTransportButton(TransportButton::Play, on);
        } break;

        case Mackie::Transport::Record: {
          const bool on     = velocity == 127;
          _transport.record = on;
          handleTransportButton(TransportButton::Record, on);
        } break;

        case Mackie::Bank::Previous:
          handleBankButton(BankButton::Previous, velocity == 127);
          break;

        case Mackie::Bank::Next:
          handleBankButton(BankButton::Next, velocity == 127);
          break;

        case Mackie::Bank::PreviousChannel:
          handleBankButton(BankButton::PreviousChannel, velocity == 127);
          break;

        case Mackie::Bank::Flip: {
          const bool on = velocity == 127;
          _bank.flip    = on;
          handleBankButton(BankButton::Flip, on);
        } break;

        case Mackie::Bank::Edit: {
          const bool on = velocity == 127;
          _bank.edit    = on;
          handleBankButton(BankButton::Edit, on);
        } break;

        case Mackie::Modifier::Shift: {
          const bool on = velocity == 127;
          handleModifierButton(ModifierButton::Shift, on);
        } break;

        case Mackie::Modifier::Option: {
          const bool on = velocity == 127;
          handleModifierButton(ModifierButton::Option, on);
        } break;

        case Mackie::Modifier::Control: {
          const bool on = velocity == 127;
          handleModifierButton(ModifierButton::Control, on);
        } break;

        case Mackie::Modifier::Alt: {
          const bool on = velocity == 127;
          handleModifierButton(ModifierButton::Alt, on);
        } break;

        case Mackie::Navigation::Up: {
          const bool on = velocity == 127;
          handleNavigationButton(NavigationButton::Up, on);
        } break;

        case Mackie::Navigation::Down: {
          const bool on = velocity == 127;
          handleNavigationButton(NavigationButton::Down, on);
        } break;

        case Mackie::Navigation::Left: {
          const bool on = velocity == 127;
          handleNavigationButton(NavigationButton::Left, on);
        } break;

        case Mackie::Navigation::Right: {
          const bool on = velocity == 127;
          handleNavigationButton(NavigationButton::Right, on);
        } break;

        case Mackie::Navigation::Zoom: {
          const bool on = velocity == 127;
          handleNavigationButton(NavigationButton::Zoom, on);
        } break;

        case Mackie::Navigation::Scrub: {
          const bool on = velocity == 127;
          handleNavigationButton(NavigationButton::Scrub, on);
        } break;
      }
    } break;

    case 15:
      switch (note) {
        case Mackie::Protocol::Ping:
          _active_usec = micros();
          break;
      }
      break;
  }
}

void V2Mackie::dispatchControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
  if (channel != 0)
    return;

  switch (controller) {
    case Mackie::Display::Time::Digit... Mackie::Display::Time::Digit + 9:
      _display.time.digits[Mackie::Display::Time::Digit + 9 - controller] = value;
      handleTime(_display.time.type);
      break;

    case Mackie::ChannelStrip::VPot::LED... Mackie::ChannelStrip::VPot::LED + 7: {
      const uint8_t strip    = controller - Mackie::ChannelStrip::VPot::LED;
      const bool center      = value & 0x40;
      const uint8_t position = value & 0x0f;

      handleStripVPotDisplay(strip, value);

      if (position == 0) {
        _strips[strip].vpot.mode   = VPotMode::Off;
        _strips[strip].vpot.center = center;
        _strips[strip].vpot.value  = 0;
        handleStripVPotDisplay(strip, VPotMode::Off, center, 0);
        return;
      }

      switch ((value >> 4) & 3) {
        case Mackie::ChannelStrip::VPot::Single: {
          const float fraction       = (float)position / 11.f;
          _strips[strip].vpot.mode   = VPotMode::Bar;
          _strips[strip].vpot.center = center;
          _strips[strip].vpot.value  = fraction;
          handleStripVPotDisplay(strip, VPotMode::Bar, center, fraction);
        } break;

        case Mackie::ChannelStrip::VPot::Boost: {
          if (position < 6) {
            float fraction             = (float)(6 - position) / -5.f;
            _strips[strip].vpot.mode   = VPotMode::Pan;
            _strips[strip].vpot.center = center;
            _strips[strip].vpot.value  = fraction;
            handleStripVPotDisplay(strip, VPotMode::Pan, center, fraction);

          } else {
            float fraction             = (float)(position - 6) / 5.f;
            _strips[strip].vpot.mode   = VPotMode::Pan;
            _strips[strip].vpot.center = center;
            _strips[strip].vpot.value  = fraction;
            handleStripVPotDisplay(strip, VPotMode::Pan, center, fraction);
          }
        } break;

        case Mackie::ChannelStrip::VPot::Bar: {
          const float fraction       = (float)position / 11.f;
          _strips[strip].vpot.mode   = VPotMode::Bar;
          _strips[strip].vpot.center = center;
          _strips[strip].vpot.value  = fraction;
          handleStripVPotDisplay(strip, VPotMode::Bar, center, fraction);
        } break;

        case Mackie::ChannelStrip::VPot::Spread: {
          const float fraction       = (float)position / 6.f;
          _strips[strip].vpot.mode   = VPotMode::Bar;
          _strips[strip].vpot.center = center;
          _strips[strip].vpot.value  = fraction;
          handleStripVPotDisplay(strip, VPotMode::Bar, center, fraction);
        } break;
      }
    } break;

    case V2MIDI::CC::AllSoundOff:
    case V2MIDI::CC::AllNotesOff:
      reset();
      break;
  }
}

void V2Mackie::dispatchAftertouchChannel(uint8_t channel, uint8_t pressure) {
  if (channel != 0)
    return;

  const uint8_t index = pressure >> 4;
  if (index > 7)
    return;

  const uint8_t value = pressure & 0xf;
  switch (value) {
    case 0 ... 12:
      _strips[index].meter.fraction = (float)value / 12.f;
      break;

    case 13:
      // TotalMix sends value == 13. This is not the original format wich was
      // driving 12 LEDs and a separate overload indicator.
      _strips[index].meter.fraction = 1;
      break;

    case 14:
      // Setting/clearing 'overload' does not reset the current meter value.
      if (!_strips[index].meter.overload)
        handleStripMeterOverload(index, true);
      _strips[index].meter.overload = true;
      break;

    case 15:
      if (_strips[index].meter.overload)
        handleStripMeterOverload(index, false);
      _strips[index].meter.overload = false;
      break;
  }

  _strips[index].meter.usec = micros();
  handleStripMeter(index, _strips[index].meter.fraction, _strips[index].meter.overload);
}

void V2Mackie::dispatchPitchBend(uint8_t channel, int16_t value) {
  if (value > 8176)
    value = 8176;

  else if (value < -8192)
    value = -8192;

  value += 8192;
  const float fraction = (float)value / (float)(8176 + 8192);

  switch (channel) {
    case 0 ... 7:
      _strips[channel].fader.position = fraction;
      handleStripFader(channel, fraction);
      break;

    case 8:
      _main.fader = fraction;
      handleFader(fraction);
      break;
  }
}

void V2Mackie::dispatchPacket(V2MIDI::Packet *packet) {
  switch (packet->getType()) {
    case V2MIDI::Packet::Status::NoteOn:
      dispatchNote(packet->getChannel(), packet->getNote(), packet->getNoteVelocity());
      break;

    case V2MIDI::Packet::Status::NoteOff:
      dispatchNote(packet->getChannel(), packet->getNote(), 0);
      break;

    case V2MIDI::Packet::Status::ControlChange:
      dispatchControlChange(packet->getChannel(), packet->getController(), packet->getControllerValue());
      break;

    case V2MIDI::Packet::Status::AftertouchChannel:
      dispatchAftertouchChannel(packet->getChannel(), packet->getAftertouchChannel());
      break;

    case V2MIDI::Packet::Status::PitchBend:
      dispatchPitchBend(packet->getChannel(), packet->getPitchBend());
      break;
  }
}

void V2Mackie::dispatchSystemExclusive(const uint8_t *buffer, uint32_t len) {
  if (len < 1 + Mackie::Message::Header::Message + 1 + 1)
    return;

  // Remove SysEx start and end byte.
  const uint8_t *p = buffer + 1;
  uint32_t l       = len - 2;

  if (memcmp(p + Mackie::Message::Header::Vendor, Mackie::Message::Vendor, sizeof(Mackie::Message::Vendor)) != 0)
    return;

  if ((p[Mackie::Message::Header::Device] != Mackie::Message::Device::Control) &&
      (p[Mackie::Message::Header::Device] != Mackie::Message::Device::ControlXT))
    return;

  switch (p[Mackie::Message::Header::Type]) {
    case Mackie::Message::Type::Display: {
      p += Mackie::Message::Header::Message;
      l -= Mackie::Message::Header::Message;

      if (len < Mackie::Message::Display::Header::Text + 1)
        break;

      // TotalMix: Strip 1, row 1
      // F0 00 00 66 14 12 00 41 4E 20 31 2F 32 20 F7     |   f   AN 1/2  |

      // TotalMix: Strip 1, row 2
      // F0 00 00 66 14 12 3F 2D 31 30 2E 30 20 20 F7     |   f  ?-10.0   |

      // Ableton: Entire display, 56 characters, two rows.
      // F0 00 00 66 14 12 00 31 2D 4D 49 44 49 20 32 2D  |   f   1-MIDI 2-|
      // 4D 49 44 49 20 33 2D 41 75 64 6F 20 34 2D 41 75  |MIDI 3-Audo 4-Au|
      // 64 6F 20 20 20 20 20 20 20 20 20 20 20 20 20 20  |do              |
      // 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 F7  |                |
      //
      // F0 00 00 66 14 12 38 20 20 20 20 20 20 20 20 20  |   f  8         |
      // 20 20 20 20 20 20 20 43 20 20 20 20 20 20 43 20  |       C      C |
      // 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20  |                |
      // 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 F7  |                |

      // Logic: entire display, 111 characters.
      // F0 00 00 66 14 12 00 74 74 74 20 20 20 20 53 52  |   f   ttt    SR|
      // 2F 30 2E 34 20 4C 48 2F 33 2E 39 20 53 74 20 4F  |/0.4 LH/3.9 St O|
      // 75 74 20 4D 61 73 74 65 72 20 20 20 20 20 20 20  |ut Master       |
      // 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 30  |               0|
      // 20 20 20 20 20 20 30 20 20 20 20 20 20 30 20 20  |      0      0  |
      // 20 20 20 20 30 20 20 20 20 20 20 20 20 20 20 20  |    0           |
      // 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20 20  |                |
      // 20 20 20 20 20 20 F7                             |       |

      const uint8_t start = p[Mackie::Message::Display::Header::Index];

      // Advance to text characters.
      p += Mackie::Message::Display::Header::Text;
      l -= Mackie::Message::Display::Header::Text;

      if (start + l > sizeof(_display.strip))
        return;

      memcpy(_display.strip + start, p, l);

      // Try to guess if the display rows are used to show a global message which
      // is not related to the associated channel strips; check if any of the separating
      // spaces are overwritten.
      bool global[2]{};
      for (uint8_t i = 0; i < 8; i++) {
        if (_display.strip[(i * 7) + 6] != ' ') {
          global[0] = true;
          break;
        }
      }
      for (uint8_t i = 8; i < 16; i++) {
        if (_display.strip[(i * 7) + 6] != ' ') {
          global[1] = true;
          break;
        }
      }

      // Generate per-strip/row events.
      const uint8_t first = start / 7;          // First of the 16 7-character ranges.
      const uint8_t last  = (start + l) / 7;    // Last of the 16 7-character ranges.
      const uint8_t count = 1 + (last - first); // Number of 7-character ranges.

      for (uint8_t i = 0; i < count; i++) {
        const uint8_t strip = (first + i) % 8;
        const uint8_t row   = (first + i) / 8;

        // Has the content changed?
        if (memcmp(_display.strip + (56 * row) + (7 * strip), _strips[strip].display[row], 7) == 0)
          continue;

        // Update copy and notify.
        memcpy(_strips[strip].display[row], _display.strip + (56 * row) + (7 * strip), 7);
        handleStripDisplay(global[row], strip, row);
      }
    } break;
  }
}
