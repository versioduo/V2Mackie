// © Kay Sievers <kay@versioduo.com>, 2020-2022
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include <V2MIDI.h>

class V2Mackie {
public:
  enum class StripButton {
    Arm,
    Mute,
    Select,
    Solo,
    Touch, // Touch-sensitive fader.
    VPot,  // Encoder click.
  };

  enum class TransportButton {
    Rewind,
    Forward,
    Stop,
    Play,
    Record,
  };

  enum class BankButton {
    Previous,
    Next,
    PreviousChannel,
    NextChannel,
    Flip,
    Edit,
  };

  enum class ModifierButton {
    Shift,
    Option,
    Control,
    Alt,
  };

  enum class NavigationButton {
    Up,
    Down,
    Left,
    Right,
    Zoom,
    Scrub,
  };

  enum class VPotMode {
    Off,
    Pan,
    Bar,
  };

  struct Time {
    struct SMPTE {
      uint16_t hours;
      uint8_t minutes;
      uint8_t seconds;
      uint16_t frames;
    };

    struct Beats {
      uint16_t bars;
      uint8_t beats;
      uint8_t subdivision;
      uint16_t ticks;
    };

    enum class Type { SMPTE, Beats } type;
    union {
      SMPTE smpte;
      Beats beats;
    };
  };

  void begin() {
    reset();
  }

  void reset();
  void loop();
  void dispatchPacket(V2MIDI::Packet *packet);
  void dispatchSystemExclusive(const uint8_t *buffer, uint32_t len);

  void getTime(Time &time);
  void getStripDisplay(uint8_t strip, uint8_t row, char text[8]);

  // Adjust the strip number in the current packet.
  static V2MIDI::Packet *setStripIndex(V2MIDI::Packet *packet, uint8_t strip);

  // Strips.
  static V2MIDI::Packet *setStripVPotDisplay(V2MIDI::Packet *packet, uint8_t strip, uint8_t value);
  static V2MIDI::Packet *setStripFader(V2MIDI::Packet *packet, uint8_t strip, float fraction);
  static V2MIDI::Packet *setStripButton(V2MIDI::Packet *packet, uint8_t strip, StripButton button, bool on);
  static V2MIDI::Packet *setStripMeter(V2MIDI::Packet *packet, uint8_t strip, float fraction);
  static V2MIDI::Packet *setStripMeterOverload(V2MIDI::Packet *packet, uint8_t strip, bool overload);
  static uint8_t setStripText(uint8_t *buffer, uint8_t strip, uint8_t row, const char *text);

  // Main volume fader.
  static V2MIDI::Packet *setFader(V2MIDI::Packet *packet, float fraction);
  static V2MIDI::Packet *setTouch(V2MIDI::Packet *packet, bool on);

  // Main buttons.
  static V2MIDI::Packet *setTransportButton(V2MIDI::Packet *packet, TransportButton button, bool on);
  static V2MIDI::Packet *setBankButton(V2MIDI::Packet *packet, BankButton button, bool on);
  static V2MIDI::Packet *setModifierButton(V2MIDI::Packet *packet, ModifierButton button, bool on);
  static V2MIDI::Packet *setNavigationButton(V2MIDI::Packet *packet, NavigationButton button, bool on);
  static V2MIDI::Packet *setFunctionButton(V2MIDI::Packet *packet, uint8_t function, bool on);

protected:
  // Strips.
  virtual void handleStripVPotDisplay(uint8_t strip, VPotMode mode, bool center, float fraction){};
  virtual void handleStripVPotDisplay(uint8_t strip, uint8_t value){};
  virtual void handleStripButton(uint8_t strip, StripButton button, bool on){};
  virtual void handleStripFader(uint8_t strip, float fraction){};
  virtual void handleStripMeter(uint8_t strip, float fraction, bool overload){};
  virtual void handleStripMeterOverload(uint8_t strip, bool overload){};

  // Display strip updates chunked into individual strip messages.
  virtual void handleStripDisplay(bool global, uint8_t strip, uint8_t row){};

  // Main volume fader.
  virtual void handleFader(float fraction){};

  // Button press events.
  virtual void handleTransportButton(TransportButton button, bool on){};
  virtual void handleBankButton(BankButton button, bool on){};
  virtual void handleModifierButton(ModifierButton button, bool on){};
  virtual void handleNavigationButton(NavigationButton button, bool on){};

  // Time/Counter display update.
  virtual void handleTime(Time::Type type){};

  // If ping messages have been received, a timeout is raised after they stop.
  virtual void handleTimeout(){};

private:
  unsigned long _active_usec{};

  struct {
    uint8_t strip[56 * 2];
    uint8_t mode[2];
    struct {
      Time::Type type;
      uint8_t digits[10];
    } time;
    unsigned long usec;
  } _display{};

  struct {
    char display[2][7];

    struct {
      VPotMode mode;
      bool center;
      float value;
      bool click;
    } vpot;

    struct {
      float position;
      bool touch;
    } fader;

    struct {
      bool arm;
      bool mute;
      bool select;
      bool solo;
    } button;

    struct {
      float fraction;
      bool overload;
      unsigned long usec;
    } meter;
  } _strips[8]{};

  struct {
    float fader;
  } _main{};

  struct {
    bool flip;
    bool edit;
  } _bank{};

  struct {
    bool rewind;
    bool forward;
    bool stop;
    bool play;
    bool record;
  } _transport{};

  struct {
    bool zoom;
    bool scrub;
  } _navigation{};

  void dispatchNote(uint8_t channel, uint8_t note, uint8_t velocity);
  void dispatchControlChange(uint8_t channel, uint8_t controller, uint8_t value);
  void dispatchAftertouchChannel(uint8_t channel, uint8_t pressure);
  void dispatchPitchBend(uint8_t channel, int16_t value);
};
