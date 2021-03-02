class Button {
  private:
    int index;
    int type;
    int debounceTime = 50;
    bool isPressed = false;
    unsigned long lastAction;

  public:
    Button(int index, buttonType type, Relay relay) {
      this->index = index;
      this->type = type;
    }

    void keydown(void (*callback)(int callbackParam), int callbackParam) {
      if (this->validateAction()) {
        return;
      }

      lastAction = millis();
      
      this->isPressed = true;
      callback(callbackParam);
    }

    void keyup(void (*callback)(int callbackParam), int callbackParam) {
      if (!this->validateAction()) {
        return;
      }

      lastAction = millis();

      this->isPressed = false;
      callback(callbackParam);
    }

    boolean isButtonPressed() {
      return this->isPressed;
    }

    boolean isNonMomentaryType() {
      return this->type == NON_MOMENTARY;
    }

    boolean isShortDelay() {
      return millis() - lastAction < debounceTime;
    }

    boolean validateAction() {
      return this->isButtonPressed() || this->isShortDelay();
    }
};
