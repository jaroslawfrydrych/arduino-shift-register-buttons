class Relay {
  private:
    int pin;
    bool isOn;

    void setupPins() {
      pinMode(this->pin, OUTPUT);
      digitalWrite(this->pin, HIGH);
    }

  public:
    Relay(int pin) {
      this->pin = pin;
      this->setupPins();
    }

    void turnOn() {
      this->isOn = true;
      digitalWrite(this->pin, LOW);
    }

    void turnOff() {
      this->isOn = false;
      digitalWrite(this->pin, HIGH);
    }

    bool isRelayOn() {
      return this->isOn;
    }
};
